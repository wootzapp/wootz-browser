// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

GEN_INCLUDE(['facegaze_test_base.js']);

FaceGazeTest = class extends FaceGazeTestBase {
  /** @override */
  testGenPreamble() {
    super.testGenPreamble();
    super.testGenPreambleCommon(
        /*extensionIdName=*/ 'kAccessibilityCommonExtensionId',
        /*failOnConsoleError=*/ true);
  }

  async startFacegazeWithConfigAndForeheadLocation_(
      config, forehead_x, forehead_y) {
    await this.configureFaceGaze(config);

    // No matter the starting location, the cursor position won't change
    // initially, and upcoming forehead locations will be computed relative to
    // this.
    const result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
        forehead_x, forehead_y);
    this.processFaceLandmarkerResult(
        result, /*triggerMouseControllerInterval=*/ true);
    const cursorPosition =
        this.mockAccessibilityPrivate.getLatestCursorPosition();
    assertEquals(config.mouseLocation.x, cursorPosition.x);
    assertEquals(config.mouseLocation.y, cursorPosition.y);
  }
};

AX_TEST_F(
    'FaceGazeTest', 'FacialGesturesInFacialGesturesToMediapipeGestures', () => {
      // Tests that all new FacialGestures are mapped to
      // MediapipeFacialGestures. FacialGestures are those set by the user,
      // while MediapipeFacialGestures are the raw gestures recognized by
      // Mediapipe. This is to help developers remember to add gestures to both.
      const facialGestures = Object.values(FacialGesture);
      const facialGesturesToMediapipeGestures =
          new Set(FacialGesturesToMediapipeGestures.keys().toArray());
      assertEquals(
          facialGestures.length, facialGesturesToMediapipeGestures.size);

      for (const gesture of facialGestures) {
        assertTrue(facialGesturesToMediapipeGestures.has(gesture));
      }
    });

AX_TEST_F('FaceGazeTest', 'IntervalReusesForeheadLocation', async function() {
  const config =
      new Config().withMouseLocation({x: 600, y: 400}).withBufferSize(1);
  await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

  // Manually executing the mouse interval sets the cursor position with the
  // most recent forehead location.
  for (let i = 0; i < 3; i++) {
    this.mockAccessibilityPrivate.clearCursorPosition();
    this.triggerMouseControllerInterval();
    const cursorPosition =
        this.mockAccessibilityPrivate.getLatestCursorPosition();
    assertEquals(600, cursorPosition.x);
    assertEquals(400, cursorPosition.y);
  }
});

AX_TEST_F('FaceGazeTest', 'CursorPositionUpdatedOnInterval', async function() {
  const config =
      new Config().withMouseLocation({x: 600, y: 400}).withBufferSize(1);
  await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

  const result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.2, 0.4);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ false);

  // Cursor position doesn't change on result.
  let cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(600, cursorPosition.x);
  assertEquals(400, cursorPosition.y);

  // Cursor position does change after interval fired.
  this.triggerMouseControllerInterval();
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertNotEquals(600, cursorPosition.x);
  assertNotEquals(400, cursorPosition.y);
});

AX_TEST_F('FaceGazeTest', 'UpdateMouseLocation', async function() {
  const config =
      new Config().withMouseLocation({x: 600, y: 400}).withBufferSize(1);
  await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

  // Move left and down. Note that increasing the x coordinate results in
  // moving left because the image is mirrored, while increasing y moves
  // downward as expected.
  let result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.11, 0.21);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);

  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(360, cursorPosition.x);
  assertEquals(560, cursorPosition.y);

  // Move to where we were. Since the buffer size is 1, should end up at the
  // exact same location.
  result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.1, 0.2);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(600, cursorPosition.x);
  assertEquals(400, cursorPosition.y);

  // Try a larger movement, 10% of the screen.
  result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.12, 0.22);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(120, cursorPosition.x);
  assertEquals(720, cursorPosition.y);

  result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.1, 0.2);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(600, cursorPosition.x);
  assertEquals(400, cursorPosition.y);

  // Try a very small movement, 0.5% of the screen, which ends up being about
  // one pixel.
  result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
      0.101, 0.201);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertEquals(580, cursorPosition.x);
  assertEquals(420, cursorPosition.y);
});

