'use strict';


define('forum/account/topics', [
	'forum/account/header',
	'forum/infinitescroll',
	'hooks',
], function (header, infinitescroll, hooks) {
	const AccountTopics = {};

	let template;
	let page = 1;

	AccountTopics.init = function () {
		header.init();

		AccountTopics.handleInfiniteScroll('account/topics');
	};

	AccountTopics.handleInfiniteScroll = function (_template) {
		template = _template;
		page = ajaxify.data.pagination.currentPage;
		if (!config.usePagination) {
			infinitescroll.init(loadMore);
		}
	};

	function loadMore(direction) {
		if (direction < 0) {
			return;
		}
		const params = utils.params();
		page += 1;
		params.page = page;

		infinitescroll.loadMoreXhr(params, function (data, done) {
			if (data.topics && data.topics.length) {
				onTopicsLoaded(data.topics, done);
			} else {
				done();
			}
		});
	}

	function onTopicsLoaded(topics, callback) {
		app.parseAndTranslate(template, 'topics', { topics: topics }, function (html) {
			$('[component="category"]').append(html);
			html.find('.timeago').timeago();
			app.createUserTooltips(html);
			utils.makeNumbersHumanReadable(html.find('.human-readable-number'));
			hooks.fire('action:topics.loaded', { topics: topics });
			callback();
		});
	}

	return AccountTopics;
});
