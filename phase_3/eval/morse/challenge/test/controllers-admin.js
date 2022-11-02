'use strict';

const async = require('async');
const assert = require('assert');
const nconf = require('nconf');
const request = require('request');

const db = require('./mocks/databasemock');
const categories = require('../src/categories');
const topics = require('../src/topics');
const user = require('../src/user');
const groups = require('../src/groups');
const helpers = require('./helpers');
const meta = require('../src/meta');

describe('Admin Controllers', () => {
	let tid;
	let cid;
	let pid;
	let regularPid;
	let adminUid;
	let regularUid;
	let regular2Uid;
	let moderatorUid;
	let jar;

	before((done) => {
		async.series({
			category: function (next) {
				categories.create({
					name: 'Test Category',
					description: 'Test category created by testing script',
				}, next);
			},
			adminUid: function (next) {
				user.create({ username: 'admin', password: 'barbar' }, next);
			},
			regularUid: function (next) {
				user.create({ username: 'regular', password: 'regularpwd' }, next);
			},
			regular2Uid: function (next) {
				user.create({ username: 'regular2' }, next);
			},
			moderatorUid: function (next) {
				user.create({ username: 'moderator', password: 'modmod' }, next);
			},
		}, async (err, results) => {
			if (err) {
				return done(err);
			}
			adminUid = results.adminUid;
			regularUid = results.regularUid;
			regular2Uid = results.regular2Uid;
			moderatorUid = results.moderatorUid;
			cid = results.category.cid;

			const adminPost = await topics.post({ uid: adminUid, title: 'test topic title', content: 'test topic content', cid: results.category.cid });
			assert.ifError(err);
			tid = adminPost.topicData.tid;
			pid = adminPost.postData.pid;

			const regularPost = await topics.post({ uid: regular2Uid, title: 'regular user\'s test topic title', content: 'test topic content', cid: results.category.cid });
			regularPid = regularPost.postData.pid;
			done();
		});
	});

	it('should 403 if user is not admin', (done) => {
		helpers.loginUser('admin', 'barbar', (err, data) => {
			assert.ifError(err);
			jar = data.jar;
			request(`${nconf.get('url')}/admin`, { jar: jar }, (err, res, body) => {
				assert.ifError(err);
				assert.equal(res.statusCode, 403);
				assert(body);
				done();
			});
		});
	});

	it('should load admin dashboard', (done) => {
		groups.join('administrators', adminUid, (err) => {
			assert.ifError(err);
			const dashboards = [
				'/admin', '/admin/dashboard/logins', '/admin/dashboard/users', '/admin/dashboard/topics', '/admin/dashboard/searches',
			];
			async.each(dashboards, (url, next) => {
				request(`${nconf.get('url')}${url}`, { jar: jar }, (err, res, body) => {
					assert.ifError(err);
					assert.equal(res.statusCode, 200, url);
					assert(body);

					next();
				});
			}, done);
		});
	});

	it('should load admin analytics', (done) => {
		request(`${nconf.get('url')}/api/admin/analytics?units=hours`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			assert(body.query);
			assert(body.result);
			done();
		});
	});

	it('should load groups page', (done) => {
		request(`${nconf.get('url')}/admin/manage/groups`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load groups detail page', (done) => {
		request(`${nconf.get('url')}/admin/manage/groups/administrators`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load global privileges page', (done) => {
		request(`${nconf.get('url')}/admin/manage/privileges`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load admin privileges page', (done) => {
		request(`${nconf.get('url')}/admin/manage/privileges/admin`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load privileges page for category 1', (done) => {
		request(`${nconf.get('url')}/admin/manage/privileges/1`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load manage digests', (done) => {
		request(`${nconf.get('url')}/admin/manage/digest`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load manage uploads', (done) => {
		request(`${nconf.get('url')}/admin/manage/uploads`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load general settings page', (done) => {
		request(`${nconf.get('url')}/admin/settings`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load email settings page', (done) => {
		request(`${nconf.get('url')}/admin/settings/email`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load user settings page', (done) => {
		request(`${nconf.get('url')}/admin/settings/user`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load info page for a user', (done) => {
		request(`${nconf.get('url')}/api/user/regular/info`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body.history);
			assert(Array.isArray(body.history.flags));
			assert(Array.isArray(body.history.bans));
			assert(Array.isArray(body.sessions));
			done();
		});
	});

	it('should 404 for edit/email page if user does not exist', (done) => {
		request(`${nconf.get('url')}/api/user/doesnotexist/edit/email`, { jar: jar, json: true }, (err, res) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 404);
			done();
		});
	});

	it('should load /admin/settings/homepage', (done) => {
		request(`${nconf.get('url')}/api/admin/settings/homepage`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body.routes);
			done();
		});
	});

	it('should load /admin/advanced/database', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/database`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);

			if (nconf.get('redis')) {
				assert(body.redis);
			} else if (nconf.get('mongo')) {
				assert(body.mongo);
			} else if (nconf.get('postgres')) {
				assert(body.postgres);
			}
			done();
		});
	});

	it('should load /admin/extend/plugins', function (done) {
		this.timeout(50000);
		request(`${nconf.get('url')}/api/admin/extend/plugins`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert(body.hasOwnProperty('installed'));
			assert(body.hasOwnProperty('upgradeCount'));
			assert(body.hasOwnProperty('download'));
			assert(body.hasOwnProperty('incompatible'));
			done();
		});
	});

	it('should load /admin/manage/users', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/users`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.strictEqual(res.statusCode, 200);
			assert(body);
			assert(body.users.length > 0);
			done();
		});
	});


	it('should load /admin/manage/users?filters=banned', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/users?filters=banned`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.strictEqual(res.statusCode, 200);
			assert(body);
			assert.strictEqual(body.users.length, 0);
			done();
		});
	});

	it('should load /admin/manage/users?filters=banned&filters=verified', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/users?filters=banned&filters=verified`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.strictEqual(res.statusCode, 200);
			assert(body);
			assert.strictEqual(body.users.length, 0);
			done();
		});
	});

	it('should load /admin/manage/users?query=admin', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/users?query=admin`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.strictEqual(res.statusCode, 200);
			assert(body);
			assert.strictEqual(body.users[0].username, 'admin');
			done();
		});
	});

	it('should return empty results if query is too short', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/users?query=a`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.strictEqual(res.statusCode, 200);
			assert(body);
			assert.strictEqual(body.users.length, 0);
			done();
		});
	});

	it('should load /admin/manage/registration', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/registration`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should 404 if users is not privileged', (done) => {
		request(`${nconf.get('url')}/api/registration-queue`, { json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 404);
			assert(body);
			done();
		});
	});

	it('should load /api/registration-queue', (done) => {
		request(`${nconf.get('url')}/api/registration-queue`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/manage/admins-mods', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/admins-mods`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/users/csv', (done) => {
		const socketAdmin = require('../src/socket.io/admin');
		socketAdmin.user.exportUsersCSV({ uid: adminUid }, {}, (err) => {
			assert.ifError(err);
			setTimeout(() => {
				request(`${nconf.get('url')}/api/admin/users/csv`, {
					jar: jar,
					headers: {
						referer: `${nconf.get('url')}/admin/manage/users`,
					},
				}, (err, res, body) => {
					assert.ifError(err);
					assert.equal(res.statusCode, 200);
					assert(body);
					done();
				});
			}, 2000);
		});
	});

	it('should return 403 if no referer', (done) => {
		request(`${nconf.get('url')}/api/admin/groups/administrators/csv`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 403);
			assert.equal(body, '[[error:invalid-origin]]');
			done();
		});
	});

	it('should return 403 if referer is not /api/admin/groups/administrators/csv', (done) => {
		request(`${nconf.get('url')}/api/admin/groups/administrators/csv`, {
			jar: jar,
			headers: {
				referer: '/topic/1/test',
			},
		}, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 403);
			assert.equal(body, '[[error:invalid-origin]]');
			done();
		});
	});

	it('should load /api/admin/groups/administrators/csv', (done) => {
		request(`${nconf.get('url')}/api/admin/groups/administrators/csv`, {
			jar: jar,
			headers: {
				referer: `${nconf.get('url')}/admin/manage/groups`,
			},
		}, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/advanced/hooks', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/hooks`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/advanced/cache', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/cache`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /api/admin/advanced/cache/dump and 404 with no query param', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/cache/dump`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 404);
			assert(body);
			done();
		});
	});

	it('should load /api/admin/advanced/cache/dump', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/cache/dump?name=post`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/advanced/errors', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/errors`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/advanced/errors/export', (done) => {
		meta.errors.clear((err) => {
			assert.ifError(err);
			request(`${nconf.get('url')}/api/admin/advanced/errors/export`, { jar: jar }, (err, res, body) => {
				assert.ifError(err);
				assert.equal(res.statusCode, 200);
				assert.strictEqual(body, '');
				done();
			});
		});
	});

	it('should load /admin/advanced/logs', (done) => {
		const fs = require('fs');
		fs.appendFile(meta.logs.path, 'dummy log', (err) => {
			assert.ifError(err);
			request(`${nconf.get('url')}/api/admin/advanced/logs`, { jar: jar, json: true }, (err, res, body) => {
				assert.ifError(err);
				assert.equal(res.statusCode, 200);
				assert(body);
				done();
			});
		});
	});

	it('should load /admin/settings/navigation', (done) => {
		const navigation = require('../src/navigation/admin');
		const data = require('../install/data/navigation.json');

		navigation.save(data, (err) => {
			assert.ifError(err);
			request(`${nconf.get('url')}/api/admin/settings/navigation`, { jar: jar, json: true }, (err, res, body) => {
				assert.ifError(err);
				assert(body);
				assert(body.available);
				assert(body.enabled);
				done();
			});
		});
	});

	it('should load /admin/development/info', (done) => {
		request(`${nconf.get('url')}/api/admin/development/info`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/development/logger', (done) => {
		request(`${nconf.get('url')}/api/admin/development/logger`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/advanced/events', (done) => {
		request(`${nconf.get('url')}/api/admin/advanced/events`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/manage/categories', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/categories`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/manage/categories/1', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/categories/1`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/manage/catgories?cid=<cid>', async () => {
		const { cid: rootCid } = await categories.create({ name: 'parent category' });
		const { cid: childCid } = await categories.create({ name: 'child category', parentCid: rootCid });
		const { res, body } = await helpers.request('get', `/api/admin/manage/categories?cid=${rootCid}`, {
			jar: jar,
			json: true,
		});
		assert.strictEqual(res.statusCode, 200);
		assert.strictEqual(body.categoriesTree[0].cid, rootCid);
		assert.strictEqual(body.categoriesTree[0].children[0].cid, childCid);
		assert.strictEqual(body.breadcrumbs[0].text, '[[admin/manage/categories:top-level]]');
		assert.strictEqual(body.breadcrumbs[1].text, 'parent category');
	});

	it('should load /admin/manage/categories/1/analytics', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/categories/1/analytics`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/extend/rewards', (done) => {
		request(`${nconf.get('url')}/api/admin/extend/rewards`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/extend/widgets', (done) => {
		request(`${nconf.get('url')}/api/admin/extend/widgets`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/settings/languages', (done) => {
		request(`${nconf.get('url')}/api/admin/settings/languages`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/settings/social', (done) => {
		const socketAdmin = require('../src/socket.io/admin');
		socketAdmin.social.savePostSharingNetworks({ uid: adminUid }, ['facebook', 'twitter'], (err) => {
			assert.ifError(err);
			request(`${nconf.get('url')}/api/admin/settings/social`, { jar: jar, json: true }, (err, res, body) => {
				assert.ifError(err);
				assert(body);
				body = body.posts.map(network => network && network.id);
				assert(body.includes('facebook'));
				assert(body.includes('twitter'));
				done();
			});
		});
	});

	it('should load /admin/manage/tags', (done) => {
		request(`${nconf.get('url')}/api/admin/manage/tags`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('/post-queue should 404 for regular user', (done) => {
		request(`${nconf.get('url')}/api/post-queue`, { json: true }, (err, res, body) => {
			assert.ifError(err);
			assert(body);
			assert.equal(res.statusCode, 404);
			done();
		});
	});

	it('should load /post-queue', (done) => {
		request(`${nconf.get('url')}/api/post-queue`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('/ip-blacklist should 404 for regular user', (done) => {
		request(`${nconf.get('url')}/api/ip-blacklist`, { json: true }, (err, res, body) => {
			assert.ifError(err);
			assert(body);
			assert.equal(res.statusCode, 404);
			done();
		});
	});

	it('should load /ip-blacklist', (done) => {
		request(`${nconf.get('url')}/api/ip-blacklist`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/appearance/themes', (done) => {
		request(`${nconf.get('url')}/api/admin/appearance/themes`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /admin/appearance/customise', (done) => {
		request(`${nconf.get('url')}/api/admin/appearance/customise`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			done();
		});
	});

	it('should load /recent in maintenance mode', (done) => {
		meta.config.maintenanceMode = 1;
		request(`${nconf.get('url')}/api/recent`, { jar: jar, json: true }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			meta.config.maintenanceMode = 0;
			done();
		});
	});

	describe('mods page', () => {
		let moderatorJar;
		let regularJar;
		before(async () => {
			moderatorJar = (await helpers.loginUser('moderator', 'modmod')).jar;
			regularJar = (await helpers.loginUser('regular', 'regularpwd')).jar;
			await groups.join(`cid:${cid}:privileges:moderate`, moderatorUid);
		});

		it('should error with no privileges', (done) => {
			request(`${nconf.get('url')}/api/flags`, { json: true }, (err, res, body) => {
				assert.ifError(err);
				assert.deepStrictEqual(body, {
					status: {
						code: 'not-authorised',
						message: 'A valid login session was not found. Please log in and try again.',
					},
					response: {},
				});
				done();
			});
		});

		it('should load flags page data', (done) => {
			request(`${nconf.get('url')}/api/flags`, { jar: moderatorJar, json: true }, (err, res, body) => {
				assert.ifError(err);
				assert(body);
				assert(body.flags);
				assert(body.filters);
				assert.equal(body.filters.cid.indexOf(cid), -1);
				done();
			});
		});

		it('should return a 404 if flag does not exist', (done) => {
			request(`${nconf.get('url')}/api/flags/123123123`, {
				jar: moderatorJar,
				json: true,
				headers: {
					Accept: 'text/html, application/json',
				},
			}, (err, res, body) => {
				assert.ifError(err);
				assert.strictEqual(res.statusCode, 404);
				done();
			});
		});

		it('should error when you attempt to flag a privileged user\'s post', async () => {
			const { res, body } = await helpers.request('post', '/api/v3/flags', {
				json: true,
				jar: regularJar,
				form: {
					id: pid,
					type: 'post',
					reason: 'spam',
				},
			});
			assert.strictEqual(res.statusCode, 400);
			assert.strictEqual(body.status.code, 'bad-request');
			assert.strictEqual(body.status.message, 'You are not allowed to flag the profiles or content of privileged users (moderators/global moderators/admins)');
		});

		it('should error with not enough reputation to flag', async () => {
			const oldValue = meta.config['min:rep:flag'];
			meta.config['min:rep:flag'] = 1000;
			const { res, body } = await helpers.request('post', '/api/v3/flags', {
				json: true,
				jar: regularJar,
				form: {
					id: regularPid,
					type: 'post',
					reason: 'spam',
				},
			});
			assert.strictEqual(res.statusCode, 400);
			assert.strictEqual(body.status.code, 'bad-request');
			assert.strictEqual(body.status.message, 'You need 1000 reputation to flag this post');

			meta.config['min:rep:flag'] = oldValue;
		});

		it('should return flag details', async () => {
			const oldValue = meta.config['min:rep:flag'];
			meta.config['min:rep:flag'] = 0;
			const result = await helpers.request('post', '/api/v3/flags', {
				json: true,
				jar: regularJar,
				form: {
					id: regularPid,
					type: 'post',
					reason: 'spam',
				},
			});
			meta.config['min:rep:flag'] = oldValue;

			const flagsResult = await helpers.request('get', `/api/flags`, {
				json: true,
				jar: moderatorJar,
			});

			assert(flagsResult.body);
			assert(Array.isArray(flagsResult.body.flags));
			const { flagId } = flagsResult.body.flags[0];

			const { body } = await helpers.request('get', `/api/flags/${flagId}`, {
				json: true,
				jar: moderatorJar,
			});
			assert(body.reports);
			assert(Array.isArray(body.reports));
			assert.strictEqual(body.reports[0].reporter.username, 'regular');
		});
	});

	it('should escape special characters in config', (done) => {
		const plugins = require('../src/plugins');
		function onConfigGet(config, callback) {
			config.someValue = '"foo"';
			config.otherValue = "'123'";
			config.script = '</script>';
			callback(null, config);
		}
		plugins.hooks.register('somePlugin', { hook: 'filter:config.get', method: onConfigGet });
		request(`${nconf.get('url')}/admin`, { jar: jar }, (err, res, body) => {
			assert.ifError(err);
			assert.equal(res.statusCode, 200);
			assert(body);
			assert(body.includes('"someValue":"\\\\"foo\\\\""'));
			assert(body.includes('"otherValue":"\\\'123\\\'"'));
			assert(body.includes('"script":"<\\/script>"'));
			request(nconf.get('url'), { jar: jar }, (err, res, body) => {
				assert.ifError(err);
				assert.equal(res.statusCode, 200);
				assert(body);
				assert(body.includes('"someValue":"\\\\"foo\\\\""'));
				assert(body.includes('"otherValue":"\\\'123\\\'"'));
				assert(body.includes('"script":"<\\/script>"'));
				plugins.hooks.unregister('somePlugin', 'filter:config.get', onConfigGet);
				done();
			});
		});
	});

	describe('admin page privileges', () => {
		let userJar;
		let uid;
		const privileges = require('../src/privileges');
		before(async () => {
			uid = await user.create({ username: 'regularjoe', password: 'barbar' });
			userJar = (await helpers.loginUser('regularjoe', 'barbar')).jar;
		});

		describe('routeMap parsing', () => {
			it('should allow normal user access to admin pages', async function () {
				this.timeout(50000);
				function makeRequest(url) {
					return new Promise((resolve, reject) => {
						request(url, { jar: userJar, json: true }, (err, res, body) => {
							if (err) reject(err);
							else resolve(res);
						});
					});
				}
				for (const route of Object.keys(privileges.admin.routeMap)) {
					/* eslint-disable no-await-in-loop */
					await privileges.admin.rescind([privileges.admin.routeMap[route]], uid);
					let res = await makeRequest(`${nconf.get('url')}/api/admin/${route}`);
					assert.strictEqual(res.statusCode, 403);

					await privileges.admin.give([privileges.admin.routeMap[route]], uid);
					res = await makeRequest(`${nconf.get('url')}/api/admin/${route}`);
					assert.strictEqual(res.statusCode, 200);

					await privileges.admin.rescind([privileges.admin.routeMap[route]], uid);
				}

				for (const route of Object.keys(privileges.admin.routeMap)) {
					/* eslint-disable no-await-in-loop */
					await privileges.admin.rescind([privileges.admin.routeMap[route]], uid);
					let res = await makeRequest(`${nconf.get('url')}/api/admin`);
					assert.strictEqual(res.statusCode, 403);

					await privileges.admin.give([privileges.admin.routeMap[route]], uid);
					res = await makeRequest(`${nconf.get('url')}/api/admin`);
					assert.strictEqual(res.statusCode, 200);

					await privileges.admin.rescind([privileges.admin.routeMap[route]], uid);
				}
			});
		});

		describe('routePrefixMap parsing', () => {
			it('should allow normal user access to admin pages', async () => {
				// this.timeout(50000);
				function makeRequest(url) {
					return new Promise((resolve, reject) => {
						request(url, { jar: userJar, json: true }, (err, res, body) => {
							if (err) reject(err);
							else resolve(res);
						});
					});
				}
				for (const route of Object.keys(privileges.admin.routePrefixMap)) {
					/* eslint-disable no-await-in-loop */
					await privileges.admin.rescind([privileges.admin.routePrefixMap[route]], uid);
					let res = await makeRequest(`${nconf.get('url')}/api/admin/${route}foobar/derp`);
					assert.strictEqual(res.statusCode, 403);

					await privileges.admin.give([privileges.admin.routePrefixMap[route]], uid);
					res = await makeRequest(`${nconf.get('url')}/api/admin/${route}foobar/derp`);
					assert.strictEqual(res.statusCode, 404);

					await privileges.admin.rescind([privileges.admin.routePrefixMap[route]], uid);
				}
			});
		});

		it('should list all admin privileges', async () => {
			const privs = await privileges.admin.getPrivilegeList();
			assert.deepStrictEqual(privs, [
				'admin:dashboard',
				'admin:categories',
				'admin:privileges',
				'admin:admins-mods',
				'admin:users',
				'admin:groups',
				'admin:tags',
				'admin:settings',
				'groups:admin:dashboard',
				'groups:admin:categories',
				'groups:admin:privileges',
				'groups:admin:admins-mods',
				'groups:admin:users',
				'groups:admin:groups',
				'groups:admin:tags',
				'groups:admin:settings',
			]);
		});
		it('should list user admin privileges', async () => {
			const privs = await privileges.admin.userPrivileges(adminUid);
			assert.deepStrictEqual(privs, {
				'admin:dashboard': false,
				'admin:categories': false,
				'admin:privileges': false,
				'admin:admins-mods': false,
				'admin:users': false,
				'admin:groups': false,
				'admin:tags': false,
				'admin:settings': false,
			});
		});

		it('should check if group has admin group privilege', async () => {
			await groups.create({ name: 'some-special-group', private: 1, hidden: 1 });
			await privileges.admin.give(['groups:admin:users', 'groups:admin:groups'], 'some-special-group');
			const can = await privileges.admin.canGroup('admin:users', 'some-special-group');
			assert.strictEqual(can, true);
			const privs = await privileges.admin.groupPrivileges('some-special-group');
			assert.deepStrictEqual(privs, {
				'groups:admin:dashboard': false,
				'groups:admin:categories': false,
				'groups:admin:privileges': false,
				'groups:admin:admins-mods': false,
				'groups:admin:users': true,
				'groups:admin:groups': true,
				'groups:admin:tags': false,
				'groups:admin:settings': false,
			});
		});

		it('should not have admin:privileges', async () => {
			const res = await privileges.admin.list(regularUid);
			assert.strictEqual(res.keys.users.includes('admin:privileges'), false);
			assert.strictEqual(res.keys.groups.includes('admin:privileges'), false);
		});
	});
});