// Test that if the forehead location is moving around a different part of
// the screen, it still has the same offsets (i.e. we aren't tracking
// absolute forehead position, but instead relative).
// This test should use the same cursor positions as the previous version,
// but different forehead locations (with the same offsets).
AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationFromDifferentForeheadLocation',
    async function() {
      const config =
          new Config().withMouseLocation({x: 600, y: 400}).withBufferSize(1);
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.6, 0.7);

      // Move left and down. Note that increasing the x coordinate results in
      // moving left because the image is mirrored.
      let result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
          0.61, 0.71);
      this.processFaceLandmarkerResult(
          result, /*triggerMouseControllerInterval=*/ true);
      cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
      assertEquals(360, cursorPosition.x);
      assertEquals(560, cursorPosition.y);

      // Move to where we were. Since the buffer size is 1, should end up at the
      // exact same location.
      result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
          0.6, 0.7);
      this.processFaceLandmarkerResult(
          result, /*triggerMouseControllerInterval=*/ true);
      cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
      assertEquals(600, cursorPosition.x);
      assertEquals(400, cursorPosition.y);
    });

// Tests that left/top offsets in ScreenBounds are respected. This should have
// the same results as the first test offset by exactly left/top.
AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithScreenNotAtZero', async function() {
      this.mockAccessibilityPrivate.setDisplayBounds(
          [{left: 100, top: 50, width: 1200, height: 800}]);

      const config =
          new Config().withMouseLocation({x: 700, y: 450}).withBufferSize(1);
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

      // Move left and down. Note that increasing the x coordinate results in
      // moving left because the image is mirrored, while increasing y moves
      // downward as expected.
      let result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
          0.11, 0.21);
      this.processFaceLandmarkerResult(
          result, /*triggerMouseControllerInterval=*/ true);

      cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
      assertEquals(460, cursorPosition.x);
      assertEquals(610, cursorPosition.y);

      // Move to where we were. Since the buffer size is 1, should end up at the
      // exact same location.
      result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
          0.1, 0.2);
      this.processFaceLandmarkerResult(
          result, /*triggerMouseControllerInterval=*/ true);
      this.mockAccessibilityPrivate.getLatestCursorPosition();
      cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
      assertEquals(700, cursorPosition.x);
      assertEquals(450, cursorPosition.y);
    });

