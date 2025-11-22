#include "hid_reveal.h"
#include <gui/elements.h>
#include "../hid.h"

#include "hid_icons.h"

#define TAG "HidReveal"

struct HidReveal {
    View* view;
    Hid* hid;
    InputKey last_long_press_key;
    bool suppress_next_short_press;
};

typedef struct {
    bool left_pressed;
    bool up_pressed;
    bool right_pressed;
    bool down_pressed;
    bool ok_pressed;
    bool back_pressed;
    bool connected;
} HidRevealModel;

static void hid_reveal_draw_arrow(Canvas* canvas, uint8_t x, uint8_t y, CanvasDirection dir) {
    canvas_draw_triangle(canvas, x, y, 5, 3, dir);
    if(dir == CanvasDirectionBottomToTop) {
        canvas_draw_line(canvas, x, y + 6, x, y - 1);
    } else if(dir == CanvasDirectionTopToBottom) {
        canvas_draw_line(canvas, x, y - 6, x, y + 1);
    } else if(dir == CanvasDirectionRightToLeft) {
        canvas_draw_line(canvas, x + 6, y, x - 1, y);
    } else if(dir == CanvasDirectionLeftToRight) {
        canvas_draw_line(canvas, x - 6, y, x + 1, y);
    }
}

static void hid_reveal_draw_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidRevealModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
#endif

    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 17, 3, AlignLeft, AlignTop, "Reveal");

    canvas_draw_icon(canvas, 68, 2, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 127, 3, AlignRight, AlignTop, "Hold to exit");

    // Up
    canvas_draw_icon(canvas, 21, 24, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, 24, 26, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, 31, 30, CanvasDirectionBottomToTop);
    // long press hint for Up: small 'F' right of arrow
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 38, 33, "F");
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, 21, 45, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, 24, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, 26, 55, CanvasDirectionTopToBottom);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 37, 33, ".");
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, 0, 45, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, 3, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, 7, 53, CanvasDirectionRightToLeft);
    // long press hint for Left: small 'esc' below arrow
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 66, "esc");
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, 42, 45, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, 45, 47, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, 53, 53, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 63, 24, &I_Space_65x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 66, 26, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 28, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 91, 36, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 63, 45, &I_Space_65x18);
    if(model->back_pressed) {
        elements_slightly_rounded_box(canvas, 66, 47, 60, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 74, 49, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 91, 57, AlignLeft, AlignBottom, "Back");
}

static void hid_reveal_draw_vertical_callback(Canvas* canvas, void* context) {
    furi_assert(context);
    HidRevealModel* model = context;

    // Header
#ifdef HID_TRANSPORT_BLE
    if(model->connected) {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_connected_15x15);
    } else {
        canvas_draw_icon(canvas, 0, 0, &I_Ble_disconnected_15x15);
    }
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 20, 3, AlignLeft, AlignTop, "Reveal");
#else
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(canvas, 12, 3, AlignLeft, AlignTop, "Reveal");
#endif

    // canvas_draw_icon(canvas, 2, 18, &I_Pin_back_arrow_10x8);
    canvas_set_font(canvas, FontSecondary);
    elements_multiline_text_aligned(canvas, 2, 19, AlignLeft, AlignTop, "hold for 2nd fn");

    const uint8_t x_2 = 23;
    const uint8_t x_1 = 2;
    const uint8_t x_3 = 44;

    const uint8_t y_1 = 44;
    const uint8_t y_2 = 65;

    // Up
    canvas_draw_icon(canvas, x_2, y_1, &I_Button_18x18);
    if(model->up_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_1 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, x_2 + 6, y_1 + 6, CanvasDirectionBottomToTop);
    // Subtle long press hint for Up: small 'F' right of arrow
    // canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, x_2 + 11, y_1 + 15, "F");
    canvas_set_color(canvas, ColorBlack);

    // Down
    canvas_draw_icon(canvas, x_2, y_2, &I_Button_18x18);
    if(model->down_pressed) {
        elements_slightly_rounded_box(canvas, x_2 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, x_2 + 6, y_2 + 10, CanvasDirectionTopToBottom);
    canvas_draw_str(canvas, x_2 + 12, y_2 + 13, ".");
    canvas_set_color(canvas, ColorBlack);

    // Left
    canvas_draw_icon(canvas, x_1, y_2, &I_Button_18x18);
    if(model->left_pressed) {
        elements_slightly_rounded_box(canvas, x_1 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, x_1 + 7, y_2 + 4, CanvasDirectionRightToLeft);
    // Subtle long press hint for Left: small 'esc' below arrow
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, x_1 + 3, y_2 + 15, "esc");
    canvas_set_color(canvas, ColorBlack);

    // Right
    canvas_draw_icon(canvas, x_3, y_2, &I_Button_18x18);
    if(model->right_pressed) {
        elements_slightly_rounded_box(canvas, x_3 + 3, y_2 + 2, 13, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    hid_reveal_draw_arrow(canvas, x_3 + 11, y_2 + 8, CanvasDirectionLeftToRight);
    canvas_set_color(canvas, ColorBlack);

    // Ok
    canvas_draw_icon(canvas, 2, 86, &I_Space_60x18);
    if(model->ok_pressed) {
        elements_slightly_rounded_box(canvas, 5, 88, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 11, 90, &I_Ok_btn_9x9);
    elements_multiline_text_aligned(canvas, 26, 98, AlignLeft, AlignBottom, "Space");
    canvas_set_color(canvas, ColorBlack);

    // Back
    canvas_draw_icon(canvas, 2, 107, &I_Space_60x18);
    if(model->back_pressed) {
        elements_slightly_rounded_box(canvas, 5, 109, 55, 13);
        canvas_set_color(canvas, ColorWhite);
    }
    canvas_draw_icon(canvas, 5, 111, &I_Pin_back_arrow_10x8);
    elements_multiline_text_aligned(canvas, 20, 119, AlignLeft, AlignBottom, "back|exit");
}

static void hid_reveal_process(HidReveal* hid_reveal, InputEvent* event) {
    // Suppress short action if it immediately follows a long press for the same key (Up, Down, Left)
    if(event->type == InputTypeShort && hid_reveal->suppress_next_short_press && event->key == hid_reveal->last_long_press_key) {
        hid_reveal->suppress_next_short_press = false;
        return;
    }
    with_view_model(
        hid_reveal->view,
        HidRevealModel * model,
        {
            if(event->type == InputTypePress) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = true;
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = true;
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = true;
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = true;
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = true;
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = true;
                }
            } else if(event->type == InputTypeRelease) {
                if(event->key == InputKeyUp) {
                    model->up_pressed = false;
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_UP_ARROW);
                } else if(event->key == InputKeyDown) {
                    model->down_pressed = false;
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_DOWN_ARROW);
                } else if(event->key == InputKeyLeft) {
                    model->left_pressed = false;
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_LEFT_ARROW);
                } else if(event->key == InputKeyRight) {
                    model->right_pressed = false;
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_RIGHT_ARROW);
                } else if(event->key == InputKeyOk) {
                    model->ok_pressed = false;
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_SPACEBAR);
                } else if(event->key == InputKeyBack) {
                    model->back_pressed = false;
                }
            } else if(event->type == InputTypeShort) {
                if(event->key == InputKeyBack) {
                    hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_DELETE);
                    hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_DELETE);
                    hid_hal_consumer_key_press(hid_reveal->hid, HID_CONSUMER_AC_BACK);
                    hid_hal_consumer_key_release(hid_reveal->hid, HID_CONSUMER_AC_BACK);
                }
            }
        },
        true);
}

