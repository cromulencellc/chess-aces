'use strict';

const db = require('../database');
const meta = require('../meta');
const privileges = require('../privileges');

module.exports = function (User) {
	User.isReadyToPost = async function (uid, cid) {
		await isReady(uid, cid, 'lastposttime');
	};

	User.isReadyToQueue = async function (uid, cid) {
		await isReady(uid, cid, 'lastqueuetime');
	};

	async function isReady(uid, cid, field) {
		if (parseInt(uid, 10) === 0) {
			return;
		}
		const [userData, isAdminOrMod] = await Promise.all([
			User.getUserFields(uid, ['uid', 'banned', 'mutedUntil', 'joindate', 'email', 'reputation'].concat([field])),
			privileges.categories.isAdminOrMod(cid, uid),
		]);

		if (!userData.uid) {
			throw new Error('[[error:no-user]]');
		}

		if (isAdminOrMod) {
			return;
		}

		if (userData.banned) {
			throw new Error('[[error:user-banned]]');
		}

		const now = Date.now();
		if (userData.mutedUntil > now) {
			let muteLeft = ((userData.mutedUntil - now) / (1000 * 60));
			if (muteLeft > 60) {
				muteLeft = (muteLeft / 60).toFixed(0);
				throw new Error(`[[error:user-muted-for-hours, ${muteLeft}]]`);
			} else {
				throw new Error(`[[error:user-muted-for-minutes, ${muteLeft.toFixed(0)}]]`);
			}
		}

		if (now - userData.joindate < meta.config.initialPostDelay * 1000) {
			throw new Error(`[[error:user-too-new, ${meta.config.initialPostDelay}]]`);
		}

		const lasttime = userData[field] || 0;

		if (
			meta.config.newbiePostDelay > 0 &&
			meta.config.newbiePostDelayThreshold > userData.reputation &&
			now - lasttime < meta.config.newbiePostDelay * 1000
		) {
			throw new Error(`[[error:too-many-posts-newbie, ${meta.config.newbiePostDelay}, ${meta.config.newbiePostDelayThreshold}]]`);
		} else if (now - lasttime < meta.config.postDelay * 1000) {
			throw new Error(`[[error:too-many-posts, ${meta.config.postDelay}]]`);
		}
	}

	User.onNewPostMade = async function (postData) {
		// For scheduled posts, use "action" time. It'll be updated in related cron job when post is published
		const lastposttime = postData.timestamp > Date.now() ? Date.now() : postData.timestamp;

		await Promise.all([
			User.addPostIdToUser(postData),
			User.setUserField(postData.uid, 'lastposttime', lastposttime),
			User.updateLastOnlineTime(postData.uid),
		]);
	};

	User.addPostIdToUser = async function (postData) {
		await db.sortedSetsAdd([
			`uid:${postData.uid}:posts`,
			`cid:${postData.cid}:uid:${postData.uid}:pids`,
		], postData.timestamp, postData.pid);
		await User.updatePostCount(postData.uid);
	};

	User.updatePostCount = async (uid) => {
		const exists = await User.exists(uid);
		if (exists) {
			const count = await db.sortedSetCard(`uid:${uid}:posts`);
			await Promise.all([
				User.setUserField(uid, 'postcount', count),
				db.sortedSetAdd('users:postcount', count, uid),
			]);
		}
	};

	User.incrementUserPostCountBy = async function (uid, value) {
		return await incrementUserFieldAndSetBy(uid, 'postcount', 'users:postcount', value);
	};

	User.incrementUserReputationBy = async function (uid, value) {
		return await incrementUserFieldAndSetBy(uid, 'reputation', 'users:reputation', value);
	};

	User.incrementUserFlagsBy = async function (uid, value) {
		return await incrementUserFieldAndSetBy(uid, 'flags', 'users:flags', value);
	};

	async function incrementUserFieldAndSetBy(uid, field, set, value) {
		value = parseInt(value, 10);
		if (!value || !field || !(parseInt(uid, 10) > 0)) {
			return;
		}
		const exists = await User.exists(uid);
		if (!exists) {
			return;
		}
		const newValue = await User.incrementUserFieldBy(uid, field, value);
		await db.sortedSetAdd(set, newValue, uid);
		return newValue;
	}

	User.getPostIds = async function (uid, start, stop) {
		return await db.getSortedSetRevRange(`uid:${uid}:posts`, start, stop);
	};
};