AX_TEST_F('FaceGazeTest', 'UpdateMouseLocationWithBuffer', async function() {
  const config =
      new Config().withMouseLocation({x: 600, y: 400}).withBufferSize(6);
  await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

  // Move left and down. Note that increasing the x coordinate results in
  // moving left because the image is mirrored.
  let result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.11, 0.21);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  cursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertTrue(cursorPosition.x < 600);
  assertTrue(cursorPosition.y > 400);

  // Move right and up. Due to smoothing, we don't exactly reach (600,400)
  // again, but do get closer to it.
  result =
      new MockFaceLandmarkerResult().setNormalizedForeheadLocation(0.1, 0.2);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  let newCursorPosition =
      this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertTrue(newCursorPosition.x > cursorPosition.x);
  assertTrue(newCursorPosition.y < cursorPosition.y);
  assertTrue(newCursorPosition.x < 600);
  assertTrue(newCursorPosition.y > 400);

  cursorPosition = newCursorPosition;
  // Process the same result again. We move even closer to (600, 400).
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  newCursorPosition = this.mockAccessibilityPrivate.getLatestCursorPosition();
  assertTrue(newCursorPosition.x > cursorPosition.x);
  assertTrue(newCursorPosition.y < cursorPosition.y);
  assertTrue(newCursorPosition.x < 600);
  assertTrue(newCursorPosition.y > 400);
});

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithSpeed1Move1', async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1);
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

      // With mouse acceleration off and buffer size 1, one-pixel head movements
      // correspond to one-pixel changes on screen.
      const px = 1.0 / 1200;
      const py = 1.0 / 800;

      for (let i = 1; i < 10; i++) {
        const result =
            new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
                0.1 + px * i, 0.2 + py * i);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(600 - i, cursorPosition.x);
        assertEquals(400 + i, cursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithSpeed1Move5', async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1);
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);
      const px = 1.0 / 1200;
      const py = 1.0 / 800;

      // Move further. Should get a linear increase in position.
      for (let i = 0; i < 5; i++) {
        result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
            0.1 + px * i * 5, 0.2 + py * i * 5);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(600 - i * 5, cursorPosition.x);
        assertEquals(400 + i * 5, cursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithSpeed1Move20', async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1);
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);
      const px = 1.0 / 1200;
      const py = 1.0 / 800;

      // Move even further. Should get a linear increase in position.
      for (let i = 0; i < 5; i++) {
        result = new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
            0.1 + px * i * 20, 0.2 + py * i * 20);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(600 - i * 20, cursorPosition.x);
        assertEquals(400 + i * 20, cursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithAccelerationMove1',
    async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1)
                         .withMouseAcceleration();
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

      // With mouse acceleration off and buffer size 1, one-pixel head movements
      // correspond to one-pixel changes on screen.
      const px = 1.0 / 1200;
      const py = 1.0 / 800;
      let xLocation = 0.1;
      let yLocation = 0.2;

      // With these settings, moving the face by one pixel at a time is not ever
      // enough to move the cursor.
      for (let i = 1; i < 10; i++) {
        xLocation += px;
        yLocation += py;
        const result =
            new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
                xLocation, yLocation);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(600, cursorPosition.x);
        assertEquals(400, cursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithAccelerationMove5',
    async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1)
                         .withMouseAcceleration();
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);
      const px = 1.0 / 1200;
      const py = 1.0 / 800;
      let xLocation = 0.1;
      let yLocation = 0.2;

      // Move by 5 pixels at a time to get a non-linear increase of 3 (movement
      // still dampened by sigmoid).
      for (let i = 1; i < 5; i++) {
        xLocation += px * 5;
        yLocation += py * 5;
        const result =
            new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
                xLocation, yLocation);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(600 - i * 3, cursorPosition.x);
        assertEquals(400 + i * 3, cursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithAccelerationMove10',
    async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1)
                         .withMouseAcceleration();
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);

      // With mouse acceleration off and buffer size 1, one-pixel head movements
      // correspond to one-pixel changes on screen.
      const px = 1.0 / 1200;
      const py = 1.0 / 800;
      let xLocation = 0.1;
      let yLocation = 0.2;

      // Move by 10 pixels at a time to get a linear increase of 10 (sigma at
      // 1).
      for (let i = 1; i < 5; i++) {
        xLocation += px * 10;
        yLocation += py * 10;
        const initialCursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        const result =
            new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
                xLocation, yLocation);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(-10, cursorPosition.x - initialCursorPosition.x);
        assertEquals(10, cursorPosition.y - initialCursorPosition.y);
      }
    });

AX_TEST_F(
    'FaceGazeTest', 'UpdateMouseLocationWithAccelerationMove20',
    async function() {
      const config = new Config()
                         .withMouseLocation({x: 600, y: 400})
                         .withBufferSize(1)
                         .withSpeeds(1, 1, 1, 1)
                         .withMouseAcceleration();
      await this.startFacegazeWithConfigAndForeheadLocation_(config, 0.1, 0.2);
      const px = 1.0 / 1200;
      const py = 1.0 / 800;
      let xLocation = 0.1;
      let yLocation = 0.2;

      // Finally, do a large movement of 20 px. It should be magnified further.
      for (let i = 1; i < 5; i++) {
        xLocation += px * 20;
        yLocation += py * 20;
        const initialCursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        const result =
            new MockFaceLandmarkerResult().setNormalizedForeheadLocation(
                xLocation, yLocation);
        this.processFaceLandmarkerResult(
            result, /*triggerMouseControllerInterval=*/ true);
        const cursorPosition =
            this.mockAccessibilityPrivate.getLatestCursorPosition();
        assertEquals(-24, cursorPosition.x - initialCursorPosition.x);
        assertEquals(24, cursorPosition.y - initialCursorPosition.y);
      }
    });

