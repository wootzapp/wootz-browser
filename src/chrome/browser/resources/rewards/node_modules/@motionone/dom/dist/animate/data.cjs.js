'use strict';

var types = require('@motionone/types');

const data = new WeakMap();
function getAnimationData(element) {
    if (!data.has(element)) {
        data.set(element, {
            transforms: [],
            values: new Map(),
        });
    }
    return data.get(element);
}
function getMotionValue(motionValues, name) {
    if (!motionValues.has(name)) {
        motionValues.set(name, new types.MotionValue());
    }
    return motionValues.get(name);
}

exports.getAnimationData = getAnimationData;
exports.getMotionValue = getMotionValue;
