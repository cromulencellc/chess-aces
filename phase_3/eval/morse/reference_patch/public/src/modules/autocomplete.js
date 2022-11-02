
'use strict';


define('autocomplete', ['api', 'alerts'], function (api, alerts) {
	const module = {};

	module.user = function (input, params, onselect) {
		if (typeof params === 'function') {
			onselect = params;
			params = {};
		}
		params = params || {};
		app.loadJQueryUI(function () {
			input.autocomplete({
				delay: 200,
				open: function () {
					$(this).autocomplete('widget').css('z-index', 100005);
				},
				select: function (event, ui) {
					handleOnSelect(input, onselect, event, ui);
				},
				source: function (request, response) {
					params.query = request.term;

					api.get('/api/users', params, function (err, result) {
						if (err) {
							return alerts.error(err);
						}

						if (result && result.users) {
							const names = result.users.map(function (user) {
								const username = $('<div></div>').html(user.username).text();
								return user && {
									label: username,
									value: username,
									user: {
										uid: user.uid,
										name: user.username,
										slug: user.userslug,
										username: user.username,
										userslug: user.userslug,
										picture: user.picture,
										banned: user.banned,
										'icon:text': user['icon:text'],
										'icon:bgColor': user['icon:bgColor'],
									},
								};
							});
							response(names);
						}

						$('.ui-autocomplete a').attr('data-ajaxify', 'false');
					});
				},
			});
		});
	};

	module.group = function (input, onselect) {
		app.loadJQueryUI(function () {
			input.autocomplete({
				delay: 200,
				open: function () {
					$(this).autocomplete('widget').css('z-index', 100005);
				},
				select: function (event, ui) {
					handleOnSelect(input, onselect, event, ui);
				},
				source: function (request, response) {
					socket.emit('groups.search', {
						query: request.term,
					}, function (err, results) {
						if (err) {
							return alerts.error(err);
						}
						if (results && results.length) {
							const names = results.map(function (group) {
								return group && {
									label: group.name,
									value: group.name,
									group: group,
								};
							});
							response(names);
						}
						$('.ui-autocomplete a').attr('data-ajaxify', 'false');
					});
				},
			});
		});
	};

	module.tag = function (input, onselect) {
		app.loadJQueryUI(function () {
			input.autocomplete({
				delay: 100,
				open: function () {
					$(this).autocomplete('widget').css('z-index', 20000);
				},
				select: function (event, ui) {
					handleOnSelect(input, onselect, event, ui);
				},
				source: function (request, response) {
					socket.emit('topics.autocompleteTags', {
						query: request.term,
						cid: ajaxify.data.cid || 0,
					}, function (err, tags) {
						if (err) {
							return alerts.error(err);
						}
						if (tags) {
							response(tags);
						}
						$('.ui-autocomplete a').attr('data-ajaxify', 'false');
					});
				},
			});
		});
	};

	function handleOnSelect(input, onselect, event, ui) {
		onselect = onselect || function () { };
		const e = jQuery.Event('keypress');
		e.which = 13;
		e.keyCode = 13;
		setTimeout(function () {
			input.trigger(e);
		}, 100);
		onselect(event, ui);
	}

	return module;
});
