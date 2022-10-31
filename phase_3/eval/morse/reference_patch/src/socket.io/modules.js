'use strict';

const db = require('../database');
const notifications = require('../notifications');
const Messaging = require('../messaging');
const utils = require('../utils');
const server = require('./index');
const user = require('../user');
const privileges = require('../privileges');

const sockets = require('.');
const api = require('../api');

const SocketModules = module.exports;

SocketModules.chats = {};
SocketModules.settings = {};

/* Chat */

SocketModules.chats.getRaw = async function (socket, data) {
	if (!data || !data.hasOwnProperty('mid')) {
		throw new Error('[[error:invalid-data]]');
	}
	const roomId = await Messaging.getMessageField(data.mid, 'roomId');
	const [isAdmin, hasMessage, inRoom] = await Promise.all([
		user.isAdministrator(socket.uid),
		db.isSortedSetMember(`uid:${socket.uid}:chat:room:${roomId}:mids`, data.mid),
		Messaging.isUserInRoom(socket.uid, roomId),
	]);

	if (!isAdmin && (!inRoom || !hasMessage)) {
		throw new Error('[[error:not-allowed]]');
	}

	return await Messaging.getMessageField(data.mid, 'content');
};

SocketModules.chats.isDnD = async function (socket, uid) {
	const status = await db.getObjectField(`user:${uid}`, 'status');
	return status === 'dnd';
};

SocketModules.chats.newRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'POST /api/v3/chats');

	if (!data) {
		throw new Error('[[error:invalid-data]]');
	}

	const roomObj = await api.chats.create(socket, {
		uids: [data.touid],
	});
	return roomObj.roomId;
};

SocketModules.chats.send = async function (socket, data) {
	sockets.warnDeprecated(socket, 'POST /api/v3/chats/:roomId');

	if (!data || !data.roomId || !socket.uid) {
		throw new Error('[[error:invalid-data]]');
	}

	const canChat = await privileges.global.can('chat', socket.uid);
	if (!canChat) {
		throw new Error('[[error:no-privileges]]');
	}

	return api.chats.post(socket, data);
};

SocketModules.chats.loadRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'GET /api/v3/chats/:roomId');

	if (!data || !data.roomId) {
		throw new Error('[[error:invalid-data]]');
	}

	return await Messaging.loadRoom(socket.uid, data);
};

SocketModules.chats.getUsersInRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'GET /api/v3/chats/:roomId/users');

	if (!data || !data.roomId) {
		throw new Error('[[error:invalid-data]]');
	}
	const isUserInRoom = await Messaging.isUserInRoom(socket.uid, data.roomId);
	if (!isUserInRoom) {
		throw new Error('[[error:no-privileges]]');
	}

	return api.chats.users(socket, data);
};

SocketModules.chats.addUserToRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'POST /api/v3/chats/:roomId/users');

	if (!data || !data.roomId || !data.username) {
		throw new Error('[[error:invalid-data]]');
	}

	const canChat = await privileges.global.can('chat', socket.uid);
	if (!canChat) {
		throw new Error('[[error:no-privileges]]');
	}

	// Revised API now takes uids, not usernames
	data.uids = [await user.getUidByUsername(data.username)];
	delete data.username;

	await api.chats.invite(socket, data);
};

SocketModules.chats.removeUserFromRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'DELETE /api/v3/chats/:roomId/users OR DELETE /api/v3/chats/:roomId/users/:uid');

	if (!data || !data.roomId) {
		throw new Error('[[error:invalid-data]]');
	}

	// Revised API can accept multiple uids now
	data.uids = [data.uid];
	delete data.uid;

	await api.chats.kick(socket, data);
};

SocketModules.chats.leave = async function (socket, roomid) {
	sockets.warnDeprecated(socket, 'DELETE /api/v3/chats/:roomId/users OR DELETE /api/v3/chats/:roomId/users/:uid');

	if (!socket.uid || !roomid) {
		throw new Error('[[error:invalid-data]]');
	}

	await Messaging.leaveRoom([socket.uid], roomid);
};

