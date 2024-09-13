'use strict';

const motionEvent = (name, target) => new CustomEvent(name, { detail: { target } });
function dispatchPointerEvent(element, name, event) {
    element.dispatchEvent(new CustomEvent(name, { detail: { originalEvent: event } }));
}
function dispatchViewEvent(element, name, entry) {
    element.dispatchEvent(new CustomEvent(name, { detail: { originalEntry: entry } }));
}

exports.dispatchPointerEvent = dispatchPointerEvent;
exports.dispatchViewEvent = dispatchViewEvent;
exports.motionEvent = motionEvent;
