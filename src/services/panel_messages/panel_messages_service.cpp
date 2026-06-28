#include "services/panel_messages/panel_messages_service.h"

#include <ArduinoJson.h>

#include "services/database/database_service.h"

namespace {

const char DEFAULT_LAYOUT1_BOTTOM[] = "MASJID BAITUROHMAH-GUMUKMOJO      ";
const char DEFAULT_LAYOUT2_RUNNING[] = "LAYOUT 2  -  INFORMASI BERJALAN DI 3 PANEL P10      ";
const char DEFAULT_LAYOUT4_BOTTOM[] = "      12 JUMADIL AKHIR 1448 H      ";

const char *DEFAULT_LAYOUT3_SLIDES[] = {
    "10 MENIT",
    "LAGI",
    "MEMASUKI",
    "WAKTU",
    "SHOLAT",
    "MAGHRIB"
};

const uint8_t DEFAULT_LAYOUT3_SLIDE_COUNT =
    sizeof(DEFAULT_LAYOUT3_SLIDES) / sizeof(DEFAULT_LAYOUT3_SLIDES[0]);

bool setStringIfMissing(JsonObject object, const char *key, const char *value)
{
    if (object[key].is<const char *>() && object[key].as<const char *>()[0] != '\0') {
        return false;
    }

    object[key] = value;
    return true;
}

bool copyNonEmptyString(JsonVariantConst source, String &target)
{
    if (!source.is<const char *>()) {
        return false;
    }

    const char *value = source.as<const char *>();
    if (value == nullptr || value[0] == '\0') {
        return false;
    }

    target = value;
    return true;
}

}

void setDefaultPanelMessages(PanelMessages &messages)
{
    messages.layout1Bottom = DEFAULT_LAYOUT1_BOTTOM;
    messages.layout2Running = DEFAULT_LAYOUT2_RUNNING;
    messages.layout3SlideCount = DEFAULT_LAYOUT3_SLIDE_COUNT;
    for (uint8_t i = 0; i < DEFAULT_LAYOUT3_SLIDE_COUNT; i++) {
        messages.layout3Slides[i] = DEFAULT_LAYOUT3_SLIDES[i];
    }
    messages.layout4Bottom = DEFAULT_LAYOUT4_BOTTOM;
}

bool ensurePanelMessages()
{
    JsonDocument database;
    loadDatabase(database);

    bool changed = false;
    if (!database["panelMessages"].is<JsonObject>()) {
        database["panelMessages"].to<JsonObject>();
        changed = true;
    }

    JsonObject panelMessages = database["panelMessages"].as<JsonObject>();
    JsonObject layout1 = panelMessages["layout1"].is<JsonObject>()
                           ? panelMessages["layout1"].as<JsonObject>()
                           : panelMessages["layout1"].to<JsonObject>();
    JsonObject layout2 = panelMessages["layout2"].is<JsonObject>()
                           ? panelMessages["layout2"].as<JsonObject>()
                           : panelMessages["layout2"].to<JsonObject>();
    JsonObject layout3 = panelMessages["layout3"].is<JsonObject>()
                           ? panelMessages["layout3"].as<JsonObject>()
                           : panelMessages["layout3"].to<JsonObject>();
    JsonObject layout4 = panelMessages["layout4"].is<JsonObject>()
                           ? panelMessages["layout4"].as<JsonObject>()
                           : panelMessages["layout4"].to<JsonObject>();

    changed |= setStringIfMissing(layout1, "bottom", DEFAULT_LAYOUT1_BOTTOM);
    changed |= setStringIfMissing(layout2, "running", DEFAULT_LAYOUT2_RUNNING);
    changed |= setStringIfMissing(layout4, "bottom", DEFAULT_LAYOUT4_BOTTOM);

    if (!layout3["slides"].is<JsonArray>() || layout3["slides"].size() == 0) {
        JsonArray slides = layout3["slides"].to<JsonArray>();
        for (uint8_t i = 0; i < DEFAULT_LAYOUT3_SLIDE_COUNT; i++) {
            slides.add(DEFAULT_LAYOUT3_SLIDES[i]);
        }
        changed = true;
    }

    return !changed || saveDatabase(database);
}

bool loadPanelMessages(PanelMessages &messages)
{
    setDefaultPanelMessages(messages);

    JsonDocument database;
    if (!loadDatabase(database) || !database["panelMessages"].is<JsonObjectConst>()) {
        return false;
    }

    JsonObjectConst panelMessages = database["panelMessages"].as<JsonObjectConst>();
    copyNonEmptyString(panelMessages["layout1"]["bottom"], messages.layout1Bottom);
    copyNonEmptyString(panelMessages["layout2"]["running"], messages.layout2Running);
    copyNonEmptyString(panelMessages["layout4"]["bottom"], messages.layout4Bottom);

    JsonArrayConst slides = panelMessages["layout3"]["slides"].as<JsonArrayConst>();
    if (!slides.isNull()) {
        uint8_t count = 0;
        for (JsonVariantConst slide : slides) {
            if (count >= PanelMessages::MAX_LAYOUT3_SLIDES) {
                break;
            }

            String value;
            if (copyNonEmptyString(slide, value)) {
                messages.layout3Slides[count++] = value;
            }
        }

        if (count > 0) {
            messages.layout3SlideCount = count;
        }
    }

    return true;
}
