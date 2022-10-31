'use strict';


define('forum/account/info', ['forum/account/header', 'alerts', 'forum/account/sessions'], function (header, alerts, sessions) {
	const Info = {};

	Info.init = function () {
		header.init();
		handleModerationNote();
		sessions.prepareSessionRevocation();
	};

	function handleModerationNote() {
		$('[component="account/save-moderation-note"]').on('click', function () {
			const note = $('[component="account/moderation-note"]').val();
			socket.emit('user.setModerationNote', { uid: ajaxify.data.uid, note: note }, function (err) {
				if (err) {
					return alerts.error(err);
				}
				$('[component="account/moderation-note"]').val('');
				alerts.success('[[user:info.moderation-note.success]]');
				const timestamp = Date.now();
				const data = [{
					note: utils.escapeHTML(note),
					user: app.user,
					timestamp: timestamp,
					timestampISO: utils.toISOString(timestamp),
				}];
				app.parseAndTranslate('account/info', 'moderationNotes', { moderationNotes: data }, function (html) {
					$('[component="account/moderation-note/list"]').prepend(html);
					html.find('.timeago').timeago();
				});
			});
		});
	}

	return Info;
});
