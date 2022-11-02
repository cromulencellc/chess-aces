'use strict';


define('admin/extend/widgets', [
	'bootbox',
	'alerts',
	'jquery-ui/widgets/sortable',
	'jquery-ui/widgets/draggable',
	'jquery-ui/widgets/droppable',
	'jquery-ui/widgets/datepicker',
], function (bootbox, alerts) {
	const Widgets = {};

	Widgets.init = function () {
		$('#widgets .nav-pills .dropdown-menu a').on('click', function (ev) {
			const $this = $(this);
			$('#widgets .tab-pane').removeClass('active');
			const templateName = $this.attr('data-template');
			$('#widgets .tab-pane[data-template="' + templateName + '"]').addClass('active');
			$('#widgets .selected-template').text(templateName);
			$('#widgets .nav-pills .dropdown').trigger('click');
			ev.preventDefault();
			return false;
		});

		$('#widget-selector').on('change', function () {
			$('.available-widgets [data-widget]').addClass('hide');
			$('.available-widgets [data-widget="' + $(this).val() + '"]').removeClass('hide');
		});

		$('#widget-selector').trigger('change');

		loadWidgetData();
		setupCloneButton();
	};

	function prepareWidgets() {
		$('[data-location="drafts"]').insertAfter($('[data-location="drafts"]').closest('.tab-content'));

		$('#widgets .available-widgets .widget-panel').draggable({
			helper: function (e) {
				return $(e.target).parents('.widget-panel').clone();
			},
			distance: 10,
			connectToSortable: '.widget-area',
		});

		$('#widgets .available-containers .containers > [data-container-html]')
			.draggable({
				helper: function (e) {
					let target = $(e.target);
					target = target.attr('data-container-html') ? target : target.parents('[data-container-html]');

					return target.clone().addClass('block').width(target.width()).css('opacity', '0.5');
				},
				distance: 10,
			})
			.each(function () {
				$(this).attr('data-container-html', $(this).attr('data-container-html').replace(/\\\{([\s\S]*?)\\\}/g, '{$1}'));
			});

		$('#widgets .widget-area').sortable({
			update: function (event, ui) {
				createDatePicker(ui.item);
				appendToggle(ui.item);
			},
			connectWith: 'div',
		}).on('click', '.delete-widget', function () {
			const panel = $(this).parents('.widget-panel');

			bootbox.confirm('[[admin/extend/widgets:alert.confirm-delete]]', function (confirm) {
				if (confirm) {
					panel.remove();
				}
			});
		}).on('mouseup', '> .panel > .panel-heading', function (evt) {
			if (!($(this).parent().is('.ui-sortable-helper') || $(evt.target).closest('.delete-widget').length)) {
				$(this).parent().children('.panel-body').toggleClass('hidden');
			}
		});

		$('#save').on('click', saveWidgets);

		function saveWidgets() {
			const saveData = [];
			$('#widgets [data-template][data-location]').each(function (i, el) {
				el = $(el);

				const template = el.attr('data-template');
				const location = el.attr('data-location');
				const area = el.children('.widget-area');
				const widgets = [];

				area.find('.widget-panel[data-widget]').each(function () {
					const widgetData = {};
					const data = $(this).find('form').serializeArray();

					for (const d in data) {
						if (data.hasOwnProperty(d)) {
							if (data[d].name) {
								if (widgetData[data[d].name]) {
									if (!Array.isArray(widgetData[data[d].name])) {
										widgetData[data[d].name] = [
											widgetData[data[d].name],
										];
									}
									widgetData[data[d].name].push(data[d].value);
								} else {
									widgetData[data[d].name] = data[d].value;
								}
							}
						}
					}

					widgets.push({
						widget: $(this).attr('data-widget'),
						data: widgetData,
					});
				});

				saveData.push({
					template: template,
					location: location,
					widgets: widgets,
				});
			});

			socket.emit('admin.widgets.set', saveData, function (err) {
				if (err) {
					alerts.error(err);
				}

				alerts.alert({
					alert_id: 'admin:widgets',
					type: 'success',
					title: '[[admin/extend/widgets:alert.updated]]',
					message: '[[admin/extend/widgets:alert.update-success]]',
					timeout: 2500,
				});
			});
		}

		$('.color-selector').on('click', '.btn', function () {
			const btn = $(this);
			const selector = btn.parents('.color-selector');
			const container = selector.parents('[data-container-html]');
			const classList = [];

			selector.children().each(function () {
				classList.push($(this).attr('data-class'));
			});

			container
				.removeClass(classList.join(' '))
				.addClass(btn.attr('data-class'));

			container.attr('data-container-html', container.attr('data-container-html')
				.replace(/class="[a-zA-Z0-9-\s]+"/, 'class="' + container[0].className.replace(' pointer ui-draggable ui-draggable-handle', '') + '"'));
		});
	}

	function createDatePicker(el) {
		const currentYear = new Date().getFullYear();
		el.find('.date-selector').datepicker({
			changeMonth: true,
			changeYear: true,
			yearRange: currentYear + ':' + (currentYear + 100),
		});
	}

	function appendToggle(el) {
		if (!el.hasClass('block')) {
			el.addClass('block').css('width', '').css('height', '')
				.droppable({
					accept: '[data-container-html]',
					drop: function (event, ui) {
						const el = $(this);

						el.find('.panel-body .container-html').val(ui.draggable.attr('data-container-html'));
						el.find('.panel-body').removeClass('hidden');
					},
					hoverClass: 'panel-info',
				})
				.children('.panel-heading')
				.append('<div class="pull-right pointer"><span class="delete-widget"><i class="fa fa-times-circle"></i></span></div><div class="pull-left pointer"><span class="toggle-widget"><i class="fa fa-chevron-circle-down"></i></span>&nbsp;</div>')
				.children('small')
				.html('');
		}
	}

	function loadWidgetData() {
		function populateWidget(widget, data) {
			if (data.title) {
				const title = widget.find('.panel-heading strong');
				title.text(title.text() + ' - ' + data.title);
			}

			widget.find('input, textarea, select').each(function () {
				const input = $(this);
				const value = data[input.attr('name')];

				if (input.attr('type') === 'checkbox') {
					input.prop('checked', !!value).trigger('change');
				} else {
					input.val(value);
				}
			});

			return widget;
		}

		$.get(config.relative_path + '/api/admin/extend/widgets', function (data) {
			const areas = data.areas;

			for (let i = 0; i < areas.length; i += 1) {
				const area = areas[i];
				const widgetArea = $('#widgets .area[data-template="' + area.template + '"][data-location="' + area.location + '"]').find('.widget-area');

				widgetArea.html('');

				for (let k = 0; k < area.data.length; k += 1) {
					const widgetData = area.data[k];
					const widgetEl = $('.available-widgets [data-widget="' + widgetData.widget + '"]').clone(true).removeClass('hide');

					widgetArea.append(populateWidget(widgetEl, widgetData.data));
					appendToggle(widgetEl);
					createDatePicker(widgetEl);
				}
			}

			prepareWidgets();
		});
	}

	function setupCloneButton() {
		const clone = $('[component="clone"]');
		const cloneBtn = $('[component="clone/button"]');

		clone.find('.dropdown-menu li').on('click', function () {
			const template = $(this).find('a').text();
			cloneBtn.translateHtml('[[admin/extend/widgets:clone-from]] <strong>' + template + '</strong>');
			cloneBtn.attr('data-template', template);
		});

		cloneBtn.on('click', function () {
			const template = cloneBtn.attr('data-template');
			if (!template) {
				return alerts.error('[[admin/extend/widgets:error.select-clone]]');
			}

			const currentTemplate = $('#active-widgets .active.tab-pane[data-template] .area');
			const templateToClone = $('#active-widgets .tab-pane[data-template="' + template + '"] .area');

			const currentAreas = currentTemplate.map(function () {
				return $(this).attr('data-location');
			}).get();

			const areasToClone = templateToClone.map(function () {
				const location = $(this).attr('data-location');
				return currentAreas.indexOf(location) !== -1 ? location : undefined;
			}).get().filter(function (i) { return i; });

			function clone(location) {
				$('#active-widgets .tab-pane[data-template="' + template + '"] [data-location="' + location + '"]').each(function () {
					$(this).find('[data-widget]').each(function () {
						const widget = $(this).clone(true);
						$('#active-widgets .active.tab-pane[data-template]:not([data-template="global"]) [data-location="' + location + '"] .widget-area').append(widget);
					});
				});
			}

			for (let i = 0, ii = areasToClone.length; i < ii; i++) {
				const location = areasToClone[i];
				clone(location);
			}

			alerts.success('[[admin/extend/widgets:alert.clone-success]]');
		});
	}

	return Widgets;
});
