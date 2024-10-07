'use strict';

var transforms = require('./transforms.cjs.js');

function getStyleName(key) {
    if (transforms.transformAlias[key])
        key = transforms.transformAlias[key];
    return transforms.isTransform(key) ? transforms.asTransformCssVar(key) : key;
}

exports.getStyleName = getStyleName;
