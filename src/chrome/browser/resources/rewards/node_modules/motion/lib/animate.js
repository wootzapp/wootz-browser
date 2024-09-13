import { animate as animateDom, withControls, } from "@motionone/dom";
import { isFunction } from "@motionone/utils";
import { Animation } from "@motionone/animation";
export function animateProgress(target, options = {}) {
    return withControls([
        () => {
            const animation = new Animation(target, [0, 1], options);
            animation.finished.catch(() => { });
            return animation;
        },
    ], options, options.duration);
}
export function animate(target, keyframesOrOptions, options) {
    const factory = isFunction(target) ? animateProgress : animateDom;
    return factory(target, keyframesOrOptions, options);
}
//# sourceMappingURL=animate.js.map