#include "pch.h"

#include "ColorUtils.h"

const RGBA RGBA::RED = {237, 10, 63, 255};
const RGBA RGBA::ORANGE = {255, 136, 100, 255};
const RGBA RGBA::YELLOW = {252, 232, 131, 255};
const RGBA RGBA::GREEN = {58, 166, 85, 255};
const RGBA RGBA::AQUAMARINE = {149, 224, 232, 255};
const RGBA RGBA::BLUE_I = {46, 180, 230, 255};
const RGBA RGBA::BLUE_II = {69, 112, 230, 255};
const RGBA RGBA::BLUE_III = {0, 102, 255, 255};
const RGBA RGBA::DEEP_BLUE = {0, 0, 89, 255};
const RGBA RGBA::VIOLET_I = {115, 46, 108, 255};
const RGBA RGBA::VIOLET_II = {131, 89, 163, 255};
const RGBA RGBA::LAVENDER = {251, 174, 210, 255};
const RGBA RGBA::BROWN = {175, 89, 62, 255};
const RGBA RGBA::GOLD_II = {230, 190, 138, 255};
const RGBA RGBA::SILVER = {201, 192, 187, 255};
const RGBA RGBA::BLACK = {0, 0, 0, 255};
const RGBA RGBA::GRAY = {139, 134, 128, 255};
const RGBA RGBA::WHITE = {255, 255, 255, 255};

BYTE HSLtoRGB_SubFunction(const float &temp1, const float &temp2, const float temp3)
{
    if ((temp3 * 6) < 1)
        return (BYTE)((temp2 + (temp1 - temp2) * 6 * temp3) * 100);
	if ((temp3 * 2) < 1)
        return (BYTE)(temp1 * 100);
    if ((temp3 * 3) < 2)
        return (BYTE)((temp2 + (temp1 - temp2) * (0.66666f - temp3) * 6) * 100);
    return (BYTE)(temp2 * 100);
}

HSV RGBAtoHSV(RGBA rgba) 
{ 
	float r_percent = ((float)rgba.r) / 255; 
	float g_percent = ((float)rgba.g) / 255; 
	float b_percent = ((float)rgba.b) / 255; 

	float max_color = fmax(r_percent, fmax(g_percent, b_percent));
	float min_color = fmin(r_percent, fmin(g_percent, b_percent));

	float L = 0;
    float S = 0;
    float H = 0;

	L = (max_color + min_color) / 2;

	if (max_color != min_color)
	{
		if (L < 0.5f)
		{
            S = (max_color - min_color) / (max_color + min_color);
		}
		else
		{
            S = (max_color - min_color) / (2 - max_color - min_color);
		}

		if (max_color == r_percent)
		{
            H = (g_percent - b_percent) / (max_color - min_color);
		}
		if (max_color == g_percent)
		{
            H = 2 + (b_percent - r_percent) / (max_color - min_color);
		}
        if (max_color == b_percent)
        {
            H = 4 + (r_percent - g_percent) / (max_color - min_color);
        }
	}

	H = H * 60;
    if (H < 0)
        H += 360;

	return HSV((SHORT)H, (SHORT)(S * 100), (SHORT)(L * 100), rgba.a);
}

RGBA HSVtoRGBA(HSV hsv) 
{ 
	BYTE r, g, b;
    r = g = b = 0;

	float L = ((float)hsv.v) / 100;
	float S = ((float)hsv.s) / 100;
	float H = ((float)hsv.h) / 360;

	if (hsv.s == 0)
	{
        r = hsv.v;
        g = hsv.v;
        b = hsv.v;
	}
	else
	{
        float temp1 = (L < 0.5f) ? L * (1 + S) : L + S - (L * S);
        float temp2 = 2.0 * L - temp1;
        float temp3 = 0.0f;

		// Red
        temp3 = H + 0.3333f;
        if (temp3 > 1)
            temp3 -= 1;
        r = HSLtoRGB_SubFunction(temp1, temp2, temp3);

		temp3 = H;
        g = HSLtoRGB_SubFunction(temp1, temp2, temp3);

		temp3 = H - 0.3333f;
        if (temp3 < 0)
            temp3 += 1;
        b = HSLtoRGB_SubFunction(temp1, temp2, temp3);
	}

	r = (BYTE)((((float)r) / 100) * 255);
	g = (BYTE)((((float)g) / 100) * 255);
	b = (BYTE)((((float)b) / 100) * 255);

	return RGBA(r, g, b, hsv.a);
}

RGBA BrightenColor(RGBA color, float amount) 
{ 
	HSV hsv = RGBAtoHSV(color);

    hsv.v += (SHORT)(100.0f * amount);
    hsv.v = min(hsv.v, 100);
	
	return HSVtoRGBA(hsv);
}

RGBA DarkenColor(RGBA color, float amount)
{
    HSV hsv = RGBAtoHSV(color);

	SHORT c = (SHORT)(100.0f * amount);
    hsv.v = (c >= hsv.v) ? 0 : hsv.v - c;

    return HSVtoRGBA(hsv);
}
