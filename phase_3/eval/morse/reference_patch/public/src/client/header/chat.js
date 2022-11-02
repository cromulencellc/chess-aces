'use strict';

define('forum/header/chat', ['components'], function (components) {
	const chat = {};

	chat.prepareDOM = function () {
		const chatsToggleEl = components.get('chat/dropdown');
		const chatsListEl = components.get('chat/list');

		chatsToggleEl.on('click', function () {
			if (chatsToggleEl.parent().hasClass('open')) {
				return;
			}
			requireAndCall('loadChatsDropdown', chatsListEl);
		});

		if (chatsToggleEl.parents('.dropdown').hasClass('open')) {
			requireAndCall('loadChatsDropdown', chatsListEl);
		}

		socket.removeListener('event:chats.receive', onChatMessageReceived);
		socket.on('event:chats.receive', onChatMessageReceived);

		socket.removeListener('event:user_status_change', onUserStatusChange);
		socket.on('event:user_status_change', onUserStatusChange);

		socket.removeListener('event:chats.roomRename', onRoomRename);
		socket.on('event:chats.roomRename', onRoomRename);

		socket.on('event:unread.updateChatCount', function (count) {
			components.get('chat/icon')
				.toggleClass('unread-count', count > 0)
				.attr('data-content', count > 99 ? '99+' : count);
		});
	};

	function onChatMessageReceived(data) {
		requireAndCall('onChatMessageReceived', data);
	}

	function onUserStatusChange(data) {
		requireAndCall('onUserStatusChange', data);
	}

	function onRoomRename(data) {
		requireAndCall('onRoomRename', data);
	}

	function requireAndCall(method, param) {
		require(['chat'], function (chat) {
			chat[method](param);
		});
	}

	return chat;
});
