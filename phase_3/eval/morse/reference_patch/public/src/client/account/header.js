'use strict';


define('forum/account/header', [
	'coverPhoto',
	'pictureCropper',
	'components',
	'translator',
	'benchpress',
	'accounts/delete',
	'api',
	'bootbox',
	'alerts',
], function (coverPhoto, pictureCropper, components, translator, Benchpress, AccountsDelete, api, bootbox, alerts) {
	const AccountHeader = {};
	let isAdminOrSelfOrGlobalMod;

	AccountHeader.init = function () {
		isAdminOrSelfOrGlobalMod = ajaxify.data.isAdmin || ajaxify.data.isSelf || ajaxify.data.isGlobalModerator;

		hidePrivateLinks();
		selectActivePill();

		if (isAdminOrSelfOrGlobalMod) {
			setupCoverPhoto();
		}

		components.get('account/follow').on('click', function () {
			toggleFollow('follow');
		});

		components.get('account/unfollow').on('click', function () {
			toggleFollow('unfollow');
		});

		components.get('account/chat').on('click', async function () {
			const roomId = await socket.emit('modules.chats.hasPrivateChat', ajaxify.data.uid);
			const chat = await app.require('chat');
			if (roomId) {
				chat.openChat(roomId);
			} else {
				chat.newChat(ajaxify.data.uid);
			}
		});

		components.get('account/new-chat').on('click', async function () {
			const chat = await app.require('chat');
			chat.newChat(ajaxify.data.uid, function () {
				components.get('account/chat').parent().removeClass('hidden');
			});
		});


		components.get('account/ban').on('click', function () {
			banAccount(ajaxify.data.theirid);
		});
		components.get('account/mute').on('click', muteAccount);
		components.get('account/unban').on('click', unbanAccount);
		components.get('account/unmute').on('click', unmuteAccount);
		components.get('account/delete-account').on('click', handleDeleteEvent.bind(null, 'account'));
		components.get('account/delete-content').on('click', handleDeleteEvent.bind(null, 'content'));
		components.get('account/delete-all').on('click', handleDeleteEvent.bind(null, 'purge'));
		components.get('account/flag').on('click', flagAccount);
		components.get('account/block').on('click', toggleBlockAccount);
	};

	function handleDeleteEvent(type) {
		AccountsDelete[type](ajaxify.data.theirid);
	}

	// TODO: This exported method is used in forum/flags/detail -- refactor??
	AccountHeader.banAccount = banAccount;

	function hidePrivateLinks() {
		if (!app.user.uid || app.user.uid !== parseInt(ajaxify.data.theirid, 10)) {
			$('.account-sub-links .plugin-link.private').addClass('hide');
		}
	}

	function selectActivePill() {
		$('.account-sub-links li').removeClass('active').each(function () {
			const href = $(this).find('a').attr('href');

			if (decodeURIComponent(href) === decodeURIComponent(window.location.pathname)) {
				$(this).addClass('active');
				return false;
			}
		});
	}

	function setupCoverPhoto() {
		coverPhoto.init(
			components.get('account/cover'),
			function (imageData, position, callback) {
				socket.emit('user.updateCover', {
					uid: ajaxify.data.uid,
					imageData: imageData,
					position: position,
				}, callback);
			},
			function () {
				pictureCropper.show({
					title: '[[user:upload_cover_picture]]',
					socketMethod: 'user.updateCover',
					aspectRatio: NaN,
					allowSkippingCrop: true,
					restrictImageDimension: false,
					paramName: 'uid',
					paramValue: ajaxify.data.theirid,
					accept: '.png,.jpg,.bmp',
				}, function (imageUrlOnServer) {
					imageUrlOnServer = (!imageUrlOnServer.startsWith('http') ? config.relative_path : '') + imageUrlOnServer + '?' + Date.now();
					components.get('account/cover').css('background-image', 'url(' + imageUrlOnServer + ')');
				});
			},
			removeCover
		);
	}

	function toggleFollow(type) {
		api[type === 'follow' ? 'put' : 'del']('/users/' + ajaxify.data.uid + '/follow', undefined, function (err) {
			if (err) {
				return alerts.error(err);
			}
			components.get('account/follow').toggleClass('hide', type === 'follow');
			components.get('account/unfollow').toggleClass('hide', type === 'unfollow');
			alerts.success('[[global:alert.' + type + ', ' + ajaxify.data.username + ']]');
		});

		return false;
	}

	function banAccount(theirid, onSuccess) {
		theirid = theirid || ajaxify.data.theirid;

		Benchpress.render('admin/partials/temporary-ban', {}).then(function (html) {
			bootbox.dialog({
				className: 'ban-modal',
				title: '[[user:ban_account]]',
				message: html,
				show: true,
				buttons: {
					close: {
						label: '[[global:close]]',
						className: 'btn-link',
					},
					submit: {
						label: '[[user:ban_account]]',
						callback: function () {
							const formData = $('.ban-modal form').serializeArray().reduce(function (data, cur) {
								data[cur.name] = cur.value;
								return data;
							}, {});

							const until = formData.length > 0 ? (
								Date.now() + (formData.length * 1000 * 60 * 60 * (parseInt(formData.unit, 10) ? 24 : 1))
							) : 0;

							api.put('/users/' + theirid + '/ban', {
								until: until,
								reason: formData.reason || '',
							}).then(() => {
								if (typeof onSuccess === 'function') {
									return onSuccess();
								}

								ajaxify.refresh();
							}).catch(alerts.error);
						},
					},
				},
			});
		});
	}

	function unbanAccount() {
		api.del('/users/' + ajaxify.data.theirid + '/ban').then(() => {
			ajaxify.refresh();
		}).catch(alerts.error);
	}

	function muteAccount() {
		Benchpress.render('admin/partials/temporary-mute', {}).then(function (html) {
			bootbox.dialog({
				className: 'mute-modal',
				title: '[[user:mute_account]]',
				message: html,
				show: true,
				buttons: {
					close: {
						label: '[[global:close]]',
						className: 'btn-link',
					},
					submit: {
						label: '[[user:mute_account]]',
						callback: function () {
							const formData = $('.mute-modal form').serializeArray().reduce(function (data, cur) {
								data[cur.name] = cur.value;
								return data;
							}, {});

							const until = formData.length > 0 ? (
								Date.now() + (formData.length * 1000 * 60 * 60 * (parseInt(formData.unit, 10) ? 24 : 1))
							) : 0;

							api.put('/users/' + ajaxify.data.theirid + '/mute', {
								until: until,
								reason: formData.reason || '',
							}).then(() => {
								ajaxify.refresh();
							}).catch(alerts.error);
						},
					},
				},
			});
		});
	}

	function unmuteAccount() {
		api.del('/users/' + ajaxify.data.theirid + '/mute').then(() => {
			ajaxify.refresh();
		}).catch(alerts.error);
	}

	function flagAccount() {
		require(['flags'], function (flags) {
			flags.showFlagModal({
				type: 'user',
				id: ajaxify.data.uid,
			});
		});
	}

	function toggleBlockAccount() {
		const targetEl = this;
		socket.emit('user.toggleBlock', {
			blockeeUid: ajaxify.data.uid,
			blockerUid: app.user.uid,
		}, function (err, blocked) {
			if (err) {
				return alerts.error(err);
			}

			translator.translate('[[user:' + (blocked ? 'unblock' : 'block') + '_user]]', function (label) {
				$(targetEl).text(label);
			});
		});

		// Keep dropdown open
		return false;
	}

	function removeCover() {
		translator.translate('[[user:remove_cover_picture_confirm]]', function (translated) {
			bootbox.confirm(translated, function (confirm) {
				if (!confirm) {
					return;
				}

				socket.emit('user.removeCover', {
					uid: ajaxify.data.uid,
				}, function (err) {
					if (!err) {
						ajaxify.refresh();
					} else {
						alerts.error(err);
					}
				});
			});
		});
	}

	return AccountHeader;
});