AX_TEST_F('FaceGazeTest', 'DetectGesturesAndPerformActions', async function() {
  const gestureToMacroName =
      new Map()
          .set(FacialGesture.JAW_OPEN, MacroName.MOUSE_CLICK_LEFT)
          .set(FacialGesture.BROW_INNER_UP, MacroName.MOUSE_CLICK_RIGHT);
  const gestureToConfidence = new Map()
                                  .set(FacialGesture.JAW_OPEN, 0.6)
                                  .set(FacialGesture.BROW_INNER_UP, 0.6);
  const config = new Config()
                     .withMouseLocation({x: 600, y: 400})
                     .withGestureToMacroName(gestureToMacroName)
                     .withGestureToConfidence(gestureToConfidence);
  await this.configureFaceGaze(config);

  let result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.JAW_OPEN, 0.9)
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_INNER_UP, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);

  assertEquals(1, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
  const pressEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[0];
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventType.PRESS,
      pressEvent.type);
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventButton.LEFT,
      pressEvent.mouseButton);
  assertEquals(600, pressEvent.x);
  assertEquals(400, pressEvent.y);

  // Reduce amount of jaw open to get the release event.
  result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.JAW_OPEN, 0.4)
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_INNER_UP, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);

  assertEquals(2, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
  const releaseEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[1];
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventType.RELEASE,
      releaseEvent.type);
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventButton.LEFT,
      releaseEvent.mouseButton);
  assertEquals(600, releaseEvent.x);
  assertEquals(400, releaseEvent.y);
});

// The BrowDown gesture is special because it is the combination of two
// separate facial gestures. This test ensures that the associated action is
// performed if either of the gestures is detected.
AX_TEST_F('FaceGazeTest', 'BrowDownGesture', async function() {
  const gestureToMacroName =
      new Map().set(FacialGesture.BROWS_DOWN, MacroName.RESET_CURSOR);
  const gestureToConfidence = new Map().set(FacialGesture.BROWS_DOWN, 0.6);
  const config = new Config()
                     .withMouseLocation({x: 0, y: 0})
                     .withGestureToMacroName(gestureToMacroName)
                     .withGestureToConfidence(gestureToConfidence);
  await this.configureFaceGaze(config);
  this.mockAccessibilityPrivate.clearCursorPosition();

  let result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_DOWN_LEFT, 0.3)
          .addGestureWithConfidence(
              MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(null, this.mockAccessibilityPrivate.getLatestCursorPosition());

  result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_DOWN_LEFT, 0.9)
          .addGestureWithConfidence(
              MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(600, this.mockAccessibilityPrivate.getLatestCursorPosition().x);
  assertEquals(400, this.mockAccessibilityPrivate.getLatestCursorPosition().y);
  this.mockAccessibilityPrivate.clearCursorPosition();
  this.clearGestureLastRecognizedTime();

  result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_DOWN_LEFT, 0.3)
          .addGestureWithConfidence(
              MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.9);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(600, this.mockAccessibilityPrivate.getLatestCursorPosition().x);
  assertEquals(400, this.mockAccessibilityPrivate.getLatestCursorPosition().y);
  this.mockAccessibilityPrivate.clearCursorPosition();
  this.clearGestureLastRecognizedTime();

  result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_DOWN_LEFT, 0.9)
          .addGestureWithConfidence(
              MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.9);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(600, this.mockAccessibilityPrivate.getLatestCursorPosition().x);
  assertEquals(400, this.mockAccessibilityPrivate.getLatestCursorPosition().y);
});

