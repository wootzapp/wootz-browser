-- Copyright 2023 The Chromium Authors
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

INCLUDE PERFETTO MODULE deprecated.v42.common.slices;

-- Hardware info is useful when using sql metrics for analysis
-- in BTP.
INCLUDE PERFETTO MODULE chrome.metadata;
INCLUDE PERFETTO MODULE chrome.scroll_jank.scroll_jank_v3_cause;
INCLUDE PERFETTO MODULE chrome.scroll_jank.utils;

-- Finds the end timestamp for a given slice's descendant with a given name.
-- If there are multiple descendants with a given name, the function will return the
-- first one, so it's most useful when working with a timeline broken down into phases,
-- where each subphase can happen only once.
CREATE PERFETTO FUNCTION _descendant_slice_end(
  -- Id of the parent slice.
  parent_id INT,
  -- Name of the child with the desired end TS.
  child_name STRING
)
-- End timestamp of the child or NULL if it doesn't exist.
RETURNS INT AS
SELECT
  CASE WHEN s.dur
    IS NOT -1 THEN s.ts + s.dur
    ELSE NULL
  END
FROM descendant_slice($parent_id) s
WHERE s.name = $child_name
LIMIT 1;

-- Given a slice id, returns the name of the slice.
CREATE PERFETTO FUNCTION _slice_name_from_id(
  -- The slice id which we need the name for.
  id LONG
)
-- The name of slice with the given id.
RETURNS STRING AS
SELECT
  name
FROM slice
WHERE $id = id;

-- Grabs all gesture updates with respective scroll ids and start/end
-- timestamps, regardless of being presented.
CREATE PERFETTO TABLE chrome_gesture_scroll_updates(
  -- The start timestamp of the scroll.
  ts INT,
  -- The duration of the scroll.
  dur INT,
  -- Slice id for the scroll.
  id INT,
  -- The id of the scroll update event.
  scroll_update_id INT,
  -- The id of the scroll.
  scroll_id INT,
  -- Whether this input event was presented.
  is_presented BOOL,
  -- Frame presentation timestamp aka the timestamp of the
  -- SwapEndToPresentationCompositorFrame substage.
  presentation_timestamp INT,
  -- EventLatency event type.
  event_type INT
) AS
SELECT
  slice.ts,
  slice.dur,
  slice.id,
  EXTRACT_arg(arg_set_id, "event_latency.event_latency_id") AS scroll_update_id,
  chrome_get_most_recent_scroll_begin_id(slice.ts) AS scroll_id,
  has_descendant_slice_with_name(slice.id, "SubmitCompositorFrameToPresentationCompositorFrame")
  AS is_presented,
  _descendant_slice_end(slice.id, "SwapEndToPresentationCompositorFrame") AS presentation_timestamp,
  EXTRACT_ARG(arg_set_id, 'event_latency.event_type') AS event_type
FROM slice JOIN args USING(arg_set_id)
WHERE name = "EventLatency"
AND args.string_value GLOB "*GESTURE_SCROLL_UPDATE";

CREATE PERFETTO TABLE _presented_gesture_scrolls AS
SELECT
  id,
  ts,
  dur,
  scroll_update_id,
  scroll_id,
  presentation_timestamp,
  event_type
FROM chrome_gesture_scroll_updates
WHERE is_presented = true
ORDER BY ts ASC;