SocketModules.chats.edit = async function (socket, data) {
	sockets.warnDeprecated(socket, 'PUT /api/v3/chats/:roomId/:mid');

	if (!data || !data.roomId || !data.message) {
		throw new Error('[[error:invalid-data]]');
	}
	await Messaging.canEdit(data.mid, socket.uid);
	await Messaging.editMessage(socket.uid, data.mid, data.roomId, data.message);
};

SocketModules.chats.delete = async function (socket, data) {
	sockets.warnDeprecated(socket, 'DELETE /api/v3/chats/:roomId/:mid');

	if (!data || !data.roomId || !data.messageId) {
		throw new Error('[[error:invalid-data]]');
	}
	await Messaging.canDelete(data.messageId, socket.uid);
	await Messaging.deleteMessage(data.messageId, socket.uid);
};

SocketModules.chats.restore = async function (socket, data) {
	sockets.warnDeprecated(socket, 'POST /api/v3/chats/:roomId/:mid');

	if (!data || !data.roomId || !data.messageId) {
		throw new Error('[[error:invalid-data]]');
	}
	await Messaging.canDelete(data.messageId, socket.uid);
	await Messaging.restoreMessage(data.messageId, socket.uid);
};

SocketModules.chats.canMessage = async function (socket, roomId) {
	await Messaging.canMessageRoom(socket.uid, roomId);
};

SocketModules.chats.markRead = async function (socket, roomId) {
	if (!socket.uid || !roomId) {
		throw new Error('[[error:invalid-data]]');
	}
	const [uidsInRoom] = await Promise.all([
		Messaging.getUidsInRoom(roomId, 0, -1),
		Messaging.markRead(socket.uid, roomId),
	]);

	Messaging.pushUnreadCount(socket.uid);
	server.in(`uid_${socket.uid}`).emit('event:chats.markedAsRead', { roomId: roomId });

	if (!uidsInRoom.includes(String(socket.uid))) {
		return;
	}

	// Mark notification read
	const nids = uidsInRoom.filter(uid => parseInt(uid, 10) !== socket.uid)
		.map(uid => `chat_${uid}_${roomId}`);

	await notifications.markReadMultiple(nids, socket.uid);
	await user.notifications.pushCount(socket.uid);
};

SocketModules.chats.markAllRead = async function (socket) {
	await Messaging.markAllRead(socket.uid);
	Messaging.pushUnreadCount(socket.uid);
};

SocketModules.chats.renameRoom = async function (socket, data) {
	sockets.warnDeprecated(socket, 'PUT /api/v3/chats/:roomId');

	if (!data || !data.roomId || !data.newName) {
		throw new Error('[[error:invalid-data]]');
	}

	data.name = data.newName;
	delete data.newName;
	await api.chats.rename(socket, data);
};

SocketModules.chats.getRecentChats = async function (socket, data) {
	if (!data || !utils.isNumber(data.after) || !utils.isNumber(data.uid)) {
		throw new Error('[[error:invalid-data]]');
	}
	const start = parseInt(data.after, 10);
	const stop = start + 9;
	return await Messaging.getRecentChats(socket.uid, data.uid, start, stop);
};

SocketModules.chats.hasPrivateChat = async function (socket, uid) {
	if (socket.uid <= 0 || uid <= 0) {
		throw new Error('[[error:invalid-data]]');
	}
	return await Messaging.hasPrivateChat(socket.uid, uid);
};

SocketModules.chats.getMessages = async function (socket, data) {
	sockets.warnDeprecated(socket, 'GET /api/v3/chats/:roomId/messages');

	if (!socket.uid || !data || !data.uid || !data.roomId) {
		throw new Error('[[error:invalid-data]]');
	}

	return await Messaging.getMessages({
		callerUid: socket.uid,
		uid: data.uid,
		roomId: data.roomId,
		start: parseInt(data.start, 10) || 0,
		count: 50,
	});
};

SocketModules.chats.getIP = async function (socket, mid) {
	const allowed = await privileges.global.can('view:users:info', socket.uid);
	if (!allowed) {
		throw new Error('[[error:no-privilege]]');
	}
	return await Messaging.getMessageField(mid, 'ip');
};

require('../promisify')(SocketModules);