AX_TEST_F('FaceGazeTest', 'DoesNotRepeatGesturesTooSoon', async function() {
  const gestureToMacroName =
      new Map()
          .set(FacialGesture.JAW_OPEN, MacroName.MOUSE_CLICK_LEFT)
          .set(FacialGesture.BROW_INNER_UP, MacroName.RESET_CURSOR)
          .set(FacialGesture.BROWS_DOWN, MacroName.MOUSE_CLICK_RIGHT);
  const gestureToConfidence = new Map()
                                  .set(FacialGesture.JAW_OPEN, 0.6)
                                  .set(FacialGesture.BROW_INNER_UP, 0.6)
                                  .set(FacialGesture.BROWS_DOWN, 0.6);
  const config = new Config()
                     .withMouseLocation({x: 600, y: 400})
                     .withGestureToMacroName(gestureToMacroName)
                     .withGestureToConfidence(gestureToConfidence)
                     .withRepeatDelayMs(1000);
  await this.configureFaceGaze(config);

  for (let i = 0; i < 5; i++) {
    const result =
        new MockFaceLandmarkerResult()
            .addGestureWithConfidence(MediapipeFacialGesture.JAW_OPEN, 0.9)
            .addGestureWithConfidence(
                MediapipeFacialGesture.BROW_INNER_UP, 0.3);
    this.processFaceLandmarkerResult(
        result, /*triggerMouseControllerInterval=*/ true);

    // 5 times in quick succession still only generates one press.
    assertEquals(1, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
    const pressEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[0];
    assertEquals(
        this.mockAccessibilityPrivate.SyntheticMouseEventType.PRESS,
        pressEvent.type);
    assertEquals(
        this.mockAccessibilityPrivate.SyntheticMouseEventButton.LEFT,
        pressEvent.mouseButton);
  }

  // Release is generated when the JAW_OPEN ends.
  let result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.JAW_OPEN, 0.5)
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_INNER_UP, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(2, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
  let releaseEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[1];
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventType.RELEASE,
      releaseEvent.type);
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventButton.LEFT,
      releaseEvent.mouseButton);

  // Another gesture is let through once and then also throttled.
  for (let i = 0; i < 5; i++) {
    const result = new MockFaceLandmarkerResult()
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.BROW_DOWN_LEFT, 0.9)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.9);
    this.processFaceLandmarkerResult(
        result, /*triggerMouseControllerInterval=*/ true);

    assertEquals(3, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
    const pressEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[2];
    assertEquals(
        this.mockAccessibilityPrivate.SyntheticMouseEventType.PRESS,
        pressEvent.type);
    assertEquals(
        this.mockAccessibilityPrivate.SyntheticMouseEventButton.RIGHT,
        pressEvent.mouseButton);
  }

  // Release is processed when BROWS_DOWN ends.
  result =
      new MockFaceLandmarkerResult()
          .addGestureWithConfidence(MediapipeFacialGesture.BROW_DOWN_LEFT, 0.4)
          .addGestureWithConfidence(
              MediapipeFacialGesture.BROW_DOWN_RIGHT, 0.3);
  this.processFaceLandmarkerResult(
      result, /*triggerMouseControllerInterval=*/ true);
  assertEquals(4, this.mockAccessibilityPrivate.syntheticMouseEvents_.length);
  releaseEvent = this.mockAccessibilityPrivate.syntheticMouseEvents_[3];
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventType.RELEASE,
      releaseEvent.type);
  assertEquals(
      this.mockAccessibilityPrivate.SyntheticMouseEventButton.RIGHT,
      releaseEvent.mouseButton);
});

