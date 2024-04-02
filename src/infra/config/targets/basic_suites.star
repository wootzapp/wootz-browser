# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This file contains suite definitions that can be used in
# //testing/buildbot/waterfalls.pyl and will also be usable for builders that
# set their tests in starlark (once that is ready). The legacy_ prefix on the
# declarations indicates the capability to be used in //testing/buildbot. Once a
# suite is no longer needed in //testing/buildbot, targets.bundle (which does
# not yet exist) can be used for grouping tests in a more flexible manner.

load("//lib/targets.star", "targets")

# TODO(gbeaty) - Make the resultdb information for tests using the same binaries
# consistent and move the information onto the binaries

targets.legacy_basic_suite(
    name = "android_12_fieldtrial_webview_tests",
    tests = {
        "webview_trichrome_64_cts_tests_no_field_trial": targets.legacy_test_config(
            ci_only = True,
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "webview_ui_test_app_test_apk_no_field_trial": targets.legacy_test_config(
            ci_only = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_ar_gtests",
    tests = {
        "monochrome_public_test_ar_apk": targets.legacy_test_config(),
        # Name is vr_*, but actually has AR tests.
        "vr_android_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "android_emulator_specific_chrome_public_tests",
    tests = {
        "chrome_public_test_apk": targets.legacy_test_config(
            mixins = [
                "emulator-8-cores",  # Use 8-core to shorten test runtime.
            ],
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
        "chrome_public_unit_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 4,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_emulator_specific_network_enabled_content_browsertests",
    tests = {
        "content_browsertests_with_emulator_network": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "android_isolated_scripts",
    tests = {
        "content_shell_crash_test": targets.legacy_test_config(
            args = [
                "--platform=android",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_monochrome_smoke_tests",
    tests = {
        "monochrome_public_bundle_smoke_test": targets.legacy_test_config(),
        "monochrome_public_smoke_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "android_oreo_standard_gtests",
    tests = {
        "chrome_public_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "chrome_public_unit_test_apk": targets.legacy_test_config(),
        "webview_instrumentation_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
                expiration_sec = 10800,
            ),
        ),
    },
)

# TODO(crbug.com/1111436): Deprecate this group in favor of
# android_pie_rel_gtests if/when android Pie capacity is fully restored.
targets.legacy_basic_suite(
    name = "android_pie_rel_reduced_capacity_gtests",
    tests = {
        "android_browsertests": targets.legacy_test_config(),
        "blink_platform_unittests": targets.legacy_test_config(),
        "cc_unittests": targets.legacy_test_config(),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
        "viz_unittests": targets.legacy_test_config(),
        "webview_instrumentation_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_smoke_tests",
    tests = {
        "chrome_public_smoke_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "android_specific_chromium_gtests",
    tests = {
        "android_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "android_sync_integration_tests": targets.legacy_test_config(
            args = [
                "--test-launcher-batch-limit=1",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
        "android_webview_unittests": targets.legacy_test_config(),
        "content_shell_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        # TODO(kbr): these are actually run on many of the GPU bots, which have
        # physical hardware for several of the desktop OSs. Once the GPU JSON
        # generation script is merged with this one, this should be promoted from
        # the Android-specific section.
        "gl_tests_validating": targets.legacy_test_config(),
        # TODO(kbr): these are actually run on many of the GPU bots, which have
        # physical hardware for several of the desktop OSs. Once the GPU JSON
        # generation script is merged with this one, this should be promoted from
        # the Android-specific section.
        "gl_unittests": targets.legacy_test_config(),
        "mojo_test_apk": targets.legacy_test_config(),
        "ui_android_unittests": targets.legacy_test_config(),
        "webview_instrumentation_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_specific_coverage_java_tests",
    tests = {
        "content_shell_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "mojo_test_apk": targets.legacy_test_config(),
        "webview_instrumentation_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_trichrome_smoke_tests",
    tests = {
        "trichrome_chrome_bundle_smoke_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "android_webview_gpu_telemetry_tests",
    tests = {
        "android_webview_pixel_skia_gold_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "android_wpr_record_replay_tests",
    tests = {
        "chrome_java_test_wpr_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ash_pixel_gtests",
    tests = {
        "ash_pixeltests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "aura_gtests",
    tests = {
        "aura_unittests": targets.legacy_test_config(),
        "compositor_unittests": targets.legacy_test_config(),
        "wm_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "bfcache_android_specific_gtests",
    tests = {
        "bf_cache_android_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "bfcache_generic_gtests",
    tests = {
        "bf_cache_content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "bfcache_linux_specific_gtests",
    tests = {
        "bf_cache_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "blink_unittests",
    tests = {
        "blink_unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "blink_web_tests_ppapi_isolated_scripts",
    tests = {
        "ppapi_blink_web_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "cast_audio_specific_chromium_gtests",
    tests = {
        "cast_audio_backend_unittests": targets.legacy_test_config(),
        "cast_base_unittests": targets.legacy_test_config(),
        "cast_cast_core_unittests": targets.legacy_test_config(),
        "cast_crash_unittests": targets.legacy_test_config(),
        "cast_media_unittests": targets.legacy_test_config(),
        "cast_shell_browsertests": targets.legacy_test_config(
            args = [
                "--enable-local-file-accesses",
                "--ozone-platform=headless",
                "--no-sandbox",
                "--test-launcher-jobs=1",
            ],
            swarming = targets.swarming(
                enable = False,  # https://crbug.com/861753
            ),
        ),
        "cast_shell_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "cast_junit_tests",
    tests = {
        "cast_base_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
        ),
        "cast_shell_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "cast_video_specific_chromium_gtests",
    tests = {
        "cast_display_settings_unittests": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
        "cast_graphics_unittests": targets.legacy_test_config(),
        "views_unittests": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "chrome_android_finch_smoke_tests",
    tests = {
        "variations_android_smoke_tests": targets.legacy_test_config(),
        "variations_webview_smoke_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chrome_finch_smoke_tests",
    tests = {
        "variations_desktop_smoke_tests": targets.legacy_test_config(
            chromeos_args = [
                "--target-platform=cros",
            ],
            lacros_args = [
                "--target-platform=lacros",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "chrome_isolated_script_tests",
    tests = {
        "chrome_sizes": targets.legacy_test_config(),
        "variations_smoke_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chrome_private_code_test_isolated_scripts",
    tests = {
        "chrome_private_code_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chrome_profile_generator_tests",
    tests = {
        "chrome_public_apk_profile_tests": targets.legacy_test_config(
            ci_only = True,
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "chrome_public_tests",
    tests = {
        "chrome_public_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 19,
            ),
        ),
        "chrome_public_unit_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chrome_public_wpt",
    tests = {
        "chrome_public_wpt": targets.legacy_test_config(
            args = [
                "--no-wpt-internal",
            ],
            swarming = targets.swarming(
                shards = 36,
                expiration_sec = 18000,
                hard_timeout_sec = 14400,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chrome_sizes",
    tests = {
        "chrome_sizes": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chrome_sizes_android",
    tests = {
        "chrome_sizes": targets.legacy_test_config(
            remove_mixins = [
                "android_r",
                "bullhead",
                "flame",
                "marshmallow",
                "mdarcy",
                "oreo_fleet",
                "oreo_mr1_fleet",
                "pie_fleet",
                "walleye",
            ],
            args = [
                "--platform=android",
            ],
            swarming = targets.swarming(
                dimensions = {
                    "cpu": "x86-64",
                    "os": "Ubuntu-22.04",
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromedriver_py_tests_isolated_scripts",
    tests = {
        "chromedriver_py_tests": targets.legacy_test_config(
            args = [
                "--test-type=integration",
            ],
        ),
        "chromedriver_py_tests_headless_shell": targets.legacy_test_config(
            args = [
                "--test-type=integration",
            ],
        ),
        "chromedriver_replay_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_annotation_scripts",
    tests = {
        "check_network_annotations": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_browser_all_tast_tests",
    tests = {
        "chrome_all_tast_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            args = [
                "--tast-retries=1",
            ],
            swarming = targets.swarming(
                shards = 10,
                # Tast test doesn't always output. See crbug.com/1306300
                io_timeout_sec = 3600,
                idempotent = False,  # https://crbug.com/923426#c27
            ),
        ),
    },
)

# Test suite for running criticalstaging Tast tests.
targets.legacy_basic_suite(
    name = "chromeos_browser_criticalstaging_tast_tests",
    tests = {
        "chrome_criticalstaging_tast_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            ci_only = True,
            swarming = targets.swarming(
                shards = 2,
                # Tast test doesn't always output. See crbug.com/1306300
                io_timeout_sec = 3600,
                idempotent = False,  # https://crbug.com/923426#c27
            ),
            experiment_percentage = 100,
        ),
    },
)

# Test suite for running disabled Tast tests to collect data to re-enable
# them. The test suite should not be critical to builders.
targets.legacy_basic_suite(
    name = "chromeos_browser_disabled_tast_tests",
    tests = {
        "chrome_disabled_tast_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            ci_only = True,
            swarming = targets.swarming(
                shards = 2,
                # Tast test doesn't always output. See crbug.com/1306300
                io_timeout_sec = 3600,
                idempotent = False,  # https://crbug.com/923426#c27
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_browser_integration_tests",
    tests = {
        "disk_usage_tast_test": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            args = [
                # Stripping gives more accurate disk usage data.
                "--strip-chrome",
            ],
            swarming = targets.swarming(
                idempotent = False,  # https://crbug.com/923426#c27
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_chrome_all_tast_tests",
    tests = {
        "chrome_all_tast_tests": targets.legacy_test_config(
            # `tast_expr` must be a non-empty string to run the tast tests. But the value of
            # would be overridden by `tast_arrt_expr` defined in chromeos/BUILD.gn, so that we
            # put the stub string here.
            tast_expr = "STUB_STRING_TO_RUN_TAST_TESTS",
            test_level_retries = 2,
            # Timeout including DUT privisioning.
            timeout_sec = 14400,
            # Number of shards. Might be overriden for slower boards.
            shards = 15,
        ),
    },
)

# Test suite for running critical Tast tests.
targets.legacy_basic_suite(
    name = "chromeos_chrome_criticalstaging_tast_tests",
    tests = {
        "chrome_criticalstaging_tast_tests": targets.legacy_test_config(
            # `tast_expr` must be a non-empty string to run the tast tests. But the value of
            # would be overridden by `tast_arrt_expr` defined in chromeos/BUILD.gn, so that we
            # put the stub string here.
            tast_expr = "STUB_STRING_TO_RUN_TAST_TESTS",
            test_level_retries = 2,
            ci_only = True,
            timeout_sec = 14400,
            experiment_percentage = 100,
            shards = 3,
        ),
    },
)

# Test suite for running disabled Tast tests to collect data to re-enable
# them. The test suite should not be critical to builders.
targets.legacy_basic_suite(
    name = "chromeos_chrome_disabled_tast_tests",
    tests = {
        "chrome_disabled_tast_tests": targets.legacy_test_config(
            # `tast_expr` must be a non-empty string to run the tast tests. But the value of
            # would be overridden by `tast_arrt_expr` defined in chromeos/BUILD.gn, so that we
            # put the stub string here.
            tast_expr = "STUB_STRING_TO_RUN_TAST_TESTS",
            test_level_retries = 1,
            ci_only = True,
            timeout_sec = 14400,
            experiment_percentage = 100,
            shards = 2,
        ),
    },
)

# GTests to run on Chrome OS devices, but not Chrome OS VMs. Any differences
# between this and chromeos_system_friendly_gtests below should only be due
# to resource constraints (ie: not enough devices).
targets.legacy_basic_suite(
    name = "chromeos_device_only_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_integration_tests",
    tests = {
        "chromeos_integration_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_isolated_scripts",
    tests = {
        "telemetry_perf_unittests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            args = [
                "--browser=cros-chrome",
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
                "--xvfb",
                # 3 is arbitrary, but if we're having more than 3 of these tests
                # fail in a single shard, then something is probably wrong, so fail
                # fast.
                "--typ-max-failures=3",
            ],
            swarming = targets.swarming(
                shards = 12,
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
        "telemetry_unittests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            args = [
                "--jobs=1",
                "--browser=cros-chrome",
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
                # 3 is arbitrary, but if we're having more than 3 of these tests
                # fail in a single shard, then something is probably wrong, so fail
                # fast.
                "--typ-max-failures=3",
            ],
            swarming = targets.swarming(
                shards = 24,
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_js_code_coverage_browser_tests",
    tests = {
        "chromeos_js_code_coverage_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 32,
            ),
        ),
    },
)

# Tests that run on Chrome OS systems (ie: VMs, Chromebooks), *not*
# linux-chromeos.
# NOTE: We only want a small subset of test suites here, because most
# suites assume that they stub out the underlying device hardware.
# https://crbug.com/865693
targets.legacy_basic_suite(
    name = "chromeos_system_friendly_gtests",
    tests = {
        "aura_unittests": targets.legacy_test_config(
            args = [
                "--ozone-platform=headless",
            ],
        ),
        "base_unittests": targets.legacy_test_config(),
        "capture_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-jobs=1",
                # Don't run CaptureMJpeg tests on ChromeOS VM because vivid,
                # which is the virtual video capture device, doesn't support MJPEG.
                "--gtest_filter=-*UsingRealWebcam_CaptureMjpeg*",
            ],
        ),
        "cc_unittests": targets.legacy_test_config(),
        "crypto_unittests": targets.legacy_test_config(),
        "display_unittests": targets.legacy_test_config(),
        "video_decode_accelerator_tests": targets.legacy_test_config(
            ci_only = True,
            experiment_percentage = 100,
        ),
        "fake_libva_driver_unittest": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
        "google_apis_unittests": targets.legacy_test_config(),
        "ipc_tests": targets.legacy_test_config(),
        "latency_unittests": targets.legacy_test_config(),
        "libcups_unittests": targets.legacy_test_config(),
        "media_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/chromeos.media_unittests.filter",
            ],
        ),
        "midi_unittests": targets.legacy_test_config(),
        "mojo_unittests": targets.legacy_test_config(),
        # net_unittests has a test-time dependency on vpython. So add a CIPD'ed
        # vpython of the right arch to the task, and tell the test runner to copy
        # it over to the VM before the test runs.
        "net_unittests": targets.legacy_test_config(
            args = [
                "--vpython-dir=../../vpython_dir_linux_amd64",
                # PythonUtils.PythonRunTime (as opposed to Python3RunTime) requires a
                # copy of Python 2, but it's testing test helpers that are only used
                # outside of net_unittests. This bot runs out of space if trying to
                # ship two vpythons, so we exclude Python 2 and the one test which
                # uses it.
                "--gtest_filter=-PythonUtils.PythonRunTime",
            ],
            swarming = targets.swarming(
                shards = 3,
                cipd_packages = [
                    targets.cipd_package(
                        package = "infra/3pp/tools/cpython3/linux-amd64",
                        location = "vpython_dir_linux_amd64",
                        revision = "version:2@3.8.10.chromium.21",
                    ),
                    targets.cipd_package(
                        package = "infra/tools/luci/vpython/linux-amd64",
                        location = "vpython_dir_linux_amd64",
                        revision = "git_revision:0f694cdc06ba054b9960aa1ae9766e45b53d02c1",
                    ),
                ],
            ),
        ),
        "ozone_gl_unittests": targets.legacy_test_config(
            args = [
                "--stop-ui",
            ],
        ),
        "ozone_unittests": targets.legacy_test_config(),
        "pdf_unittests": targets.legacy_test_config(),
        "printing_unittests": targets.legacy_test_config(),
        "profile_provider_unittest": targets.legacy_test_config(
            args = [
                "--stop-ui",
                "--test-launcher-jobs=1",
            ],
        ),
        "rust_gtest_interop_unittests": targets.legacy_test_config(),
        "sql_unittests": targets.legacy_test_config(),
        "url_unittests": targets.legacy_test_config(),
    },
)

# vaapi_unittest needs to run with fake driver in some builders but others with real driver.
# Therefore these were isolated from chromeos_system_friendly_gtests.
targets.legacy_basic_suite(
    name = "chromeos_vaapi_gtests",
    tests = {
        "vaapi_unittest": targets.legacy_test_config(
            mixins = [
                "vaapi_unittest_args",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromeos_vaapi_fakelib_gtests",
    tests = {
        "vaapi_unittest": targets.legacy_test_config(
            mixins = [
                "vaapi_unittest_args",
                "vaapi_unittest_libfake_args",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_android_scripts",
    tests = {
        "check_network_annotations": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_android_webkit_gtests",
    tests = {
        "blink_heap_unittests": targets.legacy_test_config(),
        "webkit_unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_dev_android_gtests",
    tests = {
        "chrome_public_smoke_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_dev_mac_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
        "content_unittests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(),
        "rust_gtest_interop_unittests": targets.legacy_test_config(),
        "unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_dev_win_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 4,
            ),
        ),
        "content_unittests": targets.legacy_test_config(),
        "interactive_ui_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "net_unittests": targets.legacy_test_config(),
        "rust_gtest_interop_unittests": targets.legacy_test_config(),
        "unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_dev_linux_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "cores": "8",
                },
            ),
        ),
        "browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
                dimensions = {
                    "cores": "8",
                },
            ),
        ),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
                dimensions = {
                    "cores": "8",
                },
            ),
        ),
        "content_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "cores": "2",
                },
            ),
        ),
        "interactive_ui_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
                dimensions = {
                    "cores": "8",
                },
            ),
        ),
        "net_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "cores": "8",
                },
            ),
        ),
        "rust_gtest_interop_unittests": targets.legacy_test_config(),
        "unit_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "cores": "2",
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests",
    tests = {
        "absl_hardening_tests": targets.legacy_test_config(),
        "angle_unittests": targets.legacy_test_config(
            android_args = [
                "-v",
            ],
            use_isolated_scripts_api = True,
        ),
        "base_unittests": targets.legacy_test_config(),
        "blink_common_unittests": targets.legacy_test_config(),
        "blink_heap_unittests": targets.legacy_test_config(),
        "blink_platform_unittests": targets.legacy_test_config(),
        "boringssl_crypto_tests": targets.legacy_test_config(),
        "boringssl_ssl_tests": targets.legacy_test_config(),
        "capture_unittests": targets.legacy_test_config(
            args = [
                "--gtest_filter=-*UsingRealWebcam*",
            ],
        ),
        "cast_unittests": targets.legacy_test_config(),
        "components_browsertests": targets.legacy_test_config(),
        "components_unittests": targets.legacy_test_config(
            android_swarming = targets.swarming(
                shards = 6,
            ),
        ),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
            ),
            android_swarming = targets.swarming(
                shards = 15,
            ),
        ),
        "content_unittests": targets.legacy_test_config(
            android_swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "crashpad_tests": targets.legacy_test_config(),
        "crypto_unittests": targets.legacy_test_config(),
        "env_chromium_unittests": targets.legacy_test_config(
            ci_only = True,
        ),
        "events_unittests": targets.legacy_test_config(),
        "gcm_unit_tests": targets.legacy_test_config(),
        "gin_unittests": targets.legacy_test_config(),
        "google_apis_unittests": targets.legacy_test_config(),
        "gpu_unittests": targets.legacy_test_config(),
        "gwp_asan_unittests": targets.legacy_test_config(),
        "ipc_tests": targets.legacy_test_config(),
        "latency_unittests": targets.legacy_test_config(),
        "leveldb_unittests": targets.legacy_test_config(
            ci_only = True,
        ),
        "libjingle_xmpp_unittests": targets.legacy_test_config(),
        "liburlpattern_unittests": targets.legacy_test_config(),
        "media_unittests": targets.legacy_test_config(),
        "midi_unittests": targets.legacy_test_config(),
        "mojo_unittests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(
            android_swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "perfetto_unittests": targets.legacy_test_config(),
        # TODO(crbug.com/1459686): Enable this.
        # "rust_gtest_interop_unittests": None,
        "services_unittests": targets.legacy_test_config(),
        "shell_dialogs_unittests": targets.legacy_test_config(),
        "skia_unittests": targets.legacy_test_config(),
        "sql_unittests": targets.legacy_test_config(),
        "storage_unittests": targets.legacy_test_config(),
        "ui_base_unittests": targets.legacy_test_config(),
        "ui_touch_selection_unittests": targets.legacy_test_config(),
        "url_unittests": targets.legacy_test_config(),
        "webkit_unit_tests": targets.legacy_test_config(
            android_swarming = targets.swarming(
                shards = 6,
            ),
        ),
        "wtf_unittests": targets.legacy_test_config(),
        "zlib_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests_for_devices_with_graphical_output",
    tests = {
        "cc_unittests": targets.legacy_test_config(),
        "device_unittests": targets.legacy_test_config(),
        "display_unittests": targets.legacy_test_config(),
        "gfx_unittests": targets.legacy_test_config(),
        "unit_tests": targets.legacy_test_config(
            android_swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "viz_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests_for_linux_and_chromeos_only",
    tests = {
        "dbus_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests_for_linux_and_mac_only",
    tests = {
        "openscreen_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests_for_linux_only",
    tests = {
        "ozone_unittests": targets.legacy_test_config(),
        "ozone_x11_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_gtests_for_win_and_linux_only",
    tests = {
        # pthreadpool is only built on Windows and Linux platforms, that is
        # determined by `build_tflite_with_xnnpack` defined in
        # third_party/tflite/features.gni.
        "pthreadpool_unittests": targets.legacy_test_config(
            ci_only = True,
        ),
    },
)

# Multiscreen tests for desktop platform (Windows).
targets.legacy_basic_suite(
    name = "chromium_gtests_for_windows_multiscreen",
    tests = {
        "multiscreen_interactive_ui_tests": targets.legacy_test_config(
            args = [
                "--windows-virtual-display-driver",
                "--gtest_filter=*MultiScreen*:*VirtualDisplayUtilWin*",
            ],
            swarming = targets.swarming(
                dimensions = {
                    "pool": "chromium.tests.multiscreen",
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_ios_scripts",
    tests = {
        "check_static_initializers": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_junit_tests_scripts",
    tests = {
        "android_webview_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "base_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "build_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "chrome_java_test_pagecontroller_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "chrome_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "components_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "content_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "device_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "junit_unit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "keyboard_accessory_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "media_base_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "module_installer_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "net_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "paint_preview_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "password_check_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "password_manager_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "services_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "touch_to_fill_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "ui_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "webapk_client_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "webapk_shell_apk_h2o_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
        "webapk_shell_apk_junit_tests": targets.legacy_test_config(
            mixins = [
                "x86-64",
                "linux-jammy",
                "junit-swarming-emulator",
            ],
            remove_mixins = [
                "emulator-4-cores",
                "nougat-x86-emulator",
                "oreo-x86-emulator",
                "walleye",
                "pie_fleet",
            ],
            use_isolated_scripts_api = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_linux_scripts",
    tests = {
        "check_network_annotations": targets.legacy_test_config(),
        "check_static_initializers": targets.legacy_test_config(),
        "checkdeps": targets.legacy_test_config(),
        "checkperms": targets.legacy_test_config(),
        "headless_python_unittests": targets.legacy_test_config(),
        "metrics_python_tests": targets.legacy_test_config(),
        "webkit_lint": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_mac_scripts",
    tests = {
        "check_static_initializers": targets.legacy_test_config(),
        "metrics_python_tests": targets.legacy_test_config(),
        "webkit_lint": targets.legacy_test_config(),
    },
)

# On some bots we don't have capacity to run all standard tests (for example
# Android Pie), however there are tracing integration tests we want to
# ensure are still working.
targets.legacy_basic_suite(
    name = "chromium_tracing_gtests",
    tests = {
        "services_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_web_tests_brfetch_isolated_scripts",
    tests = {
        # brfetch_blink_web_tests provides coverage for
        # running Layout Tests with BackgroundResourceFetch feature.
        "brfetch_blink_web_tests": targets.legacy_test_config(
            ci_only = True,
            swarming = targets.swarming(
                shards = 1,
            ),
            experiment_percentage = 100,
        ),
        # brfetch_blink_wpt_tests provides coverage for
        # running Layout Tests with BackgroundResourceFetch feature.
        "brfetch_blink_wpt_tests": targets.legacy_test_config(
            ci_only = True,
            swarming = targets.swarming(
                shards = 3,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_web_tests_graphite_isolated_scripts",
    tests = {
        "graphite_enabled_blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "graphite_enabled_blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_web_tests_high_dpi_isolated_scripts",
    tests = {
        # high_dpi_blink_web_tests provides coverage for
        # running Layout Tests with forced device scale factor.
        "high_dpi_blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
        # high_dpi_blink_wpt_tests provides coverage for
        # running Layout Tests with forced device scale factor.
        "high_dpi_blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_webkit_isolated_scripts",
    tests = {
        "blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_win_scripts",
    tests = {
        "check_network_annotations": targets.legacy_test_config(),
        "metrics_python_tests": targets.legacy_test_config(),
        "webkit_lint": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "chromium_wpt_tests_isolated_scripts",
    tests = {
        "chrome_wpt_tests": targets.legacy_test_config(
            args = [
                "--test-type",
                "testharness",
                "reftest",
                "crashtest",
                "print-reftest",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_wpt_tests_headful_isolated_scripts",
    tests = {
        "chrome_wpt_tests_headful": targets.legacy_test_config(
            args = [
                "--no-headless",
                "--test-type",
                "testharness",
                "reftest",
                "crashtest",
                "print-reftest",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "chromium_wpt_tests_old_headless_isolated_scripts",
    tests = {
        "chrome_wpt_tests_old_headless": targets.legacy_test_config(
            args = [
                "--test-type",
                "testharness",
                "reftest",
                "crashtest",
                "print-reftest",
                "--additional-driver-flag=--headless=old",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "clang_tot_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "client_v8_chromium_gtests",
    tests = {
        "app_shell_unittests": targets.legacy_test_config(),
        "browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "chrome_app_unittests": targets.legacy_test_config(),
        "chromedriver_unittests": targets.legacy_test_config(),
        "components_browsertests": targets.legacy_test_config(),
        "components_unittests": targets.legacy_test_config(),
        "compositor_unittests": targets.legacy_test_config(),
        "content_browsertests": targets.legacy_test_config(),
        "content_unittests": targets.legacy_test_config(),
        "device_unittests": targets.legacy_test_config(),
        "extensions_browsertests": targets.legacy_test_config(),
        "extensions_unittests": targets.legacy_test_config(),
        "gcm_unit_tests": targets.legacy_test_config(),
        "gin_unittests": targets.legacy_test_config(),
        "google_apis_unittests": targets.legacy_test_config(),
        "gpu_unittests": targets.legacy_test_config(),
        "headless_browsertests": targets.legacy_test_config(),
        "headless_unittests": targets.legacy_test_config(),
        "interactive_ui_tests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(),
        "pdf_unittests": targets.legacy_test_config(),
        "remoting_unittests": targets.legacy_test_config(),
        "services_unittests": targets.legacy_test_config(),
        "sync_integration_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "client_v8_chromium_isolated_scripts",
    tests = {
        "content_shell_crash_test": targets.legacy_test_config(),
        "telemetry_gpu_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
        "telemetry_perf_unittests": targets.legacy_test_config(
            args = [
                "--xvfb",
                # TODO(crbug.com/1077284): Remove this once Crashpad is the default.
                "--extra-browser-args=--enable-crashpad",
            ],
            swarming = targets.swarming(
                shards = 12,
                idempotent = False,  # https://crbug.com/549140
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
        "telemetry_unittests": targets.legacy_test_config(
            args = [
                "--jobs=1",
                # Disable GPU compositing, telemetry_unittests runs on VMs.
                # https://crbug.com/871955
                "--extra-browser-args=--disable-gpu",
            ],
            swarming = targets.swarming(
                shards = 4,
                idempotent = False,  # https://crbug.com/549140
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "components_perftests_isolated_scripts",
    tests = {
        "components_perftests": targets.legacy_test_config(
            args = [
                "--gtest-benchmark-name=components_perftests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "cronet_gtests",
    tests = {
        "cronet_sample_test_apk": targets.legacy_test_config(),
        "cronet_smoketests_apk": targets.legacy_test_config(),
        "cronet_smoketests_missing_native_library_instrumentation_apk": targets.legacy_test_config(),
        "cronet_smoketests_platform_only_instrumentation_apk": targets.legacy_test_config(),
        "cronet_test_instrumentation_apk": targets.legacy_test_config(
            mixins = [
                "emulator-enable-network",
            ],
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "cronet_tests_android": targets.legacy_test_config(),
        "cronet_unittests_android": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 4,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "cronet_resource_sizes",
    tests = {
        "resource_sizes_cronet_sample_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                # This suite simply analyzes build targets without running them.
                # It can thus run on a standard linux machine w/o a device.
                dimensions = {
                    "os": "Ubuntu-22.04",
                    "cpu": "x86-64",
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "cronet_sizes",
    tests = {
        "cronet_sizes": targets.legacy_test_config(
            remove_mixins = [
                "android_r",
                "bullhead",
                "flame",
                "marshmallow",
                "mdarcy",
                "oreo_fleet",
                "oreo_mr1_fleet",
                "pie_fleet",
                "walleye",
            ],
            swarming = targets.swarming(
                # This suite simply analyzes build targets without running them.
                # It can thus run on a standard linux machine w/o a device.
                dimensions = {
                    "os": "Ubuntu-22.04",
                    "cpu": "x86-64",
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "desktop_chromium_isolated_scripts",
    tests = {
        "blink_python_tests": targets.legacy_test_config(),
        "blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
        "content_shell_crash_test": targets.legacy_test_config(),
        "flatbuffers_unittests": targets.legacy_test_config(),
        "grit_python_unittests": targets.legacy_test_config(),
        "telemetry_gpu_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
        "telemetry_unittests": targets.legacy_test_config(
            args = [
                "--jobs=1",
                # Disable GPU compositing, telemetry_unittests runs on VMs.
                # https://crbug.com/871955
                "--extra-browser-args=--disable-gpu",
            ],
            swarming = targets.swarming(
                shards = 8,
                idempotent = False,  # https://crbug.com/549140
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
        "views_perftests": targets.legacy_test_config(
            args = [
                "--gtest-benchmark-name=views_perftests",
            ],
        ),
    },
)

# Script tests that only need to run on one builder per desktop platform.
targets.legacy_basic_suite(
    name = "desktop_once_isolated_scripts",
    tests = {
        "test_env_py_unittests": targets.legacy_test_config(),
        "xvfb_py_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "devtools_browser_tests",
    tests = {
        "devtools_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "devtools_webkit_and_tab_target_isolated_scripts",
    tests = {
        "blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "blink_web_tests_dt_tab_target": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "devtools_webkit_isolated_scripts",
    tests = {
        "blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 7,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "fieldtrial_android_tests",
    tests = {
        "android_browsertests_no_fieldtrial": targets.legacy_test_config(
            ci_only = True,
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "fieldtrial_browser_tests",
    tests = {
        "browser_tests_no_field_trial": targets.legacy_test_config(
            ci_only = True,
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "components_browsertests_no_field_trial": targets.legacy_test_config(
            ci_only = True,
        ),
        "interactive_ui_tests_no_field_trial": targets.legacy_test_config(
            ci_only = True,
        ),
        "sync_integration_tests_no_field_trial": targets.legacy_test_config(
            ci_only = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "finch_smoke_tests",
    tests = {
        # TODO(crbug.com/1227222): Change this to the actual finch smoke test
        # once it exists.
        "base_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "fuchsia_accessibility_content_browsertests",
    tests = {
        "accessibility_content_browsertests": targets.legacy_test_config(
            args = [
                "--test-arg=--disable-gpu",
                "--test-arg=--headless",
                "--test-arg=--ozone-platform=headless",
            ],
            swarming = targets.swarming(
                shards = 8,  # this may depend on runtime of a11y CQ
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "fuchsia_chrome_small_gtests",
    tests = {
        "courgette_unittests": targets.legacy_test_config(),
        "headless_unittests": targets.legacy_test_config(),
        "message_center_unittests": targets.legacy_test_config(),
        "views_examples_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.views_examples_unittests.filter",
            ],
        ),
        "views_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.views_unittests.filter",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "fuchsia_common_gtests",
    tests = {
        "absl_hardening_tests": targets.legacy_test_config(),
        "accessibility_unittests": targets.legacy_test_config(),
        "aura_unittests": targets.legacy_test_config(),
        "base_unittests": targets.legacy_test_config(),
        "blink_common_unittests": targets.legacy_test_config(),
        "blink_fuzzer_unittests": targets.legacy_test_config(),
        "blink_heap_unittests": targets.legacy_test_config(),
        "blink_platform_unittests": targets.legacy_test_config(),
        "blink_unittests": targets.legacy_test_config(),
        "boringssl_crypto_tests": targets.legacy_test_config(),
        "boringssl_ssl_tests": targets.legacy_test_config(),
        "capture_unittests": targets.legacy_test_config(),
        "components_browsertests": targets.legacy_test_config(
            args = [
                "--test-arg=--disable-gpu",
                "--test-arg=--headless",
                "--test-arg=--ozone-platform=headless",
            ],
        ),
        "components_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "compositor_unittests": targets.legacy_test_config(),
        "content_browsertests": targets.legacy_test_config(
            args = [
                "--gtest_filter=-All/DumpAccessibility*/fuchsia",
                "--test-arg=--disable-gpu",
                "--test-arg=--headless",
                "--test-arg=--ozone-platform=headless",
            ],
            swarming = targets.swarming(
                shards = 14,
            ),
        ),
        "content_unittests": targets.legacy_test_config(),
        "crypto_unittests": targets.legacy_test_config(),
        "events_unittests": targets.legacy_test_config(),
        "filesystem_service_unittests": targets.legacy_test_config(),
        "gcm_unit_tests": targets.legacy_test_config(),
        "gin_unittests": targets.legacy_test_config(),
        "google_apis_unittests": targets.legacy_test_config(),
        "gpu_unittests": targets.legacy_test_config(),
        "gwp_asan_unittests": targets.legacy_test_config(),
        "headless_browsertests": targets.legacy_test_config(),
        "ipc_tests": targets.legacy_test_config(),
        "latency_unittests": targets.legacy_test_config(),
        "media_unittests": targets.legacy_test_config(),
        "midi_unittests": targets.legacy_test_config(),
        "mojo_unittests": targets.legacy_test_config(),
        "native_theme_unittests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.net_unittests.filter",
            ],
            swarming = targets.swarming(
                shards = 4,
            ),
        ),
        "ozone_gl_unittests": targets.legacy_test_config(
            args = [
                "--test-arg=--ozone-platform=headless",
            ],
        ),
        "ozone_unittests": targets.legacy_test_config(),
        "perfetto_unittests": targets.legacy_test_config(),
        # TODO(crbug.com/1459686): Enable this.
        # "rust_gtest_interop_unittests": None,
        "services_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.services_unittests.filter",
            ],
        ),
        "shell_dialogs_unittests": targets.legacy_test_config(),
        "skia_unittests": targets.legacy_test_config(),
        "snapshot_unittests": targets.legacy_test_config(),
        "sql_unittests": targets.legacy_test_config(),
        "storage_unittests": targets.legacy_test_config(),
        "ui_base_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.ui_base_unittests.filter",
            ],
        ),
        "ui_touch_selection_unittests": targets.legacy_test_config(),
        "ui_unittests": targets.legacy_test_config(),
        "url_unittests": targets.legacy_test_config(),
        "wm_unittests": targets.legacy_test_config(),
        "wtf_unittests": targets.legacy_test_config(),
        "zlib_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "fuchsia_common_gtests_with_graphical_output",
    tests = {
        "cc_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "display_unittests": targets.legacy_test_config(),
        "gfx_unittests": targets.legacy_test_config(),
        "viz_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/fuchsia.viz_unittests.filter",
            ],
        ),
    },
)

# This is a set of selected tests to test the test facility only. The
# principle of the selection includes time cost, scenario coverage,
# stability, etc; and it's subject to change. In theory, it should only be
# used by the EngProd team to verify a new test facility setup.
targets.legacy_basic_suite(
    name = "fuchsia_facility_gtests",
    tests = {
        "aura_unittests": targets.legacy_test_config(),
        "blink_common_unittests": targets.legacy_test_config(),
        "courgette_unittests": targets.legacy_test_config(),
        "crypto_unittests": targets.legacy_test_config(),
        "filesystem_service_unittests": targets.legacy_test_config(),
        "web_engine_integration_tests": targets.legacy_test_config(),
        "web_engine_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "fuchsia_sizes_tests",
    tests = {
        "fuchsia_sizes": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gl_gtests_passthrough",
    tests = {
        "gl_tests_passthrough": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "gl_unittests": targets.legacy_test_config(),
    },
)

# BEGIN tests which run on the GPU bots

targets.legacy_basic_suite(
    name = "gpu_angle_fuchsia_unittests_isolated_scripts",
    tests = {
        "angle_unittests": targets.legacy_test_config(
            mixins = [
                "fuchsia_logs",
            ],
            args = [
                "bin/run_angle_unittests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_angle_ios_end2end_gtests",
    tests = {
        "angle_end2end_tests": targets.legacy_test_config(
            args = [
                "--release",
            ],
            use_isolated_scripts_api = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_angle_ios_white_box_gtests",
    tests = {
        "angle_white_box_tests": targets.legacy_test_config(
            args = [
                "--release",
            ],
            use_isolated_scripts_api = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_angle_unit_gtests",
    tests = {
        "angle_unittests": targets.legacy_test_config(
            android_args = [
                "-v",
            ],
            linux_args = [
                "--no-xvfb",
            ],
            use_isolated_scripts_api = True,
        ),
    },
)

# The command buffer perf tests are only run on Windows.
# They are mostly driver and platform independent.
targets.legacy_basic_suite(
    name = "gpu_command_buffer_perf_passthrough_isolated_scripts",
    tests = {
        "passthrough_command_buffer_perftests": targets.legacy_test_config(
            args = [
                "--gtest-benchmark-name=passthrough_command_buffer_perftests",
                "-v",
                "--use-cmd-decoder=passthrough",
                "--use-angle=gl-null",
                "--fast-run",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_common_and_optional_telemetry_tests",
    tests = {
        "info_collection_tests": targets.legacy_test_config(
            args = [
                "$$MAGIC_SUBSTITUTION_GPUExpectedVendorId",
                "$$MAGIC_SUBSTITUTION_GPUExpectedDeviceId",
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--force_high_performance_gpu",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "trace_test": targets.legacy_test_config(
            args = [
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

# GPU gtests which run on both the main and FYI waterfalls.
targets.legacy_basic_suite(
    name = "gpu_common_gtests_passthrough",
    tests = {
        "gl_tests_passthrough": targets.legacy_test_config(
            args = [
                "--use-gl=angle",
            ],
            chromeos_args = [
                "--stop-ui",
                "$$MAGIC_SUBSTITUTION_ChromeOSGtestFilterFile",
            ],
            desktop_args = [
                "--use-gpu-in-tests",
            ],
            linux_args = [
                "--no-xvfb",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "gl_unittests": targets.legacy_test_config(
            mixins = [
                "skia_gold_test",
            ],
            chromeos_args = [
                "--stop-ui",
                "--test-launcher-filter-file=../../testing/buildbot/filters/chromeos.gl_unittests.filter",
            ],
            desktop_args = [
                "--use-gpu-in-tests",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_common_gtests_validating",
    tests = {
        "gl_tests_validating": targets.legacy_test_config(
            chromeos_args = [
                "--stop-ui",
                "$$MAGIC_SUBSTITUTION_ChromeOSGtestFilterFile",
            ],
            desktop_args = [
                "--use-gpu-in-tests",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
        "gl_unittests": targets.legacy_test_config(
            mixins = [
                "skia_gold_test",
            ],
            chromeos_args = [
                "--stop-ui",
                "--test-launcher-filter-file=../../testing/buildbot/filters/chromeos.gl_unittests.filter",
            ],
            desktop_args = [
                "--use-gpu-in-tests",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_common_isolated_scripts",
    tests = {
        # Test that expectations files are well-formed.
        "telemetry_gpu_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
    },
)

# GPU gtests that test only Dawn
targets.legacy_basic_suite(
    name = "gpu_dawn_gtests",
    tests = {
        "dawn_end2end_implicit_device_sync_tests": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
            ci_only = True,  # https://crbug.com/dawn/1749
            android_swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "dawn_end2end_skip_validation_tests": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
            android_swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "dawn_end2end_tests": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
            android_swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "dawn_end2end_wire_tests": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
            android_swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_gtests_no_dxc",
    tests = {
        "dawn_end2end_no_dxc_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_gtests_no_dxc_with_validation",
    tests = {
        "dawn_end2end_no_dxc_validation_layers_tests": targets.legacy_test_config(),
    },
)

# GPU gtests that test only Dawn with backend validation layers
targets.legacy_basic_suite(
    name = "gpu_dawn_gtests_with_validation",
    tests = {
        "dawn_end2end_validation_layers_tests": targets.legacy_test_config(
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_perf_smoke_isolated_scripts",
    tests = {
        "dawn_perf_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_web_platform_webgpu_cts_force_swiftshader",
    tests = {
        "webgpu_swiftshader_web_platform_cts_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--use-webgpu-adapter=swiftshader",
                "--test-filter=*web_platform*",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "webgpu_swiftshader_web_platform_cts_with_validation_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--use-webgpu-adapter=swiftshader",
                "--test-filter=*web_platform*",
                "--enable-dawn-backend-validation",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_blink_web_tests",
    tests = {
        "webgpu_blink_web_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_cts",
            ],
            args = [
                "--flag-specific=webgpu",
            ],
        ),
        "webgpu_blink_web_tests_with_backend_validation": targets.legacy_test_config(
            mixins = [
                "webgpu_cts",
            ],
            args = [
                "--flag-specific=webgpu-with-backend-validation",
                # Increase the timeout when using backend validation layers (crbug.com/1208253)
                "--timeout-ms=30000",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_blink_web_tests_force_swiftshader",
    tests = {
        "webgpu_swiftshader_blink_web_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_cts",
            ],
            args = [
                "--flag-specific=webgpu-swiftshader",
            ],
        ),
        "webgpu_swiftshader_blink_web_tests_with_backend_validation": targets.legacy_test_config(
            mixins = [
                "webgpu_cts",
            ],
            args = [
                "--flag-specific=webgpu-swiftshader-with-backend-validation",
                # Increase the timeout when using backend validation layers (crbug.com/1208253)
                "--timeout-ms=30000",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_compat_cts",
    tests = {
        "webgpu_cts_compat_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
            ],
            args = [
                "--extra-browser-args=--use-angle=gl --use-webgpu-adapter=opengles --enable-features=WebGPUExperimentalFeatures",
            ],
            swarming = targets.swarming(
                shards = 14,
            ),
            android_swarming = targets.swarming(
                shards = 36,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_cts",
    tests = {
        "webgpu_cts_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            swarming = targets.swarming(
                shards = 14,
            ),
            android_swarming = targets.swarming(
                shards = 36,
            ),
        ),
        "webgpu_cts_with_validation_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--enable-dawn-backend-validation",
            ],
            swarming = targets.swarming(
                shards = 14,
            ),
            android_swarming = targets.swarming(
                shards = 36,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_cts_asan",
    tests = {
        "webgpu_cts_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
        "webgpu_cts_fxc_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--use-fxc",
            ],
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_dawn_webgpu_cts_fxc",
    tests = {
        "webgpu_cts_fxc_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--use-fxc",
            ],
            ci_only = True,
            swarming = targets.swarming(
                shards = 14,
            ),
        ),
        "webgpu_cts_fxc_with_validation_tests": targets.legacy_test_config(
            mixins = [
                "webgpu_telemetry_cts",
                "linux_vulkan",
            ],
            args = [
                "--enable-dawn-backend-validation",
                "--use-fxc",
            ],
            ci_only = True,
            swarming = targets.swarming(
                shards = 14,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_default_and_optional_win_media_foundation_specific_gtests",
    tests = {
        # MediaFoundation browser tests, which currently only run on Windows OS,
        # and require physical hardware.
        "media_foundation_browser_tests": targets.legacy_test_config(
            args = [
                "--use-gpu-in-tests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_default_and_optional_win_specific_gtests",
    tests = {
        "xr_browser_tests": targets.legacy_test_config(
            args = [
                # The Windows machines this is run on should always meet all the
                # requirements, so skip the runtime checks to help catch issues, e.g.
                # if we're incorrectly being told a DirectX 11.1 device isn't
                # available
                "--ignore-runtime-requirements=*",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_desktop_specific_gtests",
    tests = {
        "tab_capture_end2end_tests": targets.legacy_test_config(
            args = [
                "--enable-gpu",
                "--test-launcher-bot-mode",
                "--test-launcher-jobs=1",
                "--gtest_filter=TabCaptureApiPixelTest.EndToEnd*",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_fyi_and_optional_non_linux_gtests",
    tests = {
        # gpu_unittests is killing the Swarmed Linux GPU bots similarly to
        # how content_unittests was: http://crbug.com/763498 .
        "gpu_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_fyi_and_optional_win_specific_gtests",
    tests = {
        "gles2_conform_d3d9_test": targets.legacy_test_config(
            args = [
                "--use-gpu-in-tests",
                "--use-angle=d3d9",
            ],
        ),
        "gles2_conform_gl_test": targets.legacy_test_config(
            args = [
                "--use-gpu-in-tests",
                "--use-angle=gl",
                "--disable-gpu-sandbox",
            ],
        ),
        # WebNN DirectML backend unit tests, which currently only run on
        # Windows OS, and require physical hardware.
        "services_webnn_unittests": targets.legacy_test_config(
            args = [
                "--use-gpu-in-tests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_fyi_mac_specific_gtests",
    tests = {
        # Face and barcode detection unit tests, which currently only run on
        # Mac OS, and require physical hardware.
        "services_unittests": targets.legacy_test_config(
            args = [
                "--gtest_filter=*Detection*",
                "--use-gpu-in-tests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_fyi_vulkan_swiftshader_gtests",
    tests = {
        "vulkan_swiftshader_content_browsertests": targets.legacy_test_config(
            args = [
                "--enable-gpu",
                "--test-launcher-bot-mode",
                "--test-launcher-jobs=1",
                "--test-launcher-filter-file=../../testing/buildbot/filters/vulkan.content_browsertests.filter",
                "--enable-features=UiGpuRasterization,Vulkan",
                "--use-vulkan=swiftshader",
                "--enable-gpu-rasterization",
                "--disable-software-compositing-fallback",
                "--disable-vulkan-fallback-to-gl-for-testing",
                "--disable-headless-mode",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_gl_passthrough_ganesh_telemetry_tests",
    tests = {
        "context_lost_gl_passthrough_ganesh_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=gl --disable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "expected_color_pixel_gl_passthrough_ganesh_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=gl --disable-features=SkiaGraphite",
            ],
        ),
        "gpu_process_launch_tests": targets.legacy_test_config(),
        "hardware_accelerated_feature_tests": targets.legacy_test_config(),
        "pixel_skia_gold_gl_passthrough_ganesh_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=gl --disable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "screenshot_sync_gl_passthrough_ganesh_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=gl --disable-features=SkiaGraphite",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_gles2_conform_gtests",
    tests = {
        # The gles2_conform_tests are closed-source and deliberately only
        # run on the FYI waterfall and the optional tryservers.
        "gles2_conform_test": targets.legacy_test_config(
            args = [
                "--use-gpu-in-tests",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_info_collection_telemetry_tests",
    tests = {
        "info_collection_tests": targets.legacy_test_config(
            args = [
                "$$MAGIC_SUBSTITUTION_GPUExpectedVendorId",
                "$$MAGIC_SUBSTITUTION_GPUExpectedDeviceId",
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--force_high_performance_gpu",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_metal_passthrough_ganesh_telemetry_tests",
    tests = {
        "context_lost_metal_passthrough_ganesh_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --disable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "expected_color_pixel_metal_passthrough_ganesh_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --disable-features=SkiaGraphite",
            ],
        ),
        "gpu_process_launch_tests": targets.legacy_test_config(),
        "hardware_accelerated_feature_tests": targets.legacy_test_config(),
        "pixel_skia_gold_metal_passthrough_ganesh_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --disable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "screenshot_sync_metal_passthrough_ganesh_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --disable-features=SkiaGraphite",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_metal_passthrough_graphite_telemetry_tests",
    tests = {
        "context_lost_metal_passthrough_graphite_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --enable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "expected_color_pixel_metal_passthrough_graphite_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --enable-features=SkiaGraphite",
            ],
        ),
        "gpu_process_launch_tests": targets.legacy_test_config(),
        "hardware_accelerated_feature_tests": targets.legacy_test_config(),
        "pixel_skia_gold_metal_passthrough_graphite_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --enable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
        ),
        "screenshot_sync_metal_passthrough_graphite_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --use-angle=metal --enable-features=SkiaGraphite",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_noop_sleep_telemetry_test",
    tests = {
        # The former GPU-specific generator script contained logic to
        # detect whether the so-called "experimental" GPU bots, which test
        # newer driver versions, were identical to the "stable" versions
        # of the bots, and if so to mirror their configurations. We prefer
        # to keep this new script simpler and to just configure this by
        # hand in waterfalls.pyl.
        "noop_sleep_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
        ),
    },
)

# This is esentially a copy of gpu_passthrough_telemetry_tests running with
# Graphite. Initially limited to just the tests that pass on Android.
targets.legacy_basic_suite(
    name = "gpu_passthrough_graphite_telemetry_tests",
    tests = {
        "expected_color_pixel_passthrough_graphite_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --enable-features=SkiaGraphite",
            ],
            android_args = [
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "pixel_skia_gold_passthrough_graphite_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --enable-features=SkiaGraphite",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "screenshot_sync_passthrough_graphite_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle --enable-features=SkiaGraphite",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_passthrough_telemetry_tests",
    tests = {
        "context_lost_passthrough_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "expected_color_pixel_passthrough_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
            ],
            android_args = [
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "gpu_process_launch_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "hardware_accelerated_feature_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "pixel_skia_gold_passthrough_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "screenshot_sync_passthrough_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_pixel_passthrough_telemetry_tests",
    tests = {
        "expected_color_pixel_passthrough_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
        ),
        "pixel_skia_gold_passthrough_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=passthrough --use-gl=angle",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_skia_renderer_vulkan_passthrough_telemetry_tests",
    tests = {
        "vulkan_pixel_skia_gold_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-vulkan=native --disable-vulkan-fallback-to-gl-for-testing --enable-features=Vulkan --use-gl=angle --use-angle=gl --use-cmd-decoder=passthrough",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_validating_telemetry_tests",
    tests = {
        "context_lost_validating_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=validating",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "expected_color_pixel_validating_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=validating",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "gpu_process_launch_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "hardware_accelerated_feature_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "pixel_skia_gold_validating_test": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--test-machine-name",
                "${buildername}",
                "--extra-browser-args=--use-cmd-decoder=validating",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
        "screenshot_sync_validating_tests": targets.legacy_test_config(
            args = [
                "--dont-restore-color-profile-after-test",
                "--extra-browser-args=--use-cmd-decoder=validating",
            ],
            android_args = [
                # TODO(crbug.com/1093085): Remove this once we fix the tests.
                "--extra-browser-args=--force-online-connection-state-for-indicator",
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_vulkan_gtests",
    tests = {
        "vulkan_tests": targets.legacy_test_config(
            desktop_args = [
                "--use-gpu-in-tests",
            ],
            linux_args = [
                "--no-xvfb",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webcodecs_telemetry_test",
    tests = {
        "webcodecs_tests": targets.legacy_test_config(
            args = [
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            # TODO(https://crbug.com/1359405): having --xvfb and --no-xvfb is confusing.
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webcodecs_gl_passthrough_ganesh_telemetry_test",
    tests = {
        "webcodecs_gl_passthrough_ganesh_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webcodecs_metal_passthrough_ganesh_telemetry_test",
    tests = {
        "webcodecs_metal_passthrough_ganesh_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webcodecs_metal_passthrough_graphite_telemetry_test",
    tests = {
        "webcodecs_metal_passthrough_graphite_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webcodecs_validating_telemetry_test",
    tests = {
        "webcodecs_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-cmd-decoder=validating",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            # TODO(https://crbug.com/1359405): having --xvfb and --no-xvfb is confusing.
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_d3d11_passthrough_telemetry_tests",
    tests = {
        "webgl2_conformance_d3d11_passthrough_tests": targets.legacy_test_config(
            args = [
                "--webgl-conformance-version=2.0.1",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=d3d11 --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                # These tests currently take about an hour and fifteen minutes
                # to run. Split them into roughly 5-minute shards.
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_gl_passthrough_ganesh_telemetry_tests",
    tests = {
        "webgl2_conformance_gl_passthrough_ganesh_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                # These tests currently take about an hour and fifteen minutes
                # to run. Split them into roughly 5-minute shards.
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_gl_passthrough_telemetry_tests",
    tests = {
        "webgl2_conformance_gl_passthrough_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                # These tests currently take about an hour and fifteen minutes
                # to run. Split them into roughly 5-minute shards.
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_gles_passthrough_telemetry_tests",
    tests = {
        "webgl2_conformance_gles_passthrough_tests": targets.legacy_test_config(
            args = [
                "--webgl-conformance-version=2.0.1",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=gles --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            swarming = targets.swarming(
                # These tests currently take about an hour and fifteen minutes
                # to run. Split them into roughly 5-minute shards.
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_metal_passthrough_graphite_telemetry_tests",
    tests = {
        "webgl2_conformance_metal_passthrough_graphite_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl2_conformance_validating_telemetry_tests",
    tests = {
        "webgl2_conformance_validating_tests": targets.legacy_test_config(
            args = [
                "--webgl-conformance-version=2.0.1",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-cmd-decoder=validating --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                # These tests currently take about an hour and fifteen minutes
                # to run. Split them into roughly 5-minute shards.
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_d3d11_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_d3d11_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=d3d11 --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_d3d9_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_d3d9_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=d3d9 --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_gl_passthrough_ganesh_telemetry_tests",
    tests = {
        "webgl_conformance_gl_passthrough_ganesh_tests": targets.legacy_test_config(
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_gl_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_gl_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=gl --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_gles_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_gles_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=gles --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                shards = 6,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_metal_passthrough_ganesh_telemetry_tests",
    tests = {
        "webgl_conformance_metal_passthrough_ganesh_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_metal_passthrough_graphite_telemetry_tests",
    tests = {
        "webgl_conformance_metal_passthrough_graphite_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_swangle_passthrough_representative_telemetry_tests",
    tests = {
        "webgl_conformance_swangle_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-gl=angle --use-angle=swiftshader --use-cmd-decoder=passthrough --force_high_performance_gpu",
                # We are only interested in running a 'smoketest' to test swangle
                # integration, not the full conformance suite.
                "--test-filter=conformance/rendering/gl-drawelements.html",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_swangle_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_swangle_passthrough_tests": targets.legacy_test_config(
            args = [
                "--extra-browser-args=--use-gl=angle --use-angle=swiftshader --use-cmd-decoder=passthrough",
                "--xvfb",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_telemetry_tests",
    tests = {
        "webgl_conformance_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
            android_swarming = targets.swarming(
                shards = 12,
            ),
            chromeos_swarming = targets.swarming(
                shards = 20,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_validating_telemetry_tests",
    tests = {
        "webgl_conformance_validating_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-cmd-decoder=validating --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUWebGLRuntimeFile",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            android_args = [
                "$$MAGIC_SUBSTITUTION_GPUTelemetryNoRootForUnrootedDevices",
            ],
            chromeos_args = [
                "$$MAGIC_SUBSTITUTION_ChromeOSTelemetryRemote",
            ],
            lacros_args = [
                "--extra-browser-args=--enable-features=UseOzonePlatform --ozone-platform=wayland",
                "--xvfb",
                "--no-xvfb",
                "--use-weston",
                "--weston-use-gl",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
            android_swarming = targets.swarming(
                shards = 6,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "gpu_webgl_conformance_vulkan_passthrough_telemetry_tests",
    tests = {
        "webgl_conformance_vulkan_passthrough_tests": targets.legacy_test_config(
            args = [
                # On dual-GPU devices we want the high-performance GPU to be active
                "--extra-browser-args=--use-angle=vulkan --use-cmd-decoder=passthrough --force_high_performance_gpu",
                "$$MAGIC_SUBSTITUTION_GPUParallelJobs",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "headless_browser_gtests",
    tests = {
        "headless_browsertests": targets.legacy_test_config(),
        "headless_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ios_common_tests",
    tests = {
        "absl_hardening_tests": targets.legacy_test_config(),
        "boringssl_crypto_tests": targets.legacy_test_config(),
        "boringssl_ssl_tests": targets.legacy_test_config(),
        "crashpad_tests": targets.legacy_test_config(),
        "crypto_unittests": targets.legacy_test_config(),
        "google_apis_unittests": targets.legacy_test_config(),
        "ios_components_unittests": targets.legacy_test_config(),
        "ios_net_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "ios_remoting_unittests": targets.legacy_test_config(),
        "ios_testing_unittests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(),
        # TODO(https://bugs.chromium.org/p/gn/issues/detail?id=340): Enable this.
        # "rust_gtest_interop_unittests": None,
        "services_unittests": targets.legacy_test_config(),
        "sql_unittests": targets.legacy_test_config(),
        "url_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ios_crash_xcuitests",
    tests = {
        "ios_crash_xcuitests_module": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ios_eg2_cq_tests",
    tests = {
        "ios_chrome_integration_eg2tests_module": targets.legacy_test_config(
            mixins = [
                "ios_parallel_simulators",
            ],
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
        "ios_web_shell_eg2tests_module": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ios_eg2_tests",
    tests = {
        "ios_chrome_bookmarks_eg2tests_module": targets.legacy_test_config(),
        "ios_chrome_settings_eg2tests_module": targets.legacy_test_config(
            mixins = [
                "ios_parallel_simulators",
            ],
            swarming = targets.swarming(
                shards = 4,
            ),
        ),
        "ios_chrome_signin_eg2tests_module": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 6,
            ),
        ),
        "ios_chrome_smoke_eg2tests_module": targets.legacy_test_config(),
        "ios_chrome_ui_eg2tests_module": targets.legacy_test_config(
            mixins = [
                "ios_parallel_simulators",
            ],
            swarming = targets.swarming(
                shards = 12,
            ),
        ),
        "ios_chrome_web_eg2tests_module": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "ios_remoting_fyi_unittests",
    tests = {
        "ios_remoting_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "ios_screen_size_dependent_tests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
        "components_unittests": targets.legacy_test_config(),
        "gfx_unittests": targets.legacy_test_config(),
        "ios_chrome_unittests": targets.legacy_test_config(),
        "ios_web_inttests": targets.legacy_test_config(),
        "ios_web_unittests": targets.legacy_test_config(),
        "ios_web_view_inttests": targets.legacy_test_config(),
        "ios_web_view_unittests": targets.legacy_test_config(),
        "skia_unittests": targets.legacy_test_config(),
        "ui_base_unittests": targets.legacy_test_config(),
    },
)

# END tests which run on the GPU bots

targets.legacy_basic_suite(
    name = "js_code_coverage_browser_tests",
    tests = {
        "js_code_coverage_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 16,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "lacros_all_tast_tests",
    tests = {
        "lacros_all_tast_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            swarming = targets.swarming(
                idempotent = False,  # https://crbug.com/923426#c27
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "lacros_device_or_vm_gtests",
    tests = {
        "cc_unittests": targets.legacy_test_config(),
        "ozone_unittests": targets.legacy_test_config(),
        "vaapi_unittest": targets.legacy_test_config(
            mixins = [
                "vaapi_unittest_args",
                "vaapi_unittest_libfake_args",
            ],
        ),
    },
)

# Lacros tests that run on Skylab, and these tests are usually HW sensative,
# Currently we only run Tast tests.
targets.legacy_basic_suite(
    name = "lacros_skylab_tests",
    tests = {
        "lacros_all_tast_tests": targets.legacy_test_config(
            tast_expr = "(\"group:mainline\" && (\"dep:lacros_stable\" || \"dep:lacros\") && !informational)",
            test_level_retries = 2,
            mixins = [
                "has_native_resultdb_integration",
            ],
            timeout_sec = 10800,
            shards = 2,
        ),
    },
)

# This target should usually be the same as `lacros_skylab_tests`. We use
# a different target for version skew so we can easily disable all version skew
# tests during an outage.
targets.legacy_basic_suite(
    name = "lacros_skylab_tests_version_skew",
    tests = {
        "lacros_all_tast_tests": targets.legacy_test_config(
            tast_expr = "(\"group:mainline\" && (\"dep:lacros_stable\" || \"dep:lacros\") && !informational)",
            test_level_retries = 2,
            mixins = [
                "has_native_resultdb_integration",
            ],
            timeout_sec = 10800,
            shards = 2,
        ),
    },
)

targets.legacy_basic_suite(
    name = "lacros_skylab_tests_with_gtests",
    tests = {
        "chromeos_integration_tests": targets.legacy_test_config(),
    },
)

# This target should usually be the same as `lacros_skylab_tests_with_gtests`. We use
# a different target for version skew so we can easily disable all version skew
# tests during an outage.
targets.legacy_basic_suite(
    name = "lacros_skylab_tests_with_gtests_version_skew",
    tests = {
        "chromeos_integration_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "lacros_vm_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "leak_detection_isolated_scripts",
    tests = {
        "memory.leak_detection": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
                expiration_sec = 36000,
                hard_timeout_sec = 10800,
                io_timeout_sec = 3600,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_cfm_gtests",
    tests = {
        "chromeos_unittests": targets.legacy_test_config(),
        "unit_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "linux_chromeos_browser_tests_require_lacros",
    tests = {
        "browser_tests_require_lacros": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_chromeos_lacros_gtests",
    tests = {
        # Chrome OS (Ash) and Lacros only.
        "chromeos_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "linux_chromeos_oobe_specific_tests",
    tests = {
        # TODO(crbug.com/1071693): Merge this suite back in to the main
        # browser_tests when the tests no longer fail on MSAN.
        "oobe_only_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 20,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_chromeos_specific_gtests",
    tests = {
        # Chrome OS only.
        "ash_components_unittests": targets.legacy_test_config(),
        # TODO(crbug.com/1351793) Enable on CQ when stable.
        "ash_crosapi_tests": targets.legacy_test_config(
            ci_only = True,
        ),
        "ash_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "ash_webui_unittests": targets.legacy_test_config(),
        "aura_unittests": targets.legacy_test_config(),
        "chromeos_components_unittests": targets.legacy_test_config(),
        "exo_unittests": targets.legacy_test_config(),
        "gl_unittests_ozone": targets.legacy_test_config(),
        "keyboard_unittests": targets.legacy_test_config(),
        "ozone_gl_unittests": targets.legacy_test_config(
            args = [
                "--ozone-platform=headless",
            ],
        ),
        "ozone_unittests": targets.legacy_test_config(),
        "ozone_x11_unittests": targets.legacy_test_config(),
        "shell_encryption_unittests": targets.legacy_test_config(),
        "ui_chromeos_unittests": targets.legacy_test_config(),
        "usage_time_limit_unittests": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
        "wayland_client_perftests": targets.legacy_test_config(),
        "wayland_client_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "linux_flavor_specific_chromium_gtests",
    tests = {
        # Android, Chrome OS and Linux
        "sandbox_linux_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "linux_force_accessibility_gtests",
    tests = {
        "browser_tests": targets.legacy_test_config(
            args = [
                "--force-renderer-accessibility",
                "--test-launcher-filter-file=../../testing/buildbot/filters/accessibility-linux.browser_tests.filter",
            ],
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
        "content_browsertests": targets.legacy_test_config(
            args = [
                "--force-renderer-accessibility",
                "--test-launcher-filter-file=../../testing/buildbot/filters/accessibility-linux.content_browsertests.filter",
            ],
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
        "interactive_ui_tests": targets.legacy_test_config(
            args = [
                "--force-renderer-accessibility",
                "--test-launcher-filter-file=../../testing/buildbot/filters/accessibility-linux.interactive_ui_tests.filter",
            ],
            swarming = targets.swarming(
                shards = 6,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_lacros_chrome_browsertests_non_version_skew",
    tests = {
        "lacros_chrome_browsertests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/linux-lacros.lacros_chrome_browsertests.filter",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_lacros_chrome_browsertests_version_skew",
    tests = {
        "lacros_chrome_browsertests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/linux-lacros.lacros_chrome_browsertests.filter;../../testing/buildbot/filters/linux-lacros.lacros_chrome_browsertests.skew.filter",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_lacros_chrome_gtests",
    tests = {
        "browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 33,
            ),
        ),
        "lacros_chrome_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 6,
            ),
        ),
        "interactive_ui_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 6,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_lacros_chrome_interactive_ui_tests_version_skew",
    tests = {
        "interactive_ui_tests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/linux-lacros.interactive_ui_tests.filter;../../testing/buildbot/filters/linux-lacros.interactive_ui_tests.skew.filter",
            ],
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_lacros_specific_gtests",
    tests = {
        "ozone_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "linux_specific_chromium_isolated_scripts",
    tests = {
        # not_site_per_process_blink_web_tests provides coverage for
        # running Layout Tests without site-per-process.  This is the mode used
        # on Android and Android bots currently do not run the full set of
        # layout tests.  Running in this mode on linux compensates for lack of
        # direct Android coverage.
        "not_site_per_process_blink_web_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
            ),
        ),
        # not_site_per_process_blink_wpt_tests provides coverage for
        # running WPTs without site-per-process.  This is the mode used
        # on Android and Android bots currently do not run the full set of
        # layout tests.  Running in this mode on linux compensates for lack of
        # direct Android coverage.
        "not_site_per_process_blink_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "webdriver_wpt_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "linux_specific_xr_gtests",
    tests = {
        "xr_browser_tests": targets.legacy_test_config(),
    },
)

# TODO(crbug.com/1320449): Remove this set of test suites when LSan can be
# enabled Mac ASan bots. This list will be gradually filled with more tests
# until the bot has parity with ASan bots, and the ASan bot can then enable
# LSan and the mac-lsan-fyi-rel bot go away.
targets.legacy_basic_suite(
    name = "mac_lsan_fyi_gtests",
    tests = {
        "absl_hardening_tests": targets.legacy_test_config(),
        "accessibility_unittests": targets.legacy_test_config(),
        "app_shell_unittests": targets.legacy_test_config(),
        "base_unittests": targets.legacy_test_config(),
        "blink_heap_unittests": targets.legacy_test_config(),
        "blink_platform_unittests": targets.legacy_test_config(),
        "blink_unittests": targets.legacy_test_config(),
        "cc_unittests": targets.legacy_test_config(),
        "components_unittests": targets.legacy_test_config(),
        "content_unittests": targets.legacy_test_config(),
        "crashpad_tests": targets.legacy_test_config(),
        "cronet_unittests": targets.legacy_test_config(),
        "device_unittests": targets.legacy_test_config(),
        "net_unittests": targets.legacy_test_config(),
        # TODO(crbug.com/1459686): Enable this.
        # "rust_gtest_interop_unittests": None,
    },
)

targets.legacy_basic_suite(
    name = "mac_specific_chromium_gtests",
    tests = {
        "power_sampler_unittests": targets.legacy_test_config(),
        "sandbox_unittests": targets.legacy_test_config(),
        "updater_tests": targets.legacy_test_config(),
        "xr_browser_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "mac_specific_isolated_scripts",
    tests = {
        "mac_signing_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "minidump_uploader_tests",
    tests = {
        "minidump_uploader_test": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "jni_zero_sample_apk_test",
    tests = {
        "test_sample_jni_apk": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "model_validation_tests",
    tests = {
        "model_validation_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            args = [
                "--out_dir=.",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "mojo_python_unittests_isolated_scripts",
    tests = {
        "mojo_python_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "monochrome_public_apk_checker_isolated_script",
    tests = {
        "monochrome_public_apk_checker": targets.legacy_test_config(
            remove_mixins = [
                "android_r",
                "bullhead",
                "flame",
                "marshmallow",
                "mdarcy",
                "oreo_fleet",
                "oreo_mr1_fleet",
                "pie_fleet",
                "walleye",
            ],
            swarming = targets.swarming(
                dimensions = {
                    "os": "Ubuntu-22.04",
                    "cpu": "x86-64",
                    "device_os": None,
                    "device_os_flavor": None,
                    "device_playstore_version": None,
                    "device_type": None,
                },
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "network_sandbox_browser_tests",
    tests = {
        "browser_tests_network_sandbox": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "components_browsertests_network_sandbox": targets.legacy_test_config(),
        "content_browsertests_network_sandbox": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "extensions_browsertests_network_sandbox": targets.legacy_test_config(),
        "interactive_ui_tests_network_sandbox": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "sync_integration_tests_network_sandbox": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "network_service_fyi_gtests",
    tests = {
        "network_service_web_request_proxy_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 15,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "non_android_and_cast_and_chromeos_chromium_gtests",
    tests = {
        "cronet_tests": targets.legacy_test_config(),
        "cronet_unittests": targets.legacy_test_config(),
        "headless_browsertests": targets.legacy_test_config(),
        "headless_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "non_android_chromium_gtests",
    tests = {
        "accessibility_unittests": targets.legacy_test_config(),
        "app_shell_unittests": targets.legacy_test_config(),
        "blink_fuzzer_unittests": targets.legacy_test_config(),
        "browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "chrome_app_unittests": targets.legacy_test_config(),
        "chromedriver_unittests": targets.legacy_test_config(),
        "extensions_browsertests": targets.legacy_test_config(),
        "extensions_unittests": targets.legacy_test_config(),
        "filesystem_service_unittests": targets.legacy_test_config(),  # https://crbug.com/862712
        "interactive_ui_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "message_center_unittests": targets.legacy_test_config(),
        "nacl_loader_unittests": targets.legacy_test_config(),
        "native_theme_unittests": targets.legacy_test_config(),
        "pdf_unittests": targets.legacy_test_config(),
        "ppapi_unittests": targets.legacy_test_config(),
        "printing_unittests": targets.legacy_test_config(),
        "remoting_unittests": targets.legacy_test_config(),
        "snapshot_unittests": targets.legacy_test_config(),
        "sync_integration_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "ui_unittests": targets.legacy_test_config(),
        "views_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "non_android_chromium_gtests_no_nacl",
    tests = {
        "accessibility_unittests": targets.legacy_test_config(),
        "app_shell_unittests": targets.legacy_test_config(),
        "blink_fuzzer_unittests": targets.legacy_test_config(),
        "browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "chrome_app_unittests": targets.legacy_test_config(),
        "chromedriver_unittests": targets.legacy_test_config(),
        "extensions_browsertests": targets.legacy_test_config(),
        "extensions_unittests": targets.legacy_test_config(),
        "filesystem_service_unittests": targets.legacy_test_config(),  # https://crbug.com/862712
        "interactive_ui_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "message_center_unittests": targets.legacy_test_config(),
        "native_theme_unittests": targets.legacy_test_config(),
        "pdf_unittests": targets.legacy_test_config(),
        "printing_unittests": targets.legacy_test_config(),
        "remoting_unittests": targets.legacy_test_config(),
        "snapshot_unittests": targets.legacy_test_config(),
        "sync_integration_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "ui_unittests": targets.legacy_test_config(),
        "views_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "non_android_chromium_gtests_skia_gold",
    tests = {
        "views_examples_unittests": targets.legacy_test_config(
            mixins = [
                "skia_gold_test",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "ondevice_stability_tests",
    tests = {
        "ondevice_stability_tests": targets.legacy_test_config(
            mixins = [
                "has_native_resultdb_integration",
            ],
            linux_args = [
                "--chromedriver",
                "chromedriver",
                "--binary",
                "chrome",
            ],
            mac_args = [
                "--chromedriver",
                "chromedriver",
                "--binary",
                "Google Chrome.app/Contents/MacOS/Google Chrome",
            ],
            win_args = [
                "--chromedriver",
                "chromedriver.exe",
                "--binary",
                "Chrome.exe",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "optimization_guide_android_gtests",
    tests = {
        "optimization_guide_components_unittests": targets.legacy_test_config(),
        # TODO(mgeorgaklis): Add optimization_guide_unittests when they become Android compatible.
    },
)

targets.legacy_basic_suite(
    name = "optimization_guide_nogpu_gtests",
    tests = {
        "chrome_ml_unittests": targets.legacy_test_config(),
        "optimization_guide_browser_tests": targets.legacy_test_config(),
        "optimization_guide_components_unittests": targets.legacy_test_config(),
        "optimization_guide_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "optimization_guide_gpu_gtests",
    tests = {
        "optimization_guide_gpu_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "perfetto_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
        "browser_tests": targets.legacy_test_config(
            args = [
                "--gtest_filter=ChromeTracingDelegateBrowserTest.*",
            ],
        ),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
            ),
            android_swarming = targets.swarming(
                shards = 15,
            ),
        ),
        "perfetto_unittests": targets.legacy_test_config(),
        "services_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "perfetto_gtests_android",
    tests = {
        "android_browsertests": targets.legacy_test_config(
            args = [
                "--gtest_filter=StartupMetricsTest.*",
            ],
        ),
        "base_unittests": targets.legacy_test_config(),
        "content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 8,
            ),
            android_swarming = targets.swarming(
                shards = 15,
            ),
        ),
        "perfetto_unittests": targets.legacy_test_config(),
        "services_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "performance_smoke_test_isolated_scripts",
    tests = {
        "performance_test_suite": targets.legacy_test_config(
            args = [
                "--pageset-repeat=1",
                "--test-shard-map-filename=smoke_test_benchmark_shard_map.json",
            ],
            swarming = targets.swarming(
                shards = 2,
                hard_timeout_sec = 960,
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "pixel_browser_tests_gtests",
    tests = {
        "pixel_browser_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "pixel_interactive_ui_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "pixel_experimental_browser_tests_gtests",
    tests = {
        "pixel_experimental_browser_tests": targets.legacy_test_config(
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "private_code_failure_test_isolated_scripts",
    tests = {
        "private_code_failure_test": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "pytype_tests",
    tests = {
        "blink_pytype": targets.legacy_test_config(),
        "fuchsia_pytype": targets.legacy_test_config(),
        "gold_common_pytype": targets.legacy_test_config(),
        "gpu_pytype": targets.legacy_test_config(),
        "testing_pytype": targets.legacy_test_config(),
    },
)

# Rust tests run on all targets.
targets.legacy_basic_suite(
    name = "rust_common_gtests",
    tests = {
        "base_unittests": targets.legacy_test_config(),
        # For go/rusty-qr-code-generator
        "components_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/rust.components_unittests.filter",
            ],
        ),
        "mojo_rust_integration_unittests": targets.legacy_test_config(),
        "mojo_rust_unittests": targets.legacy_test_config(),
        "rust_gtest_interop_unittests": targets.legacy_test_config(),
        "test_cpp_including_rust_unittests": targets.legacy_test_config(),
        "test_serde_json_lenient": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "rust_native_tests",
    tests = {
        "build_rust_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "site_isolation_android_fyi_gtests",
    tests = {
        "site_per_process_android_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 1,
            ),
        ),
        "site_per_process_chrome_public_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 20,
            ),
        ),
        "site_per_process_chrome_public_unit_test_apk": targets.legacy_test_config(),
        "site_per_process_components_browsertests": targets.legacy_test_config(),
        "site_per_process_components_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 5,
            ),
        ),
        "site_per_process_content_browsertests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
        "site_per_process_content_shell_test_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
        ),
        "site_per_process_content_unittests": targets.legacy_test_config(),
        "site_per_process_unit_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "swangle_gtests",
    tests = {
        "angle_deqp_egl_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles2_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles31_rotate180_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles31_rotate270_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles31_rotate90_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles31_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            swarming = targets.swarming(
                shards = 10,
            ),
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles3_rotate180_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles3_rotate270_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles3_rotate90_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_gles3_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            swarming = targets.swarming(
                shards = 4,
            ),
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_khr_gles2_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_khr_gles31_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_deqp_khr_gles3_tests": targets.legacy_test_config(
            args = [
                "--use-angle=swiftshader",
            ],
            use_isolated_scripts_api = True,
        ),
        "angle_end2end_tests": targets.legacy_test_config(
            args = [
                "--gtest_filter=*Vulkan_SwiftShader*",
            ],
            use_isolated_scripts_api = True,
        ),
    },
)

targets.legacy_basic_suite(
    name = "system_webview_shell_instrumentation_tests",
    tests = {
        "system_webview_shell_layout_test_apk": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "system_webview_wpt",
    tests = {
        "system_webview_wpt": targets.legacy_test_config(
            args = [
                "--no-wpt-internal",
            ],
            swarming = targets.swarming(
                shards = 25,
                expiration_sec = 18000,
                hard_timeout_sec = 14400,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "telemetry_android_minidump_unittests_isolated_scripts",
    tests = {
        "telemetry_chromium_minidump_unittests": targets.legacy_test_config(),
        "telemetry_monochrome_minidump_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "telemetry_desktop_minidump_unittests_isolated_scripts",
    tests = {
        # Takes ~2.5 minutes of bot time to run.
        "telemetry_desktop_minidump_unittests": targets.legacy_test_config(
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "telemetry_perf_unittests_isolated_scripts",
    tests = {
        "telemetry_perf_unittests": targets.legacy_test_config(
            args = [
                # TODO(crbug.com/1077284): Remove this once Crashpad is the default.
                "--extra-browser-args=--enable-crashpad",
            ],
            swarming = targets.swarming(
                shards = 12,
                idempotent = False,  # https://crbug.com/549140
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "telemetry_perf_unittests_isolated_scripts_android",
    tests = {
        "telemetry_perf_unittests_android_chrome": targets.legacy_test_config(
            args = [
                # TODO(crbug.com/1077284): Remove this once Crashpad is the default.
                "--extra-browser-args=--enable-crashpad",
            ],
            swarming = targets.swarming(
                shards = 12,
                idempotent = False,  # https://crbug.com/549140
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "telemetry_perf_unittests_isolated_scripts_xvfb",
    tests = {
        "telemetry_perf_unittests": targets.legacy_test_config(
            args = [
                # TODO(crbug.com/1077284): Remove this once Crashpad is the default.
                "--extra-browser-args=--enable-crashpad",
                "--xvfb",
            ],
            swarming = targets.swarming(
                shards = 12,
                idempotent = False,  # https://crbug.com/549140
            ),
            resultdb = targets.resultdb(
                enable = True,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "test_traffic_annotation_auditor_script",
    tests = {
        "test_traffic_annotation_auditor": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "updater_gtests_linux",
    tests = {
        "updater_tests": targets.legacy_test_config(
            mixins = [
                "updater-default-pool",
            ],
        ),
        # 'updater_tests_system' is not yet supported on Linux.
    },
)

targets.legacy_basic_suite(
    name = "updater_gtests_mac",
    tests = {
        "updater_tests": targets.legacy_test_config(
            mixins = [
                "updater-default-pool",
            ],
        ),
        "updater_tests_system": targets.legacy_test_config(
            mixins = [
                "updater-mac-pool",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "updater_gtests_win",
    tests = {
        "updater_tests": targets.legacy_test_config(
            mixins = [
                "integrity_high",
                "updater-default-pool",
            ],
        ),
        "updater_tests_system": targets.legacy_test_config(
            mixins = [
                "integrity_high",
                "updater-default-pool",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "updater_gtests_win_uac",
    tests = {
        "updater_tests_system": targets.legacy_test_config(
            mixins = [
                "integrity_high",
                "updater-win-uac-pool",
            ],
        ),
        "updater_tests_win_uac": targets.legacy_test_config(
            mixins = [
                "updater-win-uac-pool",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "upload_perfetto",
    tests = {
        "upload_trace_processor": targets.legacy_test_config(),
    },
)

# Not applicable for android x86 & x64 since the targets here assert
# "enable_vr" in GN which is only true for android arm & arm64.
# For details, see the following files:
#  * //chrome/android/BUILD.gn
#  * //chrome/browser/android/vr/BUILD.gn
#  * //device/vr/buildflags/buildflags.gni
targets.legacy_basic_suite(
    name = "vr_android_specific_chromium_tests",
    tests = {
        "chrome_public_test_vr_apk": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
        "vr_android_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "vr_platform_specific_chromium_gtests",
    tests = {
        # Only run on platforms that intend to support WebVR in the near
        # future.
        "vr_common_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "vulkan_swiftshader_isolated_scripts",
    tests = {
        "vulkan_swiftshader_blink_web_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "web_engine_gtests",
    tests = {
        "cast_runner_browsertests": targets.legacy_test_config(),
        "cast_runner_integration_tests": targets.legacy_test_config(),
        "cast_runner_unittests": targets.legacy_test_config(),
        "web_engine_browsertests": targets.legacy_test_config(),
        "web_engine_integration_tests": targets.legacy_test_config(),
        "web_engine_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webrtc_chromium_gtests",
    tests = {
        "browser_tests": targets.legacy_test_config(
            args = [
                "--gtest_filter=WebRtcStatsPerfBrowserTest.*:WebRtcVideoDisplayPerfBrowserTests*:WebRtcVideoQualityBrowserTests*:WebRtcVideoHighBitrateBrowserTest*:WebRtcWebcamBrowserTests*",
                "--run-manual",
                "--ui-test-action-max-timeout=300000",
                "--test-launcher-timeout=350000",
                "--test-launcher-jobs=1",
                "--test-launcher-bot-mode",
                "--test-launcher-print-test-stdio=always",
            ],
        ),
        # TODO(b/246519185) - Py3 incompatible, decide if to keep test.:
        # "browser_tests_apprtc": targets.legacy_test_config(
        #     args = [
        #         "--gtest_filter=WebRtcApprtcBrowserTest.*",
        #         "--run-manual",
        #         "--test-launcher-jobs=1",
        #     ],
        # ),
        "browser_tests_functional": targets.legacy_test_config(),
        # Run all normal WebRTC content_browsertests. This is mostly so
        # the FYI bots can detect breakages.
        "content_browsertests": targets.legacy_test_config(
            args = [
                "--gtest_filter=WebRtc*",
            ],
        ),
        "content_browsertests_sequential": targets.legacy_test_config(),
        "content_browsertests_stress": targets.legacy_test_config(),
        "content_unittests": targets.legacy_test_config(
            args = [
                "--test-launcher-filter-file=../../testing/buildbot/filters/webrtc.content_unittests.filter",
            ],
        ),
        "remoting_unittests": targets.legacy_test_config(
            args = [
                "--gtest_filter=Webrtc*",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "webrtc_chromium_simple_gtests",
    tests = {
        "content_browsertests": targets.legacy_test_config(
            args = [
                "--gtest_filter=WebRtc*",
            ],
        ),
        "content_browsertests_sequential": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webrtc_chromium_wpt_tests",
    tests = {
        "blink_wpt_tests": targets.legacy_test_config(
            args = [
                "-t",
                "Release",
                "external/wpt/webrtc",
                "external/wpt/webrtc-encoded-transform",
                "external/wpt/webrtc-extensions",
                "external/wpt/webrtc-priority",
                "external/wpt/webrtc-stats",
                "external/wpt/webrtc-svc",
            ],
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_64_cts_tests_gtest",
    tests = {
        "webview_64_cts_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_bot_instrumentation_test_apk_gtest",
    tests = {
        "webview_instrumentation_test_apk": targets.legacy_test_config(
            args = [
                "--use-apk-under-test-flags-file",
            ],
            swarming = targets.swarming(
                shards = 12,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_bot_instrumentation_test_apk_mutations_gtest",
    tests = {
        "webview_instrumentation_test_apk_mutations": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 12,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_bot_instrumentation_test_apk_no_field_trial_gtest",
    tests = {
        "webview_instrumentation_test_apk_no_field_trial": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 12,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_bot_unittests_gtest",
    tests = {
        "android_webview_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webview_cts_tests_gtest",
    tests = {
        "webview_cts_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_cts_tests_gtest_no_field_trial",
    tests = {
        "webview_cts_tests_no_field_trial": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_trichrome_64_cts_field_trial_tests",
    tests = {
        "webview_trichrome_64_cts_tests": targets.legacy_test_config(
            args = [
                "--store-data-dependencies-in-temp",
            ],
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_trichrome_64_cts_hostside_tests",
    tests = {
        "webview_trichrome_64_cts_hostside_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webview_trichrome_64_cts_tests",
    tests = {
        "webview_trichrome_64_cts_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_trichrome_64_cts_tests_no_field_trial",
    tests = {
        "webview_trichrome_64_cts_tests_no_field_trial": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 2,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "webview_trichrome_cts_tests",
    tests = {
        "webview_trichrome_cts_tests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webview_ui_instrumentation_tests",
    tests = {
        "webview_ui_test_app_test_apk": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "webview_ui_instrumentation_tests_no_field_trial",
    tests = {
        "webview_ui_test_app_test_apk_no_field_trial": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "win_specific_chromium_gtests",
    tests = {
        "chrome_elf_unittests": targets.legacy_test_config(),
        "courgette_unittests": targets.legacy_test_config(),
        "delayloads_unittests": targets.legacy_test_config(),
        "elevation_service_unittests": targets.legacy_test_config(),
        "gcp_unittests": targets.legacy_test_config(),
        "install_static_unittests": targets.legacy_test_config(),
        "installer_util_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "integrity": "high",
                },
            ),
        ),
        "notification_helper_unittests": targets.legacy_test_config(),
        "sbox_integration_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "integrity": "high",
                },
            ),
        ),
        "sbox_unittests": targets.legacy_test_config(),
        "sbox_validation_tests": targets.legacy_test_config(),
        "setup_unittests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "integrity": "high",
                },
            ),
        ),
        "updater_tests": targets.legacy_test_config(),
        "updater_tests_system": targets.legacy_test_config(),
        "zucchini_unittests": targets.legacy_test_config(),
    },
)

targets.legacy_basic_suite(
    name = "win_specific_isolated_scripts",
    tests = {
        "mini_installer_tests": targets.legacy_test_config(
            swarming = targets.swarming(
                dimensions = {
                    "integrity": "high",
                },
            ),
        ),
        "polymer_tools_python_unittests": targets.legacy_test_config(
            experiment_percentage = 0,
        ),
    },
)

targets.legacy_basic_suite(
    name = "win_specific_xr_perf_tests",
    tests = {
        "xr.webxr.static": targets.legacy_test_config(
            # Experimental until we're sure these are stable.
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_tests_ios",
    tests = {
        "wpt_tests_ios": targets.legacy_test_config(
            args = [
                "--no-wpt-internal",
            ],
            swarming = targets.swarming(
                shards = 36,
                expiration_sec = 18000,
                hard_timeout_sec = 14400,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_web_tests_android",
    tests = {
        "chrome_public_wpt": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 15,
            ),
        ),
        "system_webview_wpt": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 15,
            ),
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_web_tests_content_shell",
    tests = {
        "wpt_tests_suite": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 15,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_web_tests_enable_leak_detection",
    tests = {
        "wpt_tests_suite": targets.legacy_test_config(
            args = [
                "--enable-leak-detection",
            ],
            swarming = targets.swarming(
                shards = 15,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_web_tests_highdpi",
    tests = {
        "wpt_tests_suite_highdpi": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 3,
            ),
            experiment_percentage = 100,
        ),
    },
)

targets.legacy_basic_suite(
    name = "wpt_web_tests_not_site_per_process",
    tests = {
        "wpt_tests_suite_not_site_per_process": targets.legacy_test_config(
            swarming = targets.swarming(
                shards = 10,
            ),
            experiment_percentage = 100,
        ),
    },
)
