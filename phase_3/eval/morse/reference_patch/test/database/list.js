'use strict';


const async = require('async');
const assert = require('assert');
const db = require('../mocks/databasemock');

describe('List methods', () => {
	describe('listAppend()', () => {
		it('should append to a list', (done) => {
			db.listAppend('testList1', 5, function (err) {
				assert.ifError(err);
				assert.equal(arguments.length, 1);
				done();
			});
		});

		it('should not add anyhing if key is falsy', (done) => {
			db.listAppend(null, 3, (err) => {
				assert.ifError(err);
				done();
			});
		});

		it('should append each element to list', async () => {
			await db.listAppend('arrayListAppend', ['a', 'b', 'c']);
			let values = await db.getListRange('arrayListAppend', 0, -1);
			assert.deepStrictEqual(values, ['a', 'b', 'c']);

			await db.listAppend('arrayListAppend', ['d', 'e']);
			values = await db.getListRange('arrayListAppend', 0, -1);
			assert.deepStrictEqual(values, ['a', 'b', 'c', 'd', 'e']);
		});
	});

	describe('listPrepend()', () => {
		it('should prepend to a list', (done) => {
			db.listPrepend('testList2', 3, function (err) {
				assert.equal(err, null);
				assert.equal(arguments.length, 1);
				done();
			});
		});

		it('should prepend 2 more elements to a list', (done) => {
			async.series([
				function (next) {
					db.listPrepend('testList2', 2, next);
				},
				function (next) {
					db.listPrepend('testList2', 1, next);
				},
			], (err) => {
				assert.equal(err, null);
				done();
			});
		});

		it('should not add anyhing if key is falsy', (done) => {
			db.listPrepend(null, 3, (err) => {
				assert.ifError(err);
				done();
			});
		});

		it('should prepend each element to list', async () => {
			await db.listPrepend('arrayListPrepend', ['a', 'b', 'c']);
			let values = await db.getListRange('arrayListPrepend', 0, -1);
			assert.deepStrictEqual(values, ['c', 'b', 'a']);

			await db.listPrepend('arrayListPrepend', ['d', 'e']);
			values = await db.getListRange('arrayListPrepend', 0, -1);
			assert.deepStrictEqual(values, ['e', 'd', 'c', 'b', 'a']);
		});
	});

	describe('getListRange()', () => {
		before((done) => {
			async.series([
				function (next) {
					db.listAppend('testList3', 7, next);
				},
				function (next) {
					db.listPrepend('testList3', 3, next);
				},
				function (next) {
					db.listAppend('testList4', 5, next);
				},
			], done);
		});

		it('should return an empty list', (done) => {
			db.getListRange('doesnotexist', 0, -1, function (err, list) {
				assert.equal(err, null);
				assert.equal(arguments.length, 2);
				assert.equal(Array.isArray(list), true);
				assert.equal(list.length, 0);
				done();
			});
		});

		it('should return a list with one element', (done) => {
			db.getListRange('testList4', 0, 0, (err, list) => {
				assert.equal(err, null);
				assert.equal(Array.isArray(list), true);
				assert.equal(list[0], 5);
				done();
			});
		});

		it('should return a list with 2 elements 3, 7', (done) => {
			db.getListRange('testList3', 0, -1, (err, list) => {
				assert.equal(err, null);
				assert.equal(Array.isArray(list), true);
				assert.equal(list.length, 2);
				assert.deepEqual(list, ['3', '7']);
				done();
			});
		});

		it('should not get anything if key is falsy', (done) => {
			db.getListRange(null, 0, -1, (err, data) => {
				assert.ifError(err);
				assert.equal(data, undefined);
				done();
			});
		});
	});

	describe('listRemoveLast()', () => {
		before((done) => {
			async.series([
				function (next) {
					db.listAppend('testList7', 12, next);
				},
				function (next) {
					db.listPrepend('testList7', 9, next);
				},
			], done);
		});

		it('should remove the last element of list and return it', (done) => {
			db.listRemoveLast('testList7', function (err, lastElement) {
				assert.equal(err, null);
				assert.equal(arguments.length, 2);
				assert.equal(lastElement, '12');
				done();
			});
		});

		it('should not remove anyhing if key is falsy', (done) => {
			db.listRemoveLast(null, (err) => {
				assert.ifError(err);
				done();
			});
		});
	});

	describe('listRemoveAll()', () => {
		before((done) => {
			async.series([
				async.apply(db.listAppend, 'testList5', 1),
				async.apply(db.listAppend, 'testList5', 1),
				async.apply(db.listAppend, 'testList5', 1),
				async.apply(db.listAppend, 'testList5', 2),
				async.apply(db.listAppend, 'testList5', 5),
			], done);
		});

		it('should remove all the matching elements of list', (done) => {
			db.listRemoveAll('testList5', '1', function (err) {
				assert.equal(err, null);
				assert.equal(arguments.length, 1);

				db.getListRange('testList5', 0, -1, (err, list) => {
					assert.equal(err, null);
					assert.equal(Array.isArray(list), true);
					assert.equal(list.length, 2);
					assert.equal(list.indexOf('1'), -1);
					done();
				});
			});
		});

		it('should not remove anyhing if key is falsy', (done) => {
			db.listRemoveAll(null, 3, (err) => {
				assert.ifError(err);
				done();
			});
		});

		it('should remove multiple elements from list', async () => {
			await db.listAppend('multiRemoveList', ['a', 'b', 'c', 'd', 'e']);
			const initial = await db.getListRange('multiRemoveList', 0, -1);
			assert.deepStrictEqual(initial, ['a', 'b', 'c', 'd', 'e']);
			await db.listRemoveAll('multiRemoveList', ['b', 'd']);
			const values = await db.getListRange('multiRemoveList', 0, -1);
			assert.deepStrictEqual(values, ['a', 'c', 'e']);
		});
	});

	describe('listTrim()', () => {
		it('should trim list to a certain range', (done) => {
			const list = ['1', '2', '3', '4', '5'];
			async.eachSeries(list, (value, next) => {
				db.listAppend('testList6', value, next);
			}, (err) => {
				if (err) {
					return done(err);
				}

				db.listTrim('testList6', 0, 2, function (err) {
					assert.equal(err, null);
					assert.equal(arguments.length, 1);
					db.getListRange('testList6', 0, -1, (err, list) => {
						assert.equal(err, null);
						assert.equal(list.length, 3);
						assert.deepEqual(list, ['1', '2', '3']);
						done();
					});
				});
			});
		});

		it('should not add anyhing if key is falsy', (done) => {
			db.listTrim(null, 0, 3, (err) => {
				assert.ifError(err);
				done();
			});
		});
	});

	describe('listLength', () => {
		it('should get the length of a list', (done) => {
			db.listAppend('getLengthList', 1, (err) => {
				assert.ifError(err);
				db.listAppend('getLengthList', 2, (err) => {
					assert.ifError(err);
					db.listLength('getLengthList', (err, length) => {
						assert.ifError(err);
						assert.equal(length, 2);
						done();
					});
				});
			});
		});

		it('should return 0 if list does not have any elements', (done) => {
			db.listLength('doesnotexist', (err, length) => {
				assert.ifError(err);
				assert.strictEqual(length, 0);
				done();
			});
		});
	});
});
