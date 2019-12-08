#include "flashy.h"

void MyLED::init()
{
    rgb_tick_b = HtmlColor(0x505050);
    rgb_tick_s = HtmlColor(0x101010);
    rgb_m = HtmlColor(0x00FF00);
    rgb_h = HtmlColor(0x0000FF);
    rgb_s = HtmlColor(0xFF0000);

    fadedelay = 20;

    defaultspeed = 25;
    rainbowspeed = 1;
    speed = 25;
    count = 1;
    rev_intensity = 3;

    _counter_mode_step = 0;
    fadetime = 1000;

    ftv_pr = 0, ftv_pg = 0, ftv_pb = 0;

    gReverseDirection = false;
    rgb_s_off = false;
    fadeIn = false;

    cooling = 50,
    sparking = 120,
    brightness = 31;

    counter20ms = 0;
    maxtime = 0;
}

void MyLED::begin()
{
    // ledstrip.Begin(); // This initializes the NeoPixelBus library.
    this->_handleMode("rainbow");
}

String MyLED::stripWrappingChar(const String &text, char wrappingChar)
{
    unsigned int length = text.length();

    if ((length >= 2) && stringWrappedWithChar(text, wrappingChar))
    {
        return text.substring(1, length - 1);
    }
    return text;
}

bool MyLED::stringWrappedWithChar(const String &text, char wrappingChar)
{
    unsigned int length = text.length();

    if (length < 2)
    {
        return false;
    }

    if (text.charAt(0) != wrappingChar)
    {
        return false;
    }
    return text.charAt(length - 1) == wrappingChar;
}

bool MyLED::isQuoteChar(char c)
{
    return c == '\'' || c == '"' || c == '`';
}
bool MyLED::isParameterSeparatorChar(char c)
{
    return c == ',' || c == ' ';
}
String MyLED::stripQuotes(const String &text)
{
    if (text.length() >= 2)
    {
        char c = text.charAt(0);

        if (isQuoteChar(c))
        {
            return stripWrappingChar(text, c);
        }
    }
    return text;
}

bool MyLED::GetArgv(const char *string, String &argvString, unsigned int argc)
{
    int pos_begin, pos_end;
    bool hasArgument = GetArgvBeginEnd(string, argc, pos_begin, pos_end);
    argvString = "";
    if (!hasArgument)
        return false;
    if (pos_begin >= 0 && pos_end >= 0 && pos_end > pos_begin)
    {
        argvString.reserve(pos_end - pos_begin);
        for (int i = pos_begin; i < pos_end; ++i)
        {
            argvString += string[i];
        }
    }
    argvString.trim();
    argvString = stripQuotes(argvString);
    return argvString.length() > 0;
}

bool MyLED::GetArgvBeginEnd(const char *string, const unsigned int argc, int &pos_begin, int &pos_end)
{
    pos_begin = -1;
    pos_end = -1;
    size_t string_len = strlen(string);
    unsigned int string_pos = 0, argc_pos = 0;
    bool parenthesis = false;
    char matching_parenthesis = '"';

    while (string_pos < string_len)
    {
        char c, d; // c = current char, d = next char (if available)
        c = string[string_pos];
        d = 0;

        if ((string_pos + 1) < string_len)
        {
            d = string[string_pos + 1];
        }

        if (!parenthesis && (c == ' ') && (d == ' '))
        {
        }
        else if (!parenthesis && (c == ' ') && (d == ','))
        {
        }
        else if (!parenthesis && (c == ',') && (d == ' '))
        {
        }
        else if (!parenthesis && (c == ' ') && (d >= 33) && (d <= 126))
        {
        }
        else if (!parenthesis && (c == ',') && (d >= 33) && (d <= 126))
        {
        }
        else
        {
            if (!parenthesis && (isQuoteChar(c) || (c == '[')))
            {
                parenthesis = true;
                matching_parenthesis = c;

                if (c == '[')
                {
                    matching_parenthesis = ']';
                }
            }
            else if (parenthesis && (c == matching_parenthesis))
            {
                parenthesis = false;
            }

            if (pos_begin == -1)
            {
                pos_begin = string_pos;
                pos_end = string_pos;
            }
            ++pos_end;

            if (!parenthesis && (isParameterSeparatorChar(d) || (d == 0))) // end of word
            {
                argc_pos++;

                if (argc_pos == argc)
                {
                    return true;
                }
                pos_begin = -1;
                pos_end = -1;
                string_pos++;
            }
        }
        string_pos++;
    }
    return false;
}

