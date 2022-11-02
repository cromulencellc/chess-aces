'use strict';

(function () {
	let logoutTimer = 0;
	let logoutMessage;
	function startLogoutTimer() {
		if (app.config.adminReloginDuration <= 0) {
			return;
		}
		if (logoutTimer) {
			clearTimeout(logoutTimer);
		}
		// pre-translate language string gh#9046
		if (!logoutMessage) {
			require(['translator'], function (translator) {
				translator.translate('[[login:logged-out-due-to-inactivity]]', function (translated) {
					logoutMessage = translated;
				});
			});
		}

		logoutTimer = setTimeout(function () {
			require(['bootbox'], function (bootbox) {
				bootbox.alert({
					closeButton: false,
					message: logoutMessage,
					callback: function () {
						window.location.reload();
					},
				});
			});
		}, 3600000);
	}

	require(['hooks'], (hooks) => {
		hooks.on('action:ajaxify.end', () => {
			showCorrectNavTab();
			startLogoutTimer();
		});
	});

	function showCorrectNavTab() {
		// show correct tab if url has #
		if (window.location.hash) {
			$('.nav-pills a[href="' + window.location.hash + '"]').tab('show');
		}
	}

	$(document).ready(function () {
		if (!/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
			require(['admin/modules/search'], function (search) {
				search.init();
			});
		}

		$('[component="logout"]').on('click', function () {
			require(['logout'], function (logout) {
				logout();
			});
			return false;
		});

		configureSlidemenu();
		setupNProgress();
	});

	$(window).on('action:ajaxify.contentLoaded', function (ev, data) {
		selectMenuItem(data.url);
		setupRestartLinks();

		componentHandler.upgradeDom();
	});

	function setupNProgress() {
		require(['nprogress', 'hooks'], function (NProgress, hooks) {
			$(window).on('action:ajaxify.start', function () {
				NProgress.set(0.7);
			});

			hooks.on('action:ajaxify.end', function () {
				NProgress.done();
			});
		});
	}

	function selectMenuItem(url) {
		require(['translator'], function (translator) {
			url = url
				.replace(/\/\d+$/, '')
				.split('/').slice(0, 3).join('/')
				.split(/[?#]/)[0].replace(/(\/+$)|(^\/+)/, '');

			// If index is requested, load the dashboard
			if (url === 'admin') {
				url = 'admin/dashboard';
			}

			url = [config.relative_path, url].join('/');
			let fallback;

			$('#main-menu li').removeClass('active');
			$('#main-menu a').removeClass('active').filter('[href="' + url + '"]').each(function () {
				const menu = $(this);
				if (menu.parent().attr('data-link')) {
					return;
				}

				menu
					.parent().addClass('active')
					.parents('.menu-item').addClass('active');
				fallback = menu.text();
			});

			let mainTitle;
			let pageTitle;
			if (/admin\/plugins\//.test(url)) {
				mainTitle = fallback;
				pageTitle = '[[admin/menu:section-plugins]] > ' + mainTitle;
			} else {
				const matches = url.match(/admin\/(.+?)\/(.+?)$/);
				if (matches) {
					mainTitle = '[[admin/menu:' + matches[1] + '/' + matches[2] + ']]';
					pageTitle = '[[admin/menu:section-' +
						(matches[1] === 'development' ? 'advanced' : matches[1]) +
						']]' + (matches[2] ? (' > ' + mainTitle) : '');
					if (matches[2] === 'settings') {
						mainTitle = translator.compile('admin/menu:settings.page-title', mainTitle);
					}
				} else {
					mainTitle = '[[admin/menu:section-dashboard]]';
					pageTitle = '[[admin/menu:section-dashboard]]';
				}
			}

			pageTitle = translator.compile('admin/admin:acp-title', pageTitle);

			translator.translate(pageTitle, function (title) {
				document.title = title.replace(/&gt;/g, '>');
			});
			translator.translate(mainTitle, function (text) {
				$('#main-page-title').text(text);
			});
		});
	}

	function setupRestartLinks() {
		$('.rebuild-and-restart').off('click').on('click', function () {
			require(['bootbox'], function (bootbox) {
				bootbox.confirm('[[admin/admin:alert.confirm-rebuild-and-restart]]', function (confirm) {
					if (confirm) {
						require(['admin/modules/instance'], function (instance) {
							instance.rebuildAndRestart();
						});
					}
				});
			});
		});

		$('.restart').off('click').on('click', function () {
			require(['bootbox'], function (bootbox) {
				bootbox.confirm('[[admin/admin:alert.confirm-restart]]', function (confirm) {
					if (confirm) {
						require(['admin/modules/instance'], function (instance) {
							instance.restart();
						});
					}
				});
			});
		});
	}

	function configureSlidemenu() {
		require(['slideout'], function (Slideout) {
			let env = utils.findBootstrapEnvironment();

			const slideout = new Slideout({
				panel: document.getElementById('panel'),
				menu: document.getElementById('menu'),
				padding: 256,
				tolerance: 70,
			});

			if (env === 'md' || env === 'lg') {
				slideout.disableTouch();
			}

			$('#mobile-menu').on('click', function () {
				slideout.toggle();
			});

			$('#menu a').on('click', function () {
				slideout.close();
			});

			$(window).on('resize', function () {
				slideout.close();

				env = utils.findBootstrapEnvironment();

				if (env === 'md' || env === 'lg') {
					slideout.disableTouch();
					$('#header').css({
						position: 'relative',
					});
				} else {
					slideout.enableTouch();
					$('#header').css({
						position: 'fixed',
					});
				}
			});

			function onOpeningMenu() {
				$('#header').css({
					top: ($('#panel').position().top * -1) + 'px',
					position: 'absolute',
				});
			}

			slideout.on('open', onOpeningMenu);

			slideout.on('close', function () {
				$('#header').css({
					top: '0px',
					position: 'fixed',
				});
			});
		});
	}

	// tell ace to use the right paths when requiring modules
	require(['ace/ace'], function (ace) {
		ace.config.set('packaged', true);
		ace.config.set('basePath', config.relative_path + '/assets/src/modules/ace/');
	});
}());
