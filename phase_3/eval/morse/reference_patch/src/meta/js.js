'use strict';

const path = require('path');
const fs = require('fs');
const util = require('util');
const mkdirp = require('mkdirp');
const rimraf = require('rimraf');

const rimrafAsync = util.promisify(rimraf);

const file = require('../file');
const plugins = require('../plugins');
const minifier = require('./minifier');

const JS = module.exports;

JS.scripts = {
	base: [
		'node_modules/socket.io-client/dist/socket.io.js',
		'node_modules/requirejs/require.js',
		'public/src/require-config.js',
		'node_modules/jquery/dist/jquery.js',
		'node_modules/textcomplete/dist/textcomplete.min.js',
		'node_modules/textcomplete.contenteditable/dist/textcomplete.codemirror.min.js',
		'node_modules/visibilityjs/lib/visibility.core.js',
		'node_modules/bootstrap/dist/js/bootstrap.js',
		'node_modules/@adactive/bootstrap-tagsinput/src/bootstrap-tagsinput.js',
		'node_modules/benchpressjs/build/benchpress.js',
		'node_modules/jquery-serializeobject/jquery.serializeObject.js',
		'node_modules/jquery-deserialize/src/jquery.deserialize.js',

		'public/vendor/bootbox/wrapper.js',

		'public/src/utils.js',
		'public/src/sockets.js',
		'public/src/app.js',
		'public/src/ajaxify.js',
		'public/src/overrides.js',
		'public/src/widgets.js',
	],

	// files listed below are only available client-side, or are bundled in to reduce # of network requests on cold load
	rjs: [
		'public/src/client/header.js',
		'public/src/client/header/chat.js',
		'public/src/client/header/notifications.js',
		'public/src/client/infinitescroll.js',
		'public/src/client/pagination.js',
		'public/src/client/recent.js',
		'public/src/client/unread.js',
		'public/src/client/topic.js',
		'public/src/client/topic/events.js',
		'public/src/client/topic/posts.js',
		'public/src/client/topic/images.js',
		'public/src/client/topic/votes.js',
		'public/src/client/topic/postTools.js',
		'public/src/client/topic/threadTools.js',
		'public/src/client/categories.js',
		'public/src/client/category.js',
		'public/src/client/category/tools.js',

		'public/src/modules/translator.js',
		'public/src/modules/components.js',
		'public/src/modules/hooks.js',
		'public/src/modules/sort.js',
		'public/src/modules/navigator.js',
		'public/src/modules/topicSelect.js',
		'public/src/modules/topicList.js',
		'public/src/modules/categoryFilter.js',
		'public/src/modules/categorySelector.js',
		'public/src/modules/categorySearch.js',
		'public/src/modules/share.js',
		'public/src/modules/alerts.js',
		'public/src/modules/taskbar.js',
		'public/src/modules/helpers.js',
		'public/src/modules/storage.js',
		'public/src/modules/handleBack.js',
		'public/src/modules/messages.js',
		'public/src/modules/search.js',
	],

	admin: [
		'node_modules/material-design-lite/material.js',
		'public/src/admin/admin.js',
		'node_modules/jquery-deserialize/src/jquery.deserialize.js',
	],

	// modules listed below are built (/src/modules) so they can be defined anonymously
	modules: {
		'Chart.js': 'node_modules/chart.js/dist/Chart.min.js',
		'mousetrap.js': 'node_modules/mousetrap/mousetrap.min.js',
		'cropper.js': 'node_modules/cropperjs/dist/cropper.min.js',
		'jquery-ui': 'node_modules/jquery-ui/ui',
		'zxcvbn.js': 'node_modules/zxcvbn/dist/zxcvbn.js',

		// only get ace files required by acp
		'ace/ace.js': 'node_modules/ace-builds/src-min/ace.js',
		'ace/mode-less.js': 'node_modules/ace-builds/src-min/mode-less.js',
		'ace/mode-javascript.js': 'node_modules/ace-builds/src-min/mode-javascript.js',
		'ace/mode-html.js': 'node_modules/ace-builds/src-min/mode-html.js',
		'ace/theme-twilight.js': 'node_modules/ace-builds/src-min/theme-twilight.js',
		'ace/worker-css.js': 'node_modules/ace-builds/src-min/worker-css.js',
		'ace/worker-javascript.js': 'node_modules/ace-builds/src-min/worker-javascript.js',
		'ace/worker-html.js': 'node_modules/ace-builds/src-min/worker-html.js',
		'ace/ext-searchbox.js': 'node_modules/ace-builds/src-min/ext-searchbox.js',

		'clipboard.js': 'node_modules/clipboard/dist/clipboard.min.js',
		'tinycon.js': 'node_modules/tinycon/tinycon.js',
		'slideout.js': 'node_modules/slideout/dist/slideout.min.js',
		'compare-versions.js': 'node_modules/compare-versions/index.js',
		'timeago/locales': 'node_modules/timeago/locales',
		'jquery-form.js': 'node_modules/jquery-form/dist/jquery.form.min.js',
		'xregexp.js': 'node_modules/xregexp/xregexp-all.js',
	},
};

