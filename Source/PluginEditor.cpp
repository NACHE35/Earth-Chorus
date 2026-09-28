#include "PluginEditor.h"
#include "EarthMap.h"
#include <cmath>

#if EARTH_HAS_THRILLER_FONT
 #include <BinaryData.h>
#endif

namespace
{
    constexpr float kPi = juce::MathConstants<float>::pi;
    constexpr float kStart = kPi * 1.25f;   // 7h30
    constexpr float kEnd   = kPi * 2.75f;   // 4h30

    // ---- Constantes faciles a modifier ----
    const char* kTitle    = "EARTH CHORUS";
    const char* kVersion  = "1.0";
    const char* kDate     = "28 September 2026";
    const char* kLinkText = "Learn more about NACHE";
    const char* kLinkUrl  = "https://linktr.ee/nachevhs";

    const juce::Colour kAccent (0xff62c8ff);
    const juce::Colour kLabel  (0xff8fb8f0);

    juce::Font mkFont (float h, bool bold, float kern = 0.0f)
    {
        juce::Font f (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
        return f.withExtraKerningFactor (kern);
    }

    juce::Font signatureFont()
    {
       #if EARTH_HAS_THRILLER_FONT
        if (auto tf = juce::Typeface::createSystemTypefaceFor (BinaryData::Thriller_ttf, (size_t) BinaryData::Thriller_ttfSize))
            return juce::Font (juce::FontOptions (tf)).withHeight (18.0f);
       #endif
        // Repli : serif a fort contraste, gras (proche de l'esprit du lettrage de Thriller)
        juce::Font f (juce::FontOptions (juce::Font::getDefaultSerifFontName(), 17.0f, juce::Font::bold));
        return f.withExtraKerningFactor (0.14f);
    }

    float smoothstep (float a, float b, float x)
    {
        const float t = juce::jlimit (0.0f, 1.0f, (x - a) / (b - a));
        return t * t * (3.0f - 2.0f * t);
    }
}

//==============================================================================
void EarthLNF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                 float a0, float a1, juce::Slider& s)
{
    if (s.getComponentID() == "globe") return;   // dessine par l'editeur

    const float cx = (float) x + (float) w * 0.5f, cy = (float) y + (float) h * 0.5f;
    const float ang = a0 + pos * (a1 - a0);
    const float R = 46.0f, Rt = 55.0f;

    juce::Path track;
    track.addCentredArc (cx, cy, Rt, Rt, 0.0f, a0, a1, true);
    g.setColour (juce::Colour (0x334a78c8));
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (cx, cy, Rt, Rt, 0.0f, a0, ang, true);
        g.setColour (kAccent);
        g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b4a78), cx, cy - R,
                                             juce::Colour (0xff0b1226), cx, cy + R, false));
    g.fillEllipse (cx - R, cy - R, 2 * R, 2 * R);
    g.setColour (juce::Colour (0x55a9c6ff));
    g.drawEllipse (cx - R, cy - R, 2 * R, 2 * R, 1.2f);

    juce::Path ind;
    ind.startNewSubPath (cx + std::sin (ang) * 14.0f, cy - std::cos (ang) * 14.0f);
    ind.lineTo (cx + std::sin (ang) * 33.0f, cy - std::cos (ang) * 33.0f);
    g.setColour (juce::Colours::white);
    g.strokePath (ind, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void EarthLNF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (2.0f);
    const bool on = b.getToggleState();
    const float rad = r.getHeight() * 0.5f;
    g.setColour (on ? juce::Colour (0x445ac8fa) : juce::Colour (0x33ffffff));
    g.fillRoundedRectangle (r, rad);
    g.setColour (on ? kAccent : juce::Colour (0x66aabbdd));
    g.drawRoundedRectangle (r, rad, 1.2f);
    const float d = r.getHeight() - 4.0f;
    const float kx = on ? r.getRight() - d - 2.0f : r.getX() + 2.0f;
    g.setColour (on ? juce::Colours::white : juce::Colour (0xff8899bb));
    g.fillEllipse (kx, r.getY() + 2.0f, d, d);
}

