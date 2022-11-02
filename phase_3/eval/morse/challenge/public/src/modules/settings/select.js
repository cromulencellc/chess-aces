'use strict';

define('settings/select', function () {
	let Settings = null;

	function addOptions(element, options) {
		for (let i = 0; i < options.length; i += 1) {
			const optionData = options[i];
			const value = optionData.text || optionData.value;
			delete optionData.text;
			element.append($(Settings.helper.createElement('option', optionData)).text(value));
		}
	}


	const SettingsSelect = {
		types: ['select'],
		use: function () {
			Settings = this;
		},
		create: function (ignore, ignored, data) {
			const element = $(Settings.helper.createElement('select'));
			// prevent data-options from being attached to DOM
			addOptions(element, data['data-options']);
			delete data['data-options'];
			return element;
		},
		init: function (element) {
			const options = element.data('options');
			if (options != null) {
				addOptions(element, options);
			}
		},
		set: function (element, value) {
			element.val(value || '');
		},
		get: function (element, ignored, empty) {
			const value = element.val();
			if (empty || value) {
				return value;
			}
		},
	};

	return SettingsSelect;
});
