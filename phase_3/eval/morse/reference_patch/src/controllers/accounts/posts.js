'use strict';

const db = require('../../database');
const user = require('../../user');
const posts = require('../../posts');
const topics = require('../../topics');
const categories = require('../../categories');
const privileges = require('../../privileges');
const pagination = require('../../pagination');
const helpers = require('../helpers');
const accountHelpers = require('./helpers');
const plugins = require('../../plugins');
const utils = require('../../utils');

const postsController = module.exports;

const templateToData = {
	'account/bookmarks': {
		type: 'posts',
		noItemsFoundKey: '[[topic:bookmarks.has_no_bookmarks]]',
		crumb: '[[user:bookmarks]]',
		getSets: function (callerUid, userData) {
			return `uid:${userData.uid}:bookmarks`;
		},
	},
	'account/posts': {
		type: 'posts',
		noItemsFoundKey: '[[user:has_no_posts]]',
		crumb: '[[global:posts]]',
		getSets: async function (callerUid, userData) {
			const cids = await categories.getCidsByPrivilege('categories:cid', callerUid, 'topics:read');
			return cids.map(c => `cid:${c}:uid:${userData.uid}:pids`);
		},
	},
	'account/upvoted': {
		type: 'posts',
		noItemsFoundKey: '[[user:has_no_upvoted_posts]]',
		crumb: '[[global:upvoted]]',
		getSets: function (callerUid, userData) {
			return `uid:${userData.uid}:upvote`;
		},
	},
	'account/downvoted': {
		type: 'posts',
		noItemsFoundKey: '[[user:has_no_downvoted_posts]]',
		crumb: '[[global:downvoted]]',
		getSets: function (callerUid, userData) {
			return `uid:${userData.uid}:downvote`;
		},
	},
	'account/best': {
		type: 'posts',
		noItemsFoundKey: '[[user:has_no_best_posts]]',
		crumb: '[[global:best]]',
		getSets: async function (callerUid, userData) {
			const cids = await categories.getCidsByPrivilege('categories:cid', callerUid, 'topics:read');
			return cids.map(c => `cid:${c}:uid:${userData.uid}:pids:votes`);
		},
		getTopics: async (sets, req, start, stop) => {
			let pids = await db.getSortedSetRevRangeByScore(sets, start, stop - start + 1, '+inf', 1);
			pids = await privileges.posts.filter('topics:read', pids, req.uid);
			const postObjs = await posts.getPostSummaryByPids(pids, req.uid, { stripTags: false });
			return { posts: postObjs, nextStart: stop + 1 };
		},
		getItemCount: async (sets) => {
			const counts = await Promise.all(sets.map(set => db.sortedSetCount(set, 1, '+inf')));
			return counts.reduce((acc, val) => acc + val, 0);
		},
	},
	'account/controversial': {
		type: 'posts',
		noItemsFoundKey: '[[user:has_no_controversial_posts]]',
		crumb: '[[global:controversial]]',
		getSets: async function (callerUid, userData) {
			const cids = await categories.getCidsByPrivilege('categories:cid', callerUid, 'topics:read');
			return cids.map(c => `cid:${c}:uid:${userData.uid}:pids:votes`);
		},
		getTopics: async (sets, req, start, stop) => {
			let pids = await db.getSortedSetRangeByScore(sets, start, stop - start + 1, '-inf', -1);
			pids = await privileges.posts.filter('topics:read', pids, req.uid);
			const postObjs = await posts.getPostSummaryByPids(pids, req.uid, { stripTags: false });
			return { posts: postObjs, nextStart: stop + 1 };
		},
		getItemCount: async (sets) => {
			const counts = await Promise.all(sets.map(set => db.sortedSetCount(set, '-inf', -1)));
			return counts.reduce((acc, val) => acc + val, 0);
		},
	},
	'account/watched': {
		type: 'topics',
		noItemsFoundKey: '[[user:has_no_watched_topics]]',
		crumb: '[[user:watched]]',
		getSets: function (callerUid, userData) {
			return `uid:${userData.uid}:followed_tids`;
		},
		getTopics: async function (set, req, start, stop) {
			const { sort } = req.query;
			const map = {
				votes: 'topics:votes',
				posts: 'topics:posts',
				views: 'topics:views',
				lastpost: 'topics:recent',
				firstpost: 'topics:tid',
			};

			if (!sort || !map[sort]) {
				return await topics.getTopicsFromSet(set, req.uid, start, stop);
			}
			const sortSet = map[sort];
			let tids = await db.getSortedSetRevRange(set, 0, -1);
			const scores = await db.sortedSetScores(sortSet, tids);
			tids = tids.map((tid, i) => ({ tid: tid, score: scores[i] }))
				.sort((a, b) => b.score - a.score)
				.slice(start, stop + 1)
				.map(t => t.tid);

			const topicsData = await topics.getTopics(tids, req.uid);
			topics.calculateTopicIndices(topicsData, start);
			return { topics: topicsData, nextStart: stop + 1 };
		},
	},
	'account/ignored': {
		type: 'topics',
		noItemsFoundKey: '[[user:has_no_ignored_topics]]',
		crumb: '[[user:ignored]]',
		getSets: function (callerUid, userData) {
			return `uid:${userData.uid}:ignored_tids`;
		},
	},
	'account/topics': {
		type: 'topics',
		noItemsFoundKey: '[[user:has_no_topics]]',
		crumb: '[[global:topics]]',
		getSets: async function (callerUid, userData) {
			const cids = await categories.getCidsByPrivilege('categories:cid', callerUid, 'topics:read');
			return cids.map(c => `cid:${c}:uid:${userData.uid}:tids`);
		},
	},
};