//==============================================================================
juce::Rectangle<int> AboutPanel::panelRect() const
{
    return juce::Rectangle<int> (0, 0, 380, 250).withCentre (getLocalBounds().getCentre());
}

juce::Rectangle<int> AboutPanel::linkRect() const
{
    return juce::Rectangle<int> (0, 0, 260, 26).withCentre ({ getWidth() / 2, panelRect().getY() + 178 });
}

void AboutPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xd8030812));
    auto p = panelRect().toFloat();
    g.setColour (juce::Colour (0xff0b1830));
    g.fillRoundedRectangle (p, 14.0f);
    g.setColour (kAccent.withAlpha (0.7f));
    g.drawRoundedRectangle (p, 14.0f, 1.5f);

    const int cx = getWidth() / 2, top = panelRect().getY();
    g.setColour (juce::Colour (0xffe4f0ff));
    g.setFont (mkFont (24.0f, true, 0.4f));
    g.drawText (kTitle, cx - 180, top + 24, 360, 34, juce::Justification::centred, false);

    g.setColour (kLabel);
    g.setFont (mkFont (15.0f, true));
    g.drawText (juce::String ("Version ") + kVersion, cx - 180, top + 78, 360, 22, juce::Justification::centred, false);
    g.setFont (mkFont (14.0f, false));
    g.drawText (juce::String ("Release date: ") + kDate, cx - 180, top + 104, 360, 22, juce::Justification::centred, false);

    auto lr = linkRect();
    g.setColour (kAccent);
    g.setFont (mkFont (16.0f, true));
    g.drawText (kLinkText, lr, juce::Justification::centred, false);
    g.drawLine ((float) lr.getX() + 30.0f, (float) lr.getBottom() - 2.0f, (float) lr.getRight() - 30.0f, (float) lr.getBottom() - 2.0f, 1.0f);

    g.setColour (juce::Colour (0x778fb8f0));
    g.setFont (mkFont (11.0f, false));
    g.drawText ("click anywhere else to close", cx - 180, top + 214, 360, 18, juce::Justification::centred, false);
}

void AboutPanel::mouseMove (const juce::MouseEvent& e)
{
    setMouseCursor (linkRect().contains (e.getPosition()) ? juce::MouseCursor::PointingHandCursor
                                                          : juce::MouseCursor::NormalCursor);
}

void AboutPanel::mouseDown (const juce::MouseEvent& e)
{
    if (linkRect().contains (e.getPosition()))
        juce::URL (kLinkUrl).launchInDefaultBrowser();
    else
        setVisible (false);
}

//==============================================================================
void PresetBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a2850), 0, r.getY(),
                                             juce::Colour (0xff0b1226), 0, r.getBottom(), false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0x665ac8fa));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    const float cy = r.getCentreY();
    g.setColour (kLabel);
    juce::Path l, rt;
    l.addTriangle (r.getX() + 15, cy, r.getX() + 21, cy - 4.5f, r.getX() + 21, cy + 4.5f);
    rt.addTriangle (r.getRight() - 15, cy, r.getRight() - 21, cy - 4.5f, r.getRight() - 21, cy + 4.5f);
    g.fillPath (l); g.fillPath (rt);

    g.setColour (juce::Colour (0xffe6eefc));
    g.setFont (mkFont (13.0f, true, 0.05f));
    g.drawFittedText (text, getLocalBounds().reduced (30, 0).withTrimmedRight (10), juce::Justification::centred, 1);

    juce::Path caret;
    caret.addTriangle (r.getRight() - 40, cy - 2.0f, r.getRight() - 32, cy - 2.0f, r.getRight() - 36, cy + 3.0f);
    g.setColour (kAccent);
    g.fillPath (caret);
}

void PresetBar::mouseUp (const juce::MouseEvent& e)
{
    if (! contains (e.getPosition())) return;
    if (e.x < 26)                 { if (onPrev) onPrev(); }
    else if (e.x > getWidth() - 26) { if (onNext) onNext(); }
    else                          { if (onMenu) onMenu(); }
}

