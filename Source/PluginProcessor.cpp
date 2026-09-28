#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    const char* kIds[5] = { "mode", "speed", "depth", "superfx", "drymix" };

    struct ModeCfg { float centerMs, depthMs, rateMul, sineAmt; bool dual; };

    // Comportements inspires du chorus du Juno-106 (I, II, I+II) + un mode "Dimension" a 2 voies par canal
    const ModeCfg kModes[4] = {
        { 3.3f, 1.6f, 1.0f, 0.0f, false },   // I     : triangle, moyen
        { 5.0f, 2.6f, 0.6f, 0.0f, false },   // II    : plus lent, plus profond
        { 2.5f, 0.7f, 6.0f, 1.0f, false },   // I+II  : rapide, faible profondeur (shimmer)
        { 4.0f, 2.0f, 0.8f, 1.0f, true  }    // DIM   : deux voies en quadrature par canal
    };

    inline float lfoFn (double p, float sineAmt)
    {
        p -= std::floor (p);
        const float tri = (float) (4.0 * std::abs (p - 0.5) - 1.0);
        const float sn  = (float) std::sin (juce::MathConstants<double>::twoPi * p);
        return tri + (sn - tri) * sineAmt;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout EarthChorusProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Mode",
                                                   StringArray { "I", "II", "I+II", "DIM" }, 0));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "speed", 1 }, "Speed",
                                                  NormalisableRange<float> (0.05f, 10.0f, 0.001f, 0.4f), 0.8f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "depth", 1 }, "Depth",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.6f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "superfx", 1 }, "Super FX",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.25f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "drymix", 1 }, "Dry/Wet",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "power", 1 }, "Power", true));
    return l;
}

EarthChorusProcessor::EarthChorusProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pMode  = apvts.getRawParameterValue ("mode");
    pSpeed = apvts.getRawParameterValue ("speed");
    pDepth = apvts.getRawParameterValue ("depth");
    pFx    = apvts.getRawParameterValue ("superfx");
    pMix   = apvts.getRawParameterValue ("drymix");
    pPower = apvts.getRawParameterValue ("power");
    apvts.state.setProperty ("presetName", "Default", nullptr);
}

bool EarthChorusProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainInputChannelSet()  == l.getMainOutputChannelSet();
}

void EarthChorusProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    const int size = (int) (0.1 * sr) + 16;
    for (int c = 0; c < 2; ++c)
    {
        chorus[c].prepare (size);
        er[c].prepare (size);
        for (int k = 0; k < 2; ++k) ap[c][k].line.prepare (size);
        lp[c] = 0.0f;
    }
    const float ms = (float) (sr / 1000.0);
    const float taps[4] = { 11.3f, 17.9f, 23.7f, 31.1f };
    for (int i = 0; i < 4; ++i) erT[i] = taps[i] * ms;

    lpA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 9000.0f / (float) sr);
    phase = apPhase = orbitPhase = 0.0;

    depthS.reset (sr, 0.05); fxS.reset (sr, 0.05); mixS.reset (sr, 0.05); powerS.reset (sr, 0.03);
    depthS.setCurrentAndTargetValue (pDepth->load());
    fxS.setCurrentAndTargetValue (pFx->load());
    mixS.setCurrentAndTargetValue (pMix->load());
    powerS.setCurrentAndTargetValue (pPower->load() > 0.5f ? 1.0f : 0.0f);
}

void EarthChorusProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2) return;

    const int mode = juce::jlimit (0, 3, (int) std::lround (pMode->load()));
    const ModeCfg& cfg = kModes[mode];

    depthS.setTargetValue (pDepth->load());
    fxS.setTargetValue (pFx->load());
    mixS.setTargetValue (pMix->load());
    powerS.setTargetValue (pPower->load() > 0.5f ? 1.0f : 0.0f);

    const double inc = (double) pSpeed->load() * (double) cfg.rateMul / sr;
    const float msToSmp = (float) (sr / 1000.0);
    const double twoPi = juce::MathConstants<double>::twoPi;
    const float halfPi = juce::MathConstants<float>::halfPi;

    static const float apBaseMs[2][2] = { { 5.1f, 8.3f }, { 6.7f, 9.9f } };
    static const float erG[4] = { 0.5f, 0.4f, 0.32f, 0.25f };

    float* d0 = buffer.getWritePointer (0);
    float* d1 = buffer.getWritePointer (1);
    float pkL = 0.0f, pkR = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        const float dep = depthS.getNextValue();
        const float fx  = fxS.getNextValue();
        const float mx  = mixS.getNextValue();
        const float pw  = powerS.getNextValue();

        phase += inc; phase -= std::floor (phase);

        const float in[2] = { d0[i], d1[i] };
        float w[2];

        // ---------- Chorus (2 lignes, LFO en opposition L/R comme le Juno) ----------
        const float mod = dep * 1.5f * cfg.depthMs;
        for (int c = 0; c < 2; ++c)
        {
            chorus[c].push (in[c]);
            const double off = (c == 0) ? 0.0 : 0.5;
            float t = chorus[c].read ((cfg.centerMs + mod * lfoFn (phase + off, cfg.sineAmt)) * msToSmp);
            if (cfg.dual)
                t = 0.5f * (t + chorus[c].read ((cfg.centerMs + mod * lfoFn (phase + off + 0.25, cfg.sineAmt)) * msToSmp));
            lp[c] += lpA * (t - lp[c]);   // legere coloration BBD
            w[c] = lp[c];
        }

        // ---------- SUPER FX : spatialisation / dimension ----------
        {
            const float chorusOnly[2] = { w[0], w[1] };

            // 1) Diffusion : allpass modules et decorrelees L/R
            apPhase += 0.23 / sr; apPhase -= std::floor (apPhase);
            const float m = (float) std::sin (twoPi * apPhase);
            const float g = 0.3f + 0.3f * fx;
            for (int c = 0; c < 2; ++c)
            {
                for (int k = 0; k < 2; ++k)
                {
                    ap[c][k].g = g;
                    const float d = apBaseMs[c][k] * msToSmp + fx * 0.0004f * (float) sr * m * (c == 0 ? 1.0f : -1.0f);
                    w[c] = ap[c][k].process (w[c], d);
                }
                er[c].push (w[c]);
            }

            // 2) Premieres reflexions croisees (sensation de piece / profondeur)
            float eL = 0.0f, eR = 0.0f;
            for (int t = 0; t < 4; ++t)
            {
                eL += erG[t] * er[t % 2].read (erT[t]);
                eR += erG[t] * er[1 - (t % 2)].read (erT[t]);
            }
            w[0] += 0.45f * fx * eL;
            w[1] += 0.45f * fx * eR;

            // 3) Elargissement Mid/Side
            const float mid  = 0.5f * (w[0] + w[1]);
            float side = 0.5f * (w[0] - w[1]);
            side *= 1.0f + 1.4f * fx;
            w[0] = mid + side; w[1] = mid - side;

            // 4) Orbite : rotation lente de l'image stereo
            orbitPhase += 0.11 / sr; orbitPhase -= std::floor (orbitPhase);
            const float th = fx * 0.4f * (float) std::sin (twoPi * orbitPhase);
            const float cs = std::cos (th), sn = std::sin (th);
            const float l2 = w[0] * cs - w[1] * sn;
            const float r2 = w[0] * sn + w[1] * cs;

            const float mk = 1.0f / (1.0f + 0.35f * fx);
            const float blend = juce::jmin (1.0f, fx * 5.0f);
            w[0] = chorusOnly[0] + (l2 * mk - chorusOnly[0]) * blend;
            w[1] = chorusOnly[1] + (r2 * mk - chorusOnly[1]) * blend;
        }

        // ---------- Dry / Wet + On/Off ----------
        const float gd = std::cos (mx * halfPi), gwt = std::sin (mx * halfPi);
        const float o0 = in[0] + (in[0] * gd + w[0] * gwt - in[0]) * pw;
        const float o1 = in[1] + (in[1] * gd + w[1] * gwt - in[1]) * pw;
        d0[i] = o0; d1[i] = o1;
        pkL = juce::jmax (pkL, std::abs (o0));
        pkR = juce::jmax (pkR, std::abs (o1));
    }

    levelL.store (pkL);
    levelR.store (pkR);
}

juce::AudioProcessorEditor* EarthChorusProcessor::createEditor() { return new EarthChorusEditor (*this); }

void EarthChorusProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, destData);
}

void EarthChorusProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// ------------------------------------------------------------------ Presets
juce::String EarthChorusProcessor::getCurrentPresetName() const
{
    return apvts.state.getProperty ("presetName", "Default").toString();
}

void EarthChorusProcessor::setParamReal (const juce::String& id, float v)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void EarthChorusProcessor::loadFactoryPreset (int index)
{
    const auto& f = EarthPresets::getFactory();
    if (index < 0 || index >= (int) f.size()) return;
    const auto& p = f[(size_t) index];
    setParamReal ("mode", (float) p.mode);
    setParamReal ("speed", p.speed);
    setParamReal ("depth", p.depth);
    setParamReal ("superfx", p.superFx);
    setParamReal ("drymix", p.mix);
    apvts.state.setProperty ("presetName", p.name, nullptr);
}

juce::File EarthChorusProcessor::getUserPresetDir()
{
    auto d = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #if JUCE_MAC
    d = d.getChildFile ("Application Support");
   #endif
    return d.getChildFile ("NACHE").getChildFile ("Earth Chorus").getChildFile ("Presets");
}

juce::Array<juce::File> EarthChorusProcessor::getUserPresetFiles() const
{
    auto files = getUserPresetDir().findChildFiles (juce::File::findFiles, false, "*.xml");
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
               { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
    return files;
}

bool EarthChorusProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty()) return false;
    auto dir = getUserPresetDir();
    if (! dir.createDirectory()) return false;

    juce::XmlElement x ("EarthChorusPreset");
    x.setAttribute ("name", clean);
    for (auto* id : kIds) x.setAttribute (id, (double) apvts.getRawParameterValue (id)->load());

    const bool ok = x.writeTo (dir.getChildFile (clean + ".xml"));
    if (ok) apvts.state.setProperty ("presetName", clean, nullptr);
    return ok;
}

void EarthChorusProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr || ! xml->hasTagName ("EarthChorusPreset")) return;
    for (auto* id : kIds)
        if (xml->hasAttribute (id)) setParamReal (id, (float) xml->getDoubleAttribute (id));
    apvts.state.setProperty ("presetName", file.getFileNameWithoutExtension(), nullptr);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EarthChorusProcessor(); }