String MyLED::parseString(const String &string, byte indexFind)
{
    String result = parseStringKeepCase(string, indexFind);
    result.toLowerCase();
    return result;
}

String MyLED::parseStringKeepCase(const String &string, byte indexFind)
{
    String result;
    if (!GetArgv(string.c_str(), result, indexFind))
    {
        return "";
    }
    result.trim();
    return stripQuotes(result);
}

void MyLED::fade(void)
{
    for (int pixel = 0; pixel < pixelCount; pixel++)
    {
        long zaehler = 20 * (counter20ms - starttime[pixel]);
        float progress = (float)zaehler / (float)fadetime;
        progress = (progress < 0) ? 0 : progress;
        progress = (progress > 1) ? 1 : progress;

#if defined(RGBW) || defined(GRBW)
        RgbwColor updatedColor = RgbwColor::LinearBlend(
            rgb_old[pixel], rgb_target[pixel],
            progress);
#else
        RgbColor updatedColor = RgbColor::LinearBlend(
            rgb_old[pixel], rgb_target[pixel],
            progress);
#endif

        if (counter20ms > maxtime && ledstrip.GetPixelColor(pixel).CalculateBrightness() == 0)
        {
            mode = Off;
        }
        else if (counter20ms > maxtime)
        {
            mode = On;
        }

        ledstrip.SetPixelColor(pixel, updatedColor);
    }
}

void MyLED::colorfade(void)
{
    float progress = 0;
    difference = (endpixel - startpixel + pixelCount) % pixelCount;
    for (uint16_t i = 0; i <= difference; i++)
    {

        progress = (float)i / (difference - 1);
        progress = (progress >= 1) ? 1 : progress;
        progress = (progress <= 0) ? 0 : progress;

#if defined(RGBW) || defined(GRBW)
        RgbwColor updatedColor = RgbwColor::LinearBlend(
            rgb, rrggbb,
            progress);
#else
        RgbColor updatedColor = RgbColor::LinearBlend(
            rgb, rrggbb,
            progress);
#endif

        ledstrip.SetPixelColor((i + startpixel) % pixelCount, updatedColor);
    }
    mode = On;
}

void MyLED::wipe(void)
{
    if (counter20ms % (SPEED_MAX / abs(speed)) == 0)
    {
        if (speed > 0)
        {
            ledstrip.SetPixelColor(_counter_mode_step, rrggbb);
            if (_counter_mode_step > 0)
                ledstrip.SetPixelColor(_counter_mode_step - 1, rgb);
        }
        else
        {
            ledstrip.SetPixelColor(pixelCount - _counter_mode_step - 1, rrggbb);
            if (_counter_mode_step > 0)
                ledstrip.SetPixelColor(pixelCount - _counter_mode_step, rgb);
        }
        if (_counter_mode_step == pixelCount)
            mode = On;
        _counter_mode_step++;
    }
}

void MyLED::dualwipe(void)
{
    if (counter20ms % (SPEED_MAX / abs(speed)) == 0)
    {
        if (speed > 0)
        {
            int i = _counter_mode_step - pixelCount;
            i = abs(i);
            ledstrip.SetPixelColor(_counter_mode_step, rrggbb);
            ledstrip.SetPixelColor(i, rgb);
            if (_counter_mode_step > 0)
            {
                ledstrip.SetPixelColor(_counter_mode_step - 1, rgb);
                ledstrip.SetPixelColor(i - 1, rrggbb);
            }
        }
        else
        {
            int i = (pixelCount / 2) - _counter_mode_step;
            i = abs(i);
            ledstrip.SetPixelColor(_counter_mode_step + (pixelCount / 2), rrggbb);
            ledstrip.SetPixelColor(i, rgb);
            if (_counter_mode_step > 0)
            {
                ledstrip.SetPixelColor(_counter_mode_step + (pixelCount / 2) - 1, rgb);
                ledstrip.SetPixelColor(i - 1, rrggbb);
            }
        }
        if (_counter_mode_step >= pixelCount / 2)
        {
            mode = On;
            ledstrip.SetPixelColor(_counter_mode_step - 1, rgb);
        }
        _counter_mode_step++;
    }
}