//==============================================================================
EarthChorusEditor::EarthChorusEditor (EarthChorusProcessor& p)
    : juce::AudioProcessorEditor (&p), audio (p)
{
    setOpaque (true);
    setSize (633, 640);

    auto setup = [this] (juce::Slider& s)
    {
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters (kStart, kEnd, true);
        s.setMouseDragSensitivity (220);
        s.setLookAndFeel (&lnf);
        addAndMakeVisible (s);
    };
    setup (modeKnob); setup (speedKnob); setup (fxKnob); setup (depthKnob); setup (globeSlider);
    globeSlider.setComponentID ("globe");
    modeKnob.setMouseDragSensitivity (110);
    globeSlider.setMouseDragSensitivity (300);

    powerBtn.setLookAndFeel (&lnf);
    addAndMakeVisible (powerBtn);

    modeAtt  = std::make_unique<SA> (audio.apvts, "mode",    modeKnob);
    speedAtt = std::make_unique<SA> (audio.apvts, "speed",   speedKnob);
    fxAtt    = std::make_unique<SA> (audio.apvts, "superfx", fxKnob);
    depthAtt = std::make_unique<SA> (audio.apvts, "depth",   depthKnob);
    mixAtt   = std::make_unique<SA> (audio.apvts, "drymix",  globeSlider);
    powerAtt = std::make_unique<BA> (audio.apvts, "power",   powerBtn);

    presetBar.onMenu = [this] { showPresetMenu(); };
    presetBar.onPrev = [this] { stepPreset (-1); };
    presetBar.onNext = [this] { stepPreset (+1); };
    addAndMakeVisible (presetBar);

    about.setVisible (false);
    addChildComponent (about);

    // Etoiles
    juce::Random rnd (20260928);
    for (int i = 0; i < 170; ++i)
    {
        Star s;
        s.x = rnd.nextFloat() * 633.0f;
        s.y = rnd.nextFloat() * 640.0f;
        s.r = 0.7f + rnd.nextFloat() * 1.3f;
        s.phase = rnd.nextFloat() * 6.28f;
        s.speed = 0.6f + rnd.nextFloat() * 3.2f;
        s.base = 0.45f + rnd.nextFloat() * 0.55f;
        const float k = rnd.nextFloat();
        s.col = k < 0.2f ? juce::Colour (0xffffe2b8) : (k < 0.5f ? juce::Colour (0xffb8d4ff) : juce::Colours::white);
        stars.push_back (s);
    }

    mask = EarthMap::buildMask (kMaskW, kMaskH);
    globeImg = juce::Image (juce::Image::ARGB, kGlobeD, kGlobeD, true);
    buildGlobeGeometry();

    lastTime = juce::Time::getMillisecondCounterHiRes() * 0.001;
    renderGlobe (0.0f, (float) globeSlider.getValue());
    startTimerHz (30);
}

EarthChorusEditor::~EarthChorusEditor()
{
    stopTimer();
    for (auto* s : { &modeKnob, &speedKnob, &fxKnob, &depthKnob, &globeSlider })
        s->setLookAndFeel (nullptr);
    powerBtn.setLookAndFeel (nullptr);
}

void EarthChorusEditor::resized()
{
    auto place = [] (juce::Component& c, int cx, int cy, int w, int h) { c.setBounds (cx - w / 2, cy - h / 2, w, h); };
    place (modeKnob,   79, 151, 120, 120);
    place (speedKnob, 551, 151, 120, 120);
    place (fxKnob,     79, 533, 120, 120);
    place (depthKnob, 551, 533, 120, 120);
    place (globeSlider, 316, 337, 340, 340);
    powerBtn.setBounds (587, 12, 38, 20);
    presetBar.setBounds (216, 598, 200, 26);
    about.setBounds (getLocalBounds());
}

void EarthChorusEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    if (titleRect.contains (e.getEventRelativeTo (this).getPosition()))
    {
        about.setVisible (true);
        about.toFront (true);
    }
}

