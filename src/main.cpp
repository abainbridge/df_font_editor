// Deadfrog lib headers
#include "df_clipboard.h"
#include "df_font.h"
#include "df_gui.h"
#include "df_time.h"
#include "df_window.h"
#include "fonts/df_mono.h"

// Standard headers
#include <string.h>
#include <stdio.h>


struct EditWidget {
    int zoomFactor;
    int top, left, width, height;
    int selectedColourIdx;

    DfBitmap *glyphs[96];

    void Init(DfFont *font);
    void Advance();
    void Render();

    void Load();
    void Save();
};


static const char APPLICATION_NAME[] = "Deadfrog Font Editor";
DfColour g_gridColour = { 0xff484848 };
DfColour g_gridBoldColour = { 0xff707070 };
DfColour g_textColours[4] = { 0xff303030, 0xff6e6e6e, 0xff8f8f8f, 0xffb8b8b8 };
EditWidget g_editWidget;
DfFont *g_font;
char g_proofText[] =
    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
    "Angel Adept Blind Bodice Clique Coast Dunce Docile Enact Eosin Furlong Focal "
    "Gnome Gondola Human Hoist Inlet Iodine Justin Jocose Knoll Koala Linden Loads "
    "Milliner Modal Number Nodule Onset Oddball Pneumo Poncho Quanta Qophs Rhone "
    "Roman Snout Sodium Tundra Tocsin Uncle Udder Vulcan Vocal Whale Woman Xmas "
    "Xenon Yunnan Young Zloty Zodiac. Angel angel adept for the nuance loads of the "
    "arena cocoa and quaalude. Blind blind bodice for the submit oboe of the club "
    "snob and abbot. Clique clique coast for the pouch loco of the franc assoc and "
    "accede. Dunce dunce docile for the loudness mastodon of the loud statehood and "
    "huddle. Enact enact eosin for the quench coed of the pique canoe and bleep. "
    "Furlong furlong focal for the genuflect profound of the motif aloof and offers. "
    "Gnome gnome gondola for the impugn logos of the unplug analog and smuggle. "
    "Human human hoist for the buddhist alcohol of the riyadh caliph and bathhouse. "
    "Inlet inlet iodine for the quince champion of the ennui scampi and shiite. "
    "Justin justin jocose for the djibouti sojourn of the oranj raj and hajjis. "
    "Knoll knoll koala for the banknote lookout of the dybbuk outlook and trekked. "
    "Linden linden loads for the ulna monolog of the consul menthol and shallot. "
    "Milliner milliner modal for the alumna solomon of the album custom and summon. "
    "Number number nodule for the unmade economic of the shotgun bison and tunnel. "
    "Onset onset oddball for the abandon podium of the antiquo tempo and moonlit. "
    "Pneumo pneumo poncho for the dauphin opossum of the holdup bishop and supplies. "
    "Quanta quanta qophs for the inquest sheqel of the cinq coq and suqqu. Rhone "
    "rhone roman for the burnt porous of the lemur clamor and carrot. Snout snout "
    "sodium for the ensnare bosom of the genus pathos and missing. Tundra tundra "
    "tocsin for the nutmeg isotope of the peasant ingot and ottoman. Uncle uncle "
    "udder for the dunes cloud of the hindu thou and continuum. Vulcan vulcan vocal "
    "for the alluvial ovoid of the yugoslav chekhov and revved. Whale whale woman "
    "for the meanwhile blowout of the forepaw meadow and glowworm. Xmas xmas xenon "
    "for the bauxite doxology of the tableaux equinox and exxon. Yunnan yunnan young "
    "for the dynamo coyote of the obloquy employ and sayyid. Zloty zloty zodiac for "
    "the gizmo ozone of the franz laissez and buzzing.";


// ****************************************************************************
// ColourButton
// ****************************************************************************

int ColourButtonDo(DfWindow *win, DfColour colour, int selected, int x, int y, int w, int h) {
    int mouseInRect = DfMouseInRect(win, x, y, w, h);
    if (mouseInRect || selected) {
        RectOutline(win->bmp, x, y, w, h, g_buttonHighlightColour);
    }

    x += 2; y += 2; w -= 4; h -= 4;
    RectOutline(win->bmp, x, y, w, h, g_frameColour);

    x++; y++; w -= 2; h -= 2;
    RectFill(win->bmp, x, y, w, h, colour);

    return mouseInRect && win->input.lmbClicked;
}


// ****************************************************************************
// EditWidget
// ****************************************************************************