void MyLED::faketv(void)
{
    if (counter20ms >= ftv_holdTime)
    {

        difference = abs(endpixel - startpixel);

        if (ftv_elapsed >= ftv_fadeTime)
        {
            // Read next 16-bit (5/6/5) color
            ftv_hi = pgm_read_byte(&ftv_colors[pixelNum * 2]);
            ftv_lo = pgm_read_byte(&ftv_colors[pixelNum * 2 + 1]);
            if (++pixelNum >= numPixels)
                pixelNum = 0;

            // Expand to 24-bit (8/8/8)
            ftv_r8 = (ftv_hi & 0xF8) | (ftv_hi >> 5);
            ftv_g8 = (ftv_hi << 5) | ((ftv_lo & 0xE0) >> 3) | ((ftv_hi & 0x06) >> 1);
            ftv_b8 = (ftv_lo << 3) | ((ftv_lo & 0x1F) >> 2);
            // Apply gamma correction, further expand to 16/16/16
            ftv_nr = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_r8]) * 257; // New R/G/B
            ftv_ng = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_g8]) * 257;
            ftv_nb = (uint8_t)pgm_read_byte(&ftv_gamma8[ftv_b8]) * 257;

            ftv_totalTime = random(12, 125);         // Semi-random pixel-to-pixel time
            ftv_fadeTime = random(0, ftv_totalTime); // Pixel-to-pixel transition time
            if (random(10) < 3)
                ftv_fadeTime = 0;                                      // Force scene cut 30% of time
            ftv_holdTime = counter20ms + ftv_totalTime - ftv_fadeTime; // Non-transition time
            ftv_startTime = counter20ms;
        }

        ftv_elapsed = counter20ms - ftv_startTime;
        if (ftv_fadeTime)
        {
            ftv_r = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pr, ftv_nr); // 16-bit interp
            ftv_g = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pg, ftv_ng);
            ftv_b = map(ftv_elapsed, 0, ftv_fadeTime, ftv_pb, ftv_nb);
        }
        else
        { // Avoid divide-by-ftv_fraczero in map()
            ftv_r = ftv_nr;
            ftv_g = ftv_ng;
            ftv_b = ftv_nb;
        }

        for (ftv_i = 0; ftv_i < difference; ftv_i++)
        {
            ftv_r8 = ftv_r >> 8; // Quantize to 8-bit
            ftv_g8 = ftv_g >> 8;
            ftv_b8 = ftv_b >> 8;
            ftv_frac = (ftv_i << 16) / difference; // LED index scaled to 0-65535 (16Bit)
            if ((ftv_r8 < 255) && ((ftv_r & 0xFF) >= ftv_frac))
                ftv_r8++; // Boost some fraction
            if ((ftv_g8 < 255) && ((ftv_g & 0xFF) >= ftv_frac))
                ftv_g8++; // of LEDs to handle
            if ((ftv_b8 < 255) && ((ftv_b & 0xFF) >= ftv_frac))
                ftv_b8++; // interp > 8bit
            ledstrip.SetPixelColor(ftv_i + startpixel, RgbColor(ftv_r8, ftv_g8, ftv_b8));
        }

        ftv_pr = ftv_nr; // Prev RGB = new RGB
        ftv_pg = ftv_ng;
        ftv_pb = ftv_nb;
    }
}

/*
* Cycles a rainbow over the entire string of LEDs.
*/
void MyLED::rainbow(void)
{
    long zaehler = 20 * (counter20ms - starttimerb);
    float progress = (float)zaehler / (float)fadetime;
    if (fadeIn == true)
    {
        ledstrip.SetBrightness(progress * 255);
        fadeIn = (progress == 1) ? false : true;
    }
    for (int i = 0; i < pixelCount; i++)
    {
        uint8_t r1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 16);
        uint8_t g1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255) >> 8);
        uint8_t b1 = (Wheel(((i * 256 / pixelCount) + counter20ms * rainbowspeed / 10) & 255));
        ledstrip.SetPixelColor(i, RgbColor(r1, g1, b1));
    }
    mode = (rainbowspeed == 0) ? On : Rainbow;
}

