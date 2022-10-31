'use strict';


define('admin/settings/email', ['ace/ace', 'alerts', 'admin/settings'], function (ace, alerts) {
	const module = {};
	let emailEditor;

	module.init = function () {
		configureEmailTester();
		configureEmailEditor();
		handleDigestHourChange();
		handleSmtpServiceChange();

		$(window).on('action:admin.settingsLoaded action:admin.settingsSaved', handleDigestHourChange);
		$(window).on('action:admin.settingsSaved', function () {
			socket.emit('admin.user.restartJobs');
		});
		$('[id="email:smtpTransport:service"]').change(handleSmtpServiceChange);
	};

	function configureEmailTester() {
		$('button[data-action="email.test"]').off('click').on('click', function () {
			socket.emit('admin.email.test', { template: $('#test-email').val() }, function (err) {
				if (err) {
					console.error(err.message);
					return alerts.error(err);
				}
				alerts.success('Test Email Sent');
			});
			return false;
		});
	}

	function configureEmailEditor() {
		$('#email-editor-selector').on('change', updateEmailEditor);

		emailEditor = ace.edit('email-editor');
		emailEditor.$blockScrolling = Infinity;
		emailEditor.setTheme('ace/theme/twilight');
		emailEditor.getSession().setMode('ace/mode/html');

		emailEditor.on('change', function () {
			const emailPath = $('#email-editor-selector').val();
			let original;
			ajaxify.data.emails.forEach(function (email) {
				if (email.path === emailPath) {
					original = email.original;
				}
			});
			const newEmail = emailEditor.getValue();
			$('#email-editor-holder').val(newEmail !== original ? newEmail : '');
		});

		$('button[data-action="email.revert"]').off('click').on('click', function () {
			ajaxify.data.emails.forEach(function (email) {
				if (email.path === $('#email-editor-selector').val()) {
					emailEditor.getSession().setValue(email.original);
					$('#email-editor-holder').val('');
				}
			});
		});

		updateEmailEditor();
	}

	function updateEmailEditor() {
		ajaxify.data.emails.forEach(function (email) {
			if (email.path === $('#email-editor-selector').val()) {
				emailEditor.getSession().setValue(email.text);
				$('#email-editor-holder')
					.val(email.text !== email.original ? email.text : '')
					.attr('data-field', 'email:custom:' + email.path);
			}
		});
	}

	function handleDigestHourChange() {
		let hour = parseInt($('#digestHour').val(), 10);

		if (isNaN(hour)) {
			hour = 17;
		} else if (hour > 23 || hour < 0) {
			hour = 0;
		}

		socket.emit('admin.getServerTime', {}, function (err, now) {
			if (err) {
				return alerts.error(err);
			}

			const date = new Date(now.timestamp);
			const offset = (new Date().getTimezoneOffset() - now.offset) / 60;
			date.setHours(date.getHours() + offset);

			$('#serverTime').text(date.toLocaleTimeString());

			date.setHours(parseInt(hour, 10) - offset, 0, 0, 0);

			// If adjusted time is in the past, move to next day
			if (date.getTime() < Date.now()) {
				date.setDate(date.getDate() + 1);
			}

			$('#nextDigestTime').text(date.toLocaleString());
		});
	}

	function handleSmtpServiceChange() {
		const isCustom = $('[id="email:smtpTransport:service"]').val() === 'nodebb-custom-smtp';
		$('[id="email:smtpTransport:custom-service"]')[isCustom ? 'slideDown' : 'slideUp'](isCustom);
	}

	return module;
});
