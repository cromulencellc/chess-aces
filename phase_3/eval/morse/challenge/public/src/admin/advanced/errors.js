'use strict';


define('admin/advanced/errors', ['bootbox', 'alerts', 'Chart'], function (bootbox, alerts, Chart) {
	const Errors = {};

	Errors.init = function () {
		Errors.setupCharts();

		$('[data-action="clear"]').on('click', Errors.clear404);
	};

	Errors.clear404 = function () {
		bootbox.confirm('[[admin/advanced/errors:clear404-confirm]]', function (ok) {
			if (ok) {
				socket.emit('admin.errors.clear', {}, function (err) {
					if (err) {
						return alerts.error(err);
					}

					ajaxify.refresh();
					alerts.success('[[admin/advanced/errors:clear404-success]]');
				});
			}
		});
	};

	Errors.setupCharts = function () {
		const notFoundCanvas = document.getElementById('not-found');
		const tooBusyCanvas = document.getElementById('toobusy');
		let dailyLabels = utils.getDaysArray();

		dailyLabels = dailyLabels.slice(-7);

		if (utils.isMobile()) {
			Chart.defaults.global.tooltips.enabled = false;
		}

		const data = {
			'not-found': {
				labels: dailyLabels,
				datasets: [
					{
						label: '',
						backgroundColor: 'rgba(186,139,175,0.2)',
						borderColor: 'rgba(186,139,175,1)',
						pointBackgroundColor: 'rgba(186,139,175,1)',
						pointHoverBackgroundColor: '#fff',
						pointBorderColor: '#fff',
						pointHoverBorderColor: 'rgba(186,139,175,1)',
						data: ajaxify.data.analytics['not-found'],
					},
				],
			},
			toobusy: {
				labels: dailyLabels,
				datasets: [
					{
						label: '',
						backgroundColor: 'rgba(151,187,205,0.2)',
						borderColor: 'rgba(151,187,205,1)',
						pointBackgroundColor: 'rgba(151,187,205,1)',
						pointHoverBackgroundColor: '#fff',
						pointBorderColor: '#fff',
						pointHoverBorderColor: 'rgba(151,187,205,1)',
						data: ajaxify.data.analytics.toobusy,
					},
				],
			},
		};

		notFoundCanvas.width = $(notFoundCanvas).parent().width();
		tooBusyCanvas.width = $(tooBusyCanvas).parent().width();

		new Chart(notFoundCanvas.getContext('2d'), {
			type: 'line',
			data: data['not-found'],
			options: {
				responsive: true,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
						},
					}],
				},
			},
		});

		new Chart(tooBusyCanvas.getContext('2d'), {
			type: 'line',
			data: data.toobusy,
			options: {
				responsive: true,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
						},
					}],
				},
			},
		});
	};

	return Errors;
});
