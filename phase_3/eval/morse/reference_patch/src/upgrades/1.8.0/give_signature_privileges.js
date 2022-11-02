'use strict';

const privileges = require('../../privileges');

module.exports = {
	name: 'Give registered users signature privilege',
	timestamp: Date.UTC(2018, 1, 28),
	method: function (callback) {
		privileges.global.give(['groups:signature'], 'registered-users', callback);
	},
};
