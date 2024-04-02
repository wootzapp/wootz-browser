# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Definitions of builders in the chromium.win builder group."""

load("//lib/args.star", "args")
load("//lib/branches.star", "branches")
load("//lib/builder_config.star", "builder_config")
load("//lib/builder_health_indicators.star", "health_spec")
load("//lib/builders.star", "os", "reclient", "sheriff_rotations")
load("//lib/ci.star", "ci")
load("//lib/consoles.star", "consoles")
load("//lib/gn_args.star", "gn_args")

ci.defaults.set(
    executable = ci.DEFAULT_EXECUTABLE,
    builder_group = "chromium.win",
    pool = ci.DEFAULT_POOL,
    cores = 8,
    os = os.WINDOWS_DEFAULT,
    sheriff_rotations = sheriff_rotations.CHROMIUM,
    tree_closing = True,
    main_console_view = "main",
    contact_team_email = "chrome-desktop-engprod@google.com",
    execution_timeout = ci.DEFAULT_EXECUTION_TIMEOUT,
    health_spec = health_spec.DEFAULT,
    reclient_instance = reclient.instance.DEFAULT_TRUSTED,
    reclient_jobs = reclient.jobs.DEFAULT,
    service_account = ci.DEFAULT_SERVICE_ACCOUNT,
    shadow_service_account = ci.DEFAULT_SHADOW_SERVICE_ACCOUNT,
)

consoles.console_view(
    name = "chromium.win",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    ordering = {
        None: ["release", "debug"],
        "debug|builder": consoles.ordering(short_names = ["64", "32"]),
        "debug|tester": consoles.ordering(short_names = ["7", "10"]),
    },
)

# TODO(gbeaty) Investigate if the testers need to run on windows, if not, switch
# them to ci.thin_tester

ci.builder(
    name = "WebKit Win10",
    triggered_by = ["Win Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    builderless = False,
    console_view_entry = consoles.console_view_entry(
        category = "misc",
        short_name = "wbk",
    ),
)

ci.builder(
    name = "Win Builder",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "release_builder",
            "reclient",
            "x86",
            "no_symbols",
        ],
    ),
    builderless = False,
    cores = 32,
    os = os.WINDOWS_ANY,
    console_view_entry = consoles.console_view_entry(
        category = "release|builder",
        short_name = "32",
    ),
)

ci.builder(
    name = "Win x64 Builder (dbg)",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "debug_builder",
            "reclient",
        ],
    ),
    builderless = True,
    cores = 32,
    os = os.WINDOWS_ANY,
    console_view_entry = consoles.console_view_entry(
        category = "debug|builder",
        short_name = "64",
    ),
)

ci.builder(
    name = "Win10 Tests x64 (dbg)",
    triggered_by = ["Win x64 Builder (dbg)"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    # Too flaky. See crbug.com/876224 for more details.
    sheriff_rotations = args.ignore_default(None),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "debug|tester",
        short_name = "10",
    ),
)

ci.builder(
    name = "Win Builder (dbg)",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_bits = 32,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "debug_builder",
            "reclient",
            "x86",
            "no_symbols",
        ],
    ),
    builderless = False,
    cores = 32,
    os = os.WINDOWS_ANY,
    console_view_entry = consoles.console_view_entry(
        category = "debug|builder",
        short_name = "32",
    ),
    cq_mirrors_console_view = "mirrors",
    # TODO(crbug/1473182): Remove once the bug is closed.
    reclient_bootstrap_env = {
        "RBE_experimental_exit_on_stuck_actions": "true",
    },
)

ci.builder(
    name = "Win x64 Builder",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "gpu_tests",
            "release_builder",
            "reclient",
            "minimal_symbols",
        ],
    ),
    builderless = False,
    cores = 32,
    os = os.WINDOWS_ANY,
    console_view_entry = consoles.console_view_entry(
        category = "release|builder",
        short_name = "64",
    ),
    cq_mirrors_console_view = "mirrors",
)

