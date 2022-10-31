'use strict';

define('admin/modules/dashboard-line-graph', ['Chart', 'translator', 'benchpress', 'api', 'hooks', 'bootbox'], function (Chart, translator, Benchpress, api, hooks, bootbox) {
	const Graph = {
		_current: null,
	};
	let isMobile = false;

	Graph.init = ({ set, dataset }) => {
		const canvas = document.getElementById('analytics-traffic');
		const canvasCtx = canvas.getContext('2d');
		const trafficLabels = utils.getHoursArray();

		isMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
		if (isMobile) {
			Chart.defaults.global.tooltips.enabled = false;
		}

		Graph.handleUpdateControls({ set });

		const t = translator.Translator.create();
		return new Promise((resolve) => {
			t.translateKey(`admin/menu:${ajaxify.data.template.name.replace('admin/', '')}`, []).then((key) => {
				const data = {
					labels: trafficLabels,
					datasets: [
						{
							label: key,
							backgroundColor: 'rgba(151,187,205,0.2)',
							borderColor: 'rgba(151,187,205,1)',
							pointBackgroundColor: 'rgba(151,187,205,1)',
							pointHoverBackgroundColor: 'rgba(151,187,205,1)',
							pointBorderColor: '#fff',
							pointHoverBorderColor: 'rgba(151,187,205,1)',
							data: dataset || [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
						},
					],
				};

				canvas.width = $(canvas).parent().width();

				data.datasets[0].yAxisID = 'left-y-axis';

				Graph._current = new Chart(canvasCtx, {
					type: 'line',
					data: data,
					options: {
						responsive: true,
						legend: {
							display: true,
						},
						scales: {
							yAxes: [{
								id: 'left-y-axis',
								ticks: {
									beginAtZero: true,
									precision: 0,
								},
								type: 'linear',
								position: 'left',
								scaleLabel: {
									display: true,
									labelString: key,
								},
							}],
						},
						tooltips: {
							mode: 'x',
						},
					},
				});

				if (!dataset) {
					Graph.update(set).then(resolve);
				} else {
					resolve(Graph._current);
				}
			});
		});
	};

	Graph.handleUpdateControls = ({ set }) => {
		$('[data-action="updateGraph"]:not([data-units="custom"])').on('click', function () {
			let until = new Date();
			const amount = $(this).attr('data-amount');
			if ($(this).attr('data-units') === 'days') {
				until.setHours(0, 0, 0, 0);
			}
			until = until.getTime();
			Graph.update(set, $(this).attr('data-units'), until, amount);

			require(['translator'], function (translator) {
				translator.translate('[[admin/dashboard:page-views-custom]]', function (translated) {
					$('[data-action="updateGraph"][data-units="custom"]').text(translated);
				});
			});
		});

		$('[data-action="updateGraph"][data-units="custom"]').on('click', function () {
			const targetEl = $(this);

			Benchpress.render('admin/partials/pageviews-range-select', {}).then(function (html) {
				const modal = bootbox.dialog({
					title: '[[admin/dashboard:page-views-custom]]',
					message: html,
					buttons: {
						submit: {
							label: '[[global:search]]',
							className: 'btn-primary',
							callback: submit,
						},
					},
				}).on('shown.bs.modal', function () {
					const date = new Date();
					const today = date.toISOString().substr(0, 10);
					date.setDate(date.getDate() - 1);
					const yesterday = date.toISOString().substr(0, 10);

					modal.find('#startRange').val(targetEl.attr('data-startRange') || yesterday);
					modal.find('#endRange').val(targetEl.attr('data-endRange') || today);
				});

				function submit() {
					// NEED TO ADD VALIDATION HERE FOR YYYY-MM-DD
					const formData = modal.find('form').serializeObject();
					const validRegexp = /\d{4}-\d{2}-\d{2}/;

					// Input validation
					if (!formData.startRange && !formData.endRange) {
						// No range? Assume last 30 days
						Graph.update(set, 'days');
						return;
					} else if (!validRegexp.test(formData.startRange) || !validRegexp.test(formData.endRange)) {
						// Invalid Input
						modal.find('.alert-danger').removeClass('hidden');
						return false;
					}

					let until = new Date(formData.endRange);
					until.setDate(until.getDate() + 1);
					until = until.getTime();
					const amount = (until - new Date(formData.startRange).getTime()) / (1000 * 60 * 60 * 24);

					Graph.update(set, 'days', until, amount);

					// Update "custom range" label
					targetEl.attr('data-startRange', formData.startRange);
					targetEl.attr('data-endRange', formData.endRange);
					targetEl.html(formData.startRange + ' &ndash; ' + formData.endRange);
				}
			});
		});
	};

	Graph.update = (
		set,
		units = ajaxify.data.query.units || 'hours',
		until = ajaxify.data.query.until,
		amount = ajaxify.data.query.count
	) => {
		if (!Graph._current) {
			return Promise.reject(new Error('[[error:invalid-data]]'));
		}

		return new Promise((resolve) => {
			api.get(`/admin/analytics/${set}`, { units, until, amount }).then((dataset) => {
				if (units === 'days') {
					Graph._current.data.xLabels = utils.getDaysArray(until, amount);
				} else {
					Graph._current.data.xLabels = utils.getHoursArray();
				}

				Graph._current.data.datasets[0].data = dataset;
				Graph._current.data.labels = Graph._current.data.xLabels;
				Graph._current.update();

				// Update address bar and "View as JSON" button url
				const apiEl = $('#view-as-json');
				const newHref = $.param({
					units: units || 'hours',
					until: until,
					count: amount,
				});
				apiEl.attr('href', `${config.relative_path}/api/v3/admin/analytics/${ajaxify.data.set}?${newHref}`);
				const url = ajaxify.removeRelativePath(ajaxify.data.url.slice(1));
				ajaxify.updateHistory(`${url}?${newHref}`, true);
				hooks.fire('action:admin.dashboard.updateGraph', {
					graph: Graph._current,
				});
				resolve(Graph._current);
			});
		});
	};

	return Graph;
});
