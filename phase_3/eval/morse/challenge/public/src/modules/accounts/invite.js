'use strict';

define('accounts/invite', ['api', 'benchpress', 'bootbox', 'alerts'], function (api, Benchpress, bootbox, alerts) {
	const Invite = {};

	function isACP() {
		return ajaxify.data.template.name.startsWith('admin/');
	}

	Invite.handle = function () {
		$('[component="user/invite"]').on('click', function (e) {
			e.preventDefault();
			api.get(`/api/v3/users/${app.user.uid}/invites/groups`, {}).then((groups) => {
				Benchpress.parse('modals/invite', { groups: groups }, function (html) {
					bootbox.dialog({
						message: html,
						title: `[[${isACP() ? 'admin/manage/users:invite' : 'users:invite'}]]`,
						onEscape: true,
						buttons: {
							cancel: {
								label: `[[${isACP() ? 'admin/manage/users:alerts.button-cancel' : 'modules:bootbox.cancel'}]]`,
								className: 'btn-default',
							},
							invite: {
								label: `[[${isACP() ? 'admin/manage/users:invite' : 'users:invite'}]]`,
								className: 'btn-primary',
								callback: Invite.send,
							},
						},
					});
				});
			}).catch(alerts.error);
		});
	};

	Invite.send = function () {
		const $emails = $('#invite-modal-emails');
		const $groups = $('#invite-modal-groups');

		const data = {
			emails: $emails.val()
				.split(',')
				.map(m => m.trim())
				.filter(Boolean)
				.filter((m, i, arr) => i === arr.indexOf(m))
				.join(','),
			groupsToJoin: $groups.val(),
		};

		if (!data.emails) {
			return;
		}

		api.post(`/users/${app.user.uid}/invites`, data).then(() => {
			alerts.success(`[[${isACP() ? 'admin/manage/users:alerts.email-sent-to' : 'users:invitation-email-sent'}, ${data.emails.replace(/,/g, '&#44; ')}]]`);
		}).catch(alerts.error);
	};

	return Invite;
});
