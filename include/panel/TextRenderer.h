#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <DMD32.h>

class TextRenderer {
  public:
    static int textWidth(DMD &dmd, const char *text, int charSpacing);
    static void clearRegion(DMD &dmd, int x, int y, int width, int height);
    static void drawTextInRegion(DMD &dmd, int regionX, int regionY, int regionWidth, int textX, int textY, const char *text, int charSpacing);
    static void drawBoldTextInRegion(DMD &dmd, int regionX, int regionY, int regionWidth, int textX, int textY, const char *text, int charSpacing);
};

#endif