async function linkIfLinux(srcPath, destPath) {
	if (process.platform === 'win32') {
		await fs.promises.copyFile(srcPath, destPath);
	} else {
		await file.link(srcPath, destPath, true);
	}
}

const basePath = path.resolve(__dirname, '../..');

async function minifyModules(modules, fork) {
	const moduleDirs = modules.reduce((prev, mod) => {
		const dir = path.resolve(path.dirname(mod.destPath));
		if (!prev.includes(dir)) {
			prev.push(dir);
		}
		return prev;
	}, []);

	await Promise.all(moduleDirs.map(dir => mkdirp(dir)));

	const filtered = modules.reduce((prev, mod) => {
		if (mod.srcPath.endsWith('.min.js') || path.dirname(mod.srcPath).endsWith('min')) {
			prev.skip.push(mod);
		} else {
			prev.minify.push(mod);
		}

		return prev;
	}, { minify: [], skip: [] });

	await Promise.all([
		minifier.js.minifyBatch(filtered.minify, fork),
		...filtered.skip.map(mod => linkIfLinux(mod.srcPath, mod.destPath)),
	]);
}

async function linkModules() {
	const { modules } = JS.scripts;

	await Promise.all(Object.keys(modules).map(async (relPath) => {
		const srcPath = path.join(__dirname, '../../', modules[relPath]);
		const destPath = path.join(__dirname, '../../build/public/src/modules', relPath);
		const [stats] = await Promise.all([
			fs.promises.stat(srcPath),
			mkdirp(path.dirname(destPath)),
		]);
		if (stats.isDirectory()) {
			await file.linkDirs(srcPath, destPath, true);
			return;
		}

		await linkIfLinux(srcPath, destPath);
	}));
}

const moduleDirs = ['modules', 'admin', 'client'];

async function getModuleList() {
	let modules = Object.keys(JS.scripts.modules).map(relPath => ({
		srcPath: path.join(__dirname, '../../', JS.scripts.modules[relPath]),
		destPath: path.join(__dirname, '../../build/public/src/modules', relPath),
	}));

	const coreDirs = moduleDirs.map(dir => ({
		srcPath: path.join(__dirname, '../../public/src', dir),
		destPath: path.join(__dirname, '../../build/public/src', dir),
	}));

	modules = modules.concat(coreDirs);

	const moduleFiles = [];
	await Promise.all(modules.map(async (module) => {
		const { srcPath } = module;
		const { destPath } = module;

		const stats = await fs.promises.stat(srcPath);
		if (!stats.isDirectory()) {
			moduleFiles.push(module);
			return;
		}

		const files = await file.walk(srcPath);

		const mods = files.filter(
			filePath => path.extname(filePath) === '.js'
		).map(filePath => ({
			srcPath: path.normalize(filePath),
			destPath: path.join(destPath, path.relative(srcPath, filePath)),
		}));

		moduleFiles.push(...mods);
	}));
	moduleFiles.forEach((mod) => {
		mod.filename = path.relative(basePath, mod.srcPath).replace(/\\/g, '/');
	});
	return moduleFiles;
}

