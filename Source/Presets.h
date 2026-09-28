#pragma once
#include <array>

// 25 presets d'usine. mode : 0 = I, 1 = II, 2 = I+II, 3 = DIM
namespace EarthPresets
{
    struct Preset { const char* name; int mode; float speed, depth, superFx, mix; };

    inline const std::array<Preset, 25>& getFactory()
    {
        static const std::array<Preset, 25> p {{
            { "Juno Classic I",       0, 0.80f, 0.60f, 0.00f, 0.50f },
            { "Juno Classic II",      1, 0.50f, 0.60f, 0.00f, 0.50f },
            { "Juno I+II Shimmer",    2, 1.20f, 0.50f, 0.00f, 0.50f },
            { "Dimension Wide",       3, 0.60f, 0.55f, 0.15f, 0.50f },
            { "Lush Pad",             1, 0.35f, 0.80f, 0.30f, 0.55f },
            { "Slow Ocean",           1, 0.15f, 0.90f, 0.40f, 0.60f },
            { "Vintage Strings",      0, 0.70f, 0.75f, 0.10f, 0.60f },
            { "E-Piano Glow",         0, 1.10f, 0.35f, 0.10f, 0.40f },
            { "Guitar Sheen",         0, 1.50f, 0.30f, 0.00f, 0.35f },
            { "Bass Widener",         3, 0.40f, 0.25f, 0.20f, 0.30f },
            { "Vocal Air",            3, 0.90f, 0.30f, 0.35f, 0.35f },
            { "Synth Lead Halo",      2, 1.00f, 0.40f, 0.30f, 0.45f },
            { "Orbit",                3, 0.50f, 0.50f, 0.70f, 0.55f },
            { "Deep Space",           1, 0.20f, 0.70f, 0.90f, 0.65f },
            { "Nebula",               3, 0.30f, 0.85f, 0.80f, 0.70f },
            { "Aurora",               1, 0.45f, 0.65f, 0.60f, 0.60f },
            { "Satellite",            2, 0.80f, 0.30f, 0.50f, 0.50f },
            { "Gravity Well",         3, 0.12f, 1.00f, 1.00f, 0.75f },
            { "Tape Wobble",          0, 4.50f, 0.55f, 0.00f, 0.50f },
            { "Rotary Dream",         0, 6.00f, 0.50f, 0.20f, 0.60f },
            { "Subtle Glue",          0, 0.60f, 0.20f, 0.00f, 0.30f },
            { "Cosmic Choir",         3, 0.55f, 0.90f, 0.75f, 0.65f },
            { "Moonlit",              1, 0.25f, 0.75f, 0.50f, 0.50f },
            { "Supernova",            2, 1.50f, 0.80f, 1.00f, 0.80f },
            { "Total Immersion",      3, 0.70f, 0.70f, 1.00f, 0.70f },
        }};
        return p;
    }
}