/*
* Put a value 0 to 255 in to get a color value.
* The colours are a transition r -> g -> b -> back to r
* Inspired by the Adafruit examples.
*/
uint32_t MyLED::Wheel(uint8_t pos)
{
    pos = 255 - pos;
    if (pos < 85)
    {
        return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
    }
    else if (pos < 170)
    {
        pos -= 85;
        return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
    }
    else
    {
        pos -= 170;
        return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
    }
}

// Larson Scanner K.I.T.T.
void MyLED::kitt(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0)
    {
        for (uint16_t i = 0; i < pixelCount; i++)
        {

#if defined(RGBW) || defined(GRBW)
            RgbwColor px_rgb = ledstrip.GetPixelColor(i);

            // fade out (divide by 2)
            px_rgb.R = px_rgb.R >> 1;
            px_rgb.G = px_rgb.G >> 1;
            px_rgb.B = px_rgb.B >> 1;
            px_rgb.W = px_rgb.W >> 1;

#else

            RgbColor px_rgb = ledstrip.GetPixelColor(i);

            // fade out (divide by 2)
            px_rgb.R = px_rgb.R >> 1;
            px_rgb.G = px_rgb.G >> 1;
            px_rgb.B = px_rgb.B >> 1;
#endif

            ledstrip.SetPixelColor(i, px_rgb);
        }

        uint16_t pos = 0;

        if (_counter_mode_step < pixelCount)
        {
            pos = _counter_mode_step;
        }
        else
        {
            pos = (pixelCount * 2) - _counter_mode_step - 2;
        }

        ledstrip.SetPixelColor(pos, rgb);

        _counter_mode_step = (_counter_mode_step + 1) % ((pixelCount * 2) - 2);
    }
}

//Firing comets from one end.
void MyLED::comet(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0)
    {
        for (uint16_t i = 0; i < pixelCount; i++)
        {

            if (speed > 0)
            {

#if defined(RGBW) || defined(GRBW)
                RgbwColor px_rgb = ledstrip.GetPixelColor(i);

                // fade out (divide by 2)
                px_rgb.R = px_rgb.R >> 1;
                px_rgb.G = px_rgb.G >> 1;
                px_rgb.B = px_rgb.B >> 1;
                px_rgb.W = px_rgb.W >> 1;

#else

                RgbColor px_rgb = ledstrip.GetPixelColor(i);

                // fade out (divide by 2)
                px_rgb.R = px_rgb.R >> 1;
                px_rgb.G = px_rgb.G >> 1;
                px_rgb.B = px_rgb.B >> 1;
#endif

                ledstrip.SetPixelColor(i, px_rgb);
            }
            else
            {

#if defined(RGBW) || defined(GRBW)
                RgbwColor px_rgb = ledstrip.GetPixelColor(pixelCount - i - 1);

                // fade out (divide by 2)
                px_rgb.R = px_rgb.R >> 1;
                px_rgb.G = px_rgb.G >> 1;
                px_rgb.B = px_rgb.B >> 1;
                px_rgb.W = px_rgb.W >> 1;

#else

                RgbColor px_rgb = ledstrip.GetPixelColor(pixelCount - i - 1);

                // fade out (divide by 2)
                px_rgb.R = px_rgb.R >> 1;
                px_rgb.G = px_rgb.G >> 1;
                px_rgb.B = px_rgb.B >> 1;
#endif

                ledstrip.SetPixelColor(pixelCount - i - 1, px_rgb);
            }
        }

        if (speed > 0)
        {
            ledstrip.SetPixelColor(_counter_mode_step, rgb);
        }
        else
        {
            ledstrip.SetPixelColor(pixelCount - _counter_mode_step - 1, rgb);
        }

        _counter_mode_step = (_counter_mode_step + 1) % pixelCount;
    }
}

//Theatre lights
void MyLED::theatre(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        if (speed > 0)
        {
            ledstrip.RotateLeft(1, 0, (pixelCount / count) * count - 1);
        }
        else
        {
            ledstrip.RotateRight(1, 0, (pixelCount / count) * count - 1);
        }
    }
}