-- Scroll updates, corresponding to all input events that were converted to a
-- presented scroll update.
CREATE PERFETTO TABLE chrome_presented_gesture_scrolls(
  -- Minimum slice id for input presented in this frame, the non-presented input.
  id INT,
  -- The start timestamp for producing the frame.
  ts INT,
  -- The duration between producing and presenting the frame.
  dur INT,
  -- The timestamp of the last input that arrived and got presented in the frame.
  last_presented_input_ts INT,
  -- The id of the scroll update event, a unique identifier to the gesture.
  scroll_update_id INT,
  -- The id of the ongoing scroll.
  scroll_id INT,
  -- Frame presentation timestamp.
  presentation_timestamp INT,
  -- EventLatency event type.
  event_type INT
) AS
WITH
scroll_updates_with_presentation_info as MATERIALIZED (
  SELECT
    id,
    ts,
    -- For each scroll update, find the latest presented update which
    -- started before it.
    (
      SELECT id
      FROM _presented_gesture_scrolls _presented
      WHERE _presented.ts <= scroll_update.ts
      ORDER BY ts DESC
      LIMIT 1
     ) as presented_to_scroll_update_slice_id
  FROM chrome_gesture_scroll_updates scroll_update
  ORDER BY presented_to_scroll_update_slice_id, ts
)
SELECT
  id,
  ts,
  dur,
  -- Find the latest input that was presented in this scroll update.
  (
    SELECT presentation_info.ts
    FROM scroll_updates_with_presentation_info presentation_info
    WHERE
      presentation_info.presented_to_scroll_update_slice_id =
        _presented_gesture_scrolls.id
    ORDER BY ts DESC
    LIMIT 1
  ) as last_presented_input_ts,
  scroll_update_id,
  scroll_id,
  presentation_timestamp,
  event_type
FROM _presented_gesture_scrolls
-- TODO(b/247542163): remove this condition when all stages
-- of EventLatency are recorded correctly.
WHERE presentation_timestamp IS NOT NULL;

-- Associate every trace_id with it's perceived delta_y on the screen after
-- prediction.
CREATE PERFETTO TABLE chrome_scroll_updates_with_deltas(
  -- The id of the scroll update event.
  scroll_update_id INT,
  -- The perceived delta_y on the screen post prediction.
  delta_y INT
) AS
SELECT
  EXTRACT_ARG(arg_set_id, 'scroll_deltas.trace_id') AS scroll_update_id,
  EXTRACT_ARG(arg_set_id, 'scroll_deltas.provided_to_compositor_delta_y') AS delta_y
FROM slice
WHERE name = "InputHandlerProxy::HandleGestureScrollUpdate_Result";

-- Obtain the subset of input events that were fully presented, as indicated
-- by the presence of SwapEndToPresentationCompositorFrame.
CREATE PERFETTO TABLE chrome_full_frame_view(
  -- ID of the frame.
  id INT,
  -- Start timestamp of the frame.
  ts INT,
  -- The timestamp of the last presented input.
  last_presented_input_ts INT,
  -- ID of the associated scroll.
  scroll_id INT,
  -- ID of the associated scroll update.
  scroll_update_id INT,
  -- ID of the associated EventLatency.
  event_latency_id INT,
  -- Duration of the associated EventLatency.
  dur INT,
  -- Frame presentation timestamp.
  presentation_timestamp INT
) AS
SELECT
  frames.id,
  frames.ts,
  frames.last_presented_input_ts,
  frames.scroll_id,
  frames.scroll_update_id,
  frames.id AS event_latency_id,
  frames.dur,
  frames.presentation_timestamp
FROM chrome_presented_gesture_scrolls frames
WHERE frames.event_type in (
          "GESTURE_SCROLL_UPDATE",
          "FIRST_GESTURE_SCROLL_UPDATE",
          "INERTIAL_GESTURE_SCROLL_UPDATE")
    AND has_descendant_slice_with_name(frames.id, "SwapEndToPresentationCompositorFrame");

