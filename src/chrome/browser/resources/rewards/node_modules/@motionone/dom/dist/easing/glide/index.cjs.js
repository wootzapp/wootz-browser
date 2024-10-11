'use strict';

var generators = require('@motionone/generators');
var createGeneratorEasing = require('../create-generator-easing.cjs.js');

const glide = createGeneratorEasing.createGeneratorEasing(generators.glide);

exports.glide = glide;