static bool hid_reveal_input_callback(InputEvent* event, void* context) {
    furi_assert(context);
    HidReveal* hid_reveal = context;
    bool consumed = false;

    if(event->type == InputTypeLong) {
        // Only suppress short action for Up, Down, Left
        if(event->key == InputKeyUp || event->key == InputKeyDown || event->key == InputKeyLeft) {
            hid_reveal->last_long_press_key = event->key;
            hid_reveal->suppress_next_short_press = true;
        }
        if(event->key == InputKeyBack) {
            hid_hal_keyboard_release_all(hid_reveal->hid);
            return false;
        } else if(event->key == InputKeyUp) {
            // Send 'F' key on long press of up arrow
            hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_F);
            hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_F);
        } else if(event->key == InputKeyDown) {
            // Send '.' key on long press of down button
            hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_DOT);
            hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_DOT);
        } else if(event->key == InputKeyLeft) {
            // Send 'ESC' key on long press of left button
            hid_hal_keyboard_press(hid_reveal->hid, HID_KEYBOARD_ESCAPE);
            hid_hal_keyboard_release(hid_reveal->hid, HID_KEYBOARD_ESCAPE);
        }
        return true;
    } else {
        hid_reveal_process(hid_reveal, event);
        consumed = true;
    }

    return consumed;
}

HidReveal* hid_reveal_alloc(Hid* hid) {
    HidReveal* hid_reveal = malloc(sizeof(HidReveal));
    hid_reveal->view = view_alloc();
    hid_reveal->hid = hid;
    hid_reveal->last_long_press_key = 0;
    hid_reveal->suppress_next_short_press = false;
    view_set_context(hid_reveal->view, hid_reveal);
    view_allocate_model(hid_reveal->view, ViewModelTypeLocking, sizeof(HidRevealModel));
    view_set_draw_callback(hid_reveal->view, hid_reveal_draw_callback);
    view_set_input_callback(hid_reveal->view, hid_reveal_input_callback);
    return hid_reveal;
}

void hid_reveal_free(HidReveal* hid_reveal) {
    furi_assert(hid_reveal);
    view_free(hid_reveal->view);
    free(hid_reveal);
}

View* hid_reveal_get_view(HidReveal* hid_reveal) {
    furi_assert(hid_reveal);
    return hid_reveal->view;
}

void hid_reveal_set_connected_status(HidReveal* hid_reveal, bool connected) {
    furi_assert(hid_reveal);
    with_view_model(
        hid_reveal->view, HidRevealModel * model, { model->connected = connected; }, true);
}

void hid_reveal_set_orientation(HidReveal* hid_reveal, bool vertical) {
    furi_assert(hid_reveal);

    if(vertical) {
        view_set_draw_callback(hid_reveal->view, hid_reveal_draw_vertical_callback);
        view_set_orientation(hid_reveal->view, ViewOrientationVerticalFlip);

    } else {
        view_set_draw_callback(hid_reveal->view, hid_reveal_draw_callback);
        view_set_orientation(hid_reveal->view, ViewOrientationHorizontal);
    }
}