async function clearModules() {
	const builtPaths = moduleDirs.map(
		p => path.join(__dirname, '../../build/public/src', p)
	);
	await Promise.all(
		builtPaths.map(builtPath => rimrafAsync(builtPath))
	);
}

JS.buildModules = async function (fork) {
	await clearModules();
	if (process.env.NODE_ENV === 'development') {
		await linkModules();
		return;
	}
	const modules = await getModuleList();
	await minifyModules(modules, fork);
};

async function requirejsOptimize(target) {
	const requirejs = require('requirejs');
	let scriptText = '';
	const sharedCfg = {
		paths: {
			jquery: 'empty:',
		},
		optimize: 'none',
		out: function (text) {
			scriptText += text;
		},
	};
	const bundledModules = [
		{
			baseUrl: path.join(basePath, 'node_modules'),
			name: 'timeago/jquery.timeago',
		},
		{
			baseUrl: path.join(basePath, 'node_modules/nprogress'),
			name: 'nprogress',
		},
		{
			baseUrl: path.join(basePath, 'node_modules/bootbox'),
			name: 'bootbox',
		},
	];
	const targetModules = {
		admin: [
			{
				baseUrl: path.join(basePath, 'node_modules/sortablejs'),
				name: 'Sortable',
			},
		],
		client: [],
	};
	const optimizeAsync = util.promisify((config, cb) => {
		requirejs.optimize(config, () => cb(), err => cb(err));
	});

	const allModules = bundledModules.concat(targetModules[target]);

	for (const moduleCfg of allModules) {
		// eslint-disable-next-line no-await-in-loop
		await optimizeAsync({ ...sharedCfg, ...moduleCfg });
	}
	const filePath = path.join(__dirname, `../../build/public/rjs-bundle-${target}.js`);
	await fs.promises.writeFile(filePath, scriptText);
}

JS.linkStatics = async function () {
	await rimrafAsync(path.join(__dirname, '../../build/public/plugins'));

	await Promise.all(Object.keys(plugins.staticDirs).map(async (mappedPath) => {
		const sourceDir = plugins.staticDirs[mappedPath];
		const destDir = path.join(__dirname, '../../build/public/plugins', mappedPath);

		await mkdirp(path.dirname(destDir));
		await file.linkDirs(sourceDir, destDir, true);
	}));
};

async function getBundleScriptList(target) {
	const pluginDirectories = [];

	if (target === 'admin') {
		target = 'acp';
	}
	let pluginScripts = plugins[`${target}Scripts`].filter((path) => {
		if (path.endsWith('.js')) {
			return true;
		}

		pluginDirectories.push(path);
		return false;
	});

	await Promise.all(pluginDirectories.map(async (directory) => {
		const scripts = await file.walk(directory);
		pluginScripts = pluginScripts.concat(scripts);
	}));

	let scripts = JS.scripts.base;

	if (target === 'client') {
		scripts = scripts.concat(JS.scripts.rjs);
	} else if (target === 'acp') {
		scripts = scripts.concat(JS.scripts.admin);
	}

	scripts = scripts.concat(pluginScripts).map((script) => {
		const srcPath = path.resolve(basePath, script).replace(/\\/g, '/');
		return {
			srcPath: srcPath,
			filename: path.relative(basePath, srcPath).replace(/\\/g, '/'),
		};
	});

	return scripts;
}

JS.buildBundle = async function (target, fork) {
	const fileNames = {
		client: 'nodebb.min.js',
		admin: 'acp.min.js',
	};
	await requirejsOptimize(target);
	const files = await getBundleScriptList(target);

	const srcPath = path.join(__dirname, `../../build/public/rjs-bundle-${target}.js`);
	files.push({
		srcPath: srcPath,
		filename: path.relative(basePath, srcPath).replace(/\\/g, '/'),
	});

	const minify = process.env.NODE_ENV !== 'development';
	const filePath = path.join(__dirname, '../../build/public', fileNames[target]);

	await minifier.js.bundle({
		files: files,
		filename: fileNames[target],
		destPath: filePath,
	}, minify, fork);
};

JS.killMinifier = function () {
	minifier.killAll();
};
