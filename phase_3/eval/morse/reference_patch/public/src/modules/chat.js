'use strict';

define('chat', [
	'components', 'taskbar', 'translator', 'hooks', 'bootbox', 'alerts', 'api',
], function (components, taskbar, translator, hooks, bootbox, alerts, api) {
	const module = {};
	let newMessage = false;

	module.openChat = function (roomId, uid) {
		if (!app.user.uid) {
			return alerts.error('[[error:not-logged-in]]');
		}

		function loadAndCenter(chatModal) {
			module.load(chatModal.attr('data-uuid'));
			module.center(chatModal);
			module.focusInput(chatModal);
		}

		if (module.modalExists(roomId)) {
			loadAndCenter(module.getModal(roomId));
		} else {
			api.get(`/chats/${roomId}`, {
				uid: uid || app.user.uid,
			}).then((roomData) => {
				roomData.users = roomData.users.filter(function (user) {
					return user && parseInt(user.uid, 10) !== parseInt(app.user.uid, 10);
				});
				roomData.uid = uid || app.user.uid;
				roomData.isSelf = true;
				module.createModal(roomData, loadAndCenter);
			}).catch(alerts.error);
		}
	};

	module.newChat = function (touid, callback) {
		function createChat() {
			api.post(`/chats`, {
				uids: [touid],
			}).then(({ roomId }) => {
				if (!ajaxify.data.template.chats) {
					module.openChat(roomId);
				} else {
					ajaxify.go('chats/' + roomId);
				}

				callback(null, roomId);
			}).catch(alerts.error);
		}

		callback = callback || function () { };
		if (!app.user.uid) {
			return alerts.error('[[error:not-logged-in]]');
		}

		if (parseInt(touid, 10) === parseInt(app.user.uid, 10)) {
			return alerts.error('[[error:cant-chat-with-yourself]]');
		}
		socket.emit('modules.chats.isDnD', touid, function (err, isDnD) {
			if (err) {
				return alerts.error(err);
			}
			if (!isDnD) {
				return createChat();
			}

			bootbox.confirm('[[modules:chat.confirm-chat-with-dnd-user]]', function (ok) {
				if (ok) {
					createChat();
				}
			});
		});
	};

	module.loadChatsDropdown = function (chatsListEl) {
		socket.emit('modules.chats.getRecentChats', {
			uid: app.user.uid,
			after: 0,
		}, function (err, data) {
			if (err) {
				return alerts.error(err);
			}

			const rooms = data.rooms.filter(function (room) {
				return room.teaser;
			});

			translator.toggleTimeagoShorthand(function () {
				for (let i = 0; i < rooms.length; i += 1) {
					rooms[i].teaser.timeago = $.timeago(new Date(parseInt(rooms[i].teaser.timestamp, 10)));
				}
				translator.toggleTimeagoShorthand();
				app.parseAndTranslate('partials/chats/dropdown', { rooms: rooms }, function (html) {
					chatsListEl.find('*').not('.navigation-link').remove();
					chatsListEl.prepend(html);
					app.createUserTooltips(chatsListEl, 'right');
					chatsListEl.off('click').on('click', '[data-roomid]', function (ev) {
						if ($(ev.target).parents('.user-link').length) {
							return;
						}
						const roomId = $(this).attr('data-roomid');
						if (!ajaxify.currentPage.match(/^chats\//)) {
							module.openChat(roomId);
						} else {
							ajaxify.go('user/' + app.user.userslug + '/chats/' + roomId);
						}
					});

					$('[component="chats/mark-all-read"]').off('click').on('click', function () {
						socket.emit('modules.chats.markAllRead', function (err) {
							if (err) {
								return alerts.error(err);
							}
						});
					});
				});
			});
		});
	};


	module.onChatMessageReceived = function (data) {
		const isSelf = data.self === 1;
		data.message.self = data.self;

		newMessage = data.self === 0;
		if (module.modalExists(data.roomId)) {
			addMessageToModal(data);
		} else if (!ajaxify.data.template.chats) {
			api.get(`/chats/${data.roomId}`, {}).then((roomData) => {
				roomData.users = roomData.users.filter(function (user) {
					return user && parseInt(user.uid, 10) !== parseInt(app.user.uid, 10);
				});
				roomData.silent = true;
				roomData.uid = app.user.uid;
				roomData.isSelf = isSelf;
				module.createModal(roomData);
			}).catch(alerts.error);
		}
	};

	function addMessageToModal(data) {
		const modal = module.getModal(data.roomId);
		const username = data.message.fromUser.username;
		const isSelf = data.self === 1;
		require(['forum/chats/messages'], function (ChatsMessages) {
			// don't add if already added
			if (!modal.find('[data-mid="' + data.message.messageId + '"]').length) {
				ChatsMessages.appendChatMessage(modal.find('.chat-content'), data.message);
			}

			if (modal.is(':visible')) {
				taskbar.updateActive(modal.attr('data-uuid'));
				if (ChatsMessages.isAtBottom(modal.find('.chat-content'))) {
					ChatsMessages.scrollToBottom(modal.find('.chat-content'));
				}
			} else if (!ajaxify.data.template.chats) {
				module.toggleNew(modal.attr('data-uuid'), true, true);
			}

			if (!isSelf && (!modal.is(':visible') || !app.isFocused)) {
				taskbar.push('chat', modal.attr('data-uuid'), {
					title: '[[modules:chat.chatting_with]] ' + (data.roomName || username),
					touid: data.message.fromUser.uid,
					roomId: data.roomId,
					isSelf: false,
				});
			}
		});
	}

	module.onUserStatusChange = function (data) {
		const modal = module.getModal(data.uid);
		app.updateUserStatus(modal.find('[component="user/status"]'), data.status);
	};

	module.onRoomRename = function (data) {
		const newTitle = $('<div></div>').html(data.newName).text();
		const modal = module.getModal(data.roomId);
		modal.find('[component="chat/room/name"]').text(newTitle);
		taskbar.update('chat', modal.attr('data-uuid'), {
			title: newTitle,
		});
		hooks.fire('action:chat.renamed', Object.assign(data, {
			modal: modal,
		}));
	};

	module.getModal = function (roomId) {
		return $('#chat-modal-' + roomId);
	};

	module.modalExists = function (roomId) {
		return $('#chat-modal-' + roomId).length !== 0;
	};

	module.createModal = function (data, callback) {
		callback = callback || function () {};
		require([
			'scrollStop', 'forum/chats', 'forum/chats/messages',
		], function (scrollStop, Chats, ChatsMessages) {
			app.parseAndTranslate('chat', data, function (chatModal) {
				if (module.modalExists(data.roomId)) {
					return callback(module.getModal(data.roomId));
				}
				const uuid = utils.generateUUID();
				let dragged = false;

				chatModal.attr('id', 'chat-modal-' + data.roomId);
				chatModal.attr('data-roomid', data.roomId);
				chatModal.attr('intervalId', 0);
				chatModal.attr('data-uuid', uuid);
				chatModal.css('position', 'fixed');
				chatModal.appendTo($('body'));
				chatModal.find('.timeago').timeago();
				module.center(chatModal);

				app.loadJQueryUI(function () {
					chatModal.find('.modal-content').resizable({
						handles: 'n, e, s, w, se',
						minHeight: 250,
						minWidth: 400,
					});

					chatModal.find('.modal-content').on('resize', function (event, ui) {
						if (ui.originalSize.height === ui.size.height) {
							return;
						}

						chatModal.find('.modal-body').css('height', module.calculateChatListHeight(chatModal));
					});

					chatModal.draggable({
						start: function () {
							taskbar.updateActive(uuid);
						},
						stop: function () {
							module.focusInput(chatModal);
						},
						distance: 10,
						handle: '.modal-header',
					});
				});

				scrollStop.apply(chatModal.find('[component="chat/messages"]'));

				chatModal.find('#chat-close-btn').on('click', function () {
					module.close(chatModal);
				});

				function gotoChats() {
					const text = components.get('chat/input').val();
					$(window).one('action:ajaxify.end', function () {
						components.get('chat/input').val(text);
					});

					ajaxify.go('user/' + app.user.userslug + '/chats/' + chatModal.attr('data-roomid'));
					module.close(chatModal);
				}

				chatModal.find('.modal-header').on('dblclick', gotoChats);
				chatModal.find('button[data-action="maximize"]').on('click', gotoChats);
				chatModal.find('button[data-action="minimize"]').on('click', function () {
					const uuid = chatModal.attr('data-uuid');
					module.minimize(uuid);
				});

				chatModal.on('mouseup', function () {
					taskbar.updateActive(chatModal.attr('data-uuid'));

					if (dragged) {
						dragged = false;
					}
				});

				chatModal.on('mousemove', function (e) {
					if (e.which === 1) {
						dragged = true;
					}
				});

				chatModal.on('mousemove keypress click', function () {
					if (newMessage) {
						socket.emit('modules.chats.markRead', data.roomId);
						newMessage = false;
					}
				});

				Chats.addActionHandlers(chatModal.find('[component="chat/messages"]'), data.roomId);
				Chats.addRenameHandler(chatModal.attr('data-roomid'), chatModal.find('[data-action="rename"]'), data.roomName);
				Chats.addLeaveHandler(chatModal.attr('data-roomid'), chatModal.find('[data-action="leave"]'));
				Chats.addSendHandlers(chatModal.attr('data-roomid'), chatModal.find('.chat-input'), chatModal.find('[data-action="send"]'));
				Chats.addMemberHandler(chatModal.attr('data-roomid'), chatModal.find('[data-action="members"]'));

				Chats.createAutoComplete(chatModal.find('[component="chat/input"]'));

				Chats.addScrollHandler(chatModal.attr('data-roomid'), data.uid, chatModal.find('.chat-content'));
				Chats.addScrollBottomHandler(chatModal.find('.chat-content'));

				Chats.addCharactersLeftHandler(chatModal);
				Chats.addIPHandler(chatModal);

				Chats.addUploadHandler({
					dragDropAreaEl: chatModal.find('.modal-content'),
					pasteEl: chatModal,
					uploadFormEl: chatModal.find('[component="chat/upload"]'),
					inputEl: chatModal.find('[component="chat/input"]'),
				});

				ChatsMessages.addSocketListeners();

				taskbar.push('chat', chatModal.attr('data-uuid'), {
					title: '[[modules:chat.chatting_with]] ' + (data.roomName || (data.users.length ? data.users[0].username : '')),
					roomId: data.roomId,
					icon: 'fa-comment',
					state: '',
					isSelf: data.isSelf,
				}, function () {
					taskbar.toggleNew(chatModal.attr('data-uuid'), !data.isSelf);
					hooks.fire('action:chat.loaded', chatModal);

					if (typeof callback === 'function') {
						callback(chatModal);
					}
				});
			});
		});
	};

	module.focusInput = function (chatModal) {
		setTimeout(function () {
			chatModal.find('[component="chat/input"]').focus();
		}, 20);
	};

	module.close = function (chatModal) {
		const uuid = chatModal.attr('data-uuid');
		clearInterval(chatModal.attr('intervalId'));
		chatModal.attr('intervalId', 0);
		chatModal.remove();
		chatModal.data('modal', null);
		taskbar.discard('chat', uuid);

		if (chatModal.attr('data-mobile')) {
			module.disableMobileBehaviour(chatModal);
		}

		hooks.fire('action:chat.closed', {
			uuid: uuid,
			modal: chatModal,
		});
	};

	// TODO: see taskbar.js:44
	module.closeByUUID = function (uuid) {
		const chatModal = $('.chat-modal[data-uuid="' + uuid + '"]');
		module.close(chatModal);
	};

	module.center = function (chatModal) {
		let hideAfter = false;
		if (chatModal.hasClass('hide')) {
			chatModal.removeClass('hide');
			hideAfter = true;
		}
		chatModal.css('left', Math.max(0, (($(window).width() - $(chatModal).outerWidth()) / 2) + $(window).scrollLeft()) + 'px');
		chatModal.css('top', Math.max(0, ($(window).height() / 2) - ($(chatModal).outerHeight() / 2)) + 'px');

		if (hideAfter) {
			chatModal.addClass('hide');
		}
		return chatModal;
	};

	module.load = function (uuid) {
		require(['forum/chats/messages'], function (ChatsMessages) {
			const chatModal = $('.chat-modal[data-uuid="' + uuid + '"]');
			chatModal.removeClass('hide');
			taskbar.updateActive(uuid);
			ChatsMessages.scrollToBottom(chatModal.find('.chat-content'));
			module.focusInput(chatModal);
			socket.emit('modules.chats.markRead', chatModal.attr('data-roomid'));

			const env = utils.findBootstrapEnvironment();
			if (env === 'xs' || env === 'sm') {
				module.enableMobileBehaviour(chatModal);
			}
		});
	};

	module.enableMobileBehaviour = function (modalEl) {
		app.toggleNavbar(false);
		modalEl.attr('data-mobile', '1');
		const messagesEl = modalEl.find('.modal-body');
		messagesEl.css('height', module.calculateChatListHeight(modalEl));
		function resize() {
			messagesEl.css('height', module.calculateChatListHeight(modalEl));
			require(['forum/chats/messages'], function (ChatsMessages) {
				ChatsMessages.scrollToBottom(modalEl.find('.chat-content'));
			});
		}

		$(window).on('resize', resize);
		$(window).one('action:ajaxify.start', function () {
			module.close(modalEl);
			$(window).off('resize', resize);
		});
	};

	module.disableMobileBehaviour = function () {
		app.toggleNavbar(true);
	};

	module.calculateChatListHeight = function (modalEl) {
		// Formula: modal height minus header height. Simple(tm).
		return modalEl.find('.modal-content').outerHeight() - modalEl.find('.modal-header').outerHeight();
	};

	module.minimize = function (uuid) {
		const chatModal = $('.chat-modal[data-uuid="' + uuid + '"]');
		chatModal.addClass('hide');
		taskbar.minimize('chat', uuid);
		clearInterval(chatModal.attr('intervalId'));
		chatModal.attr('intervalId', 0);
		hooks.fire('action:chat.minimized', {
			uuid: uuid,
			modal: chatModal,
		});
	};

	module.toggleNew = taskbar.toggleNew;

	return module;
});
