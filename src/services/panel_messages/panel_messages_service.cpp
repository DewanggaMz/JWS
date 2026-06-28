#include "services/panel_messages/panel_messages_service.h"

#include <ArduinoJson.h>
#include <string.h>

#include "services/database/database_service.h"

namespace {

const char DEFAULT_LAYOUT1_BOTTOM[] = "MASJID BAITUROHMAH-GUMUKMOJO      ";
const char DEFAULT_LAYOUT2_RUNNING[] = "LAYOUT 2  -  INFORMASI BERJALAN DI 3 PANEL P10      ";
const char DEFAULT_LAYOUT4_RUNNING[] =
    "JAGA KEBERSIHAN DAN KEKHUSYUKAN IBADAH      ";
const char LEGACY_LAYOUT4_BOTTOM[] = "      12 JUMADIL AKHIR 1448 H      ";
const bool DEFAULT_LAYOUT4_SHOW_PASARAN = true;
const bool DEFAULT_LAYOUT4_SHOW_HIJRI_DATE = true;
const uint8_t DEFAULT_LAYOUT4_REPEAT_COUNT = 1;

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

bool setBoolIfMissing(JsonObject object, const char *key, bool value)
{
    if (object[key].is<bool>()) {
        return false;
    }

    object[key] = value;
    return true;
}

bool setPositiveIntegerIfMissing(
    JsonObject object,
    const char *key,
    uint8_t value
)
{
    if (object[key].is<int>()) {
        const int currentValue = object[key].as<int>();
        if (currentValue > 0 && currentValue <= 255) {
            return false;
        }
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
    messages.layout4ShowPasaran = DEFAULT_LAYOUT4_SHOW_PASARAN;
    messages.layout4ShowHijriDate = DEFAULT_LAYOUT4_SHOW_HIJRI_DATE;
    messages.layout4RepeatCount = DEFAULT_LAYOUT4_REPEAT_COUNT;
    messages.layout4HijriCorrection = 1;
    messages.layout4Running = DEFAULT_LAYOUT4_RUNNING;
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
    changed |= setBoolIfMissing(
        layout4,
        "showPasaran",
        DEFAULT_LAYOUT4_SHOW_PASARAN
    );
    changed |= setBoolIfMissing(
        layout4,
        "showHijriDate",
        DEFAULT_LAYOUT4_SHOW_HIJRI_DATE
    );
    changed |= setPositiveIntegerIfMissing(
        layout4,
        "repeatCount",
        DEFAULT_LAYOUT4_REPEAT_COUNT
    );

    if (!layout4["running"].is<const char *>() ||
        layout4["running"].as<const char *>()[0] == '\0') {
        const char *legacyBottom = layout4["bottom"] | "";
        layout4["running"] =
            strcmp(legacyBottom, LEGACY_LAYOUT4_BOTTOM) == 0 || legacyBottom[0] == '\0'
              ? DEFAULT_LAYOUT4_RUNNING
              : legacyBottom;
        changed = true;
    }

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
    copyNonEmptyString(panelMessages["layout4"]["running"], messages.layout4Running);

    if (panelMessages["layout4"]["showPasaran"].is<bool>()) {
        messages.layout4ShowPasaran =
            panelMessages["layout4"]["showPasaran"].as<bool>();
    }

    if (panelMessages["layout4"]["showHijriDate"].is<bool>()) {
        messages.layout4ShowHijriDate =
            panelMessages["layout4"]["showHijriDate"].as<bool>();
    }

    if (panelMessages["layout4"]["repeatCount"].is<int>()) {
        const int repeatCount =
            panelMessages["layout4"]["repeatCount"].as<int>();
        if (repeatCount > 0 && repeatCount <= 255) {
            messages.layout4RepeatCount = static_cast<uint8_t>(repeatCount);
        }
    }

    if (database["hijriConfig"]["correct"].is<int>()) {
        messages.layout4HijriCorrection =
            database["hijriConfig"]["correct"].as<int>();
    }

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