-- Join deltas with EventLatency data.
CREATE PERFETTO TABLE chrome_full_frame_delta_view(
  -- ID of the frame.
  id INT,
  -- Start timestamp of the frame.
  ts INT,
  -- ID of the associated scroll.
  scroll_id INT,
  -- ID of the associated scroll update.
  scroll_update_id INT,
  -- The timestamp of the last presented input.
  last_presented_input_ts INT,
  -- The perceived delta_y on the screen post prediction.
  delta_y INT,
  -- ID of the associated EventLatency.
  event_latency_id INT,
  -- Duration of the associated EventLatency.
  dur INT,
  -- Frame presentation timestamp.
  presentation_timestamp INT
) AS
SELECT
  frames.id,
  frames.ts,
  frames.scroll_id,
  frames.scroll_update_id,
  frames.last_presented_input_ts,
  deltas.delta_y,
  frames.event_latency_id,
  frames.dur,
  frames.presentation_timestamp
FROM chrome_full_frame_view frames
LEFT JOIN chrome_scroll_updates_with_deltas deltas
  ON deltas.scroll_update_id = frames.scroll_update_id;

-- Group all gestures presented at the same timestamp together in
-- a single row.
CREATE PERFETTO VIEW chrome_merged_frame_view(
  -- ID of the frame.
  id INT,
  -- The timestamp of the last presented input.
  max_start_ts INT,
  -- The earliest frame start timestamp.
  min_start_ts INT,
  -- ID of the associated scroll.
  scroll_id INT,
  -- ID of the associated scroll update.
  scroll_update_id INT,
  -- All scroll updates associated with the frame presentation timestamp.
  encapsulated_scroll_ids INT,
  -- Sum of all perceived delta_y values at the frame presentation timestamp.
  total_delta INT,
  -- Lists all of the perceived delta_y values at the frame presentation timestamp.
  segregated_delta_y INT,
  -- ID of the associated EventLatency.
  event_latency_id INT,
  -- Maximum duration of the associated EventLatency.
  dur INT,
  -- Frame presentation timestamp.
  presentation_timestamp INT
) AS
SELECT
  id,
  MAX(last_presented_input_ts) AS max_start_ts,
  MIN(ts) AS min_start_ts,
  scroll_id,
  scroll_update_id,
  GROUP_CONCAT(scroll_update_id,',') AS encapsulated_scroll_ids,
  SUM(delta_y) AS total_delta,
  GROUP_CONCAT(delta_y, ',') AS segregated_delta_y,
  event_latency_id,
  MAX(dur) AS dur,
  presentation_timestamp
FROM chrome_full_frame_delta_view
GROUP BY presentation_timestamp
ORDER BY presentation_timestamp;

-- View contains all chrome presented frames during gesture updates
-- while calculating delay since last presented which usually should
-- equal to |VSYNC_INTERVAL| if no jank is present.
CREATE PERFETTO VIEW chrome_frame_info_with_delay(
  -- gesture scroll slice id.
  id INT,
  -- OS timestamp of the last touch move arrival within a frame.
  max_start_ts INT,
  -- OS timestamp of the first touch move arrival within a frame.
  min_start_ts INT,
  -- The scroll which the touch belongs to.
  scroll_id INT,
  -- ID of the associated scroll update.
  scroll_update_id INT,
  -- Trace ids of all frames presented in at this vsync.
  encapsulated_scroll_ids INT,
  -- Summation of all delta_y of all gesture scrolls in this frame.
  total_delta INT,
  -- All delta y of all gesture scrolls comma separated, summing those gives |total_delta|.
  segregated_delta_y INT,
  -- Event latency id of the presented frame.
  event_latency_id INT,
  -- Duration of the EventLatency.
  dur INT,
  -- Timestamp at which the frame was shown on the screen.
  presentation_timestamp INT,
  -- Time elapsed since the previous frame was presented, usually equals |VSYNC|
  -- if no frame drops happened.
  delay_since_last_frame INT,
  -- Difference in OS timestamps of inputs in the current and the previous frame.
  delay_since_last_input INT,
  -- The event latency id that will be used as a reference to determine the
  -- jank cause.
  prev_event_latency_id INT
) AS
SELECT
  *,
  (presentation_timestamp -
  LAG(presentation_timestamp, 1, presentation_timestamp)
  OVER (PARTITION BY scroll_id ORDER BY presentation_timestamp)) / 1e6 AS delay_since_last_frame,
  (min_start_ts -
  LAG(max_start_ts, 1, min_start_ts)
  OVER (PARTITION BY scroll_id ORDER BY min_start_ts)) / 1e6 AS delay_since_last_input,
  LAG(event_latency_id, 1, -1) OVER (PARTITION BY scroll_id ORDER BY min_start_ts) AS prev_event_latency_id
