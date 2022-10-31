'use strict';

define('accounts/picture', [
	'pictureCropper',
	'api',
	'bootbox',
	'alerts',
], (pictureCropper, api, bootbox, alerts) => {
	const Picture = {};

	Picture.openChangeModal = () => {
		socket.emit('user.getProfilePictures', {
			uid: ajaxify.data.uid,
		}, function (err, pictures) {
			if (err) {
				return alerts.error(err);
			}

			// boolean to signify whether an uploaded picture is present in the pictures list
			const uploaded = pictures.reduce(function (memo, cur) {
				return memo || cur.type === 'uploaded';
			}, false);

			app.parseAndTranslate('partials/modals/change_picture_modal', {
				pictures: pictures,
				uploaded: uploaded,
				icon: { text: ajaxify.data['icon:text'], bgColor: ajaxify.data['icon:bgColor'] },
				defaultAvatar: ajaxify.data.defaultAvatar,
				allowProfileImageUploads: ajaxify.data.allowProfileImageUploads,
				iconBackgrounds: config.iconBackgrounds,
				user: {
					uid: ajaxify.data.uid,
					username: ajaxify.data.username,
					picture: ajaxify.data.picture,
					'icon:text': ajaxify.data['icon:text'],
					'icon:bgColor': ajaxify.data['icon:bgColor'],
				},
			}, function (html) {
				const modal = bootbox.dialog({
					className: 'picture-switcher',
					title: '[[user:change_picture]]',
					message: html,
					show: true,
					buttons: {
						close: {
							label: '[[global:close]]',
							callback: onCloseModal,
							className: 'btn-link',
						},
						update: {
							label: '[[global:save_changes]]',
							callback: saveSelection,
						},
					},
				});

				modal.on('shown.bs.modal', updateImages);
				modal.on('click', '.list-group-item', function selectImageType() {
					modal.find('.list-group-item').removeClass('active');
					$(this).addClass('active');
				});
				modal.on('change', 'input[type="radio"][name="icon:bgColor"]', (e) => {
					const value = e.target.value;
					modal.find('.user-icon').css('background-color', value);
				});

				handleImageUpload(modal);

				function updateImages() {
					// Check to see which one is the active picture
					if (!ajaxify.data.picture) {
						modal.find('.list-group-item .user-icon').parents('.list-group-item').addClass('active');
					} else {
						modal.find('.list-group-item img').each(function () {
							if (this.getAttribute('src') === ajaxify.data.picture) {
								$(this).parents('.list-group-item').addClass('active');
							}
						});
					}

					// Update avatar background colour
					const radioEl = document.querySelector(`.modal input[type="radio"][value="${ajaxify.data['icon:bgColor']}"]`);
					if (radioEl) {
						radioEl.checked = true;
					} else {
						// Check the first one
						document.querySelector('.modal input[type="radio"]').checked = true;
					}
				}

				function saveSelection() {
					const type = modal.find('.list-group-item.active').attr('data-type');
					const iconBgColor = document.querySelector('.modal.picture-switcher input[type="radio"]:checked').value || 'transparent';

					changeUserPicture(type, iconBgColor).then(() => {
						Picture.updateHeader(type === 'default' ? '' : modal.find('.list-group-item.active img').attr('src'), iconBgColor);
						ajaxify.refresh();
					}).catch(alerts.error);
				}

				function onCloseModal() {
					modal.modal('hide');
				}
			});
		});
	};

	Picture.updateHeader = (picture, iconBgColor) => {
		if (parseInt(ajaxify.data.theirid, 10) !== parseInt(ajaxify.data.yourid, 10)) {
			return;
		}
		if (!picture && ajaxify.data.defaultAvatar) {
			picture = ajaxify.data.defaultAvatar;
		}
		$('#header [component="avatar/picture"]')[picture ? 'show' : 'hide']();
		$('#header [component="avatar/icon"]')[!picture ? 'show' : 'hide']();
		if (picture) {
			$('#header [component="avatar/picture"]').attr('src', picture);
		}

		if (iconBgColor) {
			document.querySelectorAll('[component="navbar"] [component="avatar/icon"]').forEach((el) => {
				el.style['background-color'] = iconBgColor;
			});
		}
	};

	function handleImageUpload(modal) {
		function onUploadComplete(urlOnServer) {
			urlOnServer = (!urlOnServer.startsWith('http') ? config.relative_path : '') + urlOnServer + '?' + Date.now();

			Picture.updateHeader(urlOnServer);

			if (ajaxify.data.picture && ajaxify.data.picture.length) {
				$('#user-current-picture, img.avatar').attr('src', urlOnServer);
				ajaxify.data.uploadedpicture = urlOnServer;
			} else {
				ajaxify.refresh(function () {
					$('#user-current-picture, img.avatar').attr('src', urlOnServer);
				});
			}
		}

		function onRemoveComplete() {
			if (ajaxify.data.uploadedpicture === ajaxify.data.picture) {
				ajaxify.refresh();
				Picture.updateHeader();
			}
		}

		modal.find('[data-action="upload"]').on('click', function () {
			modal.modal('hide');

			pictureCropper.show({
				socketMethod: 'user.uploadCroppedPicture',
				route: config.relative_path + '/api/user/' + ajaxify.data.userslug + '/uploadpicture',
				aspectRatio: 1 / 1,
				paramName: 'uid',
				paramValue: ajaxify.data.theirid,
				fileSize: ajaxify.data.maximumProfileImageSize,
				allowSkippingCrop: false,
				title: '[[user:upload_picture]]',
				description: '[[user:upload_a_picture]]',
				accept: ajaxify.data.allowedProfileImageExtensions,
			}, function (url) {
				onUploadComplete(url);
			});

			return false;
		});

		modal.find('[data-action="upload-url"]').on('click', function () {
			modal.modal('hide');
			app.parseAndTranslate('partials/modals/upload_picture_from_url_modal', {}, function (uploadModal) {
				uploadModal.modal('show');

				uploadModal.find('.upload-btn').on('click', function () {
					const url = uploadModal.find('#uploadFromUrl').val();
					if (!url) {
						return false;
					}

					uploadModal.modal('hide');

					pictureCropper.handleImageCrop({
						url: url,
						socketMethod: 'user.uploadCroppedPicture',
						aspectRatio: 1,
						allowSkippingCrop: false,
						paramName: 'uid',
						paramValue: ajaxify.data.theirid,
					}, onUploadComplete);

					return false;
				});
			});

			return false;
		});

		modal.find('[data-action="remove-uploaded"]').on('click', function () {
			socket.emit('user.removeUploadedPicture', {
				uid: ajaxify.data.theirid,
			}, function (err) {
				modal.modal('hide');
				if (err) {
					return alerts.error(err);
				}
				onRemoveComplete();
			});
		});
	}

	function changeUserPicture(type, bgColor) {
		return api.put(`/users/${ajaxify.data.theirid}/picture`, { type, bgColor });
	}

	return Picture;
});
