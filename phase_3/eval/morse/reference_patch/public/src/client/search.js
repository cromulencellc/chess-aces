'use strict';


define('forum/search', [
	'search',
	'autocomplete',
	'storage',
	'hooks',
	'alerts',
], function (searchModule, autocomplete, storage, hooks, alerts) {
	const Search = {};

	Search.init = function () {
		const searchQuery = $('#results').attr('data-search-query');

		const searchIn = $('#search-in');

		searchIn.on('change', function () {
			updateFormItemVisiblity(searchIn.val());
		});

		searchModule.highlightMatches(searchQuery, $('.search-result-text p, .search-result-text.search-result-title a'));

		$('#advanced-search').off('submit').on('submit', function (e) {
			e.preventDefault();
			searchModule.query(getSearchDataFromDOM(), function () {
				$('#search-input').val('');
			});
			return false;
		});

		handleSavePreferences();

		enableAutoComplete();

		fillOutForm();
	};

	function getSearchDataFromDOM() {
		const form = $('#advanced-search');
		const searchData = {
			in: $('#search-in').val(),
		};
		searchData.term = $('#search-input').val();
		if (searchData.in === 'posts' || searchData.in === 'titlesposts' || searchData.in === 'titles') {
			searchData.matchWords = form.find('#match-words-filter').val();
			searchData.by = form.find('#posted-by-user').tagsinput('items');
			searchData.categories = form.find('#posted-in-categories').val();
			searchData.searchChildren = form.find('#search-children').is(':checked');
			searchData.hasTags = form.find('#has-tags').tagsinput('items');
			searchData.replies = form.find('#reply-count').val();
			searchData.repliesFilter = form.find('#reply-count-filter').val();
			searchData.timeFilter = form.find('#post-time-filter').val();
			searchData.timeRange = form.find('#post-time-range').val();
			searchData.sortBy = form.find('#post-sort-by').val();
			searchData.sortDirection = form.find('#post-sort-direction').val();
			searchData.showAs = form.find('#show-as-topics').is(':checked') ? 'topics' : 'posts';
		}

		hooks.fire('action:search.getSearchDataFromDOM', {
			form: form,
			data: searchData,
		});

		return searchData;
	}

	function updateFormItemVisiblity(searchIn) {
		const hide = searchIn.indexOf('posts') === -1 && searchIn.indexOf('titles') === -1;
		$('.post-search-item').toggleClass('hide', hide);
	}

	function fillOutForm() {
		const params = utils.params({
			disableToType: true,
		});

		const searchData = searchModule.getSearchPreferences();
		const formData = utils.merge(searchData, params);

		if (formData) {
			if (ajaxify.data.term) {
				$('#search-input').val(ajaxify.data.term);
			}
			formData.in = formData.in || ajaxify.data.searchDefaultIn;
			$('#search-in').val(formData.in);
			updateFormItemVisiblity(formData.in);

			if (formData.matchWords) {
				$('#match-words-filter').val(formData.matchWords);
			}

			if (formData.by) {
				formData.by = Array.isArray(formData.by) ? formData.by : [formData.by];
				formData.by.forEach(function (by) {
					$('#posted-by-user').tagsinput('add', by);
				});
			}

			if (formData.categories) {
				$('#posted-in-categories').val(formData.categories);
			}

			if (formData.searchChildren) {
				$('#search-children').prop('checked', true);
			}

			if (formData.hasTags) {
				formData.hasTags = Array.isArray(formData.hasTags) ? formData.hasTags : [formData.hasTags];
				formData.hasTags.forEach(function (tag) {
					$('#has-tags').tagsinput('add', tag);
				});
			}

			if (formData.replies) {
				$('#reply-count').val(formData.replies);
				$('#reply-count-filter').val(formData.repliesFilter);
			}

			if (formData.timeRange) {
				$('#post-time-range').val(formData.timeRange);
				$('#post-time-filter').val(formData.timeFilter);
			}

			if (formData.sortBy || ajaxify.data.searchDefaultSortBy) {
				$('#post-sort-by').val(formData.sortBy || ajaxify.data.searchDefaultSortBy);
			}
			$('#post-sort-direction').val(formData.sortDirection || 'desc');

			if (formData.showAs) {
				const isTopic = formData.showAs === 'topics';
				const isPost = formData.showAs === 'posts';
				$('#show-as-topics').prop('checked', isTopic).parent().toggleClass('active', isTopic);
				$('#show-as-posts').prop('checked', isPost).parent().toggleClass('active', isPost);
			}

			hooks.fire('action:search.fillOutForm', {
				form: formData,
			});
		}
	}

	function handleSavePreferences() {
		$('#save-preferences').on('click', function () {
			storage.setItem('search-preferences', JSON.stringify(getSearchDataFromDOM()));
			alerts.success('[[search:search-preferences-saved]]');
			return false;
		});

		$('#clear-preferences').on('click', function () {
			storage.removeItem('search-preferences');
			const query = $('#search-input').val();
			$('#advanced-search')[0].reset();
			$('#search-input').val(query);
			alerts.success('[[search:search-preferences-cleared]]');
			return false;
		});
	}

	function enableAutoComplete() {
		const userEl = $('#posted-by-user');
		userEl.tagsinput({
			confirmKeys: [13, 44],
			trimValue: true,
		});
		if (app.user.privileges['search:users']) {
			autocomplete.user(userEl.siblings('.bootstrap-tagsinput').find('input'));
		}

		const tagEl = $('#has-tags');
		tagEl.tagsinput({
			confirmKeys: [13, 44],
			trimValue: true,
		});
		if (app.user.privileges['search:tags']) {
			autocomplete.tag(tagEl.siblings('.bootstrap-tagsinput').find('input'));
		}
	}

	return Search;
});
