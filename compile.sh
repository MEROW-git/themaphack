#!/bin/sh
set -eu

cd "$(dirname "$0")"

if [ -n "${ANDROID_NDK_HOME:-}" ] && [ -x "$ANDROID_NDK_HOME/ndk-build" ]; then
    ndk_build="$ANDROID_NDK_HOME/ndk-build"
elif [ -n "${ANDROID_NDK_ROOT:-}" ] && [ -x "$ANDROID_NDK_ROOT/ndk-build" ]; then
    ndk_build="$ANDROID_NDK_ROOT/ndk-build"
elif command -v ndk-build >/dev/null 2>&1; then
    ndk_build="$(command -v ndk-build)"
elif [ -x "$HOME/ndk-aide/ndk-build" ]; then
    ndk_build="$HOME/ndk-aide/ndk-build"
else
    echo "Android NDK not found. Set ANDROID_NDK_HOME or add ndk-build to PATH." >&2
    exit 1
fi

exec "$ndk_build" "$@"
