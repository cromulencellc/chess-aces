'use strict';


define('forum/account/posts', ['forum/account/header', 'forum/infinitescroll', 'hooks'], function (header, infinitescroll, hooks) {
	const AccountPosts = {};

	let template;
	let page = 1;

	AccountPosts.init = function () {
		header.init();

		$('[component="post/content"] img:not(.not-responsive)').addClass('img-responsive');

		AccountPosts.handleInfiniteScroll('account/posts');
	};

	AccountPosts.handleInfiniteScroll = function (_template) {
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
			if (data.posts && data.posts.length) {
				onPostsLoaded(data.posts, done);
			} else {
				done();
			}
		});
	}

	function onPostsLoaded(posts, callback) {
		app.parseAndTranslate(template, 'posts', { posts: posts }, function (html) {
			$('[component="posts"]').append(html);
			html.find('img:not(.not-responsive)').addClass('img-responsive');
			html.find('.timeago').timeago();
			app.createUserTooltips(html);
			utils.makeNumbersHumanReadable(html.find('.human-readable-number'));
			hooks.fire('action:posts.loaded', { posts: posts });
			callback();
		});
	}

	return AccountPosts;
});
