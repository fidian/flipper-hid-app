#include "hid_media.h"
#include <furi.h>
#include <furi_hal_usb_hid.h>
#include <extra_profiles/hid_profile.h>
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidMedia"

struct HidMedia {
    View* view;
    Hid* hid;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool connected;
    bool back_pressed;
    bool android_mode;
} HidMediaModel;

static void hid_media_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    canvas_draw_triangle(canvas, x, y, 5, 3, dir);
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_dot(canvas, x, y - 1);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_dot(canvas, x, y + 1);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_dot(canvas, x - 1, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_dot(canvas, x + 1, y);
    }
}

static void hid_media_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidMediaModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Media");
    canvas_set_font(canvas, FontSecondary);
    if(model->android_mode) {
        elements_multiline_text_aligned(canvas, 0, 25, AlignLeft, AlignTop, "Android");
    }

    // Keypad circles
    canvas_draw_icon(canvas, 58, 3, &I_OutCircles_70x51);

    // Up
    if(model->up_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 68, 6, &I_S_UP_31x15);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 79, 9, &I_Volup_8x6);
    canvas_set_color(canvas, ColorBlack);

    // Down
    if(model->down_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 68, 36, &I_S_DOWN_31x15);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 80, 41, &I_Voldwn_6x6);
    canvas_set_color(canvas, ColorBlack);

    // Left
    if(model->left_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 61, 13, &I_S_LEFT_15x31);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 67, 28, CanvasDirectionRightToLeft);
    hid_media_draw_arrow(canvas, 70, 28, CanvasDirectionRightToLeft);
    if(!model->android_mode) canvas_draw_line(canvas, 64, 26, 64, 30);
    canvas_set_color(canvas, ColorBlack);

    // Right
    if(model->right_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 91, 13, &I_S_RIGHT_15x31);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 96, 28, CanvasDirectionLeftToRight);
    hid_media_draw_arrow(canvas, 99, 28, CanvasDirectionLeftToRight);
    if(!model->android_mode) canvas_draw_line(canvas, 102, 26, 102, 30);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    if(model->ok_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 74, 19, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_media_draw_arrow(canvas, 80, 28, CanvasDirectionLeftToRight);
    canvas_draw_line(canvas, 84, 26, 84, 30);
    canvas_draw_line(canvas, 86, 26, 86, 30);
    canvas_set_color(canvas, ColorBlack);

    // Exit
    if(model->back_pressed) {
        canvas_set_bitmap_mode(canvas, true);
        canvas_draw_icon(canvas, 107, 33, &I_Pressed_Button_19x19);
        canvas_set_bitmap_mode(canvas, false);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 111, 38, &I_Pin_back_arrow_10x10);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_icon(canvas, 0, 54, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 13, 62, AlignLeft, AlignBottom, "Hold to exit");
}

static uint16_t hid_media_get_usage(InputKey key, bool android_mode) {
    // Android compatibility mode uses keyboard volume (0x80/0x81) and arrows.
    // Play/pause always uses the Consumer Control report.
    switch(key) {
    case InputKeyUp:
        return android_mode ? HID_KEYBOARD_VOLUME_UP : HID_CONSUMER_VOLUME_INCREMENT;
    case InputKeyDown:
        return android_mode ? HID_KEYBOARD_VOLUME_DOWN : HID_CONSUMER_VOLUME_DECREMENT;
    case InputKeyLeft:
        return android_mode ? HID_KEYBOARD_LEFT_ARROW : HID_CONSUMER_SCAN_PREVIOUS_TRACK;
    case InputKeyRight:
        return android_mode ? HID_KEYBOARD_RIGHT_ARROW : HID_CONSUMER_SCAN_NEXT_TRACK;
    case InputKeyOk:
        return HID_CONSUMER_PLAY_PAUSE;
    default:
        return 0;
    }
}

static void hid_media_process_key(HidMedia* hid_media, InputKey key, bool pressed) {
    bool android_mode = false;
    with_view_model(
        hid_media->view,
        HidMediaModel * model,
        {
            if(key == InputKeyUp) {
                model->up_pressed = pressed;
            } else if(key == InputKeyDown) {
                model->down_pressed = pressed;
            } else if(key == InputKeyLeft) {
                model->left_pressed = pressed;
            } else if(key == InputKeyRight) {
                model->right_pressed = pressed;
            } else if(key == InputKeyOk) {
                model->ok_pressed = pressed;
            } else if(key == InputKeyBack) {
                model->back_pressed = pressed;
            }
            android_mode = model->android_mode;
        },
        true);
    uint16_t usage = hid_media_get_usage(key, android_mode);
    if(!usage) return;
    if(android_mode && key != InputKeyOk) {
        if(pressed) {
            hid_hal_keyboard_press(hid_media->hid, usage);
        } else {
            hid_hal_keyboard_release(hid_media->hid, usage);
        }
    } else if(pressed) {
        hid_hal_consumer_key_press(hid_media->hid, usage);
    } else {
        hid_hal_consumer_key_release(hid_media->hid, usage);
    }
}

static void hid_media_exit_callback(void* context) {
    HidMedia* hid_media = context;
    hid_hal_keyboard_release_all(hid_media->hid);
    hid_hal_consumer_key_release_all(hid_media->hid);
    with_view_model(
        hid_media->view,
        HidMediaModel * model,
        {
            model->up_pressed = false;
            model->down_pressed = false;
            model->left_pressed = false;
            model->right_pressed = false;
            model->ok_pressed = false;
            model->back_pressed = false;
        },
        false);
}

static bool hid_media_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidMedia* hid_media = context;
    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        // The exit callback releases both report types before leaving the view.
        return false;
    }
    if(event->type == InputTypePress) {
        hid_media_process_key(hid_media, event->key, true);
    } else if(event->type == InputTypeRelease) {
        hid_media_process_key(hid_media, event->key, false);
    }
    return true;
}

HidMedia* hid_media_alloc(Hid* hid) {
    HidMedia* hid_media = malloc(sizeof(HidMedia));
    hid_media->view = view_alloc();
    hid_media->hid = hid;
    view_set_context(hid_media->view, hid_media);
    view_allocate_model(hid_media->view, ViewModelTypeLocking, sizeof(HidMediaModel));
    view_set_draw_callback(hid_media->view, hid_media_draw_callback);
    view_set_input_callback(hid_media->view, hid_media_input_callback);
    view_set_exit_callback(hid_media->view, hid_media_exit_callback);
    return hid_media;
}

void hid_media_free(HidMedia* hid_media) {
    furi_assert(hid_media);
    view_free(hid_media->view);
    free(hid_media);
}

View* hid_media_get_view(HidMedia* hid_media) {
    furi_assert(hid_media);
    return hid_media->view;
}

void hid_media_set_connected_status(HidMedia* hid_media, bool connected) {
    furi_assert(hid_media);
    with_view_model(
        hid_media->view, HidMediaModel * model, { model->connected = connected; }, true);
}

void hid_media_set_android_mode(HidMedia* hid_media, bool android_mode) {
    furi_assert(hid_media);
    with_view_model(
        hid_media->view, HidMediaModel * model, { model->android_mode = android_mode; }, true);
}
