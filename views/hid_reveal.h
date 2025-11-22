#pragma once

#include <gui/view.h>

typedef struct Hid Hid;
typedef struct HidReveal HidReveal;

HidReveal* hid_reveal_alloc(Hid* bt_hid);

void hid_reveal_free(HidReveal* hid_reveal);

View* hid_reveal_get_view(HidReveal* hid_reveal);

void hid_reveal_set_connected_status(HidReveal* hid_reveal, bool connected);

void hid_reveal_set_orientation(HidReveal* hid_reveal, bool vertical);
