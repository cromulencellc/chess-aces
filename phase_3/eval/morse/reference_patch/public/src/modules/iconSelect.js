'use strict';


define('iconSelect', ['benchpress', 'bootbox'], function (Benchpress, bootbox) {
	const iconSelect = {};

	iconSelect.init = function (el, onModified) {
		onModified = onModified || function () {};
		const doubleSize = el.hasClass('fa-2x');
		let selected = el.attr('class').replace('fa-2x', '').replace('fa', '').replace(/\s+/g, '');

		$('#icons .selected').removeClass('selected');

		if (selected) {
			try {
				$('#icons .fa-icons .fa.' + selected).addClass('selected');
			} catch (err) {
				selected = '';
			}
		}

		Benchpress.render('partials/fontawesome', {}).then(function (html) {
			html = $(html);
			html.find('.fa-icons').prepend($('<i class="fa fa-nbb-none"></i>'));

			const picker = bootbox.dialog({
				onEscape: true,
				backdrop: true,
				show: false,
				message: html,
				title: 'Select an Icon',
				buttons: {
					noIcon: {
						label: 'No Icon',
						className: 'btn-default',
						callback: function () {
							el.attr('class', 'fa ' + (doubleSize ? 'fa-2x ' : ''));
							el.val('');
							el.attr('value', '');

							onModified(el);
						},
					},
					success: {
						label: 'Select',
						className: 'btn-primary',
						callback: function () {
							const iconClass = $('.bootbox .selected').attr('class');
							const categoryIconClass = $('<div></div>').addClass(iconClass).removeClass('fa').removeClass('selected')
								.attr('class');
							const searchElVal = picker.find('input').val();

							if (categoryIconClass) {
								el.attr('class', 'fa ' + (doubleSize ? 'fa-2x ' : '') + categoryIconClass);
								el.val(categoryIconClass);
								el.attr('value', categoryIconClass);
							} else if (searchElVal) {
								el.attr('class', searchElVal);
								el.val(searchElVal);
								el.attr('value', searchElVal);
							}

							onModified(el);
						},
					},
				},
			});

			picker.on('show.bs.modal', function () {
				const modalEl = $(this);
				const searchEl = modalEl.find('input');

				if (selected) {
					modalEl.find('.' + selected).addClass('selected');
					searchEl.val(selected.replace('fa-', ''));
				}
			}).modal('show');

			picker.on('shown.bs.modal', function () {
				const modalEl = $(this);
				const searchEl = modalEl.find('input');
				const icons = modalEl.find('.fa-icons i');
				const submitEl = modalEl.find('button.btn-primary');

				function changeSelection(newSelection) {
					modalEl.find('i.selected').removeClass('selected');
					if (newSelection) {
						newSelection.addClass('selected');
					} else if (searchEl.val().length === 0) {
						if (selected) {
							modalEl.find('.' + selected).addClass('selected');
						}
					} else {
						modalEl.find('i:visible').first().addClass('selected');
					}
				}

				// Focus on the input box
				searchEl.selectRange(0, searchEl.val().length);

				modalEl.find('.icon-container').on('click', 'i', function () {
					searchEl.val($(this).attr('class').replace('fa fa-', '').replace('selected', ''));
					changeSelection($(this));
				});

				searchEl.on('keyup', function (e) {
					if (e.keyCode !== 13) {
						// Filter
						icons.show();
						icons.each(function (idx, el) {
							if (!el.className.match(new RegExp('^fa fa-.*' + searchEl.val() + '.*$'))) {
								$(el).hide();
							}
						});
						changeSelection();
					} else {
						submitEl.click();
					}
				});
			});
		});
	};

	return iconSelect;
});
