'use strict';
const winston = require('winston');

const validator = require('validator');
const nconf = require('nconf');

const db = require('../../database');
const user = require('../../user');
const groups = require('../../groups');
const plugins = require('../../plugins');
const meta = require('../../meta');
const utils = require('../../utils');
const privileges = require('../../privileges');
const translator = require('../../translator');
const messaging = require('../../messaging');
const categories = require('../../categories');

const helpers = module.exports;

helpers.getUserDataByUserSlug = async function (userslug, callerUID, query = {}) {
	const uid = await user.getUidByUserslug(userslug);
	if (!uid) {
		return null;
	}

	const results = await getAllData(uid, callerUID);
	if (!results.userData) {
		throw new Error('[[error:invalid-uid]]');
	}
	await parseAboutMe(results.userData);

	const { userData } = results;
	const { userSettings } = results;
	const { isAdmin } = results;
	const { isGlobalModerator } = results;
	const { isModerator } = results;
	const { canViewInfo } = results;
	const isSelf = parseInt(callerUID, 10) === parseInt(userData.uid, 10);

	userData.age = Math.max(
		0,
		userData.birthday ? Math.floor((new Date().getTime() - new Date(userData.birthday).getTime()) / 31536000000) : 0
	);

	userData.emailClass = 'hide';

	if (!results.canEdit && (!userSettings.showemail || meta.config.hideEmail)) {
		userData.email = '';
	} else if (!userSettings.showemail) {
		userData.emailClass = '';
	}

	if (!results.canEdit && (!userSettings.showfullname || meta.config.hideFullname)) {
		userData.fullname = '';
	}

	if (isAdmin || isSelf || (canViewInfo && !results.isTargetAdmin)) {
		userData.ips = results.ips;
	}

	if (!isAdmin && !isGlobalModerator && !isModerator) {
		userData.moderationNote = undefined;
	}

	userData.isBlocked = results.isBlocked;
	userData.yourid = callerUID;
	userData.theirid = userData.uid;
	userData.isTargetAdmin = results.isTargetAdmin;
	userData.isAdmin = isAdmin;
	userData.isGlobalModerator = isGlobalModerator;
	userData.isModerator = isModerator;
	userData.isAdminOrGlobalModerator = isAdmin || isGlobalModerator;
	userData.isAdminOrGlobalModeratorOrModerator = isAdmin || isGlobalModerator || isModerator;
	userData.isSelfOrAdminOrGlobalModerator = isSelf || isAdmin || isGlobalModerator;
	userData.canEdit = results.canEdit;
	userData.canBan = results.canBanUser;
	userData.canMute = results.canMuteUser;
	userData.canFlag = (await privileges.users.canFlag(callerUID, userData.uid)).flag;
	userData.canChangePassword = isAdmin || (isSelf && !meta.config['password:disableEdit']);
	userData.isSelf = isSelf;
	userData.isFollowing = results.isFollowing;
	userData.hasPrivateChat = results.hasPrivateChat;
	userData.showHidden = results.canEdit; // remove in v1.19.0
	userData.groups = Array.isArray(results.groups) && results.groups.length ? results.groups[0] : [];
	userData.disableSignatures = meta.config.disableSignatures === 1;
	userData['reputation:disabled'] = meta.config['reputation:disabled'] === 1;
	userData['downvote:disabled'] = meta.config['downvote:disabled'] === 1;
	userData['email:confirmed'] = !!userData['email:confirmed'];
	userData.profile_links = filterLinks(results.profile_menu.links, {
		self: isSelf,
		other: !isSelf,
		moderator: isModerator,
		globalMod: isGlobalModerator,
		admin: isAdmin,
		canViewInfo: canViewInfo,
	});

	userData.sso = results.sso.associations;
	userData.banned = Boolean(userData.banned);
	userData.muted = parseInt(userData.mutedUntil, 10) > Date.now();
	userData.website = escape(userData.website);
	userData.websiteLink = !userData.website.startsWith('http') ? `http://${userData.website}` : userData.website;
	userData.websiteName = userData.website.replace(validator.escape('http://'), '').replace(validator.escape('https://'), '');

	userData.fullname = escape(userData.fullname);
	userData.location = escape(userData.location);
	userData.signature = escape(userData.signature);
	userData.birthday = validator.escape(String(userData.birthday || ''));
	userData.moderationNote = validator.escape(String(userData.moderationNote || ''));

	if (userData['cover:url']) {
		userData['cover:url'] = userData['cover:url'].startsWith('http') ? userData['cover:url'] : (nconf.get('relative_path') + userData['cover:url']);
	} else {
		userData['cover:url'] = require('../../coverPhoto').getDefaultProfileCover(userData.uid);
	}

	userData['cover:position'] = validator.escape(String(userData['cover:position'] || '50% 50%'));
	userData['username:disableEdit'] = !userData.isAdmin && meta.config['username:disableEdit'];
	userData['email:disableEdit'] = !userData.isAdmin && meta.config['email:disableEdit'];

	await getCounts(userData, callerUID);

	const hookData = await plugins.hooks.fire('filter:helpers.getUserDataByUserSlug', {
		userData: userData,
		callerUID: callerUID,
		query: query,
	});
	return hookData.userData;
};