//==============================================================================
// Globe
void EarthChorusEditor::buildGlobeGeometry()
{
    globePx.clear();
    const float tilt = juce::degreesToRadians (20.0f);
    const float ct = std::cos (tilt), st = std::sin (tilt);
    const float c0 = (float) kGlobeD * 0.5f;

    for (int y = 0; y < kGlobeD; ++y)
        for (int x = 0; x < kGlobeD; ++x)
        {
            const float dx = (float) x + 0.5f - c0, dy = (float) y + 0.5f - c0;
            const float dist = std::sqrt (dx * dx + dy * dy);
            if (dist > kGlobeR + 1.0f) continue;

            float nx = dx / kGlobeR, ny = -dy / kGlobeR;
            const float len = std::sqrt (nx * nx + ny * ny);
            if (len > 1.0f) { nx /= len; ny /= len; }
            const float nz = std::sqrt (juce::jmax (0.0f, 1.0f - nx * nx - ny * ny));

            const float yy = ny * ct + nz * st;
            const float zz = -ny * st + nz * ct;

            GlobePx p;
            p.x = x; p.y = y; p.nx = nx; p.nz = nz;
            p.lat = std::asin (juce::jlimit (-1.0f, 1.0f, yy));
            p.lonBase = std::atan2 (nx, zz);
            p.alpha = juce::jlimit (0.0f, 1.0f, kGlobeR + 0.5f - dist);
            globePx.push_back (p);
        }
}

float EarthChorusEditor::sampleMask (float fx, float fy) const
{
    int x0 = (int) std::floor (fx), y0 = (int) std::floor (fy);
    const float tx = fx - (float) x0, ty = fy - (float) y0;
    int x1 = x0 + 1;
    x0 = ((x0 % kMaskW) + kMaskW) % kMaskW;
    x1 = ((x1 % kMaskW) + kMaskW) % kMaskW;
    int y1 = juce::jlimit (0, kMaskH - 1, y0 + 1);
    y0 = juce::jlimit (0, kMaskH - 1, y0);
    const float a = mask[(size_t) (y0 * kMaskW + x0)], b = mask[(size_t) (y0 * kMaskW + x1)];
    const float c = mask[(size_t) (y1 * kMaskW + x0)], d = mask[(size_t) (y1 * kMaskW + x1)];
    const float top = a + (b - a) * tx, bot = c + (d - c) * tx;
    return (top + (bot - top) * ty) / 255.0f;
}

void EarthChorusEditor::renderGlobe (float rotRad, float mix)
{
    // Le soleil passe de "derriere la Terre" (dry = nuit totale) a "face a nous" (wet = jour complet)
    const float sunA = (1.0f - juce::jlimit (0.0f, 1.0f, mix)) * kPi;
    const float sx = std::sin (sunA), sz = std::cos (sunA);
    const float centerLon = juce::degreesToRadians (100.0f);
    const float twoPi = 2.0f * kPi;

    juce::Image::BitmapData bd (globeImg, juce::Image::BitmapData::writeOnly);

    for (const auto& p : globePx)
    {
        const float lon = p.lonBase + centerLon - rotRad;
        float u = lon / twoPi + 0.5f;
        u -= std::floor (u);
        const float fx = u * (float) kMaskW;
        const float fy = (0.5f - p.lat / kPi) * (float) (kMaskH - 1);
        const float land = sampleMask (fx, fy);

        // Couleur de base
        const float n = 0.5f + 0.5f * std::sin (lon * 9.0f + p.lat * 4.0f) * std::cos (p.lat * 13.0f + lon * 3.0f);
        const float ice = smoothstep (1.05f, 1.25f, std::abs (p.lat));
        float lr = 0.20f + 0.25f * n, lg = 0.52f + 0.03f * n, lb = 0.22f + 0.03f * n;
        lr += (0.92f - lr) * ice; lg += (0.96f - lg) * ice; lb += (1.0f - lb) * ice;

        const float oz = std::pow (p.nz, 0.7f);
        float orr = 0.03f + 0.10f * oz, og = 0.16f + 0.29f * oz, ob = 0.41f + 0.40f * oz;
        const float oice = smoothstep (1.30f, 1.45f, std::abs (p.lat));
        orr += (0.85f - orr) * oice; og += (0.92f - og) * oice; ob += (1.0f - ob) * oice;

        float r = orr + (lr - orr) * land;
        float g = og  + (lg - og)  * land;
        float b = ob  + (lb - ob)  * land;

        // Eclairage jour / nuit
        const float d = p.nx * sx + p.nz * sz;
        const float lit = smoothstep (-0.05f, 0.35f, d);
        float inten = 0.05f + lit * (0.4f + 0.6f * juce::jmax (0.0f, d));
        inten = juce::jmin (1.0f, inten);
        r *= inten; g *= inten; b *= inten;

        // Reflet sur l'ocean
        const float hx = sx, hz = sz + 1.0f;
        const float hl = std::sqrt (hx * hx + hz * hz) + 1.0e-4f;
        const float spec = std::pow (juce::jmax (0.0f, (p.nx * hx + p.nz * hz) / hl), 70.0f) * 0.55f * lit * (1.0f - land);
        r += spec; g += spec; b += spec;

        // Liseré du terminateur (crepuscule) et atmosphere
        const float dusk = std::exp (-(d * d) / 0.012f) * 0.16f;
        r += dusk * 0.9f; g += dusk * 0.45f; b += dusk * 0.15f;
        const float rim = std::pow (1.0f - p.nz, 2.5f) * (0.15f + 0.85f * lit) * 0.9f;
        r += 0.25f * rim; g += 0.55f * rim; b += 1.0f * rim;

        bd.setPixelColour (p.x, p.y, juce::Colour::fromFloatRGBA (juce::jlimit (0.0f, 1.0f, r),
                                                                  juce::jlimit (0.0f, 1.0f, g),
                                                                  juce::jlimit (0.0f, 1.0f, b), p.alpha));
    }
}

