'use strict';

Object.defineProperty(exports, '__esModule', { value: true });

var dom = require('@motionone/dom');
var types = require('@motionone/types');
var animate = require('./animate.cjs.js');



exports.animate = animate.animate;
Object.keys(dom).forEach(function (k) {
	if (k !== 'default' && !exports.hasOwnProperty(k)) Object.defineProperty(exports, k, {
		enumerable: true,
		get: function () { return dom[k]; }
	});
});
Object.keys(types).forEach(function (k) {
	if (k !== 'default' && !exports.hasOwnProperty(k)) Object.defineProperty(exports, k, {
		enumerable: true,
		get: function () { return types[k]; }
	});
});
