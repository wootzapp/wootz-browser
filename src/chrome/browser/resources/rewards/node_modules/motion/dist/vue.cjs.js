'use strict';

Object.defineProperty(exports, '__esModule', { value: true });

var vue = require('@motionone/vue');



Object.keys(vue).forEach(function (k) {
	if (k !== 'default' && !exports.hasOwnProperty(k)) Object.defineProperty(exports, k, {
		enumerable: true,
		get: function () { return vue[k]; }
	});
});