//==============================================================================
void EarthChorusEditor::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const float dt = juce::jlimit (0.0f, 0.2f, (float) (now - lastTime));
    lastTime = now;
    timeSec = (float) now;

    rot += dt * juce::degreesToRadians (6.0f + 3.0f * (float) speedKnob.getValue());

    const float lv = audio.levelL.exchange (0.0f), rv = audio.levelR.exchange (0.0f);
    dispL = juce::jmax (lv, dispL * 0.86f);
    dispR = juce::jmax (rv, dispR * 0.86f);

    renderGlobe (rot, (float) globeSlider.getValue());
    presetBar.text = audio.getCurrentPresetName();
    repaint();
}

juce::String EarthChorusEditor::valueText (int which) const
{
    switch (which)
    {
        case 0: return audio.apvts.getParameter ("mode")->getCurrentValueAsText();
        case 1: return juce::String (speedKnob.getValue(), 2) + " Hz";
        case 2: return juce::String ((int) std::lround (fxKnob.getValue() * 100.0)) + " %";
        default: return juce::String ((int) std::lround (depthKnob.getValue() * 100.0)) + " %";
    }
}

void EarthChorusEditor::drawMeter (juce::Graphics& g, float y, float level)
{
    const int segs = 28;
    const float w = 6.0f, gap = 1.8f;
    const float total = (float) segs * (w + gap) - gap;
    const float x0 = ((float) getWidth() - total) * 0.5f;
    const float db = juce::Decibels::gainToDecibels (level, -60.0f);
    for (int i = 0; i < segs; ++i)
    {
        const float thr = -48.0f + (float) i * (48.0f / (float) (segs - 1));
        const juce::Colour c = i < 20 ? juce::Colour (0xff3ddc84) : (i < 25 ? juce::Colour (0xffffb020) : juce::Colour (0xffff3b30));
        g.setColour (db >= thr ? c : c.withAlpha (0.14f));
        g.fillRoundedRectangle (x0 + (float) i * (w + gap), y, w, 5.0f, 1.0f);
    }
}

