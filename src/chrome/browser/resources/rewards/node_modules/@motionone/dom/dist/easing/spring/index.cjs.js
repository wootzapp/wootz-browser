'use strict';

var generators = require('@motionone/generators');
var createGeneratorEasing = require('../create-generator-easing.cjs.js');

const spring = createGeneratorEasing.createGeneratorEasing(generators.spring);

exports.spring = spring;
