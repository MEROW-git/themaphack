#pragma once

#include <string>

namespace startup {

constexpr int kJniError = -1;

struct Paths {
    std::string source;
    std::string copied;
};

inline bool BuildPaths(const std::string& package, const std::string& abi, Paths& paths) {
    if (package.empty() || package.size() > 255 || abi != "arm64-v8a") return false;
    for (char ch : package) {
        const bool safe = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                          (ch >= '0' && ch <= '9') || ch == '.' || ch == '_';
        if (!safe) return false;
    }
    paths.source = "/sdcard/Android/data/" + package +
                   "/files/dragon2017/assets/comlibs/" + abi + "/libTMH.bytes";
    paths.copied = "/data/user/0/" + package + "/app_libs/libTMH.bytes";
    return true;
}

struct Operations {
    bool (*copy)(const char*, const char*);
    void* (*open)(const char*);
    void* (*symbol)(void*, const char*);
    int (*invoke)(void*, void*, void*);
};

enum class Status {
    InvalidInput,
    OriginalUnavailable,
    EntryMissing,
    SelfForward,
    OriginalRejected,
    UnsupportedVersion,
    Success
};

inline const char* StatusName(Status status) {
    switch (status) {
        case Status::InvalidInput: return "invalid input";
        case Status::OriginalUnavailable: return "original library unavailable";
        case Status::EntryMissing: return "JNI_OnLoad missing";
        case Status::SelfForward: return "self-forwarding detected";
        case Status::OriginalRejected: return "original JNI_OnLoad returned JNI_ERR";
        case Status::UnsupportedVersion: return "unsupported JNI version";
        case Status::Success: return "success";
    }
    return "unknown";
}

struct Result {
    Status status = Status::InvalidInput;
    int jniVersion = kJniError;
    bool copySucceeded = false;
    bool usedCopy = false;
};

inline Result ForwardOriginal(const Paths& paths, void* vm, void* reserved,
                              void* ownEntry, const Operations& ops) {
    Result result;
    if (paths.source.empty() || paths.copied.empty() || !vm ||
        !ops.copy || !ops.open || !ops.symbol || !ops.invoke) return result;

    // A failed copy must never make an older destination file the first choice.
    result.copySucceeded = ops.copy(paths.source.c_str(), paths.copied.c_str());
    void* handle = result.copySucceeded ? ops.open(paths.copied.c_str()) : nullptr;
    result.usedCopy = handle != nullptr;
    if (!handle) handle = ops.open(paths.source.c_str());
    if (!handle) {
        result.status = Status::OriginalUnavailable;
        return result;
    }

    void* entry = ops.symbol(handle, "JNI_OnLoad");
    if (!entry) {
        result.status = Status::EntryMissing;
        return result;
    }
    if (entry == ownEntry) {
        result.status = Status::SelfForward;
        return result;
    }

    result.jniVersion = ops.invoke(entry, vm, reserved);
    if (result.jniVersion == kJniError) {
        result.status = Status::OriginalRejected;
    } else if (result.jniVersion != 0x00010001 && result.jniVersion != 0x00010002 &&
               result.jniVersion != 0x00010004 && result.jniVersion != 0x00010006) {
        result.status = Status::UnsupportedVersion;
    } else {
        result.status = Status::Success;
    }
    return result;
}

}  // namespace startup