void EarthChorusEditor::paint (juce::Graphics& g)
{
    const float W = (float) getWidth(), H = (float) getHeight();
    const float cx = 316.0f, cy = 337.0f;

    // Fond
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff02030a), 0, 0, juce::Colour (0xff0a1735), 0, H, false));
    g.fillAll();

    // Etoiles scintillantes
    for (const auto& s : stars)
    {
        float tw = 0.5f + 0.5f * std::sin (timeSec * s.speed + s.phase);
        tw *= tw;
        const float a = juce::jlimit (0.0f, 1.0f, s.base * (0.12f + 0.88f * tw));
        g.setColour (s.col.withAlpha (a));
        g.fillEllipse (s.x - s.r * 0.5f, s.y - s.r * 0.5f, s.r, s.r);
        if (s.r > 1.5f && tw > 0.75f)
        {
            g.setColour (s.col.withAlpha (a * 0.5f));
            g.drawLine (s.x - 3.5f, s.y, s.x + 3.5f, s.y, 0.7f);
            g.drawLine (s.x, s.y - 3.5f, s.x, s.y + 3.5f, 0.7f);
        }
    }

    // Halo
    {
        juce::ColourGradient glow (juce::Colour (0x005ac8fa), cx, cy, juce::Colour (0x005ac8fa), cx + 225.0f, cy, true);
        glow.addColour (0.70, juce::Colour (0x345ac8fa));
        g.setGradientFill (glow);
        g.fillEllipse (cx - 225.0f, cy - 225.0f, 450.0f, 450.0f);
    }

    // Anneau Dry/Wet
    {
        const float pr = (float) globeSlider.valueToProportionOfLength (globeSlider.getValue());
        const float ang = kStart + pr * (kEnd - kStart);
        const float Rr = 170.0f;

        juce::Path track;
        track.addCentredArc (cx, cy, Rr, Rr, 0.0f, kStart, kEnd, true);
        g.setColour (juce::Colour (0x30507ab8));
        g.strokePath (track, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (pr > 0.001f)
        {
            juce::Path val;
            val.addCentredArc (cx, cy, Rr, Rr, 0.0f, kStart, ang, true);
            g.setColour (kAccent.withAlpha (0.25f));
            g.strokePath (val, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour (kAccent);
            g.strokePath (val, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        g.setColour (juce::Colours::white);
        g.fillEllipse (cx + std::sin (ang) * Rr - 5.0f, cy - std::cos (ang) * Rr - 5.0f, 10.0f, 10.0f);
    }

    // Globe
    g.drawImageAt (globeImg, (int) (cx - (float) kGlobeD * 0.5f), (int) (cy - (float) kGlobeD * 0.5f));
    g.setColour (juce::Colour (0x66aadcff));
    g.drawEllipse (cx - kGlobeR, cy - kGlobeR, kGlobeR * 2, kGlobeR * 2, 1.2f);
    g.setColour (juce::Colour (0x22aadcff));
    g.drawEllipse (cx - kGlobeR - 5.0f, cy - kGlobeR - 5.0f, (kGlobeR + 5.0f) * 2, (kGlobeR + 5.0f) * 2, 1.0f);

    // VU-metre (style Xone : rangees de LED horizontales)
    drawMeter (g, 9.0f, dispL);
    drawMeter (g, 16.0f, dispR);
    g.setColour (juce::Colour (0x998fb8f0));
    g.setFont (mkFont (8.0f, true));
    const float mx0 = (W - (28.0f * 7.8f - 1.8f)) * 0.5f;
    g.drawText ("L", (int) mx0 - 14, 8, 10, 7, juce::Justification::centredRight, false);
    g.drawText ("R", (int) mx0 - 14, 15, 10, 7, juce::Justification::centredRight, false);

    // Titre
    const juce::Rectangle<int> tr (0, 36, getWidth(), 34);
    g.setFont (mkFont (24.0f, true, 0.55f));
    g.setColour (kAccent.withAlpha (0.28f));
    g.drawText (kTitle, tr.translated (0, 1), juce::Justification::centred, false);
    g.setColour (juce::Colour (0xffe4f0ff));
    g.drawText (kTitle, tr, juce::Justification::centred, false);

    // "by NACHE"
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffffc46b), 0, 71.0f, juce::Colour (0xffe2431d), 0, 89.0f, false));
    g.setFont (signatureFont());
    g.drawText ("by NACHE", juce::Rectangle<int> (0, 70, getWidth(), 22), juce::Justification::centred, false);

    // Libelles des boutons
    auto label = [&] (const char* t, int x, int y)
    {
        g.setColour (kLabel);
        g.setFont (mkFont (11.5f, true, 0.12f));
        g.drawText (t, x - 60, y - 9, 120, 18, juce::Justification::centred, false);
    };
    auto value = [&] (const juce::String& t, int x, int y)
    {
        g.setColour (juce::Colour (0xffe6eefc));
        g.setFont (mkFont (13.0f, false));
        g.drawText (t, x - 60, y - 9, 120, 18, juce::Justification::centred, false);
    };
    label ("MODE", 79, 102);      value (valueText (0), 79, 255);
    label ("SPEED", 551, 102);    value (valueText (1), 551, 255);
    label ("SUPER FX", 79, 456);  value (valueText (2), 79, 609);
    label ("DEPTH", 551, 456);    value (valueText (3), 551, 609);

    g.setColour (kLabel);
    g.setFont (mkFont (10.5f, true, 0.35f));
    g.drawText ("DRY / WET", (int) cx - 80, 523, 160, 18, juce::Justification::centred, false);
}

