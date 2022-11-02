'use strict';

/* global XRegExp */
(function (factory) {
	if (typeof define === 'function' && define.amd) {
		define('slugify', ['xregexp'], factory);
	} else if (typeof exports === 'object') {
		module.exports = factory(require('xregexp'));
	} else {
		window.slugify = factory(XRegExp);
	}
}(function (XRegExp) {
	const invalidUnicodeChars = XRegExp('[^\\p{L}\\s\\d\\-_]', 'g');
	const invalidLatinChars = /[^\w\s\d\-_]/g;
	const trimRegex = /^\s+|\s+$/g;
	const collapseWhitespace = /\s+/g;
	const collapseDash = /-+/g;
	const trimTrailingDash = /-$/g;
	const trimLeadingDash = /^-/g;
	const isLatin = /^[\w\d\s.,\-@]+$/;

	// http://dense13.com/blog/2009/05/03/converting-string-to-slug-javascript/
	return function slugify(str, preserveCase) {
		if (!str) {
			return '';
		}
		str = String(str).replace(trimRegex, '');
		if (isLatin.test(str)) {
			str = str.replace(invalidLatinChars, '-');
		} else {
			str = XRegExp.replace(str, invalidUnicodeChars, '-');
		}
		str = !preserveCase ? str.toLocaleLowerCase() : str;
		str = str.replace(collapseWhitespace, '-');
		str = str.replace(collapseDash, '-');
		str = str.replace(trimTrailingDash, '');
		str = str.replace(trimLeadingDash, '');
		return str;
	};
}));