FROM chrome_merged_frame_view;

-- Calculate |VSYNC_INTERVAL| as the lowest delay between frames larger than
-- zero.
-- TODO(b/286222128): Emit this data from Chrome instead of calculating it.
CREATE PERFETTO VIEW chrome_vsyncs(
  -- The lowest delay between frames larger than zero.
  vsync_interval INT
) AS
SELECT
  MIN(delay_since_last_frame) AS vsync_interval
FROM chrome_frame_info_with_delay
WHERE delay_since_last_frame > 0;

-- Filter the frame view only to frames that had missed vsyncs.
CREATE PERFETTO VIEW chrome_janky_frames_no_cause(
  -- Time elapsed since the previous frame was presented, will be more than |VSYNC| in this view.
  delay_since_last_frame INT,
  -- Event latency id of the presented frame.
  event_latency_id INT,
  -- Vsync interval at the time of recording the trace.
  vsync_interval INT,
  -- Device brand and model.
  hardware_class STRING,
  -- The scroll corresponding to this frame.
  scroll_id INT,
  -- The event latency id that will be used as a reference to determine the jank cause.
  prev_event_latency_id INT
) AS
SELECT
  delay_since_last_frame,
  event_latency_id,
  (SELECT vsync_interval FROM chrome_vsyncs) AS vsync_interval,
  chrome_hardware_class() AS hardware_class,
  scroll_id,
  prev_event_latency_id
FROM chrome_frame_info_with_delay
WHERE delay_since_last_frame > (select vsync_interval + vsync_interval / 2 from chrome_vsyncs)
      AND delay_since_last_input < (select vsync_interval + vsync_interval / 2 from chrome_vsyncs);

-- Janky frame information including the jank cause.
CREATE PERFETTO VIEW chrome_janky_frames_no_subcause(
  -- Time elapsed since the previous frame was presented, will be more than |VSYNC| in this view.
  delay_since_last_frame INT,
  -- Event latency id of the presented frame.
  event_latency_id INT,
  -- Vsync interval at the time of recording the trace.
  vsync_interval INT,
  -- Device brand and model.
  hardware_class STRING,
  -- The scroll corresponding to this frame.
  scroll_id INT,
  -- The event latency id that will be used as a reference to determine the jank cause.
  prev_event_latency_id INT,
  -- Id of the slice corresponding to the offending stage.
  cause_id INT
) AS
SELECT
  *,
  chrome_get_v3_jank_cause_id(event_latency_id, prev_event_latency_id) AS cause_id
FROM chrome_janky_frames_no_cause;

-- Finds all causes of jank for all janky frames, and a cause of sub jank
-- if the cause of jank was GPU related.
CREATE PERFETTO VIEW chrome_janky_frames(
  -- The reason the Vsync was missed.
  cause_of_jank INT,
  -- Further breakdown if the root cause was GPU related.
  sub_cause_of_jank INT,
  -- Time elapsed since the previous frame was presented, will be more than |VSYNC| in this view.
  delay_since_last_frame INT,
  -- Event latency id of the presented frame.
  event_latency_id INT,
  -- Vsync interval at the time of recording the trace.
  vsync_interval INT,
  -- Device brand and model.
  hardware_class STRING,
  -- The scroll corresponding to this frame.
  scroll_id INT
) AS
SELECT
  _slice_name_from_id(cause_id) AS cause_of_jank,
  _slice_name_from_id(
    -- Getting sub-cause
    chrome_get_v3_jank_cause_id(
      -- Here the cause itself is the parent.
      cause_id,
      -- Get the previous cause id as a child to the previous |EventLatency|.
     (SELECT
      id
      FROM slice
      WHERE name = _slice_name_from_id(cause_id)
        AND parent_id = prev_event_latency_id)
    )) AS sub_cause_of_jank,
  delay_since_last_frame,
  event_latency_id,
  vsync_interval,
  hardware_class,
  scroll_id
