'use strict';

const winston = require('winston');
const jsesc = require('jsesc');
const nconf = require('nconf');
const semver = require('semver');

const user = require('../user');
const meta = require('../meta');
const plugins = require('../plugins');
const privileges = require('../privileges');
const utils = require('../../public/src/utils');
const versions = require('../admin/versions');
const helpers = require('./helpers');

const controllers = {
	api: require('../controllers/api'),
	helpers: require('../controllers/helpers'),
};

const middleware = module.exports;

middleware.buildHeader = helpers.try(async (req, res, next) => {
	res.locals.renderAdminHeader = true;
	if (req.method === 'GET') {
		await require('./index').applyCSRFasync(req, res);
	}

	res.locals.config = await controllers.api.loadConfig(req);
	next();
});

middleware.renderHeader = async (req, res, data) => {
	const custom_header = {
		plugins: [],
		authentication: [],
	};
	res.locals.config = res.locals.config || {};

	const results = await utils.promiseParallel({
		userData: user.getUserFields(req.uid, ['username', 'userslug', 'email', 'picture', 'email:confirmed']),
		scripts: getAdminScripts(),
		custom_header: plugins.hooks.fire('filter:admin.header.build', custom_header),
		configs: meta.configs.list(),
		latestVersion: getLatestVersion(),
		privileges: privileges.admin.get(req.uid),
		tags: meta.tags.parse(req, {}, [], []),
	});

	const { userData } = results;
	userData.uid = req.uid;
	userData['email:confirmed'] = userData['email:confirmed'] === 1;
	userData.privileges = results.privileges;

	let acpPath = req.path.slice(1).split('/');
	acpPath.forEach((path, i) => {
		acpPath[i] = path.charAt(0).toUpperCase() + path.slice(1);
	});
	acpPath = acpPath.join(' > ');

	const version = nconf.get('version');

	res.locals.config.userLang = res.locals.config.acpLang || res.locals.config.userLang;
	let templateValues = {
		config: res.locals.config,
		configJSON: jsesc(JSON.stringify(res.locals.config), { isScriptContext: true }),
		relative_path: res.locals.config.relative_path,
		adminConfigJSON: encodeURIComponent(JSON.stringify(results.configs)),
		metaTags: results.tags.meta,
		linkTags: results.tags.link,
		user: userData,
		userJSON: jsesc(JSON.stringify(userData), { isScriptContext: true }),
		plugins: results.custom_header.plugins,
		authentication: results.custom_header.authentication,
		scripts: results.scripts,
		'cache-buster': meta.config['cache-buster'] || '',
		env: !!process.env.NODE_ENV,
		title: `${acpPath || 'Dashboard'} | NodeBB Admin Control Panel`,
		bodyClass: data.bodyClass,
		version: version,
		latestVersion: results.latestVersion,
		upgradeAvailable: results.latestVersion && semver.gt(results.latestVersion, version),
		showManageMenu: results.privileges.superadmin || ['categories', 'privileges', 'users', 'admins-mods', 'groups', 'tags', 'settings'].some(priv => results.privileges[`admin:${priv}`]),
	};

	templateValues.template = { name: res.locals.template };
	templateValues.template[res.locals.template] = true;
	({ templateData: templateValues } = await plugins.hooks.fire('filter:middleware.renderAdminHeader', {
		req,
		res,
		templateData: templateValues,
		data,
	}));

	return await req.app.renderAsync('admin/header', templateValues);
};

async function getAdminScripts() {
	const scripts = await plugins.hooks.fire('filter:admin.scripts.get', []);
	return scripts.map(script => ({ src: script }));
}

async function getLatestVersion() {
	try {
		const result = await versions.getLatestVersion();
		return result;
	} catch (err) {
		winston.error(`[acp] Failed to fetch latest version${err.stack}`);
	}
	return null;
}

middleware.renderFooter = async function (req, res, data) {
	return await req.app.renderAsync('admin/footer', data);
};

middleware.checkPrivileges = helpers.try(async (req, res, next) => {
	// Kick out guests, obviously
	if (req.uid <= 0) {
		return controllers.helpers.notAllowed(req, res);
	}

	// Otherwise, check for privilege based on page (if not in mapping, deny access)
	const path = req.path.replace(/^(\/api)?(\/v3)?\/admin\/?/g, '');
	if (path) {
		const privilege = privileges.admin.resolve(path);
		if (!await privileges.admin.can(privilege, req.uid)) {
			return controllers.helpers.notAllowed(req, res);
		}
	} else {
		// If accessing /admin, check for any valid admin privs
		const privilegeSet = await privileges.admin.get(req.uid);
		if (!Object.values(privilegeSet).some(Boolean)) {
			return controllers.helpers.notAllowed(req, res);
		}
	}

	// If user does not have password
	const hasPassword = await user.hasPassword(req.uid);
	if (!hasPassword) {
		return next();
	}

	// Reject if they need to re-login (due to ACP timeout), otherwise extend logout timer
	const loginTime = req.session.meta ? req.session.meta.datetime : 0;
	const adminReloginDuration = meta.config.adminReloginDuration * 60000;
	const disabled = meta.config.adminReloginDuration === 0;
	if (disabled || (loginTime && parseInt(loginTime, 10) > Date.now() - adminReloginDuration)) {
		const timeLeft = parseInt(loginTime, 10) - (Date.now() - adminReloginDuration);
		if (req.session.meta && timeLeft < Math.min(60000, adminReloginDuration)) {
			req.session.meta.datetime += Math.min(60000, adminReloginDuration);
		}

		return next();
	}

	let returnTo = req.path;
	if (nconf.get('relative_path')) {
		returnTo = req.path.replace(new RegExp(`^${nconf.get('relative_path')}`), '');
	}
	returnTo = returnTo.replace(/^\/api/, '');

	req.session.returnTo = returnTo;
	req.session.forceLogin = 1;

	await plugins.hooks.fire('response:auth.relogin', { req, res });
	if (res.headersSent) {
		return;
	}

	if (res.locals.isAPI) {
		res.status(401).json({});
	} else {
		res.redirect(`${nconf.get('relative_path')}/login?local=1`);
	}
});