/*
* Runs a single pixel back and forth.
*/
void MyLED::scan(void)
{
    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        if (_counter_mode_step > uint16_t((pixelCount * 2) - 2))
        {
            _counter_mode_step = 0;
        }
        _counter_mode_step++;

        int i = _counter_mode_step - (pixelCount - 1);
        i = abs(i);

        //ledstrip.ClearTo(rrggbb);
        for (int i = 0; i < pixelCount; i++)
            ledstrip.SetPixelColor(i, rrggbb);
        ledstrip.SetPixelColor(abs(i), rgb);
    }
}

/*
* Runs two pixel back and forth in opposite directions.
*/
void MyLED::dualscan(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        if (_counter_mode_step > uint16_t((pixelCount * 2) - 2))
        {
            _counter_mode_step = 0;
        }

        _counter_mode_step++;

        int i = _counter_mode_step - (pixelCount - 1);
        i = abs(i);

        //ledstrip.ClearTo(rrggbb);
        for (int i = 0; i < pixelCount; i++)
            ledstrip.SetPixelColor(i, rrggbb);
        ledstrip.SetPixelColor(abs(i), rgb);
        ledstrip.SetPixelColor(pixelCount - (i + 1), rgb);
    }
}

/*
* Blink several LEDs on, reset, repeat.
* Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
*/
void MyLED::twinkle(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        if (_counter_mode_step == 0)
        {
            //ledstrip.ClearTo(rrggbb);
            for (int i = 0; i < pixelCount; i++)
                ledstrip.SetPixelColor(i, rrggbb);
            uint16_t min_leds = _max(1, pixelCount / 5); // make sure, at least one LED is on
            uint16_t max_leds = _max(1, pixelCount / 2); // make sure, at least one LED is on
            _counter_mode_step = random(min_leds, max_leds);
        }

        ledstrip.SetPixelColor(random(pixelCount), rgb);

        _counter_mode_step--;
    }
}

/*
* Blink several LEDs on, fading out.
*/
void MyLED::twinklefade(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        for (uint16_t i = 0; i < pixelCount; i++)
        {

#if defined(RGBW) || defined(GRBW)
            RgbwColor px_rgb = ledstrip.GetPixelColor(pixelCount - i - 1);

            // fade out (divide by 2)
            px_rgb.R = px_rgb.R >> 1;
            px_rgb.G = px_rgb.G >> 1;
            px_rgb.B = px_rgb.B >> 1;
            px_rgb.W = px_rgb.W >> 1;

#else

            RgbColor px_rgb = ledstrip.GetPixelColor(pixelCount - i - 1);

            // fade out (divide by 2)
            px_rgb.R = px_rgb.R >> 1;
            px_rgb.G = px_rgb.G >> 1;
            px_rgb.B = px_rgb.B >> 1;
#endif

            ledstrip.SetPixelColor(i, px_rgb);
        }

        if (random(count) < 50)
        {
            ledstrip.SetPixelColor(random(pixelCount), rgb);
        }
    }
}

/*
* Blinks one LED at a time.
* Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
*/
void MyLED::sparkle(void)
{

    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        //ledstrip.ClearTo(rrggbb);
        for (int i = 0; i < pixelCount; i++)
            ledstrip.SetPixelColor(i, rrggbb);
        ledstrip.SetPixelColor(random(pixelCount), rgb);
    }
}

//Fire
unsigned long fireTimer;
RgbColor leds[ARRAYSIZE];

void MyLED::fire(void)
{

    if (counter20ms > fireTimer + 50 / fps)
    {
        fireTimer = counter20ms;
        Fire2012();
        RgbColor pixel;

        for (int i = 0; i < pixelCount; i++)
        {
            pixel = leds[i];
            pixel = RgbColor::LinearBlend(pixel, RgbColor(0, 0, 0), (255 - brightness) / 255.0);
            ledstrip.SetPixelColor(i, pixel);
        }
    }
}

/// random number seed
uint16_t rand16seed; // = RAND16_SEED;

