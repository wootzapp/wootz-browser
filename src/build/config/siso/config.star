# -*- bazel-starlark -*-
# Copyright 2023 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""Config module for checking siso -config flags."""

load("@builtin//struct.star", "module")

__KNOWN_CONFIG_OPTIONS = [
    # Indicates that the build runs on a builder.
    "builder",

    # TODO: b/308405411 - Enable this config for all builders.
    "remote-devtools-frontend-typescript",

    # TODO: b/316267242 - Enable remote links after confirming performance.
    "remote-library-link",
    "remote-exec-link",
]

def __check(ctx):
    if "config" in ctx.flags:
        for cfg in ctx.flags["config"].split(","):
            if cfg not in __KNOWN_CONFIG_OPTIONS:
                print("unknown config: %s" % cfg)

def __get(ctx, key):
    if "config" in ctx.flags:
        for cfg in ctx.flags["config"].split(","):
            if cfg == key:
                return True
    return False

config = module(
    "config",
    check = __check,
    get = __get,
)
