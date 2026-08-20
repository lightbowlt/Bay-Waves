#include "target.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Target target_table[] = {
    { TARGET_WINDOWS_X64, "windows-x64", "x86_64-w64-mingw32", "pe", "windows", "x86_64" },
    { TARGET_WINDOWS_ARM64, "windows-arm64", "aarch64-w64-mingw32", "pe", "windows", "arm64" },
    { TARGET_LINUX_X64, "linux-x64", "x86_64-linux-gnu", "elf", "linux", "x86_64" },
    { TARGET_LINUX_ARM64, "linux-arm64", "aarch64-linux-gnu", "elf", "linux", "arm64" },
    { TARGET_MACOS_X64, "macos-x64", "x86_64-apple-darwin", "macho", "macos", "x86_64" },
    { TARGET_MACOS_ARM64, "macos-arm64", "aarch64-apple-darwin", "macho", "macos", "arm64" }
};

const char* target_kind_to_name(TargetKind kind) {
    for (size_t i = 0; i < sizeof(target_table) / sizeof(target_table[0]); ++i) {
        if (target_table[i].kind == kind) {
            return target_table[i].name;
        }
    }
    return "invalid";
}

const char* target_detect_host(void) {
#if defined(_WIN64)
    return "windows-x64";
#elif defined(__linux__)
#if defined(__aarch64__)
    return "linux-arm64";
#else
    return "linux-x64";
#endif
#elif defined(__APPLE__)
#if defined(__aarch64__)
    return "macos-arm64";
#else
    return "macos-x64";
#endif
#else
    return "linux-x64";
#endif
}

Target target_parse(const char* text) {
    Target target = { TARGET_INVALID, "invalid", NULL, NULL, NULL, NULL };
    if (text == NULL) {
        return target;
    }
    for (size_t i = 0; i < sizeof(target_table) / sizeof(target_table[0]); ++i) {
        if (strcmp(text, target_table[i].name) == 0) {
            return target_table[i];
        }
    }
    return target;
}

int target_is_valid(const char* text) {
    return target_parse(text).kind != TARGET_INVALID;
}

void target_print_supported(void) {
    printf("Supported targets:\n");
    for (size_t i = 0; i < sizeof(target_table) / sizeof(target_table[0]); ++i) {
        printf("  %s\n", target_table[i].name);
    }
}
