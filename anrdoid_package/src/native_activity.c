#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window.h>

#include <stddef.h>
#include <stdint.h>

#include "hello_world.h"

enum {
    GLYPH_WIDTH = 5,
    GLYPH_HEIGHT = 7,
    GLYPH_ADVANCE = 6
};

static uint32_t rgba(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    return (uint32_t)red | ((uint32_t)green << 8) |
           ((uint32_t)blue << 16) | ((uint32_t)alpha << 24);
}

static const uint8_t* glyph_for(char character)
{
    static const uint8_t blank[GLYPH_HEIGHT] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    static const uint8_t h[GLYPH_HEIGHT] = {
        0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11
    };
    static const uint8_t e[GLYPH_HEIGHT] = {
        0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f
    };
    static const uint8_t l[GLYPH_HEIGHT] = {
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f
    };
    static const uint8_t o[GLYPH_HEIGHT] = {
        0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e
    };
    static const uint8_t w[GLYPH_HEIGHT] = {
        0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0a
    };
    static const uint8_t r[GLYPH_HEIGHT] = {
        0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11
    };
    static const uint8_t d[GLYPH_HEIGHT] = {
        0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e
    };

    if (character >= 'a' && character <= 'z') {
        character = (char)(character - 'a' + 'A');
    }

    switch (character) {
    case 'H': return h;
    case 'E': return e;
    case 'L': return l;
    case 'O': return o;
    case 'W': return w;
    case 'R': return r;
    case 'D': return d;
    default: return blank;
    }
}

static size_t string_length(const char* text)
{
    size_t length = 0;
    while (text[length] != '\0') {
        ++length;
    }
    return length;
}

static void fill_rect(ANativeWindow_Buffer* buffer, int left, int top,
                      int right, int bottom, uint32_t color)
{
    uint32_t* pixels = (uint32_t*)buffer->bits;

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > buffer->width) right = buffer->width;
    if (bottom > buffer->height) bottom = buffer->height;

    for (int y = top; y < bottom; ++y) {
        uint32_t* row = pixels + y * buffer->stride;
        for (int x = left; x < right; ++x) {
            row[x] = color;
        }
    }
}

static void draw_message(ANativeWindow* window)
{
    const char* message = hello_world_message();
    const size_t length = string_length(message);
    ANativeWindow_Buffer buffer;

    const int geometry_result = ANativeWindow_setBuffersGeometry(
        window, 0, 0, WINDOW_FORMAT_RGBA_8888);
    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Drawing native window (geometry=%d)", geometry_result);
    if (ANativeWindow_lock(window, &buffer, NULL) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "NobsHelloWorld",
                            "Could not lock the native window");
        return;
    }

    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Locked %dx%d, stride=%d, format=%d", buffer.width,
                        buffer.height, buffer.stride, buffer.format);

    const uint32_t coffee_black = rgba(24, 15, 11, 255);
    const uint32_t warm_cream = rgba(241, 222, 184, 255);
    fill_rect(&buffer, 0, 0, buffer.width, buffer.height, coffee_black);

    const int text_units = length > 0
        ? (int)(length * GLYPH_ADVANCE - 1)
        : 0;
    int scale_x = text_units > 0 ? buffer.width / (text_units + 8) : 1;
    int scale_y = buffer.height / (GLYPH_HEIGHT + 8);
    int scale = scale_x < scale_y ? scale_x : scale_y;
    if (scale < 2) scale = 2;

    const int text_width = text_units * scale;
    const int text_height = GLYPH_HEIGHT * scale;
    const int origin_x = (buffer.width - text_width) / 2;
    const int origin_y = (buffer.height - text_height) / 2;

    for (size_t index = 0; index < length; ++index) {
        const uint8_t* glyph = glyph_for(message[index]);
        const int glyph_x = origin_x + (int)index * GLYPH_ADVANCE * scale;

        for (int row = 0; row < GLYPH_HEIGHT; ++row) {
            for (int column = 0; column < GLYPH_WIDTH; ++column) {
                if ((glyph[row] & (1u << (GLYPH_WIDTH - 1 - column))) != 0) {
                    const int left = glyph_x + column * scale;
                    const int top = origin_y + row * scale;
                    fill_rect(&buffer, left, top, left + scale, top + scale,
                              warm_cream);
                }
            }
        }
    }

    const int post_result = ANativeWindow_unlockAndPost(window);
    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Posted native window (result=%d)", post_result);
}

static void on_native_window_created(ANativeActivity* activity,
                                     ANativeWindow* window)
{
    (void)activity;
    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Native window created");
    draw_message(window);
}

static void on_native_window_resized(ANativeActivity* activity,
                                     ANativeWindow* window)
{
    (void)activity;
    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Native window resized");
    draw_message(window);
}

static void on_native_window_redraw_needed(ANativeActivity* activity,
                                           ANativeWindow* window)
{
    (void)activity;
    __android_log_print(ANDROID_LOG_DEBUG, "NobsHelloWorld",
                        "Native window redraw requested");
    draw_message(window);
}

__attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity* activity, void* saved_state,
                              size_t saved_state_size)
{
    (void)saved_state;
    (void)saved_state_size;

    activity->callbacks->onNativeWindowCreated = on_native_window_created;
    activity->callbacks->onNativeWindowResized = on_native_window_resized;
    activity->callbacks->onNativeWindowRedrawNeeded =
        on_native_window_redraw_needed;

    __android_log_print(ANDROID_LOG_INFO, "NobsHelloWorld", "%s",
                        hello_world_message());
}