AX_TEST_F('FaceGazeTest', 'KeyEvents', async function() {
  const gestureToMacroName =
      new Map()
          .set(FacialGesture.EYE_SQUINT_LEFT, MacroName.KEY_PRESS_SPACE)
          .set(FacialGesture.EYE_SQUINT_RIGHT, MacroName.KEY_PRESS_UP)
          .set(FacialGesture.MOUTH_SMILE, MacroName.KEY_PRESS_DOWN)
          .set(FacialGesture.MOUTH_UPPER_UP, MacroName.KEY_PRESS_LEFT)
          .set(FacialGesture.EYES_BLINK, MacroName.KEY_PRESS_RIGHT);
  const gestureToConfidence = new Map()
                                  .set(FacialGesture.EYE_SQUINT_LEFT, 0.7)
                                  .set(FacialGesture.EYE_SQUINT_RIGHT, 0.7)
                                  .set(FacialGesture.MOUTH_SMILE, 0.7)
                                  .set(FacialGesture.MOUTH_UPPER_UP, 0.7)
                                  .set(FacialGesture.EYES_BLINK, 0.7);
  const config = new Config()
                     .withMouseLocation({x: 600, y: 400})
                     .withGestureToMacroName(gestureToMacroName)
                     .withGestureToConfidence(gestureToConfidence)
                     .withRepeatDelayMs(1000);
  await this.configureFaceGaze(config);

  const makeResultAndProcess = (gestures) => {
    const result = new MockFaceLandmarkerResult()
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.EYE_SQUINT_LEFT,
                           gestures.squintLeft ? gestures.squintLeft : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.EYE_SQUINT_RIGHT,
                           gestures.squintRight ? gestures.squintRight : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.MOUTH_SMILE_LEFT,
                           gestures.smileLeft ? gestures.smileLeft : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.MOUTH_SMILE_RIGHT,
                           gestures.smileRight ? gestures.smileRight : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.MOUTH_UPPER_UP_LEFT,
                           gestures.upperUpLeft ? gestures.upperUpLeft : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.MOUTH_UPPER_UP_RIGHT,
                           gestures.upperUpRight ? gestures.upperUpRight : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.EYE_BLINK_LEFT,
                           gestures.blinkLeft ? gestures.blinkLeft : 0.3)
                       .addGestureWithConfidence(
                           MediapipeFacialGesture.EYE_BLINK_RIGHT,
                           gestures.blinkRight ? gestures.blinkRight : 0.3);
    this.processFaceLandmarkerResult(
        result, /*triggerMouseControllerInterval=*/ true);
    return this.mockAccessibilityPrivate.syntheticKeyEvents_;
  };

  // Squint left for space key press.
  let keyEvents = makeResultAndProcess({squintLeft: .75});
  assertEquals(1, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYDOWN,
      keyEvents[0].type);
  assertEquals(KeyCode.SPACE, keyEvents[0].keyCode);

  // Stop squinting left for space key release.
  keyEvents = makeResultAndProcess({});
  assertEquals(2, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYUP,
      keyEvents[1].type);
  assertEquals(KeyCode.SPACE, keyEvents[1].keyCode);

  // Squint right eye for up key press.
  keyEvents = makeResultAndProcess({squintRight: 0.8});
  assertEquals(3, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYDOWN,
      keyEvents[2].type);
  assertEquals(KeyCode.UP, keyEvents[2].keyCode);

  // Start smiling on both sides to create down arrow key press.
  keyEvents = makeResultAndProcess(
      {squintRight: 0.8, smileLeft: 0.9, smileRight: 0.85});
  assertEquals(4, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYDOWN,
      keyEvents[3].type);
  assertEquals(KeyCode.DOWN, keyEvents[3].keyCode);

  // Stop squinting right eye for up arrow key release.
  keyEvents = makeResultAndProcess({smileLeft: 0.9, smileRight: 0.85});
  assertEquals(5, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYUP,
      keyEvents[4].type);
  assertEquals(KeyCode.UP, keyEvents[4].keyCode);

  // Stop smiling for down arrow key release.
  keyEvents = makeResultAndProcess({});
  assertEquals(6, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYUP,
      keyEvents[5].type);
  assertEquals(KeyCode.DOWN, keyEvents[5].keyCode);

  // Mouth upper up on both sides for left key press.
  keyEvents = makeResultAndProcess({upperUpLeft: 0.9, upperUpRight: 0.8});
  assertEquals(7, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYDOWN,
      keyEvents[6].type);
  assertEquals(KeyCode.LEFT, keyEvents[6].keyCode);

  // Blink both eyes for right key press.
  keyEvents = makeResultAndProcess({
    upperUpLeft: 0.85,
    upperUpRight: 0.9,
    blinkLeft: 0.85,
    blinkRight: 0.75,
  });
  assertEquals(8, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYDOWN,
      keyEvents[7].type);
  assertEquals(KeyCode.RIGHT, keyEvents[7].keyCode);

  // Stop blinking, get right key up.
  keyEvents = makeResultAndProcess({upperUpLeft: 0.85, upperUpRight: 0.95});
  assertEquals(9, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYUP,
      keyEvents[8].type);
  assertEquals(KeyCode.RIGHT, keyEvents[8].keyCode);

  // Stop all gestures, get final left key up.
  keyEvents = makeResultAndProcess({});
  assertEquals(10, keyEvents.length);
  assertEquals(
      chrome.accessibilityPrivate.SyntheticKeyboardEventType.KEYUP,
      keyEvents[9].type);
  assertEquals(KeyCode.LEFT, keyEvents[9].keyCode);
});
