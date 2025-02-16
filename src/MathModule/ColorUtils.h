#pragma once

struct RGBA
{
    BYTE r = 0;
    BYTE g = 0;
    BYTE b = 0;
    BYTE a = 0;

    RGBA() = default;
    RGBA(BYTE inR, BYTE inG, BYTE inB, BYTE inA = 255) : r(inR), g(inG), b(inB), a(inA) {}

    
    static const RGBA RED;
    static const RGBA ORANGE;
    static const RGBA YELLOW;
    static const RGBA GREEN;
    static const RGBA AQUAMARINE;
    static const RGBA BLUE_I;
    static const RGBA BLUE_II;
    static const RGBA BLUE_III;
    static const RGBA DEEP_BLUE;
    static const RGBA VIOLET_I;
    static const RGBA VIOLET_II;
    static const RGBA LAVENDER;
    static const RGBA BROWN;
    static const RGBA GOLD_II;
    static const RGBA SILVER;
    static const RGBA BLACK;
    static const RGBA GRAY;
    static const RGBA WHITE;
};

struct HSV
{
    SHORT h = 0;
    SHORT s = 0;
    SHORT v = 0;
    SHORT a = 0;

    HSV() = default;
    HSV(SHORT inH, SHORT inS, SHORT inV, SHORT inA = 255) : h(inH), s(inS), v(inV), a(inA) {}
};

HSV RGBAtoHSV(RGBA rgba);

RGBA HSVtoRGBA(HSV hsv);

RGBA BrightenColor(RGBA color, float amount);
RGBA DarkenColor(RGBA color, float amount);