FROM chrome_janky_frames_no_subcause;

-- Counting all unique frame presentation timestamps.
CREATE PERFETTO VIEW chrome_unique_frame_presentation_ts(
  -- The unique frame presentation timestamp.
  presentation_timestamp INT
) AS
SELECT DISTINCT
presentation_timestamp
FROM chrome_presented_gesture_scrolls;

-- Dividing missed frames over total frames to get janky frame percentage.
-- This represents the v3 scroll jank metrics.
-- Reflects Event.Jank.DelayedFramesPercentage UMA metric.
CREATE PERFETTO VIEW chrome_janky_frames_percentage(
  -- The percent of missed frames relative to total frames - aka the percent of janky frames.
  delayed_frame_percentage FLOAT
) AS
SELECT
(SELECT
  COUNT()
 FROM chrome_janky_frames) * 1.0
/ (SELECT
    COUNT()
  FROM chrome_unique_frame_presentation_ts) * 100 AS delayed_frame_percentage;

-- Number of frames and janky frames per scroll.
CREATE PERFETTO VIEW chrome_frames_per_scroll(
  -- The ID of the scroll.
  scroll_id INT,
  -- The number of frames in the scroll.
  num_frames INT,
  -- The number of delayed/janky frames.
  num_janky_frames INT,
  -- The percentage of janky frames relative to total frames.
  scroll_jank_percentage INT
) AS
WITH
  frames AS (
    SELECT scroll_id, COUNT(*) AS num_frames
    FROM
      chrome_frame_info_with_delay
    GROUP BY scroll_id
  ),
  janky_frames AS (
    SELECT scroll_id, COUNT(*) AS num_janky_frames
    FROM
      chrome_janky_frames
    GROUP BY scroll_id
  )
SELECT
  frames.scroll_id AS scroll_id,
  frames.num_frames AS num_frames,
  janky_frames.num_janky_frames AS num_janky_frames,
  100.0 * janky_frames.num_janky_frames / frames.num_frames
    AS scroll_jank_percentage
FROM frames
LEFT JOIN janky_frames
  ON frames.scroll_id = janky_frames.scroll_id;

-- Scroll jank causes per scroll.
CREATE PERFETTO VIEW chrome_causes_per_scroll(
  -- The ID of the scroll.
  scroll_id INT,
  -- The maximum time a frame was delayed after the presentation of the previous
  -- frame.
  max_delay_since_last_frame INT,
  -- The expected vsync interval.
  vsync_interval INT,
  -- A proto amalgamation of each scroll jank cause including cause name, sub
  -- cause and the duration of the delay since the previous frame was presented.
  scroll_jank_causes BYTES
) AS
SELECT
  scroll_id,
  MAX(1.0 * delay_since_last_frame / vsync_interval)
    AS max_delay_since_last_frame,
  -- MAX does not matter, since `vsync_interval` is the computed as the
  -- same value for a single trace.
  MAX(vsync_interval) AS vsync_interval,
  RepeatedField(
    ChromeScrollJankV3_Scroll_ScrollJankCause(
      'cause',
      cause_of_jank,
      'sub_cause',
      sub_cause_of_jank,
      'delay_since_last_frame',
      1.0 * delay_since_last_frame / vsync_interval))
    AS scroll_jank_causes
FROM
  chrome_janky_frames
GROUP BY scroll_id;