function escape(value) {
	return translator.escape(validator.escape(String(value || '')));
}

async function getAllData(uid, callerUID) {
	return await utils.promiseParallel({
		userData: user.getUserData(uid),
		isTargetAdmin: user.isAdministrator(uid),
		userSettings: user.getSettings(uid),
		isAdmin: user.isAdministrator(callerUID),
		isGlobalModerator: user.isGlobalModerator(callerUID),
		isModerator: user.isModeratorOfAnyCategory(callerUID),
		isFollowing: user.isFollowing(callerUID, uid),
		ips: user.getIPs(uid, 4),
		profile_menu: getProfileMenu(uid, callerUID),
		groups: groups.getUserGroups([uid]),
		sso: plugins.hooks.fire('filter:auth.list', { uid: uid, associations: [] }),
		canEdit: privileges.users.canEdit(callerUID, uid),
		canBanUser: privileges.users.canBanUser(callerUID, uid),
		canMuteUser: privileges.users.canMuteUser(callerUID, uid),
		isBlocked: user.blocks.is(uid, callerUID),
		canViewInfo: privileges.global.can('view:users:info', callerUID),
		hasPrivateChat: messaging.hasPrivateChat(callerUID, uid),
	});
}

async function getCounts(userData, callerUID) {
	const { uid } = userData;
	const cids = await categories.getCidsByPrivilege('categories:cid', callerUID, 'topics:read');
	const promises = {
		posts: db.sortedSetsCardSum(cids.map(c => `cid:${c}:uid:${uid}:pids`)),
		best: Promise.all(cids.map(async c => db.sortedSetCount(`cid:${c}:uid:${uid}:pids:votes`, 1, '+inf'))),
		controversial: Promise.all(cids.map(async c => db.sortedSetCount(`cid:${c}:uid:${uid}:pids:votes`, '-inf', -1))),
		topics: db.sortedSetsCardSum(cids.map(c => `cid:${c}:uid:${uid}:tids`)),
	};
	if (userData.isAdmin || userData.isSelf) {
		promises.ignored = db.sortedSetCard(`uid:${uid}:ignored_tids`);
		promises.watched = db.sortedSetCard(`uid:${uid}:followed_tids`);
		promises.upvoted = db.sortedSetCard(`uid:${uid}:upvote`);
		promises.downvoted = db.sortedSetCard(`uid:${uid}:downvote`);
		promises.bookmarks = db.sortedSetCard(`uid:${uid}:bookmarks`);
		promises.uploaded = db.sortedSetCard(`uid:${uid}:uploads`);
		promises.categoriesWatched = user.getWatchedCategories(uid);
		promises.blocks = user.getUserField(userData.uid, 'blocksCount');
	}
	const counts = await utils.promiseParallel(promises);
	counts.best = counts.best.reduce((sum, count) => sum + count, 0);
	counts.controversial = counts.controversial.reduce((sum, count) => sum + count, 0);
	counts.categoriesWatched = counts.categoriesWatched && counts.categoriesWatched.length;
	counts.groups = userData.groups.length;
	counts.following = userData.followingCount;
	counts.followers = userData.followerCount;
	userData.blocksCount = counts.blocks || 0; // for backwards compatibility, remove in 1.16.0
	userData.counts = counts;
}

async function getProfileMenu(uid, callerUID) {
	const links = [{
		id: 'info',
		route: 'info',
		name: '[[user:account_info]]',
		icon: 'fa-info',
		visibility: {
			self: false,
			other: false,
			moderator: false,
			globalMod: false,
			admin: true,
			canViewInfo: true,
		},
	}, {
		id: 'sessions',
		route: 'sessions',
		name: '[[pages:account/sessions]]',
		icon: 'fa-group',
		visibility: {
			self: true,
			other: false,
			moderator: false,
			globalMod: false,
			admin: false,
			canViewInfo: false,
		},
	}];

	if (meta.config.gdpr_enabled) {
		links.push({
			id: 'consent',
			route: 'consent',
			name: '[[user:consent.title]]',
			icon: 'fa-thumbs-o-up',
			visibility: {
				self: true,
				other: false,
				moderator: false,
				globalMod: false,
				admin: false,
				canViewInfo: false,
			},
		});
	}

	return await plugins.hooks.fire('filter:user.profileMenu', {
		uid: uid,
		callerUID: callerUID,
		links: links,
	});
}

async function parseAboutMe(userData) {
	if (!userData.aboutme) {
		userData.aboutme = '';
		userData.aboutmeParsed = '';
		return;
	}
	userData.aboutme = validator.escape(String(userData.aboutme || ''));
	const parsed = await plugins.hooks.fire('filter:parse.aboutme', userData.aboutme);
	userData.aboutme = translator.escape(userData.aboutme); // PATCH_3
	userData.aboutmeParsed = translator.escape(parsed);
}

function filterLinks(links, states) {
	return links.filter((link, index) => {
		// Default visibility
		link.visibility = {
			self: true,
			other: true,
			moderator: true,
			globalMod: true,
			admin: true,
			canViewInfo: true,
			...link.visibility,
		};

		const permit = Object.keys(states).some(state => states[state] && link.visibility[state]);

		links[index].public = permit;
		return permit;
	});
}

require('../../promisify')(helpers);