//==============================================================================
// Presets
void EarthChorusEditor::stepPreset (int dir)
{
    const auto& f = EarthPresets::getFactory();
    auto users = audio.getUserPresetFiles();
    const int total = (int) f.size() + users.size();
    const auto name = audio.getCurrentPresetName();

    int cur = -1;
    for (int i = 0; i < (int) f.size(); ++i) if (name == f[(size_t) i].name) cur = i;
    for (int j = 0; j < users.size(); ++j) if (name == users[j].getFileNameWithoutExtension()) cur = (int) f.size() + j;

    if (cur < 0) cur = dir > 0 ? -1 : 0;
    const int next = (cur + dir + total) % total;
    if (next < (int) f.size()) audio.loadFactoryPreset (next);
    else audio.loadUserPreset (users[next - (int) f.size()]);
}

void EarthChorusEditor::showPresetMenu()
{
    const auto& f = EarthPresets::getFactory();
    auto users = audio.getUserPresetFiles();
    const auto cur = audio.getCurrentPresetName();

    juce::PopupMenu menu, factoryMenu, userMenu, deleteMenu;
    for (int i = 0; i < (int) f.size(); ++i)
        factoryMenu.addItem (1 + i, f[(size_t) i].name, true, cur == f[(size_t) i].name);
    for (int j = 0; j < users.size(); ++j)
    {
        userMenu.addItem (100 + j, users[j].getFileNameWithoutExtension(), true, cur == users[j].getFileNameWithoutExtension());
        deleteMenu.addItem (300 + j, users[j].getFileNameWithoutExtension());
    }

    menu.addSubMenu ("Factory presets", factoryMenu);
    if (users.size() > 0) menu.addSubMenu ("User presets", userMenu);
    menu.addSeparator();
    menu.addItem (1000, "Save current settings as...");
    if (users.size() > 0) menu.addSubMenu ("Delete user preset", deleteMenu);

    juce::Component::SafePointer<EarthChorusEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetBar),
                        [safe, users] (int r)
    {
        if (safe == nullptr || r == 0) return;
        auto& a = safe->audio;
        if (r >= 1 && r <= 25)           a.loadFactoryPreset (r - 1);
        else if (r >= 100 && r < 300)    a.loadUserPreset (users[r - 100]);
        else if (r >= 300 && r < 1000)   users[r - 300].deleteFile();
        else if (r == 1000)              safe->askSavePreset();
    });
}

void EarthChorusEditor::askSavePreset()
{
    auto* w = new juce::AlertWindow ("Save preset", "Give a name to your preset:", juce::MessageBoxIconType::NoIcon);
    const auto cur = audio.getCurrentPresetName();
    w->addTextEditor ("name", cur == "Default" ? juce::String ("My Preset") : cur, "Name");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<EarthChorusEditor> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int result)
    {
        if (result == 1 && safe != nullptr)
        {
            const auto n = w->getTextEditorContents ("name").trim();
            if (n.isNotEmpty()) safe->audio.saveUserPreset (n);
        }
    }), true);
}
