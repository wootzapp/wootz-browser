'use strict';

Object.defineProperty(exports, '__esModule', { value: true });

var dom = require('@motionone/dom');
var utils = require('@motionone/utils');
var animation = require('@motionone/animation');

function animateProgress(target, options = {}) {
    return dom.withControls([
        () => {
            const animation$1 = new animation.Animation(target, [0, 1], options);
            animation$1.finished.catch(() => { });
            return animation$1;
        },
    ], options, options.duration);
}
function animate(target, keyframesOrOptions, options) {
    const factory = utils.isFunction(target) ? animateProgress : dom.animate;
    return factory(target, keyframesOrOptions, options);
}

exports.animate = animate;
exports.animateProgress = animateProgress;