void EditWidget::Init(DfFont *font) {
    zoomFactor = 7;
    top = 10;
    left = 548;
    width = 16 * zoomFactor * font->maxCharWidth + 1;
    height = 6 * zoomFactor * font->charHeight;
    selectedColourIdx = 3;

    for (int i = 32; i < 128; i++) {
        DfBitmap **g = &glyphs[i - 32];
        *g = BitmapCreate(font->maxCharWidth, font->charHeight);
        BitmapClear(*g, g_backgroundColour);
        char s[2] = { i };
        DrawTextSimple(font, g_textColours[3], *g, 0, 0, s);
    }
}

void EditWidget::Advance() {
    int fontWidth = glyphs[0]->width;
    int fontHeight = glyphs[0]->height;

    if (g_window->input.lmb && 
            DfMouseInRect(g_window, left, top, width - 1, height)) {
        int x = g_window->input.mouseX - left;
        x /= zoomFactor;
        int y = g_window->input.mouseY - top;
        y /= zoomFactor;

        int glyphIdx = (y / fontHeight) * 16;
        glyphIdx += x / fontWidth;

        x %= fontWidth;
        y %= fontHeight;

        PutPix(glyphs[glyphIdx], x, y, g_textColours[selectedColourIdx]);
    }

    if (g_window->input.keyDowns[KEY_1]) selectedColourIdx = 0;
    if (g_window->input.keyDowns[KEY_2]) selectedColourIdx = 1;
    if (g_window->input.keyDowns[KEY_3]) selectedColourIdx = 2;
    if (g_window->input.keyDowns[KEY_4]) selectedColourIdx = 3;
}

void EditWidget::Render() {
    int fontWidth = glyphs[0]->width;
    int fontHeight = glyphs[0]->height;

    // Draw glyph set onto font bitmap.
    for (int y = 0; y < 6; y++) {
        int tmpY = top + y * fontHeight * zoomFactor;
        for (int x = 0; x < 16; x++) {
            int glyphIdx = y * 16 + x;
            int tmpX = left + x * fontWidth * zoomFactor;
            ScaleUpBlit(g_window->bmp, tmpX, tmpY, zoomFactor, glyphs[glyphIdx]);
        }
    }

    // Draw pixel grid over zoomed glyph set.
    for (int i = 0; i <= 6 * fontHeight; i++) {
        int tmpY = top + i * zoomFactor;
        DfColour col = g_gridColour;
        if ((i % g_font->charHeight) == 0)
            col = g_gridBoldColour;
        HLine(g_window->bmp, left, tmpY, width, col);
    }
    for (int i = 0; i <= 16 * fontWidth; i++) {
        int tmpX = left + i * zoomFactor;
        DfColour col = g_gridColour;
        if ((i % g_font->maxCharWidth) == 0)
            col = g_gridBoldColour;
        VLine(g_window->bmp, tmpX, top + 1, height - 1, col);
    }

    for (int i = 0; i < 4; i++) {
        int buttonWidth = 80;
        int buttonPitch = buttonWidth + 10;
        int x = left + buttonPitch * i;
        int y = top + height + 10;
        int selected = i == selectedColourIdx;
        int clicked = ColourButtonDo(g_window, g_textColours[i], selected, x, y, buttonWidth, 20);
        if (clicked) selectedColourIdx = i;
    }
}


void EditWidget::Load() {
    FILE *f = fopen("font.dfbf", "rb");
    unsigned char hdr[10];
    fread(hdr, 1, 10, f);
    ReleaseAssert(memcmp(hdr, "dfbf\1\1", 6) == 0, "Bad header");

    fgetc(f); // width
    fgetc(f); // height
    fgetc(f); // flags

    for (int i = 0; i < 95; i++) {
        DfBitmap *g = g_editWidget.glyphs[i];

        for (int y = 0; y < g->height; y++) {
            for (int x = 0; x < g->width; x++) {
                unsigned char b = fgetc(f);
                ReleaseAssert(b < 4, "Bad colour in font");
                PutPix(g, x, y, g_textColours[b]);
            }
        }
    }
    
    fclose(f);
}


