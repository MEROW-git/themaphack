#include "../jni/StartupForwarder.h"

#include <cassert>
#include <cstring>
#include <string>

namespace {
struct Fake {
    bool copySucceeds = true;
    bool copiedOpens = true;
    bool sourceOpens = true;
    bool hasEntry = true;
    bool selfEntry = false;
    int originalVersion = 0x00010006;
    int copiedOpenCalls = 0;
    int sourceOpenCalls = 0;
    int invokeCalls = 0;
    std::string copiedPath;
    std::string sourcePath;
} fake;

bool Copy(const char* source, const char* copied) {
    fake.sourcePath = source;
    fake.copiedPath = copied;
    return fake.copySucceeds;
}

void* Open(const char* path) {
    if (fake.copiedPath == path) {
        ++fake.copiedOpenCalls;
        return fake.copiedOpens ? reinterpret_cast<void*>(1) : nullptr;
    }
    ++fake.sourceOpenCalls;
    return fake.sourceOpens ? reinterpret_cast<void*>(2) : nullptr;
}

void* Symbol(void*, const char* name) {
    assert(std::strcmp(name, "JNI_OnLoad") == 0);
    return fake.hasEntry ? reinterpret_cast<void*>(fake.selfEntry ? 4 : 3) : nullptr;
}

int Invoke(void*, void*, void*) {
    ++fake.invokeCalls;
    return fake.originalVersion;
}

startup::Result Run(const startup::Paths& paths) {
    const startup::Operations ops{Copy, Open, Symbol, Invoke};
    return startup::ForwardOriginal(paths, reinterpret_cast<void*>(5), nullptr,
                                    reinterpret_cast<void*>(4), ops);
}
}  // namespace

int main() {
    startup::Paths paths;
    assert(startup::BuildPaths("com.mobile.legends", "arm64-v8a", paths));
    assert(paths.source == "/sdcard/Android/data/com.mobile.legends/files/dragon2017/assets/comlibs/arm64-v8a/libTMH.bytes");
    assert(paths.copied == "/data/user/0/com.mobile.legends/app_libs/libTMH.bytes");
    assert(!startup::BuildPaths("../invalid", "arm64-v8a", paths));
    assert(!startup::BuildPaths("com.mobile.legends", "x86_64", paths));
    assert(startup::BuildPaths("com.mobile.legends", "arm64-v8a", paths));

    auto result = Run(paths);
    assert(result.status == startup::Status::Success && result.usedCopy);
    assert(result.jniVersion == 0x00010006 && fake.invokeCalls == 1);

    fake = Fake{};
    fake.copySucceeds = false;
    result = Run(paths);
    assert(result.status == startup::Status::Success && !result.usedCopy);
    assert(fake.copiedOpenCalls == 0 && fake.sourceOpenCalls == 1);

    fake = Fake{};
    fake.copiedOpens = false;
    result = Run(paths);
    assert(result.status == startup::Status::Success && !result.usedCopy);
    assert(fake.copiedOpenCalls == 1 && fake.sourceOpenCalls == 1);

    fake = Fake{};
    fake.copySucceeds = false;
    fake.sourceOpens = false;
    result = Run(paths);
    assert(result.status == startup::Status::OriginalUnavailable && fake.invokeCalls == 0);

    fake = Fake{};
    fake.hasEntry = false;
    result = Run(paths);
    assert(result.status == startup::Status::EntryMissing && fake.invokeCalls == 0);

    fake = Fake{};
    fake.selfEntry = true;
    result = Run(paths);
    assert(result.status == startup::Status::SelfForward && fake.invokeCalls == 0);

    fake = Fake{};
    fake.originalVersion = startup::kJniError;
    result = Run(paths);
    assert(result.status == startup::Status::OriginalRejected && fake.invokeCalls == 1);

    fake = Fake{};
    fake.originalVersion = 0;
    result = Run(paths);
    assert(result.status == startup::Status::UnsupportedVersion && fake.invokeCalls == 1);
}
