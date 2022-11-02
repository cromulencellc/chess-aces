'use strict';

define('api', ['hooks'], (hooks) => {
	const api = {};
	const baseUrl = config.relative_path + '/api/v3';

	function call(options, callback) {
		options.url = options.url.startsWith('/api') ?
			config.relative_path + options.url :
			baseUrl + options.url;

		async function doAjax(cb) {
			// Allow options to be modified by plugins, etc.
			({ options } = await hooks.fire('filter:api.options', { options }));

			$.ajax(options)
				.done((res) => {
					cb(null, (
						res &&
						res.hasOwnProperty('status') &&
						res.hasOwnProperty('response') ? res.response : (res || {})
					));
				})
				.fail((ev) => {
					let errMessage;
					if (ev.responseJSON) {
						errMessage = ev.responseJSON.status && ev.responseJSON.status.message ?
							ev.responseJSON.status.message :
							ev.responseJSON.error;
					}

					cb(new Error(errMessage || ev.statusText));
				});
		}

		if (typeof callback === 'function') {
			doAjax(callback);
			return;
		}

		return new Promise((resolve, reject) => {
			doAjax(function (err, data) {
				if (err) reject(err);
				else resolve(data);
			});
		});
	}

	api.get = (route, payload, onSuccess) => call({
		url: route + (payload && Object.keys(payload).length ? ('?' + $.param(payload)) : ''),
	}, onSuccess);

	api.head = (route, payload, onSuccess) => call({
		url: route + (payload && Object.keys(payload).length ? ('?' + $.param(payload)) : ''),
		method: 'head',
	}, onSuccess);

	api.post = (route, payload, onSuccess) => call({
		url: route,
		method: 'post',
		data: JSON.stringify(payload || {}),
		contentType: 'application/json; charset=utf-8',
		headers: {
			'x-csrf-token': config.csrf_token,
		},
	}, onSuccess);

	api.patch = (route, payload, onSuccess) => call({
		url: route,
		method: 'patch',
		data: JSON.stringify(payload || {}),
		contentType: 'application/json; charset=utf-8',
		headers: {
			'x-csrf-token': config.csrf_token,
		},
	}, onSuccess);

	api.put = (route, payload, onSuccess) => call({
		url: route,
		method: 'put',
		data: JSON.stringify(payload || {}),
		contentType: 'application/json; charset=utf-8',
		headers: {
			'x-csrf-token': config.csrf_token,
		},
	}, onSuccess);

	api.del = (route, payload, onSuccess) => call({
		url: route,
		method: 'delete',
		data: JSON.stringify(payload),
		contentType: 'application/json; charset=utf-8',
		headers: {
			'x-csrf-token': config.csrf_token,
		},
	}, onSuccess);
	api.delete = api.del;

	return api;
});
