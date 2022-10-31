'use strict';

define('forum/groups/list', [
	'forum/infinitescroll', 'benchpress', 'api', 'bootbox', 'alerts',
], function (infinitescroll, Benchpress, api, bootbox, alerts) {
	const Groups = {};

	Groups.init = function () {
		infinitescroll.init(Groups.loadMoreGroups);

		// Group creation
		$('button[data-action="new"]').on('click', function () {
			bootbox.prompt('[[groups:new-group.group_name]]', function (name) {
				if (name && name.length) {
					api.post('/groups', {
						name: name,
					}).then((res) => {
						ajaxify.go('groups/' + res.slug);
					}).catch(alerts.error);
				}
			});
		});
		const params = utils.params();
		$('#search-sort').val(params.sort || 'alpha');

		// Group searching
		$('#search-text').on('keyup', Groups.search);
		$('#search-button').on('click', Groups.search);
		$('#search-sort').on('change', function () {
			ajaxify.go('groups?sort=' + $('#search-sort').val());
		});
	};

	Groups.loadMoreGroups = function (direction) {
		if (direction < 0) {
			return;
		}

		infinitescroll.loadMore('groups.loadMore', {
			sort: $('#search-sort').val(),
			after: $('[component="groups/container"]').attr('data-nextstart'),
		}, function (data, done) {
			if (data && data.groups.length) {
				Benchpress.render('partials/groups/list', {
					groups: data.groups,
				}).then(function (html) {
					$('#groups-list').append(html);
					done();
				});
			} else {
				done();
			}

			if (data && data.nextStart) {
				$('[component="groups/container"]').attr('data-nextstart', data.nextStart);
			}
		});
	};

	Groups.search = function () {
		const groupsEl = $('#groups-list');
		const queryEl = $('#search-text');
		const sortEl = $('#search-sort');

		socket.emit('groups.search', {
			query: queryEl.val(),
			options: {
				sort: sortEl.val(),
				filterHidden: true,
				showMembers: true,
				hideEphemeralGroups: true,
			},
		}, function (err, groups) {
			if (err) {
				return alerts.error(err);
			}
			groups = groups.filter(function (group) {
				return group.name !== 'registered-users' && group.name !== 'guests';
			});
			Benchpress.render('partials/groups/list', {
				groups: groups,
			}).then(function (html) {
				groupsEl.empty().append(html);
			});
		});
		return false;
	};

	return Groups;
});
