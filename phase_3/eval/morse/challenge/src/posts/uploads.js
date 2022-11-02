'use strict';

const nconf = require('nconf');
const crypto = require('crypto');
const path = require('path');
const winston = require('winston');
const mime = require('mime');
const validator = require('validator');

const db = require('../database');
const image = require('../image');
const user = require('../user');
const topics = require('../topics');
const file = require('../file');
const meta = require('../meta');

module.exports = function (Posts) {
	Posts.uploads = {};

	const md5 = filename => crypto.createHash('md5').update(filename).digest('hex');
	const pathPrefix = path.join(nconf.get('upload_path'));
	const searchRegex = /\/assets\/uploads\/(files\/[^\s")]+\.?[\w]*)/g;

	const _getFullPath = relativePath => path.join(pathPrefix, relativePath);
	const _filterValidPaths = async filePaths => (await Promise.all(filePaths.map(async (filePath) => {
		const fullPath = _getFullPath(filePath);
		return fullPath.startsWith(pathPrefix) && await file.exists(fullPath) ? filePath : false;
	}))).filter(Boolean);

	Posts.uploads.sync = async function (pid) {
		// Scans a post's content and updates sorted set of uploads

		const [content, currentUploads, isMainPost] = await Promise.all([
			Posts.getPostField(pid, 'content'),
			Posts.uploads.list(pid),
			Posts.isMain(pid),
		]);

		// Extract upload file paths from post content
		let match = searchRegex.exec(content);
		const uploads = [];
		while (match) {
			uploads.push(match[1].replace('-resized', ''));
			match = searchRegex.exec(content);
		}

		// Main posts can contain topic thumbs, which are also tracked by pid
		if (isMainPost) {
			const tid = await Posts.getPostField(pid, 'tid');
			let thumbs = await topics.thumbs.get(tid);
			const replacePath = path.posix.join(`${nconf.get('relative_path')}${nconf.get('upload_url')}/`);
			thumbs = thumbs.map(thumb => thumb.url.replace(replacePath, '')).filter(path => !validator.isURL(path, {
				require_protocol: true,
			}));
			uploads.push(...thumbs);
		}

		// Create add/remove sets
		const add = uploads.filter(path => !currentUploads.includes(path));
		const remove = currentUploads.filter(path => !uploads.includes(path));
		await Promise.all([
			Posts.uploads.associate(pid, add),
			Posts.uploads.dissociate(pid, remove),
		]);
	};

	Posts.uploads.list = async function (pid) {
		return await db.getSortedSetMembers(`post:${pid}:uploads`);
	};

	Posts.uploads.listWithSizes = async function (pid) {
		const paths = await Posts.uploads.list(pid);
		const sizes = await db.getObjects(paths.map(path => `upload:${md5(path)}`)) || [];

		return sizes.map((sizeObj, idx) => ({
			...sizeObj,
			name: paths[idx],
		}));
	};

	Posts.uploads.isOrphan = async function (filePath) {
		const length = await db.sortedSetCard(`upload:${md5(filePath)}:pids`);
		return length === 0;
	};

	Posts.uploads.getUsage = async function (filePaths) {
		// Given an array of file names, determines which pids they are used in
		if (!Array.isArray(filePaths)) {
			filePaths = [filePaths];
		}

		const keys = filePaths.map(fileObj => `upload:${md5(fileObj.name.replace('-resized', ''))}:pids`);
		return await Promise.all(keys.map(k => db.getSortedSetRange(k, 0, -1)));
	};

	Posts.uploads.associate = async function (pid, filePaths) {
		// Adds an upload to a post's sorted set of uploads
		filePaths = !Array.isArray(filePaths) ? [filePaths] : filePaths;
		if (!filePaths.length) {
			return;
		}
		filePaths = await _filterValidPaths(filePaths); // Only process files that exist and are within uploads directory

		const now = Date.now();
		const scores = filePaths.map(() => now);
		const bulkAdd = filePaths.map(path => [`upload:${md5(path)}:pids`, now, pid]);
		await Promise.all([
			db.sortedSetAdd(`post:${pid}:uploads`, scores, filePaths),
			db.sortedSetAddBulk(bulkAdd),
			Posts.uploads.saveSize(filePaths),
		]);
	};

	Posts.uploads.dissociate = async function (pid, filePaths) {
		// Removes an upload from a post's sorted set of uploads
		filePaths = !Array.isArray(filePaths) ? [filePaths] : filePaths;
		if (!filePaths.length) {
			return;
		}

		const bulkRemove = filePaths.map(path => [`upload:${md5(path)}:pids`, pid]);
		const promises = [
			db.sortedSetRemove(`post:${pid}:uploads`, filePaths),
			db.sortedSetRemoveBulk(bulkRemove),
		];

		await Promise.all(promises);

		if (!meta.config.preserveOrphanedUploads) {
			const deletePaths = (await Promise.all(
				filePaths.map(async filePath => (await Posts.uploads.isOrphan(filePath) ? filePath : false))
			)).filter(Boolean);

			const uploaderUids = (await db.getObjectsFields(deletePaths.map(path => `upload:${md5(path)}`, ['uid']))).map(o => (o ? o.uid || null : null));
			await Promise.all(uploaderUids.map((uid, idx) => (
				uid && isFinite(uid) ? user.deleteUpload(uid, uid, deletePaths[idx]) : null
			)).filter(Boolean));
			await Posts.uploads.deleteFromDisk(deletePaths);
		}
	};

	Posts.uploads.dissociateAll = async (pid) => {
		const current = await Posts.uploads.list(pid);
		await Posts.uploads.dissociate(pid, current);
	};

	Posts.uploads.deleteFromDisk = async (filePaths) => {
		if (typeof filePaths === 'string') {
			filePaths = [filePaths];
		} else if (!Array.isArray(filePaths)) {
			throw new Error(`[[error:wrong-parameter-type, filePaths, ${typeof filePaths}, array]]`);
		}

		filePaths = (await _filterValidPaths(filePaths)).map(_getFullPath);
		await Promise.all(filePaths.map(file.delete));
	};

	Posts.uploads.saveSize = async (filePaths) => {
		filePaths = filePaths.filter((fileName) => {
			const type = mime.getType(fileName);
			return type && type.match(/image./);
		});
		await Promise.all(filePaths.map(async (fileName) => {
			try {
				const size = await image.size(_getFullPath(fileName));
				await db.setObject(`upload:${md5(fileName)}`, {
					width: size.width,
					height: size.height,
				});
			} catch (err) {
				winston.error(`[posts/uploads] Error while saving post upload sizes (${fileName}): ${err.message}`);
			}
		}));
	};
};
