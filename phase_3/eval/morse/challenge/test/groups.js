'use strict';

const assert = require('assert');
const async = require('async');
const fs = require('fs');
const path = require('path');
const nconf = require('nconf');

const db = require('./mocks/databasemock');
const helpers = require('./helpers');
const Groups = require('../src/groups');
const User = require('../src/user');
const socketGroups = require('../src/socket.io/groups');
const apiGroups = require('../src/api/groups');
const meta = require('../src/meta');
const navigation = require('../src/navigation/admin');


describe('Groups', () => {
	let adminUid;
	let testUid;
	before(async () => {
		const navData = require('../install/data/navigation.json');
		await navigation.save(navData);

		await Groups.create({
			name: 'Test',
			description: 'Foobar!',
		});

		await Groups.create({
			name: 'PrivateNoJoin',
			description: 'Private group',
			private: 1,
			disableJoinRequests: 1,
		});

		await Groups.create({
			name: 'PrivateCanJoin',
			description: 'Private group',
			private: 1,
			disableJoinRequests: 0,
		});

		await Groups.create({
			name: 'PrivateNoLeave',
			description: 'Private group',
			private: 1,
			disableLeave: 1,
		});

		await Groups.create({
			name: 'Global Moderators',
			userTitle: 'Global Moderator',
			description: 'Forum wide moderators',
			hidden: 0,
			private: 1,
			disableJoinRequests: 1,
		});

		// Also create a hidden group
		await Groups.join('Hidden', 'Test');
		// create another group that starts with test for search/sort
		await Groups.create({ name: 'Test2', description: 'Foobar!' });

		testUid = await User.create({
			username: 'testuser',
			email: 'b@c.com',
		});

		adminUid = await User.create({
			username: 'admin',
			email: 'admin@admin.com',
			password: '123456',
		});
		await Groups.join('administrators', adminUid);
	});

	describe('.list()', () => {
		it('should list the groups present', (done) => {
			Groups.getGroupsFromSet('groups:visible:createtime', 0, -1, (err, groups) => {
				assert.ifError(err);
				assert.equal(groups.length, 5);
				done();
			});
		});
	});

	describe('.get()', () => {
		before((done) => {
			Groups.join('Test', testUid, done);
		});

		it('with no options, should show group information', (done) => {
			Groups.get('Test', {}, (err, groupObj) => {
				assert.ifError(err);
				assert.equal(typeof groupObj, 'object');
				assert(Array.isArray(groupObj.members));
				assert.strictEqual(groupObj.name, 'Test');
				assert.strictEqual(groupObj.description, 'Foobar!');
				assert.strictEqual(groupObj.memberCount, 1);
				assert.equal(typeof groupObj.members[0], 'object');

				done();
			});
		});

		it('should return null if group does not exist', (done) => {
			Groups.get('doesnotexist', {}, (err, groupObj) => {
				assert.ifError(err);
				assert.strictEqual(groupObj, null);
				done();
			});
		});
	});

	describe('.search()', () => {
		const socketGroups = require('../src/socket.io/groups');

		it('should return empty array if query is falsy', (done) => {
			Groups.search(null, {}, (err, groups) => {
				assert.ifError(err);
				assert.equal(0, groups.length);
				done();
			});
		});

		it('should return the groups when search query is empty', (done) => {
			socketGroups.search({ uid: adminUid }, { query: '' }, (err, groups) => {
				assert.ifError(err);
				assert.equal(5, groups.length);
				done();
			});
		});

		it('should return the "Test" group when searched for', (done) => {
			socketGroups.search({ uid: adminUid }, { query: 'test' }, (err, groups) => {
				assert.ifError(err);
				assert.equal(2, groups.length);
				assert.strictEqual('Test', groups[0].name);
				done();
			});
		});

		it('should return the "Test" group when searched for and sort by member count', (done) => {
			Groups.search('test', { filterHidden: true, sort: 'count' }, (err, groups) => {
				assert.ifError(err);
				assert.equal(2, groups.length);
				assert.strictEqual('Test', groups[0].name);
				done();
			});
		});

		it('should return the "Test" group when searched for and sort by creation time', (done) => {
			Groups.search('test', { filterHidden: true, sort: 'date' }, (err, groups) => {
				assert.ifError(err);
				assert.equal(2, groups.length);
				assert.strictEqual('Test', groups[1].name);
				done();
			});
		});

		it('should return all users if no query', (done) => {
			function createAndJoinGroup(username, email, callback) {
				async.waterfall([
					function (next) {
						User.create({ username: username, email: email }, next);
					},
					function (uid, next) {
						Groups.join('Test', uid, next);
					},
				], callback);
			}
			async.series([
				function (next) {
					createAndJoinGroup('newuser', 'newuser@b.com', next);
				},
				function (next) {
					createAndJoinGroup('bob', 'bob@b.com', next);
				},
			], (err) => {
				assert.ifError(err);

				socketGroups.searchMembers({ uid: adminUid }, { groupName: 'Test', query: '' }, (err, data) => {
					assert.ifError(err);
					assert.equal(data.users.length, 3);
					done();
				});
			});
		});

		it('should search group members', (done) => {
			socketGroups.searchMembers({ uid: adminUid }, { groupName: 'Test', query: 'test' }, (err, data) => {
				assert.ifError(err);
				assert.strictEqual('testuser', data.users[0].username);
				done();
			});
		});

		it('should not return hidden groups', async () => {
			await Groups.create({
				name: 'hiddenGroup',
				hidden: '1',
			});
			const result = await socketGroups.search({ uid: testUid }, { query: 'hiddenGroup' });
			assert.equal(result.length, 0);
		});
	});

	describe('.isMember()', () => {
		it('should return boolean true when a user is in a group', (done) => {
			Groups.isMember(1, 'Test', (err, isMember) => {
				assert.ifError(err);
				assert.strictEqual(isMember, true);
				done();
			});
		});

		it('should return boolean false when a user is not in a group', (done) => {
			Groups.isMember(2, 'Test', (err, isMember) => {
				assert.ifError(err);
				assert.strictEqual(isMember, false);
				done();
			});
		});

		it('should return true for uid 0 and guests group', (done) => {
			Groups.isMembers([1, 0], 'guests', (err, isMembers) => {
				assert.ifError(err);
				assert.deepStrictEqual(isMembers, [false, true]);
				done();
			});
		});

		it('should return true for uid 0 and guests group', (done) => {
			Groups.isMemberOfGroups(0, ['guests', 'registered-users'], (err, isMembers) => {
				assert.ifError(err);
				assert.deepStrictEqual(isMembers, [true, false]);
				done();
			});
		});
	});

	describe('.isMemberOfGroupList', () => {
		it('should report that a user is part of a groupList, if they are', (done) => {
			Groups.isMemberOfGroupList(1, 'Hidden', (err, isMember) => {
				assert.ifError(err);
				assert.strictEqual(isMember, true);
				done();
			});
		});

		it('should report that a user is not part of a groupList, if they are not', (done) => {
			Groups.isMemberOfGroupList(2, 'Hidden', (err, isMember) => {
				assert.ifError(err);
				assert.strictEqual(isMember, false);
				done();
			});
		});
	});

	describe('.exists()', () => {
		it('should verify that the test group exists', (done) => {
			Groups.exists('Test', (err, exists) => {
				assert.ifError(err);
				assert.strictEqual(exists, true);
				done();
			});
		});

		it('should verify that a fake group does not exist', (done) => {
			Groups.exists('Derp', (err, exists) => {
				assert.ifError(err);
				assert.strictEqual(exists, false);
				done();
			});
		});

		it('should check if group exists using an array', (done) => {
			Groups.exists(['Test', 'Derp'], (err, groupsExists) => {
				assert.ifError(err);
				assert.strictEqual(groupsExists[0], true);
				assert.strictEqual(groupsExists[1], false);
				done();
			});
		});
	});

	describe('.create()', () => {
		it('should create another group', (done) => {
			Groups.create({
				name: 'foo',
				description: 'bar',
			}, (err) => {
				assert.ifError(err);
				Groups.get('foo', {}, done);
			});
		});

		it('should create a hidden group if hidden is 1', (done) => {
			Groups.create({
				name: 'hidden group',
				hidden: '1',
			}, (err) => {
				assert.ifError(err);
				db.isSortedSetMember('groups:visible:memberCount', 'visible group', (err, isMember) => {
					assert.ifError(err);
					assert(!isMember);
					done();
				});
			});
		});

		it('should create a visible group if hidden is 0', (done) => {
			Groups.create({
				name: 'visible group',
				hidden: '0',
			}, (err) => {
				assert.ifError(err);
				db.isSortedSetMember('groups:visible:memberCount', 'visible group', (err, isMember) => {
					assert.ifError(err);
					assert(isMember);
					done();
				});
			});
		});

		it('should create a visible group if hidden is not passed in', (done) => {
			Groups.create({
				name: 'visible group 2',
			}, (err) => {
				assert.ifError(err);
				db.isSortedSetMember('groups:visible:memberCount', 'visible group 2', (err, isMember) => {
					assert.ifError(err);
					assert(isMember);
					done();
				});
			});
		});

		it('should fail to create group with duplicate group name', (done) => {
			Groups.create({ name: 'foo' }, (err) => {
				assert(err);
				assert.equal(err.message, '[[error:group-already-exists]]');
				done();
			});
		});

		it('should fail to create group if slug is empty', (done) => {
			Groups.create({ name: '>>>>' }, (err) => {
				assert.equal(err.message, '[[error:invalid-group-name]]');
				done();
			});
		});

		it('should fail if group name is invalid', (done) => {
			Groups.create({ name: 'not/valid' }, (err) => {
				assert.equal(err.message, '[[error:invalid-group-name]]');
				done();
			});
		});

		it('should fail if group name is invalid', (done) => {
			Groups.create({ name: ['array/'] }, (err) => {
				assert.equal(err.message, '[[error:invalid-group-name]]');
				done();
			});
		});

		it('should fail if group name is invalid', async () => {
			try {
				await apiGroups.create({ uid: adminUid }, { name: ['test', 'administrators'] });
			} catch (err) {
				return assert.equal(err.message, '[[error:invalid-group-name]]');
			}
			assert(false);
		});

		it('should not create a system group', async () => {
			await apiGroups.create({ uid: adminUid }, { name: 'mysystemgroup', system: true });
			const data = await Groups.getGroupData('mysystemgroup');
			assert.strictEqual(data.system, 0);
		});

		it('should fail if group name is invalid', (done) => {
			Groups.create({ name: 'not:valid' }, (err) => {
				assert.equal(err.message, '[[error:invalid-group-name]]');
				done();
			});
		});

		it('should return falsy for userTitleEnabled', (done) => {
			Groups.create({ name: 'userTitleEnabledGroup' }, (err) => {
				assert.ifError(err);
				Groups.setGroupField('userTitleEnabledGroup', 'userTitleEnabled', 0, (err) => {
					assert.ifError(err);
					Groups.getGroupData('userTitleEnabledGroup', (err, data) => {
						assert.ifError(err);
						assert.strictEqual(data.userTitleEnabled, 0);
						done();
					});
				});
			});
		});
	});

	describe('.hide()', () => {
		it('should mark the group as hidden', (done) => {
			Groups.hide('foo', (err) => {
				assert.ifError(err);

				Groups.get('foo', {}, (err, groupObj) => {
					assert.ifError(err);
					assert.strictEqual(1, groupObj.hidden);
					done();
				});
			});
		});
	});

	describe('.update()', () => {
		before((done) => {
			Groups.create({
				name: 'updateTestGroup',
				description: 'bar',
				system: 0,
				hidden: 0,
			}, done);
		});

		it('should change an aspect of a group', (done) => {
			Groups.update('updateTestGroup', {
				description: 'baz',
			}, (err) => {
				assert.ifError(err);

				Groups.get('updateTestGroup', {}, (err, groupObj) => {
					assert.ifError(err);
					assert.strictEqual('baz', groupObj.description);
					done();
				});
			});
		});

		it('should rename a group and not break navigation routes', async () => {
			await Groups.update('updateTestGroup', {
				name: 'updateTestGroup?',
			});

			const groupObj = await Groups.get('updateTestGroup?', {});
			assert.strictEqual('updateTestGroup?', groupObj.name);
			assert.strictEqual('updatetestgroup', groupObj.slug);

			const navItems = await navigation.get();
			assert.strictEqual(navItems[0].route, '&#x2F;categories');
		});

		it('should fail if system groups is being renamed', (done) => {
			Groups.update('administrators', {
				name: 'administrators_fail',
			}, (err) => {
				assert.equal(err.message, '[[error:not-allowed-to-rename-system-group]]');
				done();
			});
		});

		it('should fail to rename if group name is invalid', async () => {
			try {
				await apiGroups.update({ uid: adminUid }, { slug: ['updateTestGroup?'], values: {} });
			} catch (err) {
				return assert.strictEqual(err.message, '[[error:invalid-group-name]]');
			}
			assert(false);
		});

		it('should fail to rename if group name is too short', async () => {
			try {
				const slug = await Groups.getGroupField('updateTestGroup?', 'slug');
				await apiGroups.update({ uid: adminUid }, { slug: slug, name: '' });
			} catch (err) {
				return assert.strictEqual(err.message, '[[error:group-name-too-short]]');
			}
			assert(false);
		});

		it('should fail to rename if group name is invalid', async () => {
			try {
				const slug = await Groups.getGroupField('updateTestGroup?', 'slug');
				await apiGroups.update({ uid: adminUid }, { slug: slug, name: ['invalid'] });
			} catch (err) {
				return assert.strictEqual(err.message, '[[error:invalid-group-name]]');
			}
			assert(false);
		});

		it('should fail to rename if group name is invalid', async () => {
			try {
				const slug = await Groups.getGroupField('updateTestGroup?', 'slug');
				await apiGroups.update({ uid: adminUid }, { slug: slug, name: 'cid:0:privileges:ban' });
			} catch (err) {
				return assert.strictEqual(err.message, '[[error:invalid-group-name]]');
			}
			assert(false);
		});

		it('should fail to rename if group name is too long', async () => {
			try {
				const slug = await Groups.getGroupField('updateTestGroup?', 'slug');
				await apiGroups.update({ uid: adminUid }, { slug: slug, name: 'verylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstringverylongstring' });
			} catch (err) {
				return assert.strictEqual(err.message, '[[error:group-name-too-long]]');
			}
			assert(false);
		});

		it('should fail to rename if group name is invalid', async () => {
			const slug = await Groups.getGroupField('updateTestGroup?', 'slug');
			const invalidNames = ['test:test', 'another/test', '---'];
			for (const name of invalidNames) {
				try {
					// eslint-disable-next-line no-await-in-loop
					await apiGroups.update({ uid: adminUid }, { slug: slug, name: name });
					assert(false);
				} catch (err) {
					assert.strictEqual(err.message, '[[error:invalid-group-name]]');
				}
			}
		});

		it('should fail to rename group to an existing group', (done) => {
			Groups.create({
				name: 'group2',
				system: 0,
				hidden: 0,
			}, (err) => {
				assert.ifError(err);
				Groups.update('group2', {
					name: 'updateTestGroup?',
				}, (err) => {
					assert.equal(err.message, '[[error:group-already-exists]]');
					done();
				});
			});
		});
	});

	describe('.destroy()', () => {
		before((done) => {
			Groups.join('foobar?', 1, done);
		});

		it('should destroy a group', (done) => {
			Groups.destroy('foobar?', (err) => {
				assert.ifError(err);

				Groups.get('foobar?', {}, (err, groupObj) => {
					assert.ifError(err);
					assert.strictEqual(groupObj, null);
					done();
				});
			});
		});

		it('should also remove the members set', (done) => {
			db.exists('group:foo:members', (err, exists) => {
				assert.ifError(err);
				assert.strictEqual(false, exists);
				done();
			});
		});

		it('should remove group from privilege groups', (done) => {
			const privileges = require('../src/privileges');
			const cid = 1;
			const groupName = '1';
			const uid = 1;
			async.waterfall([
				function (next) {
					Groups.create({ name: groupName }, next);
				},
				function (groupData, next) {
					privileges.categories.give(['groups:topics:create'], cid, groupName, next);
				},
				function (next) {
					Groups.isMember(groupName, 'cid:1:privileges:groups:topics:create', next);
				},
				function (isMember, next) {
					assert(isMember);
					Groups.destroy(groupName, next);
				},
				function (next) {
					Groups.isMember(groupName, 'cid:1:privileges:groups:topics:create', next);
				},
				function (isMember, next) {
					assert(!isMember);
					Groups.isMember(uid, 'registered-users', next);
				},
				function (isMember, next) {
					assert(isMember);
					next();
				},
			], done);
		});
	});

	describe('.join()', () => {
		before((done) => {
			Groups.leave('Test', testUid, done);
		});

		it('should add a user to a group', (done) => {
			Groups.join('Test', testUid, (err) => {
				assert.ifError(err);

				Groups.isMember(testUid, 'Test', (err, isMember) => {
					assert.ifError(err);
					assert.strictEqual(true, isMember);

					done();
				});
			});
		});

		it('should fail to add user to admin group', async () => {
			const oldValue = meta.config.allowPrivateGroups;
			try {
				meta.config.allowPrivateGroups = false;
				const newUid = await User.create({ username: 'newadmin' });
				await apiGroups.join({ uid: newUid }, { slug: ['test', 'administrators'], uid: newUid }, 1);
				const isMember = await Groups.isMember(newUid, 'administrators');
				assert(!isMember);
			} catch (err) {
				assert.strictEqual(err.message, '[[error:no-group]]');
			}
			meta.config.allowPrivateGroups = oldValue;
		});

		it('should fail to add user to group if group name is invalid', (done) => {
			Groups.join(0, 1, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				Groups.join(null, 1, (err) => {
					assert.equal(err.message, '[[error:invalid-data]]');
					Groups.join(undefined, 1, (err) => {
						assert.equal(err.message, '[[error:invalid-data]]');
						done();
					});
				});
			});
		});

		it('should fail to add user to group if uid is invalid', (done) => {
			Groups.join('Test', 0, (err) => {
				assert.equal(err.message, '[[error:invalid-uid]]');
				Groups.join('Test', null, (err) => {
					assert.equal(err.message, '[[error:invalid-uid]]');
					Groups.join('Test', undefined, (err) => {
						assert.equal(err.message, '[[error:invalid-uid]]');
						done();
					});
				});
			});
		});

		it('should add user to Global Moderators group', async () => {
			const uid = await User.create({ username: 'glomod' });
			const slug = await Groups.getGroupField('Global Moderators', 'slug');
			await apiGroups.join({ uid: adminUid }, { slug: slug, uid: uid });
			const isGlobalMod = await User.isGlobalModerator(uid);
			assert.strictEqual(isGlobalMod, true);
		});

		it('should add user to multiple groups', (done) => {
			const groupNames = ['test-hidden1', 'Test', 'test-hidden2', 'empty group'];
			Groups.create({ name: 'empty group' }, (err) => {
				assert.ifError(err);
				Groups.join(groupNames, testUid, (err) => {
					assert.ifError(err);
					Groups.isMemberOfGroups(testUid, groupNames, (err, isMembers) => {
						assert.ifError(err);
						assert(isMembers.every(Boolean));
						db.sortedSetScores('groups:visible:memberCount', groupNames, (err, memberCounts) => {
							assert.ifError(err);
							// hidden groups are not in "groups:visible:memberCount" so they are null
							assert.deepEqual(memberCounts, [null, 3, null, 1]);
							done();
						});
					});
				});
			});
		});

		it('should set group title when user joins the group', (done) => {
			const groupName = 'this will be title';
			User.create({ username: 'needstitle' }, (err, uid) => {
				assert.ifError(err);
				Groups.create({ name: groupName }, (err) => {
					assert.ifError(err);
					Groups.join([groupName], uid, (err) => {
						assert.ifError(err);
						User.getUserData(uid, (err, data) => {
							assert.ifError(err);
							assert.equal(data.groupTitle, `["${groupName}"]`);
							assert.deepEqual(data.groupTitleArray, [groupName]);
							done();
						});
					});
				});
			});
		});

		it('should fail to add user to system group', async () => {
			const uid = await User.create({ username: 'eviluser' });
			const oldValue = meta.config.allowPrivateGroups;
			meta.config.allowPrivateGroups = 0;
			async function test(groupName) {
				let err;
				try {
					const slug = await Groups.getGroupField(groupName, 'slug');
					await apiGroups.join({ uid: uid }, { slug: slug, uid: uid });
					const isMember = await Groups.isMember(uid, groupName);
					assert.strictEqual(isMember, false);
				} catch (_err) {
					err = _err;
				}
				assert.strictEqual(err.message, '[[error:not-allowed]]');
			}
			const groups = ['Global Moderators', 'verified-users', 'unverified-users'];
			for (const g of groups) {
				// eslint-disable-next-line no-await-in-loop
				await test(g);
			}
			meta.config.allowPrivateGroups = oldValue;
		});

		it('should allow admins to join private groups', async () => {
			await apiGroups.join({ uid: adminUid }, { uid: adminUid, slug: 'global-moderators' });
			assert(await Groups.isMember(adminUid, 'Global Moderators'));
		});
	});

	describe('.leave()', () => {
		it('should remove a user from a group', (done) => {
			Groups.leave('Test', testUid, (err) => {
				assert.ifError(err);

				Groups.isMember(testUid, 'Test', (err, isMember) => {
					assert.ifError(err);
					assert.strictEqual(false, isMember);

					done();
				});
			});
		});
	});

	describe('.leaveAllGroups()', () => {
		it('should remove a user from all groups', (done) => {
			Groups.leaveAllGroups(testUid, (err) => {
				assert.ifError(err);

				const groups = ['Test', 'Hidden'];
				async.every(groups, (group, next) => {
					Groups.isMember(testUid, group, (err, isMember) => {
						next(err, !isMember);
					});
				}, (err, result) => {
					assert.ifError(err);
					assert(result);

					done();
				});
			});
		});
	});

	describe('.show()', () => {
		it('should make a group visible', (done) => {
			Groups.show('Test', function (err) {
				assert.ifError(err);
				assert.equal(arguments.length, 1);
				db.isSortedSetMember('groups:visible:createtime', 'Test', (err, isMember) => {
					assert.ifError(err);
					assert.strictEqual(isMember, true);
					done();
				});
			});
		});
	});

	describe('.hide()', () => {
		it('should make a group hidden', (done) => {
			Groups.hide('Test', function (err) {
				assert.ifError(err);
				assert.equal(arguments.length, 1);
				db.isSortedSetMember('groups:visible:createtime', 'Test', (err, isMember) => {
					assert.ifError(err);
					assert.strictEqual(isMember, false);
					done();
				});
			});
		});
	});

	describe('socket methods', () => {
		it('should error if data is null', (done) => {
			socketGroups.before({ uid: 0 }, 'groups.join', null, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should not error if data is valid', (done) => {
			socketGroups.before({ uid: 0 }, 'groups.join', {}, (err) => {
				assert.ifError(err);
				done();
			});
		});

		it('should return error if not logged in', async () => {
			try {
				await apiGroups.join({ uid: 0 }, {});
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-uid]]');
			}
		});

		it('should return error if group name is special', async () => {
			try {
				await apiGroups.join({ uid: testUid }, { slug: 'administrators', uid: testUid });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:not-allowed]]');
			}
		});

		it('should error if group does not exist', async () => {
			try {
				await apiGroups.join({ uid: adminUid }, { slug: 'doesnotexist', uid: adminUid });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:no-group]]');
			}
		});

		it('should join test group', async () => {
			meta.config.allowPrivateGroups = 0;
			await apiGroups.join({ uid: adminUid }, { slug: 'test', uid: adminUid });
			const isMember = await Groups.isMember(adminUid, 'Test');
			assert(isMember);
		});

		it('should error if not logged in', async () => {
			try {
				await apiGroups.leave({ uid: 0 }, {});
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-uid]]');
			}
		});

		it('should return error if group name is special', async () => {
			try {
				await apiGroups.leave({ uid: adminUid }, { slug: 'administrators', uid: adminUid });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:cant-remove-self-as-admin]]');
			}
		});

		it('should leave test group', async () => {
			await apiGroups.leave({ uid: adminUid }, { slug: 'test', uid: adminUid });
			const isMember = await Groups.isMember(adminUid, 'Test');
			assert(!isMember);
		});

		it('should fail to join if group is private and join requests are disabled', async () => {
			meta.config.allowPrivateGroups = 1;
			try {
				await apiGroups.join({ uid: testUid }, { slug: 'privatenojoin', uid: testUid });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:group-join-disabled]]');
			}
		});

		it('should fail to leave if group is private and leave is disabled', async () => {
			await Groups.join('PrivateNoLeave', testUid);
			const isMember = await Groups.isMember(testUid, 'PrivateNoLeave');
			assert(isMember);
			try {
				await apiGroups.leave({ uid: testUid }, { slug: 'privatenoleave', uid: testUid });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:group-leave-disabled]]');
			}
		});

		it('should join if user is admin', async () => {
			await apiGroups.join({ uid: adminUid }, { slug: 'privatecanjoin', uid: adminUid });
			const isMember = await Groups.isMember(adminUid, 'PrivateCanJoin');
			assert(isMember);
		});

		it('should request membership for regular user', async () => {
			await apiGroups.join({ uid: testUid }, { slug: 'privatecanjoin', uid: testUid });
			const isPending = await Groups.isPending(testUid, 'PrivateCanJoin');
			assert(isPending);
		});

		it('should reject membership of user', (done) => {
			socketGroups.reject({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: testUid }, (err) => {
				assert.ifError(err);
				Groups.isInvited(testUid, 'PrivateCanJoin', (err, invited) => {
					assert.ifError(err);
					assert.equal(invited, false);
					done();
				});
			});
		});

		it('should error if not owner or admin', (done) => {
			socketGroups.accept({ uid: 0 }, { groupName: 'PrivateCanJoin', toUid: testUid }, (err) => {
				assert.equal(err.message, '[[error:no-privileges]]');
				done();
			});
		});

		it('should accept membership of user', async () => {
			await apiGroups.join({ uid: testUid }, { slug: 'privatecanjoin', uid: testUid });
			await socketGroups.accept({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: testUid });
			const isMember = await Groups.isMember(testUid, 'PrivateCanJoin');
			assert(isMember);
		});

		it('should reject/accept all memberships requests', async () => {
			async function requestMembership(uid1, uid2) {
				await apiGroups.join({ uid: uid1 }, { slug: 'privatecanjoin', uid: uid1 });
				await apiGroups.join({ uid: uid2 }, { slug: 'privatecanjoin', uid: uid2 });
			}
			const [uid1, uid2] = await Promise.all([
				User.create({ username: 'groupuser1' }),
				User.create({ username: 'groupuser2' }),
			]);
			await requestMembership(uid1, uid2);
			await socketGroups.rejectAll({ uid: adminUid }, { groupName: 'PrivateCanJoin' });
			const pending = await Groups.getPending('PrivateCanJoin');
			assert.equal(pending.length, 0);
			await requestMembership(uid1, uid2);
			await socketGroups.acceptAll({ uid: adminUid }, { groupName: 'PrivateCanJoin' });
			const isMembers = await Groups.isMembers([uid1, uid2], 'PrivateCanJoin');
			assert.deepStrictEqual(isMembers, [true, true]);
		});

		it('should issue invite to user', (done) => {
			User.create({ username: 'invite1' }, (err, uid) => {
				assert.ifError(err);
				socketGroups.issueInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: uid }, (err) => {
					assert.ifError(err);
					Groups.isInvited(uid, 'PrivateCanJoin', (err, isInvited) => {
						assert.ifError(err);
						assert(isInvited);
						done();
					});
				});
			});
		});

		it('should fail with invalid data', (done) => {
			socketGroups.issueMassInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', usernames: null }, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should issue mass invite to users', (done) => {
			User.create({ username: 'invite2' }, (err, uid) => {
				assert.ifError(err);
				socketGroups.issueMassInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', usernames: 'invite1, invite2' }, (err) => {
					assert.ifError(err);
					Groups.isInvited([adminUid, uid], 'PrivateCanJoin', (err, isInvited) => {
						assert.ifError(err);
						assert.deepStrictEqual(isInvited, [false, true]);
						done();
					});
				});
			});
		});

		it('should rescind invite', (done) => {
			User.create({ username: 'invite3' }, (err, uid) => {
				assert.ifError(err);
				socketGroups.issueInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: uid }, (err) => {
					assert.ifError(err);
					socketGroups.rescindInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: uid }, (err) => {
						assert.ifError(err);
						Groups.isInvited(uid, 'PrivateCanJoin', (err, isInvited) => {
							assert.ifError(err);
							assert(!isInvited);
							done();
						});
					});
				});
			});
		});

		it('should error if user is not invited', (done) => {
			socketGroups.acceptInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin' }, (err) => {
				assert.equal(err.message, '[[error:not-invited]]');
				done();
			});
		});

		it('should accept invite', (done) => {
			User.create({ username: 'invite4' }, (err, uid) => {
				assert.ifError(err);
				socketGroups.issueInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: uid }, (err) => {
					assert.ifError(err);
					socketGroups.acceptInvite({ uid: uid }, { groupName: 'PrivateCanJoin' }, (err) => {
						assert.ifError(err);
						Groups.isMember(uid, 'PrivateCanJoin', (err, isMember) => {
							assert.ifError(err);
							assert(isMember);
							done();
						});
					});
				});
			});
		});

		it('should reject invite', (done) => {
			User.create({ username: 'invite5' }, (err, uid) => {
				assert.ifError(err);
				socketGroups.issueInvite({ uid: adminUid }, { groupName: 'PrivateCanJoin', toUid: uid }, (err) => {
					assert.ifError(err);
					socketGroups.rejectInvite({ uid: uid }, { groupName: 'PrivateCanJoin' }, (err) => {
						assert.ifError(err);
						Groups.isInvited(uid, 'PrivateCanJoin', (err, isInvited) => {
							assert.ifError(err);
							assert(!isInvited);
							done();
						});
					});
				});
			});
		});

		it('should grant ownership to user', async () => {
			await apiGroups.grant({ uid: adminUid }, { slug: 'privatecanjoin', uid: testUid });
			const isOwner = await Groups.ownership.isOwner(testUid, 'PrivateCanJoin');
			assert(isOwner);
		});

		it('should rescind ownership from user', async () => {
			await apiGroups.rescind({ uid: adminUid }, { slug: 'privatecanjoin', uid: testUid });
			const isOwner = await Groups.ownership.isOwner(testUid, 'PrivateCanJoin');
			assert(!isOwner);
		});

		it('should fail to kick user with invalid data', (done) => {
			socketGroups.kick({ uid: adminUid }, { groupName: 'PrivateCanJoin', uid: adminUid }, (err) => {
				assert.equal(err.message, '[[error:cant-kick-self]]');
				done();
			});
		});

		it('should kick user from group', (done) => {
			socketGroups.kick({ uid: adminUid }, { groupName: 'PrivateCanJoin', uid: testUid }, (err) => {
				assert.ifError(err);
				Groups.isMember(testUid, 'PrivateCanJoin', (err, isMember) => {
					assert.ifError(err);
					assert(!isMember);
					done();
				});
			});
		});

		it('should fail to create group with invalid data', async () => {
			try {
				await apiGroups.create({ uid: 0 }, {});
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:no-privileges]]');
			}
		});

		it('should fail to create group if group creation is disabled', async () => {
			try {
				await apiGroups.create({ uid: testUid }, { name: 'avalidname' });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:no-privileges]]');
			}
		});

		it('should fail to create group if name is privilege group', async () => {
			try {
				await apiGroups.create({ uid: 1 }, { name: 'cid:1:privileges:groups:find' });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-group-name]]');
			}
		});

		it('should create/update group', async () => {
			const groupData = await apiGroups.create({ uid: adminUid }, { name: 'createupdategroup' });
			assert(groupData);
			const data = {
				slug: 'createupdategroup',
				name: 'renamedupdategroup',
				description: 'cat group',
				userTitle: 'cats',
				userTitleEnabled: 1,
				disableJoinRequests: 1,
				hidden: 1,
				private: 0,
			};
			await apiGroups.update({ uid: adminUid }, data);
			const updatedData = await Groups.get('renamedupdategroup', {});
			assert.equal(updatedData.name, 'renamedupdategroup');
			assert.equal(updatedData.userTitle, 'cats');
			assert.equal(updatedData.description, 'cat group');
			assert.equal(updatedData.hidden, true);
			assert.equal(updatedData.disableJoinRequests, true);
			assert.equal(updatedData.private, false);
		});

		it('should fail to create a group with name guests', async () => {
			try {
				await apiGroups.create({ uid: adminUid }, { name: 'guests' });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-group-name]]');
			}
		});

		it('should fail to rename guests group', async () => {
			const data = {
				slug: 'guests',
				name: 'guests2',
			};

			try {
				await apiGroups.update({ uid: adminUid }, data);
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-group-name]]');
			}
		});

		it('should delete group', async () => {
			await apiGroups.delete({ uid: adminUid }, { slug: 'renamedupdategroup' });
			const exists = await Groups.exists('renamedupdategroup');
			assert(!exists);
		});

		it('should fail to delete group if name is special', async () => {
			const specialGroups = [
				'administrators', 'registered-users', 'verified-users',
				'unverified-users', 'global-moderators',
			];
			for (const slug of specialGroups) {
				try {
					// eslint-disable-next-line no-await-in-loop
					await apiGroups.delete({ uid: adminUid }, { slug: slug });
					assert(false);
				} catch (err) {
					assert.equal(err.message, '[[error:not-allowed]]');
				}
			}
		});

		it('should fail to delete group if name is special', async () => {
			try {
				await apiGroups.delete({ uid: adminUid }, { slug: 'guests' });
				assert(false);
			} catch (err) {
				assert.equal(err.message, '[[error:invalid-group-name]]');
			}
		});

		it('should fail to load more groups with invalid data', (done) => {
			socketGroups.loadMore({ uid: adminUid }, {}, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should load more groups', (done) => {
			socketGroups.loadMore({ uid: adminUid }, { after: 0, sort: 'count' }, (err, data) => {
				assert.ifError(err);
				assert(Array.isArray(data.groups));
				done();
			});
		});

		it('should fail to load more members with invalid data', (done) => {
			socketGroups.loadMoreMembers({ uid: adminUid }, {}, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should load more members', (done) => {
			socketGroups.loadMoreMembers({ uid: adminUid }, { after: 0, groupName: 'PrivateCanJoin' }, (err, data) => {
				assert.ifError(err);
				assert(Array.isArray(data.users));
				done();
			});
		});
	});

	describe('api methods', () => {
		const apiGroups = require('../src/api/groups');
		it('should fail to create group with invalid data', async () => {
			let err;
			try {
				await apiGroups.create({ uid: adminUid }, null);
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:invalid-data]]');
		});

		it('should fail to create group if group name is privilege group', async () => {
			let err;
			try {
				await apiGroups.create({ uid: adminUid }, { name: 'cid:1:privileges:read' });
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:invalid-group-name]]');
		});

		it('should create a group', async () => {
			const groupData = await apiGroups.create({ uid: adminUid }, { name: 'newgroup', description: 'group created by admin' });
			assert.equal(groupData.name, 'newgroup');
			assert.equal(groupData.description, 'group created by admin');
			assert.equal(groupData.private, 1);
			assert.equal(groupData.hidden, 0);
			assert.equal(groupData.memberCount, 1);
		});

		it('should fail to join with invalid data', async () => {
			let err;
			try {
				await apiGroups.join({ uid: adminUid }, null);
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:invalid-data]]');
		});

		it('should add user to group', async () => {
			await apiGroups.join({ uid: adminUid }, { uid: testUid, slug: 'newgroup' });
			const isMember = await Groups.isMember(testUid, 'newgroup');
			assert(isMember);
		});

		it('should not error if user is already member', async () => {
			await apiGroups.join({ uid: adminUid }, { uid: testUid, slug: 'newgroup' });
		});

		it('it should fail with invalid data', async () => {
			let err;
			try {
				await apiGroups.leave({ uid: adminUid }, null);
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:invalid-data]]');
		});

		it('it should fail if admin tries to remove self', async () => {
			let err;
			try {
				await apiGroups.leave({ uid: adminUid }, { uid: adminUid, slug: 'administrators' });
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:cant-remove-self-as-admin]]');
		});

		it('should not error if user is not member', async () => {
			await apiGroups.leave({ uid: adminUid }, { uid: 3, slug: 'newgroup' });
		});

		it('should fail if trying to remove someone else from group', async () => {
			let err;
			try {
				await apiGroups.leave({ uid: testUid }, { uid: adminUid, slug: 'newgroup' });
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:no-privileges]]');
		});

		it('should remove user from group', async () => {
			await apiGroups.leave({ uid: adminUid }, { uid: testUid, slug: 'newgroup' });
			const isMember = await Groups.isMember(testUid, 'newgroup');
			assert(!isMember);
		});

		it('should fail with invalid data', async () => {
			let err;
			try {
				await apiGroups.update({ uid: adminUid }, null);
			} catch (_err) {
				err = _err;
			}
			assert.strictEqual(err.message, '[[error:invalid-data]]');
		});

		it('should update group', async () => {
			const data = {
				slug: 'newgroup',
				name: 'renamedgroup',
				description: 'cat group',
				userTitle: 'cats',
				userTitleEnabled: 1,
				disableJoinRequests: 1,
				hidden: 1,
				private: 0,
			};
			await apiGroups.update({ uid: adminUid }, data);
			const groupData = await Groups.get('renamedgroup', {});
			assert.equal(groupData.name, 'renamedgroup');
			assert.equal(groupData.userTitle, 'cats');
			assert.equal(groupData.description, 'cat group');
			assert.equal(groupData.hidden, true);
			assert.equal(groupData.disableJoinRequests, true);
			assert.equal(groupData.private, false);
		});
	});

	describe('groups cover', () => {
		const socketGroups = require('../src/socket.io/groups');
		let regularUid;
		const logoPath = path.join(__dirname, '../test/files/test.png');
		const imagePath = path.join(__dirname, '../test/files/groupcover.png');
		before((done) => {
			User.create({ username: 'regularuser', password: '123456' }, (err, uid) => {
				assert.ifError(err);
				regularUid = uid;
				async.series([
					function (next) {
						Groups.join('Test', adminUid, next);
					},
					function (next) {
						Groups.join('Test', regularUid, next);
					},
					function (next) {
						helpers.copyFile(logoPath, imagePath, next);
					},
				], done);
			});
		});

		it('should fail if user is not logged in or not owner', (done) => {
			socketGroups.cover.update({ uid: 0 }, { imageData: 'asd' }, (err) => {
				assert.equal(err.message, '[[error:no-privileges]]');
				socketGroups.cover.update({ uid: regularUid }, { groupName: 'Test', imageData: 'asd' }, (err) => {
					assert.equal(err.message, '[[error:no-privileges]]');
					done();
				});
			});
		});

		it('should upload group cover image from file', (done) => {
			const data = {
				groupName: 'Test',
				file: {
					path: imagePath,
					type: 'image/png',
				},
			};
			Groups.updateCover({ uid: adminUid }, data, (err, data) => {
				assert.ifError(err);
				Groups.getGroupFields('Test', ['cover:url'], (err, groupData) => {
					assert.ifError(err);
					assert.equal(nconf.get('relative_path') + data.url, groupData['cover:url']);
					if (nconf.get('relative_path')) {
						assert(!data.url.startsWith(nconf.get('relative_path')));
						assert(groupData['cover:url'].startsWith(nconf.get('relative_path')), groupData['cover:url']);
					}
					done();
				});
			});
		});


		it('should upload group cover image from data', (done) => {
			const data = {
				groupName: 'Test',
				imageData: 'data:image/png;base64,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',
			};
			socketGroups.cover.update({ uid: adminUid }, data, (err, data) => {
				assert.ifError(err);
				Groups.getGroupFields('Test', ['cover:url'], (err, groupData) => {
					assert.ifError(err);
					assert.equal(nconf.get('relative_path') + data.url, groupData['cover:url']);
					done();
				});
			});
		});

		it('should fail to upload group cover with invalid image', (done) => {
			const data = {
				groupName: 'Test',
				file: {
					path: imagePath,
					type: 'image/png',
				},
			};
			socketGroups.cover.update({ uid: adminUid }, data, (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should fail to upload group cover with invalid image', (done) => {
			const data = {
				groupName: 'Test',
				imageData: 'data:image/svg;base64,iVBORw0KGgoAAAANSUhEUgAAABwA',
			};
			socketGroups.cover.update({ uid: adminUid }, data, (err, data) => {
				assert.equal(err.message, '[[error:invalid-image]]');
				done();
			});
		});

		it('should update group cover position', (done) => {
			const data = {
				groupName: 'Test',
				position: '50% 50%',
			};
			socketGroups.cover.update({ uid: adminUid }, data, (err) => {
				assert.ifError(err);
				Groups.getGroupFields('Test', ['cover:position'], (err, groupData) => {
					assert.ifError(err);
					assert.equal('50% 50%', groupData['cover:position']);
					done();
				});
			});
		});

		it('should fail to update cover position if group name is missing', (done) => {
			Groups.updateCoverPosition('', '50% 50%', (err) => {
				assert.equal(err.message, '[[error:invalid-data]]');
				done();
			});
		});

		it('should fail to remove cover if not logged in', (done) => {
			socketGroups.cover.remove({ uid: 0 }, { groupName: 'Test' }, (err) => {
				assert.equal(err.message, '[[error:no-privileges]]');
				done();
			});
		});

		it('should fail to remove cover if not owner', (done) => {
			socketGroups.cover.remove({ uid: regularUid }, { groupName: 'Test' }, (err) => {
				assert.equal(err.message, '[[error:no-privileges]]');
				done();
			});
		});

		it('should remove cover', async () => {
			const fields = ['cover:url', 'cover:thumb:url'];
			const values = await Groups.getGroupFields('Test', fields);
			await socketGroups.cover.remove({ uid: adminUid }, { groupName: 'Test' });

			fields.forEach((field) => {
				const filename = values[field].split('/').pop();
				const filePath = path.join(nconf.get('upload_path'), 'files', filename);
				assert.strictEqual(fs.existsSync(filePath), false);
			});

			const groupData = await db.getObjectFields('group:Test', ['cover:url']);
			assert(!groupData['cover:url']);
		});
	});
});