ci.builder(
    name = "Win10 Tests x64",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    triggered_by = ["ci/Win x64 Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    builderless = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|tester",
        short_name = "w10",
    ),
    cq_mirrors_console_view = "mirrors",
)

ci.thin_tester(
    name = "Win11 Tests x64",
    triggered_by = ["ci/Win x64 Builder"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    builder_config_settings = builder_config.ci_settings(
        retry_failed_shards = True,
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|tester",
        short_name = "w11",
    ),
)

ci.builder(
    name = "win-arm64-rel",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    description_html = "Windows ARM64 Release Builder.",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "arm64",
            "gpu_tests",
            "release_builder",
            "reclient",
            "minimal_symbols",
        ],
    ),
    builderless = False,
    cores = 16,
    os = os.WINDOWS_DEFAULT,
    tree_closing = True,
    console_view_entry = consoles.console_view_entry(
        category = "release|builder",
        short_name = "a64",
    ),
    cq_mirrors_console_view = "mirrors",
    contact_team_email = "chrome-desktop-engprod@google.com",
)

ci.thin_tester(
    name = "win11-arm64-rel-tests",
    branch_selector = branches.selector.WINDOWS_BRANCHES,
    description_html = "Windows11 ARM64 Release Tester.",
    triggered_by = ["ci/win-arm64-rel"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
            apply_configs = [
                "use_clang_coverage",
            ],
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.RELEASE,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    builder_config_settings = builder_config.ci_settings(
        retry_failed_shards = True,
    ),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "release|tester",
        short_name = "a64",
    ),
    contact_team_email = "chrome-desktop-engprod@google.com",
)

ci.builder(
    name = "win-arm64-dbg",
    description_html = "Windows ARM64 Debug Builder.",
    builder_spec = builder_config.builder_spec(
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    gn_args = gn_args.config(
        configs = [
            "arm64",
            "gpu_tests",
            "debug_builder",
            "reclient",
        ],
    ),
    builderless = True,
    cores = 32,
    os = os.WINDOWS_DEFAULT,
    tree_closing = True,
    console_view_entry = consoles.console_view_entry(
        category = "debug|builder",
        short_name = "a64",
    ),
    contact_team_email = "chrome-desktop-engprod@google.com",
)

ci.thin_tester(
    name = "win11-arm64-dbg-tests",
    description_html = "Windows11 ARM64 Debug Tester.",
    triggered_by = ["ci/win-arm64-dbg"],
    builder_spec = builder_config.builder_spec(
        execution_mode = builder_config.execution_mode.TEST,
        gclient_config = builder_config.gclient_config(
            config = "chromium",
        ),
        chromium_config = builder_config.chromium_config(
            config = "chromium",
            apply_configs = [
                "mb",
            ],
            build_config = builder_config.build_config.DEBUG,
            target_arch = builder_config.target_arch.ARM,
            target_bits = 64,
            target_platform = builder_config.target_platform.WIN,
        ),
        build_gs_bucket = "chromium-win-archive",
    ),
    builder_config_settings = builder_config.ci_settings(
        retry_failed_shards = True,
    ),
    # TODO(crbug.com/1383380): Enable sheriff when stable and green.
    sheriff_rotations = args.ignore_default(None),
    tree_closing = False,
    console_view_entry = consoles.console_view_entry(
        category = "debug|tester",
        short_name = "a64",
    ),
    contact_team_email = "chrome-desktop-engprod@google.com",
)

ci.builder(
    name = "Windows deterministic",
    executable = "recipe:swarming/deterministic_build",
    gn_args = gn_args.config(
        configs = [
            "release_builder",
            "reclient",
            "x86",
            "minimal_symbols",
        ],
    ),
    builderless = False,
    cores = 32,
    console_view_entry = consoles.console_view_entry(
        category = "misc",
        short_name = "det",
    ),
    execution_timeout = 12 * time.hour,
    reclient_bootstrap_env = {
        "RBE_ip_timeout": "10m",
    },
)
