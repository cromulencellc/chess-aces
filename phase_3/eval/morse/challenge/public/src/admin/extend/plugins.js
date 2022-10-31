'use strict';


define('admin/extend/plugins', [
	'translator',
	'benchpress',
	'bootbox',
	'alerts',
	'jquery-ui/widgets/sortable',
], function (translator, Benchpress, bootbox, alerts) {
	const Plugins = {};
	Plugins.init = function () {
		const pluginsList = $('.plugins');
		const numPlugins = pluginsList[0].querySelectorAll('li').length;
		let pluginID;

		if (!numPlugins) {
			translator.translate('<li><p><i>[[admin/extend/plugins:none-found]]</i></p></li>', function (html) {
				pluginsList.append(html);
			});
			return;
		}

		const searchInputEl = document.querySelector('#plugin-search');
		searchInputEl.value = '';

		pluginsList.on('click', 'button[data-action="toggleActive"]', function () {
			const pluginEl = $(this).parents('li');
			pluginID = pluginEl.attr('data-plugin-id');
			const btn = $('[id="' + pluginID + '"] [data-action="toggleActive"]');

			const pluginData = ajaxify.data.installed[pluginEl.attr('data-plugin-index')];

			function toggleActivate() {
				socket.emit('admin.plugins.toggleActive', pluginID, function (err, status) {
					if (err) {
						return alerts.error(err);
					}
					translator.translate('<i class="fa fa-power-off"></i> [[admin/extend/plugins:plugin-item.' + (status.active ? 'deactivate' : 'activate') + ']]', function (buttonText) {
						btn.html(buttonText);
						btn.toggleClass('btn-warning', status.active).toggleClass('btn-success', !status.active);

						// clone it to active plugins tab
						if (status.active && !$('#active [id="' + pluginID + '"]').length) {
							$('#active ul').prepend(pluginEl.clone(true));
						}

						// Toggle active state in template data
						pluginData.active = !pluginData.active;

						alerts.alert({
							alert_id: 'plugin_toggled',
							title: '[[admin/extend/plugins:alert.' + (status.active ? 'enabled' : 'disabled') + ']]',
							message: '[[admin/extend/plugins:alert.' + (status.active ? 'activate-success' : 'deactivate-success') + ']]',
							type: status.active ? 'warning' : 'success',
							timeout: 5000,
							clickfn: function () {
								require(['admin/modules/instance'], function (instance) {
									instance.rebuildAndRestart();
								});
							},
						});
					});
				});
			}

			if (pluginData.license && pluginData.active !== true) {
				Benchpress.render('admin/partials/plugins/license', pluginData).then(function (html) {
					bootbox.dialog({
						title: '[[admin/extend/plugins:license.title]]',
						message: html,
						size: 'large',
						buttons: {
							cancel: {
								label: '[[modules:bootbox.cancel]]',
								className: 'btn-link',
							},
							save: {
								label: '[[modules:bootbox.confirm]]',
								className: 'btn-primary',
								callback: toggleActivate,
							},
						},
						onShown: function () {
							const saveEl = this.querySelector('button.btn-primary');
							if (saveEl) {
								saveEl.focus();
							}
						},
					});
				});
			} else {
				toggleActivate(pluginID);
			}
		});

		pluginsList.on('click', 'button[data-action="toggleInstall"]', function () {
			const btn = $(this);
			btn.attr('disabled', true);
			pluginID = $(this).parents('li').attr('data-plugin-id');

			if ($(this).attr('data-installed') === '1') {
				return Plugins.toggleInstall(pluginID, $(this).parents('li').attr('data-version'));
			}

			Plugins.suggest(pluginID, function (err, payload) {
				if (err) {
					bootbox.confirm(translator.compile('admin/extend/plugins:alert.suggest-error', err.status, err.responseText), function (confirm) {
						if (confirm) {
							Plugins.toggleInstall(pluginID, 'latest');
						} else {
							btn.removeAttr('disabled');
						}
					});
					return;
				}

				if (payload.version !== 'latest') {
					Plugins.toggleInstall(pluginID, payload.version);
				} else if (payload.version === 'latest') {
					confirmInstall(pluginID, function (confirm) {
						if (confirm) {
							Plugins.toggleInstall(pluginID, 'latest');
						} else {
							btn.removeAttr('disabled');
						}
					});
				} else {
					btn.removeAttr('disabled');
				}
			});
		});

		pluginsList.on('click', 'button[data-action="upgrade"]', function () {
			const btn = $(this);
			const parent = btn.parents('li');
			pluginID = parent.attr('data-plugin-id');

			Plugins.suggest(pluginID, function (err, payload) {
				if (err) {
					return bootbox.alert('[[admin/extend/plugins:alert.package-manager-unreachable]]');
				}

				require(['compare-versions'], function (compareVersions) {
					const currentVersion = parent.find('.currentVersion').text();
					if (payload.version !== 'latest' && compareVersions.compare(payload.version, currentVersion, '>')) {
						upgrade(pluginID, btn, payload.version);
					} else if (payload.version === 'latest') {
						confirmInstall(pluginID, function () {
							upgrade(pluginID, btn, payload.version);
						});
					} else {
						bootbox.alert(translator.compile('admin/extend/plugins:alert.incompatible', app.config.version, payload.version));
					}
				});
			});
		});

		$(searchInputEl).on('input propertychange', function () {
			const term = $(this).val();
			$('.plugins li').each(function () {
				const pluginId = $(this).attr('data-plugin-id');
				$(this).toggleClass('hide', pluginId && pluginId.indexOf(term) === -1);
			});
		});

		$('#plugin-submit-usage').on('click', function () {
			socket.emit('admin.config.setMultiple', {
				submitPluginUsage: $(this).prop('checked') ? '1' : '0',
			}, function (err) {
				if (err) {
					return alerts.error(err);
				}
			});
		});

		$('#plugin-order').on('click', function () {
			$('#order-active-plugins-modal').modal('show');
			socket.emit('admin.plugins.getActive', function (err, activePlugins) {
				if (err) {
					return alerts.error(err);
				}
				let html = '';
				activePlugins.forEach(function (plugin) {
					html += '<li class="">' + plugin + '<span class="pull-right"><i class="fa fa-chevron-up"></i><i class="fa fa-chevron-down"></i></span></li>';
				});
				if (!activePlugins.length) {
					translator.translate('[[admin/extend/plugins:none-active]]', function (text) {
						$('#order-active-plugins-modal .plugin-list').html(text).sortable();
					});
					return;
				}
				const list = $('#order-active-plugins-modal .plugin-list');
				list.html(html).sortable();

				list.find('.fa-chevron-up').on('click', function () {
					const item = $(this).parents('li');
					item.prev().before(item);
				});

				list.find('.fa-chevron-down').on('click', function () {
					const item = $(this).parents('li');
					item.next().after(item);
				});
			});
		});

		$('#save-plugin-order').on('click', function () {
			const plugins = $('#order-active-plugins-modal .plugin-list').children();
			const data = [];
			plugins.each(function (index, el) {
				data.push({ name: $(el).text(), order: index });
			});

			socket.emit('admin.plugins.orderActivePlugins', data, function (err) {
				if (err) {
					return alerts.error(err);
				}
				$('#order-active-plugins-modal').modal('hide');

				alerts.alert({
					alert_id: 'plugin_reordered',
					title: '[[admin/extend/plugins:alert.reorder]]',
					message: '[[admin/extend/plugins:alert.reorder-success]]',
					type: 'success',
					timeout: 5000,
					clickfn: function () {
						require(['admin/modules/instance'], function (instance) {
							instance.rebuildAndRestart();
						});
					},
				});
			});
		});

		populateUpgradeablePlugins();
		populateActivePlugins();
		searchInputEl.focus();
	};

	function confirmInstall(pluginID, callback) {
		bootbox.confirm(translator.compile('admin/extend/plugins:alert.possibly-incompatible', pluginID), function (confirm) {
			callback(confirm);
		});
	}

	function upgrade(pluginID, btn, version) {
		btn.attr('disabled', true).find('i').attr('class', 'fa fa-refresh fa-spin');
		socket.emit('admin.plugins.upgrade', {
			id: pluginID,
			version: version,
		}, function (err, isActive) {
			if (err) {
				return alerts.error(err);
			}
			const parent = btn.parents('li');
			parent.find('.fa-exclamation-triangle').remove();
			parent.find('.currentVersion').text(version);
			btn.remove();
			if (isActive) {
				alerts.alert({
					alert_id: 'plugin_upgraded',
					title: '[[admin/extend/plugins:alert.upgraded]]',
					message: '[[admin/extend/plugins:alert.upgrade-success]]',
					type: 'warning',
					timeout: 5000,
					clickfn: function () {
						require(['admin/modules/instance'], function (instance) {
							instance.rebuildAndRestart();
						});
					},
				});
			}
		});
	}

	Plugins.toggleInstall = function (pluginID, version, callback) {
		const btn = $('li[data-plugin-id="' + pluginID + '"] button[data-action="toggleInstall"]');
		btn.find('i').attr('class', 'fa fa-refresh fa-spin');

		socket.emit('admin.plugins.toggleInstall', {
			id: pluginID,
			version: version,
		}, function (err, pluginData) {
			if (err) {
				btn.removeAttr('disabled');
				return alerts.error(err);
			}

			ajaxify.refresh();

			alerts.alert({
				alert_id: 'plugin_toggled',
				title: '[[admin/extend/plugins:alert.' + (pluginData.installed ? 'installed' : 'uninstalled') + ']]',
				message: '[[admin/extend/plugins:alert.' + (pluginData.installed ? 'install-success' : 'uninstall-success') + ']]',
				type: 'info',
				timeout: 5000,
			});

			if (typeof callback === 'function') {
				callback.apply(this, arguments);
			}
		});
	};

	Plugins.suggest = function (pluginId, callback) {
		const nbbVersion = app.config.version.match(/^\d+\.\d+\.\d+/);
		$.ajax((app.config.registry || 'https://packages.nodebb.org') + '/api/v1/suggest', {
			type: 'GET',
			data: {
				package: pluginId,
				version: nbbVersion[0],
			},
			dataType: 'json',
		}).done(function (payload) {
			callback(undefined, payload);
		}).fail(callback);
	};

	function populateUpgradeablePlugins() {
		$('#installed ul li').each(function () {
			if ($(this).children('[data-action="upgrade"]').length) {
				$('#upgrade ul').append($(this).clone(true));
			}
		});
	}

	function populateActivePlugins() {
		$('#installed ul li').each(function () {
			if ($(this).hasClass('active')) {
				$('#active ul').append($(this).clone(true));
			} else {
				$('#deactive ul').append($(this).clone(true));
			}
		});
	}

	return Plugins;
});
