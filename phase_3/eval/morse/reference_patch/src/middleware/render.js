'use strict';

const nconf = require('nconf');
const validator = require('validator');


const plugins = require('../plugins');
const meta = require('../meta');
const translator = require('../translator');
const widgets = require('../widgets');
const utils = require('../utils');
const helpers = require('./helpers');

const relative_path = nconf.get('relative_path');

module.exports = function (middleware) {
	middleware.processRender = function processRender(req, res, next) {
		// res.render post-processing, modified from here: https://gist.github.com/mrlannigan/5051687
		const { render } = res;

		res.render = async function renderOverride(template, options, fn) {
			const self = this;
			const { req } = this;
			async function renderMethod(template, options, fn) {
				options = options || {};
				if (typeof options === 'function') {
					fn = options;
					options = {};
				}

				options.loggedIn = req.uid > 0;
				options.relative_path = relative_path;
				options.template = { name: template, [template]: true };
				options.url = (req.baseUrl + req.path.replace(/^\/api/, ''));
				options.bodyClass = helpers.buildBodyClass(req, res, options);

				if (req.loggedIn) {
					res.set('cache-control', 'private');
				}

				const buildResult = await plugins.hooks.fire(`filter:${template}.build`, { req: req, res: res, templateData: options });
				if (res.headersSent) {
					return;
				}
				const templateToRender = buildResult.templateData.templateToRender || template;

				const renderResult = await plugins.hooks.fire('filter:middleware.render', { req: req, res: res, templateData: buildResult.templateData });
				if (res.headersSent) {
					return;
				}
				options = renderResult.templateData;
				options._header = {
					tags: await meta.tags.parse(req, renderResult, res.locals.metaTags, res.locals.linkTags),
				};
				options.widgets = await widgets.render(req.uid, {
					template: `${template}.tpl`,
					url: options.url,
					templateData: options,
					req: req,
					res: res,
				});
				res.locals.template = template;
				options._locals = undefined;

				if (res.locals.isAPI) {
					if (req.route && req.route.path === '/api/') {
						options.title = '[[pages:home]]';
					}
					req.app.set('json spaces', global.env === 'development' || req.query.pretty ? 4 : 0);
					return res.json(options);
				}
				const optionsString = JSON.stringify(options).replace(/<\//g, '<\\/');
				const results = await utils.promiseParallel({
					header: renderHeaderFooter('renderHeader', req, res, options),
					content: renderContent(render, templateToRender, req, res, options),
					footer: renderHeaderFooter('renderFooter', req, res, options),
				});

				const str = `${results.header +
					(res.locals.postHeader || '') +
					results.content
				}<script id="ajaxify-data" type="application/json">${
					optionsString
				}</script>${
					res.locals.preFooter || ''
				}${results.footer}`;

				if (typeof fn !== 'function') {
					self.send(str);
				} else {
					fn(null, str);
				}
			}

			try {
				await renderMethod(template, options, fn);
			} catch (err) {
				next(err);
			}
		};

		next();
	};

	async function renderContent(render, tpl, req, res, options) {
		return new Promise((resolve, reject) => {
			render.call(res, tpl, options, async (err, str) => {
				if (err) reject(err);
				else resolve(await translate(str, getLang(req, res)));
			});
		});
	}

	async function renderHeaderFooter(method, req, res, options) {
		let str = '';
		if (res.locals.renderHeader) {
			str = await middleware[method](req, res, options);
		} else if (res.locals.renderAdminHeader) {
			str = await middleware.admin[method](req, res, options);
		} else {
			str = '';
		}
		return await translate(str, getLang(req, res));
	}

	function getLang(req, res) {
		let language = (res.locals.config && res.locals.config.userLang) || 'en-GB';
		if (res.locals.renderAdminHeader) {
			language = (res.locals.config && res.locals.config.acpLang) || 'en-GB';
		}
		return req.query.lang ? validator.escape(String(req.query.lang)) : language;
	}

	async function translate(str, language) {
		const translated = await translator.translate(str, language);
		return translator.unescape(translated);
	}
};
