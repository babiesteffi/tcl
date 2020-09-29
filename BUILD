# Copyright 2020 Google LLC
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Supporting infrastructure for implementing and testing PINS.

# load("//tools/build_defs/testing:bzl_library.bzl", "bzl_library")

package(
    default_visibility = ["//visibility:public"],
    licenses = ["notice"],
)

exports_files(["LICENSE"])

# copybara:insert_begin(For format.sh script)
# load("@com_github_bazelbuild_buildtools//buildifier:def.bzl", "buildifier")
#
# buildifier(
#     name = "buildifier",
# )
# copybara:insert_end

# This is the libtcl
cc_library(
    name = "tcl",
    srcs = [
        "tcl8.6.4/generic/regcomp.c",
        "tcl8.6.4/generic/regerror.c",
        "tcl8.6.4/generic/regexec.c",
        "tcl8.6.4/generic/regfree.c",
        "tcl8.6.4/unix/tclLoadDl.c",
    ] + glob(
        [
            "tcl8.6.4/generic/tcl*.c",
            "tcl8.6.4/unix/tcl*.c",
            "tcl8.6.4/libtommath/*.c",
        ],
        exclude = [
            "tcl8.6.4/generic/tclLoadNone.c",
            "tcl8.6.4/generic/tclPkgConfig.c",
            "tcl8.6.4/unix/tclAppInit.c",
            "tcl8.6.4/unix/tclLoad*.c",
            "tcl8.6.4/unix/tclUnixInit.c",
            "tcl8.6.4/unix/tclXtNotify.c",
            "tcl8.6.4/unix/tclXtTest.c",
        ],
    ),
    hdrs = glob([
        "tcl8.6.4/generic/*.h",
        "tcl8.6.4/generic/reg*.c",
        "tcl8.6.4/libtommath/*.h",
    ]),
#    compatible_with = ["//buildenv/target:vendor"],
    copts = [
        "-w",
        varref("DEFINES1"),
        varref("DEFINES2"),
        varref("DEFINES3"),
        varref("DEFINES4"),
        varref("DEFINES5"),
        varref("DEFINES6"),
        varref("DEFINES7"),
        varref("DEFINES8"),
        varref("DEFINES9"),
        varref("DEFINES10"),
        varref("FLAGS1"),
        varref("FLAGS2"),
        "$(STACK_FRAME_UNLIMITED)",  # regexec.c
        "-DHAVE_ZLIB=",
    ],
    includes = [
        "tcl8.6.4/generic/",
        "tcl8.6.4/libtommath/",
        "tcl8.6.4/unix/",
        "tcl8.6.4/xlib/",
    ],
    linkopts = ["-ldl"],
    textual_hdrs = glob([
        "tcl8.6.4/generic/*.decls",
    ]),
    deps = [
        ":tclAlloc",
        ":tclPkgConfig",
        ":tclUnixInit",
        "//third_party/zlib:zlibonly",
    ],
)
