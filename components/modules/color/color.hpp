#pragma once
#include <cstdint>
#include <array>

namespace color {

    struct CRGB {
        union {
            struct {
                uint8_t r;
                uint8_t g;
                uint8_t b;
            };
            uint8_t raw[3];
        };  

        CRGB(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            : r(red), g(green), b(blue) {};

        template<typename TargetFormat>
        TargetFormat toColor() const;

        /* Predefined static color constants*/
        static const CRGB Black, Red, Green, Blue, White;
    };

    struct CGRB {
        union { 
            struct {
                uint8_t g;
                uint8_t r;
                uint8_t b;
            };
            uint8_t raw[3];
        };  

        CGRB(uint8_t green = 0, uint8_t red = 0, uint8_t blue = 0)
            : g(green), r(red), b(blue) {};

        CRGB toRGB(void) const;

        /* Predefined static color constants*/
        static const CGRB Black, Red, Green, Blue, White;
    };

    /* Color constants*/
    inline const CRGB CRGB::Black = CRGB(0, 0, 0);
    inline const CRGB CRGB::Red   = CRGB(255, 0, 0);
    inline const CRGB CRGB::Green = CRGB(0, 255, 0);
    inline const CRGB CRGB::Blue  = CRGB(0, 0, 255);
    inline const CRGB CRGB::White = CRGB(255, 255, 255);

    inline const CGRB CGRB::Black = CGRB(0, 0, 0);
    inline const CGRB CGRB::Red   = CGRB(0, 255, 0);
    inline const CGRB CGRB::Green = CGRB(255, 0, 0);
    inline const CGRB CGRB::Blue  = CGRB(0, 0, 255);
    inline const CGRB CGRB::White = CGRB(255, 255, 255);

    inline CRGB CGRB::toRGB(void) const {
        return CRGB{r, g, b};
    }

    template<>
    inline CGRB CRGB::toColor<CGRB>() const {
        return CGRB(g, r, b);
    }
}