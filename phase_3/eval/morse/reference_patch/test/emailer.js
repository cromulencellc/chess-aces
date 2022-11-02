'use strict';

const { SMTPServer } = require('smtp-server');
const assert = require('assert');
const fs = require('fs');
const path = require('path');

const db = require('./mocks/databasemock');
const Plugins = require('../src/plugins');
const Emailer = require('../src/emailer');
const user = require('../src/user');
const meta = require('../src/meta');
const Meta = require('../src/meta');

describe('emailer', () => {
	let onMail = function (address, session, callback) { callback(); };
	let onTo = function (address, session, callback) { callback(); };

	const template = 'test';
	const email = 'test@example.org';
	const language = 'en-GB';
	const params = {
		subject: 'Welcome to NodeBB',
	};

	before((done) => {
		const server = new SMTPServer({
			allowInsecureAuth: true,
			onAuth: function (auth, session, callback) {
				callback(null, {
					user: auth.username,
				});
			},
			onMailFrom: function (address, session, callback) {
				onMail(address, session, callback);
			},
			onRcptTo: function (address, session, callback) {
				onTo(address, session, callback);
			},
		});

		server.on('error', (err) => {
			throw err;
		});
		server.listen(4000, done);
	});

	// TODO: test sendmail here at some point

	it('plugin hook should work', (done) => {
		const error = new Error();
		const method = function (data, next) {
			assert(data);
			assert.equal(data.to, email);
			assert.equal(data.subject, `[NodeBB] ${params.subject}`);

			next(error);
		};

		Plugins.hooks.register('emailer-test', {
			hook: 'filter:email.send',
			method,
		});

		Emailer.sendToEmail(template, email, language, params, (err) => {
			assert.equal(err, error);

			Plugins.hooks.unregister('emailer-test', 'filter:email.send', method);
			done();
		});
	});

	it('should build custom template on config change', (done) => {
		const text = 'a random string of text';

		// make sure it's not already set
		Emailer.renderAndTranslate('test', {}, 'en-GB', (err, output) => {
			assert.ifError(err);

			assert.notEqual(output, text);

			Meta.configs.set('email:custom:test', text, (err) => {
				assert.ifError(err);

				// wait for pubsub stuff
				setTimeout(() => {
					Emailer.renderAndTranslate('test', {}, 'en-GB', (err, output) => {
						assert.ifError(err);

						assert.equal(output, text);
						done();
					});
				}, 2000);
			});
		});
	});

	it('should send via SMTP', (done) => {
		const from = 'admin@example.org';
		const username = 'another@example.com';

		onMail = function (address, session, callback) {
			assert.equal(address.address, from);
			assert.equal(session.user, username);

			callback();
		};

		onTo = function (address, session, callback) {
			assert.equal(address.address, email);

			callback();
			done();
		};

		Meta.configs.setMultiple({
			'email:smtpTransport:enabled': '1',
			'email:smtpTransport:user': username,
			'email:smtpTransport:pass': 'anything',
			'email:smtpTransport:service': 'nodebb-custom-smtp',
			'email:smtpTransport:port': 4000,
			'email:smtpTransport:host': 'localhost',
			'email:smtpTransport:security': 'NONE',
			'email:from': from,
		}, (err) => {
			assert.ifError(err);

			// delay so emailer has a chance to update after config changes
			setTimeout(() => {
				assert.equal(Emailer.fallbackTransport, Emailer.transports.smtp);

				Emailer.sendToEmail(template, email, language, params, (err) => {
					assert.ifError(err);
				});
			}, 200);
		});
	});

	after((done) => {
		fs.unlinkSync(path.join(__dirname, '../build/public/templates/emails/test.js'));
		Meta.configs.setMultiple({
			'email:smtpTransport:enabled': '0',
			'email:custom:test': '',
		}, done);
	});

	describe('emailer.send()', () => {
		let recipientUid;

		before(async () => {
			recipientUid = await user.create({ username: 'recipient', email: 'test@example.org' });
			await user.email.confirmByUid(recipientUid);
		});

		it('should not send email to a banned user', async () => {
			const method = async () => {
				assert(false); // if thrown, email was sent
			};
			Plugins.hooks.register('emailer-test', {
				hook: 'filter:email.send',
				method,
			});

			await user.bans.ban(recipientUid);
			await Emailer.send('test', recipientUid, {});

			Plugins.hooks.unregister('emailer-test', 'filter:email.send', method);
		});

		it('should return true if the template is "banned"', async () => {
			const method = async () => {
				assert(true); // if thrown, email was sent
			};
			Plugins.hooks.register('emailer-test', {
				hook: 'filter:email.send',
				method,
			});

			await Emailer.send('banned', recipientUid, {});
			Plugins.hooks.unregister('emailer-test', 'filter:email.send', method);
		});

		it('should return true if system settings allow sending to banned users', async () => {
			const method = async () => {
				assert(true); // if thrown, email was sent
			};
			Plugins.hooks.register('emailer-test', {
				hook: 'filter:email.send',
				method,
			});

			meta.config.sendEmailToBanned = 1;
			await Emailer.send('test', recipientUid, {});
			meta.config.sendEmailToBanned = 0;
			await user.bans.unban(recipientUid);

			Plugins.hooks.unregister('emailer-test', 'filter:email.send', method);
		});
	});
});
