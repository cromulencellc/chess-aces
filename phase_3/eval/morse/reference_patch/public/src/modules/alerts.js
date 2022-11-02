'use strict';


define('alerts', ['translator', 'components', 'hooks'], function (translator, components, hooks) {
	const module = {};

	module.alert = function (params) {
		params.alert_id = 'alert_button_' + (params.alert_id ? params.alert_id : new Date().getTime());
		params.title = params.title ? params.title.trim() || '' : '';
		params.message = params.message ? params.message.trim() : '';
		params.type = params.type || 'info';

		const alert = $('#' + params.alert_id);
		if (alert.length) {
			updateAlert(alert, params);
		} else {
			createNew(params);
		}
	};

	module.success = function (message, timeout) {
		module.alert({
			alert_id: utils.generateUUID(),
			title: '[[global:alert.success]]',
			message: message,
			type: 'success',
			timeout: timeout || 5000,
		});
	};

	module.error = function (message, timeout) {
		message = (message && message.message) || message;

		if (message === '[[error:revalidate-failure]]') {
			socket.disconnect();
			app.reconnect();
			return;
		}

		module.alert({
			alert_id: utils.generateUUID(),
			title: '[[global:alert.error]]',
			message: message,
			type: 'danger',
			timeout: timeout || 10000,
		});
	};

	module.remove = function (id) {
		$('#alert_button_' + id).remove();
	};

	function createNew(params) {
		app.parseAndTranslate('alert', params, function (html) {
			let alert = $('#' + params.alert_id);
			if (alert.length) {
				return updateAlert(alert, params);
			}
			alert = html;
			alert.fadeIn(200);

			components.get('toaster/tray').prepend(alert);

			if (typeof params.closefn === 'function') {
				alert.find('button').on('click', function () {
					params.closefn();
					fadeOut(alert);
					return false;
				});
			}

			if (params.timeout) {
				startTimeout(alert, params);
			}

			if (typeof params.clickfn === 'function') {
				alert
					.addClass('pointer')
					.on('click', function (e) {
						if (!$(e.target).is('.close')) {
							params.clickfn(alert, params);
						}
						fadeOut(alert);
					});
			}

			hooks.fire('action:alert.new', { alert, params });
		});
	}

	function updateAlert(alert, params) {
		alert.find('strong').translateHtml(params.title);
		alert.find('p').translateHtml(params.message);
		alert.attr('class', 'alert alert-dismissable alert-' + params.type + ' clearfix');

		clearTimeout(parseInt(alert.attr('timeoutId'), 10));
		if (params.timeout) {
			startTimeout(alert, params);
		}

		hooks.fire('action:alert.update', { alert, params });

		// Handle changes in the clickfn
		alert.off('click').removeClass('pointer');
		if (typeof params.clickfn === 'function') {
			alert
				.addClass('pointer')
				.on('click', function (e) {
					if (!$(e.target).is('.close')) {
						params.clickfn();
					}
					fadeOut(alert);
				});
		}
	}

	function fadeOut(alert) {
		alert.fadeOut(500, function () {
			$(this).remove();
		});
	}

	function startTimeout(alert, params) {
		const timeout = params.timeout;

		const timeoutId = setTimeout(function () {
			fadeOut(alert);

			if (typeof params.timeoutfn === 'function') {
				params.timeoutfn(alert, params);
			}
		}, timeout);

		alert.attr('timeoutId', timeoutId);

		// Reset and start animation
		alert.css('transition-property', 'none');
		alert.removeClass('animate');

		setTimeout(function () {
			alert.css('transition-property', '');
			alert.css('transition', 'width ' + (timeout + 450) + 'ms linear, background-color ' + (timeout + 450) + 'ms ease-in');
			alert.addClass('animate');
			hooks.fire('action:alert.animate', { alert, params });
		}, 50);

		// Handle mouseenter/mouseleave
		alert
			.on('mouseenter', function () {
				$(this).css('transition-duration', 0);
			});
	}

	return module;
});
