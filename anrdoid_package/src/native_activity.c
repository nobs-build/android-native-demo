#include <android/log.h>
#include <android/native_activity.h>

#include <stddef.h>

#include "hello_world.h"

__attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity* activity, void* saved_state,
                              size_t saved_state_size)
{
    (void)activity;
    (void)saved_state;
    (void)saved_state_size;
    __android_log_print(ANDROID_LOG_INFO, "NobsHelloWorld", "%s",
                        hello_world_message());
}
