#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/*
  HERTZ MAGIC — Ric Hertz Mastering
  Chain (drag-reorderable): Hertzteq EQ · Multiband Comp · Saturation
  Default routing: EQ -> Comp -> Saturation. Master strip always last.
  Saturation module: Phoenix-style TAPE stage into ECC83-style VALVE stage,
  independent drives and bypasses, both inside one 4x oversampled block.
*/
class HertzMagicAudioProcessor : public juce::AudioProcessor
{
public:
    enum class Module { EQ = 0, Comp = 1, Sat = 2 };
    static constexpr int kNumModules = 3;

    HertzMagicAudioProcessor();
    ~HertzMagicAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                     { return true; }
    const juce::String getName() const override         { return "Hertz Magic"; }
    bool acceptsMidi() const override                   { return false; }
    bool producesMidi() const override                  { return false; }
    bool isMidiEffect() const override                  { return false; }
    double getTailLengthSeconds() const override        { return 0.0; }
    int getNumPrograms() override                       { return 1; }
    int getCurrentProgram() override                    { return 0; }
    void setCurrentProgram (int) override               {}
    const juce::String getProgramName (int) override    { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==========================================================================
    juce::AudioProcessorValueTreeState apvts;

    std::array<int, kNumModules> chainOrder { 0, 1, 2 };  // EQ -> Comp -> Sat

    std::atomic<float> inLevelDb  { -90.0f };
    std::atomic<float> outLevelDb { -90.0f };
    std::atomic<float> heat       { 0.0f };
    std::atomic<float> bandGrDb[3] { {0.0f}, {0.0f}, {0.0f} };
    std::atomic<float> limGrDb { 0.0f };
    std::atomic<float> rmsDb   { -90.0f };
    std::atomic<float> lufsDb  { -90.0f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // ---- EQ ----
    void updateEqCoefficients();
    using Filter    = juce::dsp::IIR::Filter<float>;
    using Coeffs    = juce::dsp::IIR::Coefficients<float>;
    using StereoIIR = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;
    StereoIIR lfBoostShelf, lfAttenShelf, hfBoostPeak, hfAttenShelf;
    StereoIIR notch1, notch2;
    float cLfBoost{-1},cLfAtten{-1},cHfBoost{-1},cHfBw{-1},cHfAtten{-1};
    int   cLfFreq{-1},cHfFreq{-1},cHfAttenSel{-1};
    float cN1F{-1},cN1D{-1},cN1Q{-1},cN2F{-1},cN2D{-1},cN2Q{-1};

    // ---- Multiband comp ----
    struct BandComp
    {
        juce::dsp::IIR::Filter<float> lpA[2], lpB[2], hpA[2], hpB[2];
        float envDb = -90.0f;
    };
    BandComp bands[3];
    void prepareCrossovers (double sr);
    void updateCrossoverCoeffs (double sr);
    void processMultibandComp (juce::AudioBuffer<float>&);
    juce::AudioBuffer<float> bandBuf[3];
    float xo1 = 200.0f, xo2 = 4000.0f;

    // ---- Saturation (tape -> valve, 4x oversampled) ----
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    float dcX1[2]{}, dcY1[2]{};
    float dcR = 0.999f;
    float tapeLPz[2]{};              // tape HF-softening one-pole state
    float sideLPz[2]{};              // side low-pass pre-saturation state (M/S mode)
    float sideLPzOut[2]{};           // output of side LP (what gets saturated)
    void processSaturation (juce::dsp::AudioBlock<float>&);

    // ---- Final stage (FIXED at end): clipper -> lookahead limiter ----
    std::unique_ptr<juce::dsp::Oversampling<float>> clipOversampler;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> laDelay { 8192 };
    float limEnv = 1.0f, limAvgGr = 0.0f;
    juce::dsp::IIR::Filter<float> kShelf[2], kHip[2];   // K-weighting (BS.1770)
    float rmsState = 0.0f, lufsState = 0.0f;
    int lookaheadSamples = 0;
    float pokeFast = 0.0f, pokeSlow = 0.0f;             // transient detector envs
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> deltaDelay { 16384 };
    juce::AudioBuffer<float> deltaBuffer;
    void processFinal (juce::AudioBuffer<float>&);

    // ---- Master ----
    double currentSampleRate = 44100.0;
    juce::SmoothedValue<float> inGain, outGain, mixSmooth;
    juce::AudioBuffer<float> dryBuffer;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> dryDelay { 8192 };
    int latencySamples = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HertzMagicAudioProcessor)
};
