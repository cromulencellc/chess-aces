'use strict';


define('admin/manage/category-analytics', ['Chart'], function (Chart) {
	const CategoryAnalytics = {};

	CategoryAnalytics.init = function () {
		const hourlyCanvas = document.getElementById('pageviews:hourly');
		const dailyCanvas = document.getElementById('pageviews:daily');
		const topicsCanvas = document.getElementById('topics:daily');
		const postsCanvas = document.getElementById('posts:daily');
		const hourlyLabels = utils.getHoursArray().map(function (text, idx) {
			return idx % 3 ? '' : text;
		});
		const dailyLabels = utils.getDaysArray().map(function (text, idx) {
			return idx % 3 ? '' : text;
		});

		if (utils.isMobile()) {
			Chart.defaults.global.tooltips.enabled = false;
		}

		const data = {
			'pageviews:hourly': {
				labels: hourlyLabels,
				datasets: [
					{
						label: '',
						backgroundColor: 'rgba(186,139,175,0.2)',
						borderColor: 'rgba(186,139,175,1)',
						pointBackgroundColor: 'rgba(186,139,175,1)',
						pointHoverBackgroundColor: '#fff',
						pointBorderColor: '#fff',
						pointHoverBorderColor: 'rgba(186,139,175,1)',
						data: ajaxify.data.analytics['pageviews:hourly'],
					},
				],
			},
			'pageviews:daily': {
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
						data: ajaxify.data.analytics['pageviews:daily'],
					},
				],
			},
			'topics:daily': {
				labels: dailyLabels.slice(-7),
				datasets: [
					{
						label: '',
						backgroundColor: 'rgba(171,70,66,0.2)',
						borderColor: 'rgba(171,70,66,1)',
						pointBackgroundColor: 'rgba(171,70,66,1)',
						pointHoverBackgroundColor: '#fff',
						pointBorderColor: '#fff',
						pointHoverBorderColor: 'rgba(171,70,66,1)',
						data: ajaxify.data.analytics['topics:daily'],
					},
				],
			},
			'posts:daily': {
				labels: dailyLabels.slice(-7),
				datasets: [
					{
						label: '',
						backgroundColor: 'rgba(161,181,108,0.2)',
						borderColor: 'rgba(161,181,108,1)',
						pointBackgroundColor: 'rgba(161,181,108,1)',
						pointHoverBackgroundColor: '#fff',
						pointBorderColor: '#fff',
						pointHoverBorderColor: 'rgba(161,181,108,1)',
						data: ajaxify.data.analytics['posts:daily'],
					},
				],
			},
		};

		hourlyCanvas.width = $(hourlyCanvas).parent().width();
		dailyCanvas.width = $(dailyCanvas).parent().width();
		topicsCanvas.width = $(topicsCanvas).parent().width();
		postsCanvas.width = $(postsCanvas).parent().width();

		new Chart(hourlyCanvas.getContext('2d'), {
			type: 'line',
			data: data['pageviews:hourly'],
			options: {
				responsive: true,
				animation: false,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
							precision: 0,
						},
					}],
				},
			},
		});

		new Chart(dailyCanvas.getContext('2d'), {
			type: 'line',
			data: data['pageviews:daily'],
			options: {
				responsive: true,
				animation: false,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
							precision: 0,
						},
					}],
				},
			},
		});

		new Chart(topicsCanvas.getContext('2d'), {
			type: 'line',
			data: data['topics:daily'],
			options: {
				responsive: true,
				animation: false,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
							precision: 0,
						},
					}],
				},
			},
		});

		new Chart(postsCanvas.getContext('2d'), {
			type: 'line',
			data: data['posts:daily'],
			options: {
				responsive: true,
				animation: false,
				legend: {
					display: false,
				},
				scales: {
					yAxes: [{
						ticks: {
							beginAtZero: true,
							precision: 0,
						},
					}],
				},
			},
		});
	};

	return CategoryAnalytics;
});
