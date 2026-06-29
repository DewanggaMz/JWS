#pragma once

#include <Arduino.h>

struct PanelMessages {
    static const uint8_t MAX_LAYOUT3_SLIDES = 12;

    String layout1Bottom;
    uint8_t layout1RepeatCount;
    uint16_t layout1SpeedMs;
    bool layout1ShowImsak;
    bool layout1ShowSunrise;
    bool layout1ShowDhuha;
    String layout2Running;
    uint16_t layout2SpeedMs;
    String layout3Slides[MAX_LAYOUT3_SLIDES];
    uint8_t layout3SlideCount;
    bool layout4ShowPasaran;
    bool layout4ShowHijriDate;
    uint8_t layout4RepeatCount;
    int layout4HijriCorrection;
    String layout4Running;
    uint16_t layout4SpeedMs;
    uint16_t layout5SpeedMs;
};