void EditWidget::Save() {
    FILE *f = fopen("font.dfbf", "wb");

    // Write top-level header.
    fwrite("dfbf\1\1", 1, 6, f);
    fwrite("\0\0\0\x0a", 1, 4, f);

    // Write header for this font size.
    fputc(g_editWidget.glyphs[0]->width, f);
    fputc(g_editWidget.glyphs[0]->height, f);
    fputc('\0', f); // flags = fixed-width.

    for (int i = 0; i < 95; i++) {
        DfBitmap *g = g_editWidget.glyphs[i];
        for (int y = 0; y < g->height; y++) {
            for (int x = 0; x < g->width; x++) {
                DfColour c = GetPix(g, x, y);
                if (c.c == g_textColours[0].c)
                    fputc('\0', f);
                else if (c.c == g_textColours[1].c)
                    fputc('\x1', f);
                else if (c.c == g_textColours[2].c)
                    fputc('\x2', f);
                else if (c.c == g_textColours[3].c)
                    fputc('\x3', f);
                else 
                    ReleaseAssert(0, "Bad colour in bitmap");
            }
        }
    }

    fclose(f);
}


// ****************************************************************************
// ****************************************************************************

void DrawCharFromEditableGlyphs(DfBitmap *bmp, int x, int y, unsigned char c) {
    // The sequence of glyphs is:
    // 0 to 95: ASCII 32 to 127 - ie space to tilde.
    //   !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    // 96 to 127: ASCII 161 to 191:
    //  ¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿
    // 128 to 143: Half-height hex digits 0 to f.
    // 144: Æ
    // 145: Ð
    // 146: ×Ø
    // 148: Þß
    // 150: æ
    // 151: ð
    // 152: ÷ø
    // 153: þ

    // 0123456789abcdef
    // ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ
    // ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß
    // àáâãäåæçèéêëìíîï
    // ðñòóôõö÷øùúûüýþÿ

    if (c < 32) {
        // Unprintable
    }
    else if (c < 128) {
        // Normal.
        int glyphIdx = c - 32;
        Blit(bmp, x, y, g_editWidget.glyphs[glyphIdx]);
    }
    else if (c < 161) {
        // Unprintable
    }
    else if (c < 192) {
        // Normal.
        int glyphIdx = c - 65;
    }
}


void DrawStringFromEditableGlyphs(DfBitmap *bmp, int x, int y, char *str, int numChars) {
    for (int i = 0; i < numChars; i++) {
        if (str[i] == '\0')
            break;
        DrawCharFromEditableGlyphs(bmp, x, y, str[i]);
        x += g_editWidget.glyphs[0]->width;
    }
}


static void draw_frame() {
    BitmapClear(g_window->bmp, g_backgroundColour);

    // Draw proof text
    int columnWidth = 48;
    int x = 10;
    int y = 10;
    for (int i = 0; i < sizeof(g_proofText);) {
        int lineLen = columnWidth;
        while (lineLen > 0 && g_proofText[i + lineLen - 1] != ' ')
            lineLen--;
        if (lineLen == 0)
            lineLen = columnWidth;

        DrawStringFromEditableGlyphs(g_window->bmp, x, y, g_proofText + i, lineLen);
        y += g_font->charHeight;
        i += lineLen;
    }

    g_editWidget.Render();

    if (g_window->input.keyDowns[KEY_S] && g_window->input.keys[KEY_CONTROL]) {
        g_editWidget.Save();
        DrawTextCentre(g_font, g_colourWhite, g_window->bmp,
            g_window->bmp->width / 2, g_window->bmp->height - 20,
            "File saved!!!");
    }

    UpdateWin(g_window);
}


void main() {
    g_window = CreateWin(1800, 1124, WT_WINDOWED_RESIZEABLE, APPLICATION_NAME);
    RegisterRedrawCallback(g_window, draw_frame);
    SetWindowIcon(g_window);
    BitmapClear(g_window->bmp, g_backgroundColour);
    UpdateWin(g_window);

    g_font = LoadFontFromMemory(df_mono_11x20, sizeof(df_mono_11x20));
    g_editWidget.Init(g_font);
    g_editWidget.Load();


    //
    // Main loop

    double next_force_frame_time = GetRealTime() + 0.2;
    while (!g_window->windowClosed && !g_window->input.keyDowns[KEY_ESC]) {
        bool force_frame = GetRealTime() > next_force_frame_time;
        if (force_frame) {
            next_force_frame_time = GetRealTime() + 0.2;
        }

        if (InputPoll(g_window) || force_frame) {
            g_editWidget.Advance();
            draw_frame();
        }
         
        WaitVsync();
    }
}


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
int WINAPI WinMain(HINSTANCE _hInstance, HINSTANCE /*_hPrevInstance*/,
    LPSTR cmdLine, int /*_iCmdShow*/) {
    main();
    return 0;
}
