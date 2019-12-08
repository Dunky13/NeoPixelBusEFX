#include <Arduino.h>
#include <NeoPixelBrightnessBus.h>
#include "faketv.h" //color pattern for FakeTV

#define SPEED_MAX 50
#define ARRAYSIZE 140 //Max LED Count

#define GRBW                               //This is used for SK6812rgbw pixels that have the separate white led in them.
#define NEOPIXEL_LIB NeoPixelBrightnessBus // Neopixel library type
#define METHOD NeoSk6812Method             //NeoEsp8266Uart1800KbpsMethod //GPIO2 - use NeoEsp8266Uart0800KbpsMethod for GPIO1(TX)
#define FEATURE NeoGrbwFeature

typedef NEOPIXEL_LIB<FEATURE, METHOD> NeoPixelBusType;
#define numPixels (sizeof(ftv_colors) / sizeof(ftv_colors[0]))

static const float pi = 3.1415926535897932384626433832795;
static const uint16_t pixelCount = ARRAYSIZE;

class MyLED
{
public:
    MyLED(NeoPixelBusType &pixelStrip) : ledstrip(pixelStrip)
    {
    }
    void init();
    void begin();
    void _handleMode(String string);
    void anim_loop();

private:
    uint16_t pos,
        color,
        r_pixel,
        startpixel,
        endpixel,
        difference,
        fps = 50,
        colorcount;

    RgbwColor rgb_target[ARRAYSIZE],
        rgb_old[ARRAYSIZE],
        rgb, rrggbb,
        rgb_tick_b = HtmlColor(0x505050),
        rgb_tick_s = HtmlColor(0x101010),
        rgb_m = HtmlColor(0x00FF00),
        rgb_h = HtmlColor(0x0000FF),
        rgb_s = HtmlColor(0xFF0000);

    int16_t fadedelay = 20;

    int8_t defaultspeed = 25,
           rainbowspeed = 1,
           speed = 25,
           count = 1,
           rev_intensity = 3;

    uint32_t _counter_mode_step = 0,
             fadetime = 1000,
             ftv_holdTime,
             pixelNum;

    uint16_t ftv_pr = 0, ftv_pg = 0, ftv_pb = 0; // Prev R, G, B;
    uint32_t ftv_totalTime, ftv_fadeTime, ftv_startTime, ftv_elapsed;
    uint16_t ftv_nr, ftv_ng, ftv_nb, ftv_r, ftv_g, ftv_b, ftv_i, ftv_frac;
    uint8_t ftv_hi, ftv_lo, ftv_r8, ftv_g8, ftv_b8;

    String colorStr,
        backgroundcolorStr;

    bool gReverseDirection = false;
    bool rgb_s_off = false;
    bool fadeIn = false;

    byte cooling = 50,
         sparking = 120,
         brightness = 31;

    unsigned long counter20ms = 0,
                  starttime[ARRAYSIZE],
                  starttimerb,
                  maxtime = 0;

    enum modetype
    {
        Off,
        On,
        Fade,
        ColorFade,
        Rainbow,
        Kitt,
        Comet,
        Theatre,
        Scan,
        Dualscan,
        Twinkle,
        TwinkleFade,
        Sparkle,
        Fire,
        FireFlicker,
        Wipe,
        Dualwipe,
        FakeTV
    };
    modetype mode, savemode, lastmode;
    NeoPixelBusType &ledstrip;
    void colorfade(void);
    void comet(void);
    void dualscan(void);
    void dualwipe(void);
    void fade(void);
    void faketv(void);
    void fire_flicker();
    void Fire2012(void);
    void fire(void);
    bool GetArgvBeginEnd(const char *string, const unsigned int argc, int &pos_begin, int &pos_end);
    bool GetArgv(const char *string, String &argvString, unsigned int argc);
    void hex2rgb_pixel(String hexcolor);
    void hex2rgb(String hexcolor);
    void hex2rrggbb(String hexcolor);
    bool isParameterSeparatorChar(char c);
    bool isQuoteChar(char c);
    void kitt(void);
    String parseStringKeepCase(const String &string, byte indexFind);
    String parseString(const String &string, byte indexFind);
    uint8_t qadd8(uint8_t i, uint8_t j);
    uint8_t qsub8(uint8_t i, uint8_t j);
    void rainbow(void);
    uint8_t random8();
    uint8_t random8(uint8_t lim);
    uint8_t random8(uint8_t min, uint8_t lim);
    uint32_t rgbStr2Num(String rgbStr);
    uint8_t scale8_video(uint8_t i, uint8_t scale);
    void scan(void);
    void sparkle(void);
    bool stringWrappedWithChar(const String &text, char wrappingChar);
    String stripQuotes(const String &text);
    String stripWrappingChar(const String &text, char wrappingChar);
    void theatre(void);
    void twinklefade(void);
    void twinkle(void);
    uint32_t Wheel(uint8_t pos);
    void wipe(void);
};
