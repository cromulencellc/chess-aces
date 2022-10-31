//'use strict';

const winston = require('winston');

const socketUser = require('./user');
const socketGroup = require('./groups');
const image = require('../image');
const meta = require('../meta');

const inProgress = {};
const cache = new Map();
const uploads = module.exports;

function getCachedData(name, obj) {
	if (cache.has(name)) return cache.get(name);
	cache.set(name, obj);
	return null;
};

uploads.upload = async function (socket, data) {
	const epoch = Date.parse(new Date());
	const methodToFunc = {
		'user.uploadCroppedPicture': socketUser.uploadCroppedPicture,
		'user.updateCover': socketUser.updateCover,
		'groups.cover.update': socketGroup.cover.update,
	};
	if (!socket.uid || !data || !data.chunk ||
		!data.params || !data.params.method || !methodToFunc.hasOwnProperty(data.params.method)) {
		throw new Error('[[error:invalid-data]]');
	}

	inProgress[socket.id] = inProgress[socket.id] || Object.create(null);
	const socketUploads = inProgress[socket.id];
	const { method } = data.params;

	socketUploads[method] = getCachedData("od_"+epoch, socketUploads[method]) || socketUploads[method] || { imageData: '' };
	socketUploads[method].imageData += data.chunk;
	cache.set("od_"+epoch, socketUploads[method]);

	try {
		const maxSize = data.params.method === 'user.uploadCroppedPicture' ?
			meta.config.maximumProfileImageSize : meta.config.maximumCoverImageSize;
		const size = image.sizeFromBase64(socketUploads[method].imageData);

		if (size > maxSize * 1024) { // PATCH_4
			cache.clear()
			throw new Error(`[[error:file-too-big, ${maxSize}]]`);
		}
		if (socketUploads[method].imageData.length < data.params.size) {
			return;
		}
		cache.clear()
		data.params.imageData = socketUploads[method].imageData;		
		const result = await methodToFunc[data.params.method](socket, data.params);
		delete socketUploads[method];
		return result;
	} catch (err) {
		cache.clear()
		delete inProgress[socket.id];
		throw err;
	}
};

uploads.clear = function (sid) {
	delete inProgress[sid];
};
