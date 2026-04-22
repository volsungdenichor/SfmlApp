load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def _sfml_extension_impl(module_ctx):
    # SFML 3.0.1 — mirrors the GIT_TAG in dependencies.cmake.
    # To pin the integrity hash (recommended for reproducible builds), run:
    #   bazel fetch //...
    # Bazel will report the correct hash; then add e.g.:
    #   integrity = "sha256-<base64-encoded-sha256>",
    http_archive(
        name = "sfml_src",
        urls = ["https://github.com/SFML/SFML/archive/refs/tags/3.0.1.tar.gz"],
        strip_prefix = "SFML-3.0.1",
        # Expose all sources as a single filegroup consumed by the cmake() rule.
        build_file_content = """\
filegroup(
    name = "all_srcs",
    srcs = glob(
        ["**"],
        exclude = ["BUILD", "BUILD.bazel", "WORKSPACE", "WORKSPACE.bazel"],
    ),
    visibility = ["//visibility:public"],
)
""",
    )

sfml_extension = module_extension(
    implementation = _sfml_extension_impl,
)
