'use strict';

const assert = require('assert');

const db = require('./mocks/databasemock');
const upgrade = require('../src/upgrade');

describe('Upgrade', () => {
	it('should get all upgrade scripts', async () => {
		const files = await upgrade.getAll();
		assert(Array.isArray(files) && files.length > 0);
	});

	it('should throw error', async () => {
		let err;
		try {
			await upgrade.check();
		} catch (_err) {
			err = _err;
		}
		assert.equal(err.message, 'schema-out-of-date');
	});

	it('should run all upgrades', async () => {
		// for upgrade scripts to run
		await db.set('schemaDate', 1);
		await upgrade.run();
	});

	it('should run particular upgrades', async () => {
		const files = await upgrade.getAll();
		await db.set('schemaDate', 1);
		await upgrade.runParticular(files.slice(0, 2));
	});
});