/// Generate an 8-bit random number
uint8_t MyLED::random8()
{
    rand16seed = (rand16seed * ((uint16_t)(2053))) + ((uint16_t)(13849));
    // return the sum of the high and low bytes, for better
    //  mixing and non-sequential correlation
    return (uint8_t)(((uint8_t)(rand16seed & 0xFF)) +
                     ((uint8_t)(rand16seed >> 8)));
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result
uint8_t MyLED::random8(uint8_t lim)
{
    uint8_t r = random8();
    r = (r * lim) >> 8;
    return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number
/// @param lim the upper bound for the random number
uint8_t MyLED::random8(uint8_t min, uint8_t lim)
{
    uint8_t delta = lim - min;
    uint8_t r = random8(delta) + min;
    return r;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
uint8_t MyLED::qsub8(uint8_t i, uint8_t j)
{
    int t = i - j;
    if (t < 0)
        t = 0;
    return t;
}

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
uint8_t MyLED::qadd8(uint8_t i, uint8_t j)
{
    unsigned int t = i + j;
    if (t > 255)
        t = 255;
    return t;
}

///  The "video" version of scale8 guarantees that the output will
///  be only be zero if one or both of the inputs are zero.  If both
///  inputs are non-zero, the output is guaranteed to be non-zero.
///  This makes for better 'video'/LED dimming, at the cost of
///  several additional cycles.
uint8_t MyLED::scale8_video(uint8_t i, uint8_t scale)
{
    uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
    // uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
    // uint8_t j = (i == 0) ? 0 : (((int)i * (int)(scale) ) >> 8) + nonzeroscale;
    return j;
}

void MyLED::Fire2012(void)
{
    // Array of temperature readings at each simulation cell
    static byte heat[ARRAYSIZE];

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < pixelCount; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / pixelCount) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = pixelCount - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < sparking)
    {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < pixelCount; j++)
    {

        RgbColor heatcolor;

        // Scale 'heat' down from 0-255 to 0-191,
        // which can then be easily divided into three
        // equal 'thirds' of 64 units each.
        uint8_t t192 = scale8_video(heat[j], 191);

        // calculate a value that ramps up from
        // zero to 255 in each 'third' of the scale.
        uint8_t heatramp = t192 & 0x3F; // 0..63
        heatramp <<= 2;                 // scale up to 0..252

        // now figure out which third of the spectrum we're in:
        if (t192 & 0x80)
        {
            // we're in the hottest third
            heatcolor.R = 255;      // full red
            heatcolor.G = 255;      // full green
            heatcolor.B = heatramp; // ramp up blue
        }
        else if (t192 & 0x40)
        {
            // we're in the middle third
            heatcolor.R = 255;      // full red
            heatcolor.G = heatramp; // ramp up green
            heatcolor.B = 0;        // no blue
        }
        else
        {
            // we're in the coolest third
            heatcolor.R = heatramp; // ramp up red
            heatcolor.G = 0;        // no green
            heatcolor.B = 0;        // no blue
        }

        int pixelnumber;
        if (gReverseDirection)
        {
            pixelnumber = (pixelCount - 1) - j;
        }
        else
        {
            pixelnumber = j;
        }
        leds[pixelnumber] = heatcolor;
    }
}

/*
 * Fire flicker function
 */
void MyLED::fire_flicker()
{
    if (counter20ms % (SPEED_MAX / abs(speed)) == 0 && speed != 0)
    {
        byte w = 0;   //(SEGMENT.colors[0] >> 24) & 0xFF;
        byte r = 255; //(SEGMENT.colors[0] >> 16) & 0xFF;
        byte g = 96;  //(SEGMENT.colors[0] >>  8) & 0xFF;
        byte b = 12;  //(SEGMENT.colors[0]        & 0xFF);
        byte lum = max(w, max(r, max(g, b))) / rev_intensity;
        for (uint16_t i = 0; i <= numPixels - 1; i++)
        {
            int flicker = random8(lum);

#if defined(RGBW) || defined(GRBW)
            ledstrip.SetPixelColor(i, RgbwColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0)));
#else
            ledstrip.SetPixelColor(i, RgbColor(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0)));
#endif
        }
    }
}

uint32_t MyLED::rgbStr2Num(String rgbStr)
{
    uint32_t rgbDec = (int)strtoul(&rgbStr[0], NULL, 16);
    return rgbDec;
}

void MyLED::hex2rgb(String hexcolor)
{
    colorStr = hexcolor;
#if defined(RGBW) || defined(GRBW)
    hexcolor.length() <= 6
        ? rgb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
        : rgb = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#else
    rgb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#endif
}

