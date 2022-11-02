'use strict';

const nconf = require('nconf');
const winston = require('winston');

const user = require('./index');
const groups = require('../groups');
const utils = require('../utils');
const batch = require('../batch');

const db = require('../database');
const meta = require('../meta');
const emailer = require('../emailer');
const Password = require('../password');

const UserReset = module.exports;

const twoHours = 7200000;

UserReset.validate = async function (code) {
	const uid = await db.getObjectField('reset:uid', code);
	if (!uid) {
		return false;
	}
	const issueDate = await db.sortedSetScore('reset:issueDate', code);
	return parseInt(issueDate, 10) > Date.now() - twoHours;
};

UserReset.generate = async function (uid) {
	const code = utils.generateUUID();

	// Invalidate past tokens (must be done prior)
	await UserReset.cleanByUid(uid);

	await Promise.all([
		db.setObjectField('reset:uid', code, uid),
		db.sortedSetAdd('reset:issueDate', Date.now(), code),
	]);
	return code;
};

async function canGenerate(uid) {
	const score = await db.sortedSetScore('reset:issueDate:uid', uid);
	if (score > Date.now() - (1000 * 60)) {
		throw new Error('[[error:reset-rate-limited]]');
	}
}

UserReset.send = async function (email) {
	const uid = await user.getUidByEmail(email);
	if (!uid) {
		throw new Error('[[error:invalid-email]]');
	}
	await canGenerate(uid);
	await db.sortedSetAdd('reset:issueDate:uid', Date.now(), uid);
	const code = await UserReset.generate(uid);
	await emailer.send('reset', uid, {
		reset_link: `${nconf.get('url')}/reset/${code}`,
		subject: '[[email:password-reset-requested]]',
		template: 'reset',
		uid: uid,
	}).catch(err => winston.error(`[emailer.send] ${err.stack}`));

	return code;
};

UserReset.commit = async function (code, password) {
	user.isPasswordValid(password);
	const validated = await UserReset.validate(code);
	if (!validated) {
		throw new Error('[[error:reset-code-not-valid]]');
	}
	const uid = await db.getObjectField('reset:uid', code);
	if (!uid) {
		throw new Error('[[error:reset-code-not-valid]]');
	}
	const userData = await db.getObjectFields(
		`user:${uid}`,
		['password', 'passwordExpiry', 'password:shaWrapped']
	);
	const ok = await Password.compare(password, userData.password, !!parseInt(userData['password:shaWrapped'], 10));
	if (ok) {
		throw new Error('[[error:reset-same-password]]');
	}
	const hash = await user.hashPassword(password);
	const data = {
		password: hash,
		'password:shaWrapped': 1,
	};

	// don't verify email if password reset is due to expiry
	const isPasswordExpired = userData.passwordExpiry && userData.passwordExpiry < Date.now();
	if (!isPasswordExpired) {
		data['email:confirmed'] = 1;
		await groups.join('verified-users', uid);
		await groups.leave('unverified-users', uid);
	}

	await Promise.all([
		user.setUserFields(uid, data),
		db.deleteObjectField('reset:uid', code),
		db.sortedSetRemoveBulk([
			['reset:issueDate', code],
			['reset:issueDate:uid', uid],
		]),
		user.reset.updateExpiry(uid),
		user.auth.resetLockout(uid),
		user.auth.revokeAllSessions(uid),
		user.email.expireValidation(uid),
	]);
};

UserReset.updateExpiry = async function (uid) {
	const expireDays = meta.config.passwordExpiryDays;
	if (expireDays > 0) {
		const oneDay = 1000 * 60 * 60 * 24;
		const expiry = Date.now() + (oneDay * expireDays);
		await user.setUserField(uid, 'passwordExpiry', expiry);
	} else {
		await db.deleteObjectField(`user:${uid}`, 'passwordExpiry');
	}
};

UserReset.clean = async function () {
	const [tokens, uids] = await Promise.all([
		db.getSortedSetRangeByScore('reset:issueDate', 0, -1, '-inf', Date.now() - twoHours),
		db.getSortedSetRangeByScore('reset:issueDate:uid', 0, -1, '-inf', Date.now() - twoHours),
	]);
	if (!tokens.length && !uids.length) {
		return;
	}

	winston.verbose(`[UserReset.clean] Removing ${tokens.length} reset tokens from database`);
	await cleanTokensAndUids(tokens, uids);
};

UserReset.cleanByUid = async function (uid) {
	const tokensToClean = [];
	uid = parseInt(uid, 10);

	await batch.processSortedSet('reset:issueDate', async (tokens) => {
		const results = await db.getObjectFields('reset:uid', tokens);
		for (const [code, result] of Object.entries(results)) {
			if (parseInt(result, 10) === uid) {
				tokensToClean.push(code);
			}
		}
	}, { batch: 500 });

	if (!tokensToClean.length) {
		winston.verbose(`[UserReset.cleanByUid] No tokens found for uid (${uid}).`);
		return;
	}

	winston.verbose(`[UserReset.cleanByUid] Found ${tokensToClean.length} token(s), removing...`);
	await cleanTokensAndUids(tokensToClean, uid);
};

async function cleanTokensAndUids(tokens, uids) {
	await Promise.all([
		db.deleteObjectFields('reset:uid', tokens),
		db.sortedSetRemove('reset:issueDate', tokens),
		db.sortedSetRemove('reset:issueDate:uid', uids),
	]);
}
