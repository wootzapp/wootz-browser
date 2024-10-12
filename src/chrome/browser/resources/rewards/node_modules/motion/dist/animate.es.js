import { animate as animate$1, withControls } from '@motionone/dom';
import { isFunction } from '@motionone/utils';
import { Animation } from '@motionone/animation';

function animateProgress(target, options = {}) {
    return withControls([
        () => {
            const animation = new Animation(target, [0, 1], options);
            animation.finished.catch(() => { });
            return animation;
        },
    ], options, options.duration);
}
function animate(target, keyframesOrOptions, options) {
    const factory = isFunction(target) ? animateProgress : animate$1;
    return factory(target, keyframesOrOptions, options);
}

export { animate, animateProgress };
