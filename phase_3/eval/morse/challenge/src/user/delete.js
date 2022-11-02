'use strict';

const async = require('async');
const _ = require('lodash');
const path = require('path');
const nconf = require('nconf');
const util = require('util');
const rimrafAsync = util.promisify(require('rimraf'));

const db = require('../database');
const posts = require('../posts');
const flags = require('../flags');
const topics = require('../topics');
const groups = require('../groups');
const messaging = require('../messaging');
const plugins = require('../plugins');
const batch = require('../batch');

module.exports = function (User) {
	const deletesInProgress = {};

	User.delete = async (callerUid, uid) => {
		await User.deleteContent(callerUid, uid);
		return await User.deleteAccount(uid);
	};

	User.deleteContent = async function (callerUid, uid) {
		if (parseInt(uid, 10) <= 0) {
			throw new Error('[[error:invalid-uid]]');
		}
		if (deletesInProgress[uid]) {
			throw new Error('[[error:already-deleting]]');
		}
		deletesInProgress[uid] = 'user.delete';
		await deletePosts(callerUid, uid);
		await deleteTopics(callerUid, uid);
		await deleteUploads(callerUid, uid);
		await deleteQueued(uid);
		delete deletesInProgress[uid];
	};

	async function deletePosts(callerUid, uid) {
		await batch.processSortedSet(`uid:${uid}:posts`, async (ids) => {
			await async.eachSeries(ids, async (pid) => {
				await posts.purge(pid, callerUid);
			});
		}, { alwaysStartAt: 0 });
	}

	async function deleteTopics(callerUid, uid) {
		await batch.processSortedSet(`uid:${uid}:topics`, async (ids) => {
			await async.eachSeries(ids, async (tid) => {
				await topics.purge(tid, callerUid);
			});
		}, { alwaysStartAt: 0 });
	}

	async function deleteUploads(callerUid, uid) {
		const uploads = await db.getSortedSetMembers(`uid:${uid}:uploads`);
		await User.deleteUpload(callerUid, uid, uploads);
	}

	async function deleteQueued(uid) {
		let deleteIds = [];
		await batch.processSortedSet('post:queue', async (ids) => {
			const data = await db.getObjects(ids.map(id => `post:queue:${id}`));
			const userQueuedIds = data.filter(d => parseInt(d.uid, 10) === parseInt(uid, 10)).map(d => d.id);
			deleteIds = deleteIds.concat(userQueuedIds);
		}, { batch: 500 });
		await async.eachSeries(deleteIds, posts.removeFromQueue);
	}

	async function removeFromSortedSets(uid) {
		await db.sortedSetsRemove([
			'users:joindate',
			'users:postcount',
			'users:reputation',
			'users:banned',
			'users:banned:expire',
			'users:flags',
			'users:online',
			'digest:day:uids',
			'digest:week:uids',
			'digest:month:uids',
		], uid);
	}

	User.deleteAccount = async function (uid) {
		if (deletesInProgress[uid] === 'user.deleteAccount') {
			throw new Error('[[error:already-deleting]]');
		}
		deletesInProgress[uid] = 'user.deleteAccount';

		await removeFromSortedSets(uid);
		const userData = await db.getObject(`user:${uid}`);

		if (!userData || !userData.username) {
			delete deletesInProgress[uid];
			throw new Error('[[error:no-user]]');
		}

		await plugins.hooks.fire('static:user.delete', { uid: uid, userData: userData });
		await deleteVotes(uid);
		await deleteChats(uid);
		await User.auth.revokeAllSessions(uid);

		const keys = [
			`uid:${uid}:notifications:read`,
			`uid:${uid}:notifications:unread`,
			`uid:${uid}:bookmarks`,
			`uid:${uid}:tids_read`,
			`uid:${uid}:tids_unread`,
			`uid:${uid}:followed_tids`,
			`uid:${uid}:ignored_tids`,
			`uid:${uid}:blocked_uids`,
			`user:${uid}:settings`,
			`user:${uid}:usernames`,
			`user:${uid}:emails`,
			`uid:${uid}:topics`, `uid:${uid}:posts`,
			`uid:${uid}:chats`, `uid:${uid}:chats:unread`,
			`uid:${uid}:chat:rooms`, `uid:${uid}:chat:rooms:unread`,
			`uid:${uid}:upvote`, `uid:${uid}:downvote`,
			`uid:${uid}:flag:pids`,
			`uid:${uid}:sessions`, `uid:${uid}:sessionUUID:sessionId`,
			`invitation:uid:${uid}`,
		];

		const bulkRemove = [
			['username:uid', userData.username],
			['username:sorted', `${userData.username.toLowerCase()}:${uid}`],
			['userslug:uid', userData.userslug],
			['fullname:uid', userData.fullname],
		];
		if (userData.email) {
			bulkRemove.push(['email:uid', userData.email.toLowerCase()]);
			bulkRemove.push(['email:sorted', `${userData.email.toLowerCase()}:${uid}`]);
		}

		if (userData.fullname) {
			bulkRemove.push(['fullname:sorted', `${userData.fullname.toLowerCase()}:${uid}`]);
		}

		await Promise.all([
			db.sortedSetRemoveBulk(bulkRemove),
			db.decrObjectField('global', 'userCount'),
			db.deleteAll(keys),
			db.setRemove('invitation:uids', uid),
			deleteUserIps(uid),
			deleteUserFromFollowers(uid),
			deleteImages(uid),
			groups.leaveAllGroups(uid),
			flags.resolveFlag('user', uid, uid),
			User.reset.cleanByUid(uid),
		]);
		await db.deleteAll([`followers:${uid}`, `following:${uid}`, `user:${uid}`]);
		delete deletesInProgress[uid];
		return userData;
	};

	async function deleteVotes(uid) {
		const [upvotedPids, downvotedPids] = await Promise.all([
			db.getSortedSetRange(`uid:${uid}:upvote`, 0, -1),
			db.getSortedSetRange(`uid:${uid}:downvote`, 0, -1),
		]);
		const pids = _.uniq(upvotedPids.concat(downvotedPids).filter(Boolean));
		await async.eachSeries(pids, async (pid) => {
			await posts.unvote(pid, uid);
		});
	}

	async function deleteChats(uid) {
		const roomIds = await db.getSortedSetRange(`uid:${uid}:chat:rooms`, 0, -1);
		const userKeys = roomIds.map(roomId => `uid:${uid}:chat:room:${roomId}:mids`);

		await Promise.all([
			messaging.leaveRooms(uid, roomIds),
			db.deleteAll(userKeys),
		]);
	}

	async function deleteUserIps(uid) {
		const ips = await db.getSortedSetRange(`uid:${uid}:ip`, 0, -1);
		await db.sortedSetsRemove(ips.map(ip => `ip:${ip}:uid`), uid);
		await db.delete(`uid:${uid}:ip`);
	}

	async function deleteUserFromFollowers(uid) {
		const [followers, following] = await Promise.all([
			db.getSortedSetRange(`followers:${uid}`, 0, -1),
			db.getSortedSetRange(`following:${uid}`, 0, -1),
		]);

		async function updateCount(uids, name, fieldName) {
			await async.each(uids, async (uid) => {
				let count = await db.sortedSetCard(name + uid);
				count = parseInt(count, 10) || 0;
				await db.setObjectField(`user:${uid}`, fieldName, count);
			});
		}

		const followingSets = followers.map(uid => `following:${uid}`);
		const followerSets = following.map(uid => `followers:${uid}`);

		await Promise.all([
			db.sortedSetsRemove(followerSets.concat(followingSets), uid),
			updateCount(following, 'followers:', 'followerCount'),
			updateCount(followers, 'following:', 'followingCount'),
		]);
	}

	async function deleteImages(uid) {
		const folder = path.join(nconf.get('upload_path'), 'profile');
		await Promise.all([
			rimrafAsync(path.join(folder, `${uid}-profilecover*`)),
			rimrafAsync(path.join(folder, `${uid}-profileavatar*`)),
		]);
	}
};
