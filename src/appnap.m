#include "appnap.h"
#include <Foundation/Foundation.h>

static id activity;

void disableAppNap() {
    activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityLatencyCritical | NSActivityUserInitiated
                                                              reason:@"Disable App Nap"];
}

void enableAppNap() {
    [[NSProcessInfo processInfo] endActivity:activity];
}
