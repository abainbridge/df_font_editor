// Deadfrog lib headers
#include "df_clipboard.h"
#include "df_font.h"
#include "df_time.h"
#include "df_window.h"
#include "fonts/df_mono.h"

// Project headers

// Standard headers

static const char APPLICATION_NAME[] = "Deadfrog Font Editor";
DfColour g_backgroundColour = { 0xff303030 };
DfColour g_gridColour = { 0xff484848 };
DfColour g_gridBoldColour = { 0xff707070 };
DfColour g_textColour = { 0xffb8b8b8 };
double g_drawScale = 1.0;
DfFont *g_font;
DfBitmap *g_fontBitmap;
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


static void draw_frame() {
    BitmapClear(g_window->bmp, g_backgroundColour);

    // Draw proof text
    int columnWidth = 48;
    int zoomFactor = 7;
    int x = 10;
    int y = 10;
    for (int i = 0; i < sizeof(g_proofText); i += columnWidth) {
        DrawTextSimpleLen(g_font, g_textColour, g_window->bmp, x, y, g_proofText + i, columnWidth);
        y += g_font->charHeight;
    }

    // Draw glyph set onto font bitmap.
    BitmapClear(g_fontBitmap, g_backgroundColour);
    for (y = 0; y < 6; y++) {
        int tmpY = y * g_font->charHeight;
        for (x = 0; x < 16; x++) {
            int tmpX = x * g_font->maxCharWidth;
            char s[2] = { x + y * 16 + 32 };
            DrawTextSimple(g_font, g_textColour, g_fontBitmap, tmpX, tmpY, s);
        }
    }

    // Draw zoomed view of font bitmap on main window bitmap.
    x += columnWidth * g_font->maxCharWidth + 10;
    y = 10;
    ScaleUpBlit(g_window->bmp, x, y, zoomFactor, g_fontBitmap);

    // Draw pixel grid over zoomed glyph set.
    for (int i = 0; i <= g_fontBitmap->height; i++) {
        int tmpY = y + i * zoomFactor;
        DfColour col = g_gridColour;
        if ((i % g_font->charHeight) == 0)
            col = g_gridBoldColour;
        HLine(g_window->bmp, x, tmpY, g_fontBitmap->width * zoomFactor + 1, col);
    }
    for (int i = 0; i <= g_fontBitmap->width; i++) {
        int tmpX = x + i * zoomFactor;
        DfColour col = g_gridColour;
        if ((i % g_font->maxCharWidth) == 0)
            col = g_gridBoldColour;
        VLine(g_window->bmp, tmpX, y+1, g_fontBitmap->height * zoomFactor-1, col);
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
    g_fontBitmap = BitmapCreate(g_font->maxCharWidth * 16, g_font->charHeight * 6);


    //
    // Main loop

    double next_force_frame_time = GetRealTime() + 0.2;
    while (!g_window->windowClosed && !g_window->input.keyDowns[KEY_ESC]) {
        bool force_frame = GetRealTime() > next_force_frame_time;
        if (force_frame) {
            next_force_frame_time = GetRealTime() + 0.2;
        }

        if (InputPoll(g_window) || force_frame) {
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