void MyLED::hex2rrggbb(String hexcolor)
{
    backgroundcolorStr = hexcolor;
#if defined(RGBW) || defined(GRBW)
    hexcolor.length() <= 6
        ? rrggbb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
        : rrggbb = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#else
    rrggbb = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#endif
}

void MyLED::hex2rgb_pixel(String hexcolor)
{
    colorStr = hexcolor;
    for (int i = 0; i < pixelCount; i++)
    {
#if defined(RGBW) || defined(GRBW)
        hexcolor.length() <= 6
            ? rgb_target[i] = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor))
            : rgb_target[i] = RgbwColor(rgbStr2Num(hexcolor) >> 24, rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#else
        rgb_target[i] = RgbColor(rgbStr2Num(hexcolor) >> 16, rgbStr2Num(hexcolor) >> 8, rgbStr2Num(hexcolor));
#endif
    }
}

void MyLED::anim_loop()
{
    counter20ms++;

    switch (this->mode)
    {
    case Fade:
        this->fade();
        break;

    case ColorFade:
        this->colorfade();
        break;

    case Rainbow:
        this->rainbow();
        break;

    case Kitt:
        this->kitt();
        break;

    case Comet:
        this->comet();
        break;

    case Theatre:
        this->theatre();
        break;

    case Scan:
        this->scan();
        break;

    case Dualscan:
        this->dualscan();
        break;

    case Twinkle:
        this->twinkle();
        break;

    case TwinkleFade:
        this->twinklefade();
        break;

    case Sparkle:
        this->sparkle();
        break;

    case Fire:
        this->fire();
        break;

    case FireFlicker:
        this->fire_flicker();
        break;

    case Wipe:
        this->wipe();
        break;

    case Dualwipe:
        this->dualwipe();
        break;

    case FakeTV:
        this->faketv();
        break;

    default:
        break;
    } // switch mode

    this->ledstrip.Show();
}

