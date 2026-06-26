#include "panel/TextRenderer.h"

#include <Arduino.h>

int TextRenderer::textWidth(DMD &dmd, const char *text, int charSpacing)
{
    int width = 0;
    for (size_t i = 0; text[i] != '\0'; i++) {
        width += dmd.charWidth(text[i]) + charSpacing;
    }
    return width;
}

void TextRenderer::clearRegion(DMD &dmd, int x, int y, int width, int height)
{
    dmd.drawFilledBox(x, y, x + width - 1, y + height - 1, GRAPHICS_INVERSE);
}

void TextRenderer::drawTextInRegion(DMD &dmd, int regionX, int regionY, int regionWidth, int textX, int textY, const char *text, int charSpacing)
{
    int cursorX = textX;
    const int regionRight = regionX + regionWidth - 1;

    for (size_t i = 0; text[i] != '\0'; i++) {
        const int charWide = dmd.charWidth(text[i]);
        if (cursorX >= regionX && cursorX + charWide < regionRight) {
            dmd.drawChar(cursorX, regionY + textY, text[i], GRAPHICS_NORMAL);
        }
        cursorX += charWide + charSpacing;
    }
}

void TextRenderer::drawBoldTextInRegion(DMD &dmd, int regionX, int regionY, int regionWidth, int textX, int textY, const char *text, int charSpacing)
{
    int cursorX = textX;
    const int regionRight = regionX + regionWidth - 1;

    for (size_t i = 0; text[i] != '\0'; i++) {
        const int charWide = dmd.charWidth(text[i]);
        if (cursorX >= regionX && cursorX + charWide < regionRight) {
            dmd.drawChar(cursorX, regionY + textY, text[i], GRAPHICS_NORMAL);
            dmd.drawChar(cursorX + 1, regionY + textY, text[i], GRAPHICS_OR);
        }
        cursorX += charWide + charSpacing;
    }
}
