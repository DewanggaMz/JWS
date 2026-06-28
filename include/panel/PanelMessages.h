#pragma once

#include <Arduino.h>

struct PanelMessages {
    static const uint8_t MAX_LAYOUT3_SLIDES = 12;

    String layout1Bottom;
    String layout2Running;
    String layout3Slides[MAX_LAYOUT3_SLIDES];
    uint8_t layout3SlideCount;
    String layout4Bottom;
};
