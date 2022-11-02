'use strict';

define('settings/checkbox', function () {
	let Settings = null;

	const SettingsCheckbox = {
		types: ['checkbox'],
		use: function () {
			Settings = this;
		},
		create: function () {
			return Settings.helper.createElement('input', {
				type: 'checkbox',
			});
		},
		set: function (element, value) {
			element.prop('checked', value);
			element.closest('.mdl-switch').toggleClass('is-checked', element.is(':checked'));
		},
		get: function (element, trim, empty) {
			const value = element.prop('checked');
			if (value == null) {
				return;
			}
			if (!empty) {
				if (value) {
					return value;
				}
				return;
			}
			if (trim) {
				return value ? 1 : 0;
			}
			return value;
		},
	};

	return SettingsCheckbox;
});
