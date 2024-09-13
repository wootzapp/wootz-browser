import { Mesh, Vector3 } from 'three';
import { ModelScene } from './ModelScene.js';
import { Side } from './Shadow.js';
/**
 * This class is a set of two coincident planes. The first is just a cute box
 * outline with rounded corners and damped opacity to indicate the floor extents
 * of a scene. It is purposely larger than the scene's bounding box by RADIUS on
 * all sides so that small scenes are still visible / selectable. Its center is
 * actually carved out by vertices to ensure its fragment shader doesn't add
 * much time.
 *
 * The child plane is a simple plane with the same extents for use in hit
 * testing (translation is triggered when the touch hits the plane, rotation
 * otherwise).
 */
export declare class PlacementBox extends Mesh {
    private hitPlane;
    private shadowHeight;
    private side;
    private goalOpacity;
    private opacityDamper;
    constructor(scene: ModelScene, side: Side);
    /**
     * Get the world hit position if the touch coordinates hit the box, and null
     * otherwise. Pass the scene in to get access to its raycaster.
     */
    getHit(scene: ModelScene, screenX: number, screenY: number): Vector3 | null;
    getExpandedHit(scene: ModelScene, screenX: number, screenY: number): Vector3 | null;
    /**
     * Offset the height of the box relative to the bottom of the scene. Positive
     * is up, so generally only negative values are used.
     */
    set offsetHeight(offset: number);
    get offsetHeight(): number;
    /**
     * Set the box's visibility; it will fade in and out.
     */
    set show(visible: boolean);
    /**
     * Call on each frame with the frame delta to fade the box.
     */
    updateOpacity(delta: number): void;
    /**
     * Call this to clean up Three's cache when you remove the box.
     */
    dispose(): void;
}