void MyLED::_handleMode(String string)
{
    string = "abc " + string;
    String subCommand = parseString(string, 2);
    if (subCommand == F("line"))
    {
        mode = On;

        hex2rgb(parseString(string, 5));

        for (int i = 0; i <= (parseString(string, 4).toInt() - parseString(string, 3).toInt() + pixelCount) % pixelCount; i++)
        {
            ledstrip.SetPixelColor((i + parseString(string, 3).toInt() - 1) % pixelCount, rgb);
        }
    }

    else if (subCommand == F("tick"))
    {
        mode = On;

        hex2rgb(parseString(string, 4));

        //          for (int i = 0; i < pixelCount ; i = i + (pixelCount / parseString(string, 3).toInt())) {
        for (int i = 0; i < parseString(string, 3).toInt(); i++)
        {
            ledstrip.SetPixelColor(i * pixelCount / parseString(string, 3).toInt(), rgb);
        }
    }

    else if (subCommand == F("one"))
    {
        mode = On;

        uint16_t pixnum = parseString(string, 3).toInt() - 1;
        hex2rgb(parseString(string, 4));

        ledstrip.SetPixelColor(pixnum, rgb);
    }

    else if (subCommand == F("fade") || subCommand == F("all") || subCommand == F("rgb"))
    {
        mode = Fade;

        if (subCommand == F("all") || subCommand == F("rgb"))
        {
            fadedelay = 0;
        }

        hex2rgb(parseString(string, 3));
        hex2rgb_pixel(parseString(string, 3));

        fadetime = (parseString(string, 4) == "")
                       ? fadetime
                       : parseString(string, 4).toInt();
        fadedelay = (parseString(string, 5) == "")
                        ? fadedelay
                        : parseString(string, 5).toInt();

        for (int pixel = 0; pixel < pixelCount; pixel++)
        {

            r_pixel = (fadedelay < 0)
                          ? pixelCount - pixel - 1
                          : pixel;

            starttime[r_pixel] = counter20ms + (pixel * abs(fadedelay) / 20);

            rgb_old[pixel] = ledstrip.GetPixelColor(pixel);
        }
        maxtime = starttime[r_pixel] + (fadetime / 20);
    }

    else if (subCommand == F("rainbow"))
    {
        fadeIn = (mode == Off) ? true : false;
        mode = Rainbow;
        starttimerb = counter20ms;

        rainbowspeed = (parseString(string, 3) == "")
                           ? speed
                           : parseString(string, 3).toInt();

        fadetime = (parseString(string, 4) == "")
                       ? fadetime
                       : parseString(string, 4).toInt();
    }

    else if (subCommand == F("colorfade"))
    {
        mode = ColorFade;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
            hex2rrggbb(parseString(string, 4));

        startpixel = (parseString(string, 5) == "")
                         ? 0
                         : parseString(string, 5).toInt() - 1;
        endpixel = (parseString(string, 6) == "")
                       ? pixelCount - 1
                       : parseString(string, 6).toInt() - 1;
    }

    else if (subCommand == F("kitt"))
    {
        mode = Kitt;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));

        speed = (parseString(string, 4) == "")
                    ? defaultspeed
                    : parseString(string, 4).toInt();
    }

    else if (subCommand == F("comet"))
    {
        mode = Comet;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));

        speed = (parseString(string, 4) == "")
                    ? defaultspeed
                    : parseString(string, 4).toInt();
    }

    else if (subCommand == F("theatre"))
    {
        mode = Theatre;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
            hex2rrggbb(parseString(string, 4));

        count = (parseString(string, 5) == "")
                    ? count
                    : parseString(string, 5).toInt();

        speed = (parseString(string, 6) == "")
                    ? defaultspeed
                    : parseString(string, 6).toInt();

        for (int i = 0; i < pixelCount; i++)
        {
            if ((i / count) % 2 == 0)
            {
                ledstrip.SetPixelColor(i, rgb);
            }
            else
            {
                ledstrip.SetPixelColor(i, rrggbb);
            }
        }
    }

    else if (subCommand == F("scan"))
    {
        mode = Scan;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
            hex2rrggbb(parseString(string, 4));

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("dualscan"))
    {
        mode = Dualscan;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
            hex2rrggbb(parseString(string, 4));

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("twinkle"))
    {
        mode = Twinkle;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
            hex2rrggbb(parseString(string, 4));

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("twinklefade"))
    {
        mode = TwinkleFade;

        hex2rgb(parseString(string, 3));

        count = (parseString(string, 4) == "")
                    ? count
                    : parseString(string, 4).toInt();

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("sparkle"))
    {
        mode = Sparkle;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        hex2rrggbb(parseString(string, 4));

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("wipe"))
    {
        mode = Wipe;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
        {
            hex2rrggbb(parseString(string, 4));
        }
        else
        {
            hex2rrggbb("000000");
        }

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("dualwipe"))
    {
        mode = Dualwipe;

        _counter_mode_step = 0;

        hex2rgb(parseString(string, 3));
        if (parseString(string, 4) != "")
        {
            hex2rrggbb(parseString(string, 4));
        }
        else
        {
            hex2rrggbb("000000");
        }

        speed = (parseString(string, 5) == "")
                    ? defaultspeed
                    : parseString(string, 5).toInt();
    }

    else if (subCommand == F("faketv"))
    {
        mode = FakeTV;
        _counter_mode_step = 0;

        randomSeed(analogRead(A0));
        pixelNum = random(numPixels); // Begin at random point

        startpixel = (parseString(string, 3) == "")
                         ? 0
                         : parseString(string, 3).toInt() - 1;
        endpixel = (parseString(string, 4) == "")
                       ? pixelCount
                       : parseString(string, 4).toInt();
    }

    else if (subCommand == F("fire"))
    {
        mode = Fire;

        fps = (parseString(string, 3) == "")
                  ? fps
                  : parseString(string, 3).toInt();

        fps = (fps == 0 || fps > 50) ? 50 : fps;

        brightness = (parseString(string, 4) == "")
                         ? brightness
                         : parseString(string, 4).toFloat();
        cooling = (parseString(string, 5) == "")
                      ? cooling
                      : parseString(string, 5).toFloat();
        sparking = (parseString(string, 6) == "")
                       ? sparking
                       : parseString(string, 6).toFloat();
    }

    else if (subCommand == F("fireflicker"))
    {
        mode = FireFlicker;

        rev_intensity = (parseString(string, 3) == "")
                            ? rev_intensity
                            : parseString(string, 3).toInt();

        speed = (parseString(string, 4) == "")
                    ? defaultspeed
                    : parseString(string, 4).toInt();
    }
}