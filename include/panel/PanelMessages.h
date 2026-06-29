#pragma once

#include <Arduino.h>

struct PanelMessages {
    static const uint8_t MAX_LAYOUT3_SLIDES = 12;

    String layout1Bottom;
    uint8_t layout1RepeatCount;
    String layout2Running;
    String layout3Slides[MAX_LAYOUT3_SLIDES];
    uint8_t layout3SlideCount;
    bool layout4ShowPasaran;
    bool layout4ShowHijriDate;
    uint8_t layout4RepeatCount;
    int layout4HijriCorrection;
    String layout4Running;
};
