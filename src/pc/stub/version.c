#include <stdio.h>
#include "version.h"
#include "types.h"

static char sVersionString[MAX_VERSION_LENGTH] = { 0 };

const char* get_version(void) {
#if 1
    snprintf(sVersionString, MAX_VERSION_LENGTH, "%s", SM64COOPDX_VERSION);
#else
    snprintf(sVersionString, MAX_VERSION_LENGTH, "%s %s", SM64COOPDX_VERSION, VERSION_REGION);
#endif
    return sVersionString;
}

#ifdef COMPILE_TIME
const char* get_version_with_build_date(void) {
#if 1
    snprintf(sVersionString, MAX_VERSION_LENGTH, "%s, %s", SM64COOPDX_VERSION, COMPILE_TIME);
#else
    snprintf(sVersionString, MAX_VERSION_LENGTH, "%s %s, %s", SM64COOPDX_VERSION, VERSION_REGION, COMPILE_TIME);
#endif
    return sVersionString;
}
#endif