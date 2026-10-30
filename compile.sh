#!/usr/bin/env sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$project_dir"

if [ -n "${ANDROID_NDK_HOME:-}" ] && [ -x "$ANDROID_NDK_HOME/ndk-build" ]; then
    ndk_build="$ANDROID_NDK_HOME/ndk-build"
elif [ -n "${ANDROID_NDK_ROOT:-}" ] && [ -x "$ANDROID_NDK_ROOT/ndk-build" ]; then
    ndk_build="$ANDROID_NDK_ROOT/ndk-build"
elif command -v ndk-build >/dev/null 2>&1; then
    ndk_build="$(command -v ndk-build)"
elif [ -x "$HOME/ndk-aide/ndk-build" ]; then
    ndk_build="$HOME/ndk-aide/ndk-build"
else
    echo "ndk-build not found. Set ANDROID_NDK_HOME to an NDK that runs on this host, or add ndk-build to PATH." >&2
    exit 1
fi

mkdir -p "$project_dir/artifacts/logs"
log_file="$project_dir/artifacts/logs/ndk-build-arm64.log"
library="$project_dir/artifacts/libs/arm64-v8a/libAkSoundEngine2.so"

if {
    printf 'Host: %s\n' "$(uname -sm)"
    printf 'ndk-build: %s\n' "$ndk_build"
    "$ndk_build" -C "$project_dir" \
        "NDK_OUT=artifacts/obj" \
        "NDK_LIBS_OUT=artifacts/libs" \
        "$@" "APP_ABI=arm64-v8a"
} > "$log_file" 2>&1; then
    cat "$log_file"
else
    cat "$log_file" >&2
    exit 1
fi

if [ ! -s "$library" ]; then
    echo "Build reported success, but $library is missing or empty." >&2
    exit 1
fi

if command -v sha256sum >/dev/null 2>&1; then
    (cd "$project_dir/artifacts/libs/arm64-v8a" && sha256sum libAkSoundEngine2.so > libAkSoundEngine2.so.sha256)
fi
printf 'Artifact: %s\nBuild log: %s\n' "$library" "$log_file"