postsController.getBookmarks = async function (req, res, next) {
	await getPostsFromUserSet('account/bookmarks', req, res, next);
};

postsController.getPosts = async function (req, res, next) {
	await getPostsFromUserSet('account/posts', req, res, next);
};

postsController.getUpVotedPosts = async function (req, res, next) {
	await getPostsFromUserSet('account/upvoted', req, res, next);
};

postsController.getDownVotedPosts = async function (req, res, next) {
	await getPostsFromUserSet('account/downvoted', req, res, next);
};

postsController.getBestPosts = async function (req, res, next) {
	await getPostsFromUserSet('account/best', req, res, next);
};

postsController.getControversialPosts = async function (req, res, next) {
	await getPostsFromUserSet('account/controversial', req, res, next);
};

postsController.getWatchedTopics = async function (req, res, next) {
	await getPostsFromUserSet('account/watched', req, res, next);
};

postsController.getIgnoredTopics = async function (req, res, next) {
	await getPostsFromUserSet('account/ignored', req, res, next);
};

postsController.getTopics = async function (req, res, next) {
	await getPostsFromUserSet('account/topics', req, res, next);
};

async function getPostsFromUserSet(template, req, res, next) {
	const data = templateToData[template];
	const page = Math.max(1, parseInt(req.query.page, 10) || 1);

	const [userData, settings] = await Promise.all([
		accountHelpers.getUserDataByUserSlug(req.params.userslug, req.uid, req.query),
		user.getSettings(req.uid),
	]);

	if (!userData) {
		return next();
	}
	const itemsPerPage = data.type === 'topics' ? settings.topicsPerPage : settings.postsPerPage;
	const start = (page - 1) * itemsPerPage;
	const stop = start + itemsPerPage - 1;
	const sets = await data.getSets(req.uid, userData);
	let result;
	if (plugins.hooks.hasListeners('filter:account.getPostsFromUserSet')) {
		result = await plugins.hooks.fire('filter:account.getPostsFromUserSet', {
			req: req,
			template: template,
			userData: userData,
			settings: settings,
			data: data,
			start: start,
			stop: stop,
			itemCount: 0,
			itemData: [],
		});
	} else {
		result = await utils.promiseParallel({
			itemCount: getItemCount(sets, data, settings),
			itemData: getItemData(sets, data, req, start, stop),
		});
	}
	const { itemCount, itemData } = result;
	userData[data.type] = itemData[data.type];
	userData.nextStart = itemData.nextStart;

	const pageCount = Math.ceil(itemCount / itemsPerPage);
	userData.pagination = pagination.create(page, pageCount, req.query);

	userData.noItemsFoundKey = data.noItemsFoundKey;
	userData.title = `[[pages:${template}, ${userData.username}]]`;
	userData.breadcrumbs = helpers.buildBreadcrumbs([{ text: userData.username, url: `/user/${userData.userslug}` }, { text: data.crumb }]);
	userData.showSort = template === 'account/watched';
	const baseUrl = (req.baseUrl + req.path.replace(/^\/api/, ''));
	userData.sortOptions = [
		{ url: `${baseUrl}?sort=votes`, name: '[[global:votes]]' },
		{ url: `${baseUrl}?sort=posts`, name: '[[global:posts]]' },
		{ url: `${baseUrl}?sort=views`, name: '[[global:views]]' },
		{ url: `${baseUrl}?sort=lastpost`, name: '[[global:lastpost]]' },
		{ url: `${baseUrl}?sort=firstpost`, name: '[[global:firstpost]]' },
	];
	userData.sortOptions.forEach((option) => {
		option.selected = option.url.includes(`sort=${req.query.sort}`);
	});

	res.render(template, userData);
}

async function getItemData(sets, data, req, start, stop) {
	if (data.getTopics) {
		return await data.getTopics(sets, req, start, stop);
	}
	const method = data.type === 'topics' ? topics.getTopicsFromSet : posts.getPostSummariesFromSet;
	return await method(sets, req.uid, start, stop);
}

async function getItemCount(sets, data, settings) {
	if (!settings.usePagination) {
		return 0;
	}
	if (data.getItemCount) {
		return await data.getItemCount(sets);
	}
	return await db.sortedSetsCardSum(sets);
}
