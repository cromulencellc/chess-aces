'use strict';

define('forum/account/edit/email', ['forum/account/header', 'api', 'alerts'], function (header, api, alerts) {
	const AccountEditEmail = {};

	AccountEditEmail.init = function () {
		header.init();

		$('#submitBtn').on('click', function () {
			const curPasswordEl = $('#inputCurrentPassword');
			const userData = {
				uid: $('#inputUID').val(),
				email: $('#inputNewEmail').val(),
				password: curPasswordEl.val(),
			};

			if (!userData.email) {
				return;
			}

			if (userData.email === userData.password) {
				curPasswordEl.parents('.control-group').toggleClass('has-error', true);
				return alerts.error('[[user:email_same_as_password]]');
			}

			const btn = $(this);
			btn.addClass('disabled').find('i').removeClass('hide');

			api.put('/users/' + userData.uid, userData).then((res) => {
				btn.removeClass('disabled').find('i').addClass('hide');
				ajaxify.go('user/' + res.userslug + '/edit');
			}).catch((err) => {
				setTimeout(() => {
					btn.removeClass('disabled').find('i').addClass('hide');
					alerts.error(err);
				}, 300); // for UX: this call is too fast.
			});

			return false;
		});
	};

	return AccountEditEmail;
});
