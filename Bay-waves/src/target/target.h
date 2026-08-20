#ifndef BAY_TARGET_H
#define BAY_TARGET_H

#include <stddef.h>

typedef enum {
    TARGET_INVALID = 0,
    TARGET_WINDOWS_X64,
    TARGET_WINDOWS_ARM64,
    TARGET_LINUX_X64,
    TARGET_LINUX_ARM64,
    TARGET_MACOS_X64,
    TARGET_MACOS_ARM64
} TargetKind;

typedef struct {
    TargetKind kind;
    const char* name;
    const char* triple;
    const char* object_format;
    const char* os_name;
    const char* cpu_name;
} Target;

const char* target_kind_to_name(TargetKind kind);
const char* target_detect_host(void);
Target target_parse(const char* text);
int target_is_valid(const char* text);
void target_print_supported(void);

#endif
