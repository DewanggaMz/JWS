#include "services/panel_messages/panel_messages_service.h"

#include <ArduinoJson.h>
#include <string.h>

#include "services/database/database_service.h"

namespace {

const char DEFAULT_LAYOUT1_BOTTOM[] = "MASJID BAITUROHMAH-GUMUKMOJO      ";
const uint8_t DEFAULT_LAYOUT1_REPEAT_COUNT = 3;
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
const size_t MAX_MESSAGE_LENGTH = 512;
const size_t MAX_SLIDE_LENGTH = 128;

bool isValidMessage(JsonVariantConst value, size_t maxLength)
{
    if (!value.is<const char *>()) {
        return false;
    }

    const char *text = value.as<const char *>();
    const size_t length = text == nullptr ? 0 : strlen(text);
    return length > 0 && length <= maxLength;
}

bool isValidRepeatCount(JsonVariantConst value)
{
    if (!value.is<int>()) {
        return false;
    }

    const int repeatCount = value.as<int>();
    return repeatCount > 0 && repeatCount <= 255;
}

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
    messages.layout1RepeatCount = DEFAULT_LAYOUT1_REPEAT_COUNT;
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
    changed |= setPositiveIntegerIfMissing(
        layout1,
        "repeatCount",
        DEFAULT_LAYOUT1_REPEAT_COUNT
    );
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
    if (panelMessages["layout1"]["repeatCount"].is<int>()) {
        const int repeatCount =
            panelMessages["layout1"]["repeatCount"].as<int>();
        if (repeatCount > 0 && repeatCount <= 255) {
            messages.layout1RepeatCount = static_cast<uint8_t>(repeatCount);
        }
    }
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

bool updatePanelLayoutMessages(
    uint8_t layoutNumber,
    JsonVariantConst payload,
    PanelMessages &messages,
    String &message
)
{
    if (!payload.is<JsonObjectConst>()) {
        message = "Payload message layout harus berupa object";
        return false;
    }

    if (layoutNumber < 1 || layoutNumber > 4) {
        message = "Nomor layout harus antara 1 sampai 4";
        return false;
    }

    JsonObjectConst input = payload.as<JsonObjectConst>();
    JsonDocument database;
    loadDatabase(database);
    JsonObject panelMessages = database["panelMessages"].is<JsonObject>()
                                 ? database["panelMessages"].as<JsonObject>()
                                 : database["panelMessages"].to<JsonObject>();
    char layoutKey[] = "layout0";
    layoutKey[6] = static_cast<char>('0' + layoutNumber);
    JsonObject layout = panelMessages[layoutKey].is<JsonObject>()
                          ? panelMessages[layoutKey].as<JsonObject>()
                          : panelMessages[layoutKey].to<JsonObject>();
    bool changed = false;

    if (layoutNumber == 1) {
        if (!input["bottom"].isUnbound()) {
            if (!isValidMessage(input["bottom"], MAX_MESSAGE_LENGTH)) {
                message = "Field bottom wajib berupa string 1 sampai 512 karakter";
                return false;
            }
            layout["bottom"] = input["bottom"].as<const char *>();
            changed = true;
        }

        if (!input["repeatCount"].isUnbound()) {
            if (!isValidRepeatCount(input["repeatCount"])) {
                message = "Field repeatCount harus berupa angka 1 sampai 255";
                return false;
            }
            layout["repeatCount"] = input["repeatCount"].as<int>();
            changed = true;
        }
    } else if (layoutNumber == 2) {
        if (!input["running"].isUnbound()) {
            if (!isValidMessage(input["running"], MAX_MESSAGE_LENGTH)) {
                message = "Field running wajib berupa string 1 sampai 512 karakter";
                return false;
            }
            layout["running"] = input["running"].as<const char *>();
            changed = true;
        }
    } else if (layoutNumber == 3) {
        if (!input["slides"].isUnbound()) {
            if (!input["slides"].is<JsonArrayConst>()) {
                message = "Field slides harus berupa array string";
                return false;
            }

            JsonArrayConst slides = input["slides"].as<JsonArrayConst>();
            if (slides.size() == 0 ||
                slides.size() > PanelMessages::MAX_LAYOUT3_SLIDES) {
                message = "Field slides harus berisi 1 sampai 12 item";
                return false;
            }

            for (JsonVariantConst slide : slides) {
                if (!isValidMessage(slide, MAX_SLIDE_LENGTH)) {
                    message = "Setiap slide wajib berupa string 1 sampai 128 karakter";
                    return false;
                }
            }

            layout["slides"].set(slides);
            changed = true;
        }
    } else {
        if (!input["running"].isUnbound()) {
            if (!isValidMessage(input["running"], MAX_MESSAGE_LENGTH)) {
                message = "Field running wajib berupa string 1 sampai 512 karakter";
                return false;
            }
            layout["running"] = input["running"].as<const char *>();
            changed = true;
        }

        if (!input["showPasaran"].isUnbound()) {
            if (!input["showPasaran"].is<bool>()) {
                message = "Field showPasaran harus berupa boolean";
                return false;
            }
            layout["showPasaran"] = input["showPasaran"].as<bool>();
            changed = true;
        }

        if (!input["showHijriDate"].isUnbound()) {
            if (!input["showHijriDate"].is<bool>()) {
                message = "Field showHijriDate harus berupa boolean";
                return false;
            }
            layout["showHijriDate"] = input["showHijriDate"].as<bool>();
            changed = true;
        }

        if (!input["repeatCount"].isUnbound()) {
            if (!isValidRepeatCount(input["repeatCount"])) {
                message = "Field repeatCount harus berupa angka 1 sampai 255";
                return false;
            }
            layout["repeatCount"] = input["repeatCount"].as<int>();
            changed = true;
        }
    }

    if (!changed) {
        message = "Tidak ada field layout yang dikenali";
        return false;
    }

    if (!saveDatabase(database)) {
        message = "Gagal menyimpan message layout";
        return false;
    }

    if (!loadPanelMessages(messages)) {
        message = "Message tersimpan tetapi gagal dimuat ulang";
        return false;
    }

    message = "Message layout berhasil diperbarui";
    return true;
}
