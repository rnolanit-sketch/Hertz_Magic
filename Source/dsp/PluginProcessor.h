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
class HertzMagicAudioProcessor : public juce::AudioProcessor,
                                 private juce::AudioProcessorValueTreeState::Listener,
                                 private juce::AsyncUpdater
{
public:
    enum class Module { EQ = 0, Comp = 1, Sat = 2 };
    static constexpr int kNumModules = 3;

    HertzMagicAudioProcessor();
    ~HertzMagicAudioProcessor() override;

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
    std::atomic<float> inRmsDb    { -90.0f };
    std::atomic<float> outLevelDb { -90.0f };
    std::atomic<float> heat       { 0.0f };
    std::atomic<float> bandGrDb[3] { {0.0f}, {0.0f}, {0.0f} };
    std::atomic<float> limGrDb  { 0.0f };
    std::atomic<float> pokeMeter { 0.0f };   // transient-poke activity (0..1)
    std::atomic<float> clipMeter { 0.0f };   // clipper reduction (0..1)
    std::atomic<float> rmsDb   { -90.0f };
    std::atomic<float> lufsDb  { -90.0f };

    static constexpr int kSSBands = 6;
    std::atomic<float> ssGrDb[kSSBands] { {0.f},{0.f},{0.f},{0.f},{0.f},{0.f} };

    // Saturation "extremity" meters (0..1 harmonic-activity per stage)
    std::atomic<float> tapeSat  { 0.0f };
    std::atomic<float> valveSat { 0.0f };

    // ---- Spectrum analyser scope (audio thread writes, editor reads) ------
    static constexpr int kScopeSize = 2048;   // power of two
    void copyScope (float* dst, int num) const;   // newest `num` samples, time order

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    // Runtime oversampling switch (message-thread rebuild via AsyncUpdater)
    void parameterChanged (const juce::String&, float) override;
    void handleAsyncUpdate() override;
    void rebuildClipOversampling();
    int  lastBlockSize = 0, lastNumCh = 2, satLatSamples = 0;

    // Lock-free mono ring for the analyser
    std::array<std::atomic<float>, kScopeSize> scopeBuf;
    std::atomic<int> scopeWrite { 0 };
    void pushScope (const float* L, const float* R, int n);

    // ---- EQ ----
    void updateEqCoefficients();
    using Filter    = juce::dsp::IIR::Filter<float>;
    using Coeffs    = juce::dsp::IIR::Coefficients<float>;
    using StereoIIR = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;
    StereoIIR lfBoostShelf, lfAttenShelf, hfBoostPeak, hfAttenShelf;
    StereoIIR notch1, notch2;
    StereoIIR lowCutA, lowCutB;      // LR4 (24 dB/oct) high-pass — "low cut"
    float cLfBoost{-1},cLfAtten{-1},cHfBoost{-1},cHfBw{-1},cHfAtten{-1};
    int   cLfFreq{-1},cHfFreq{-1},cHfAttenSel{-1};
    float cN1F{-1},cN1D{-1},cN1Q{-1},cN2F{-1},cN2D{-1},cN2Q{-1};
    float cLcFreq{-1};

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
    float valveLPz[2]{};             // valve output low-pass state (stereo mode)
    void processSaturation (juce::dsp::AudioBlock<float>&);

    // ---- Spectral tame (post-saturation dynamic resonance suppression) ----
    juce::dsp::IIR::Filter<float> ssDet[kSSBands];      // mono detection bandpasses
    juce::dsp::IIR::Filter<float> ssCut[kSSBands][2];   // per-channel dynamic cuts
    float ssEnvDb[kSSBands]{}, ssGrSm[kSSBands]{};
    float cSsFreq[kSSBands]{};   // cached band centres (ss_freq0..5), rebuild detection filter on change
    void processSpectralTame (juce::AudioBuffer<float>&);

    // ---- Final stage (FIXED at end): clipper -> lookahead limiter ----
    std::unique_ptr<juce::dsp::Oversampling<float>> clipOversampler;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> laDelay { 8192 };
    float limEnv = 1.0f, limAvgGr = 0.0f;
    juce::dsp::IIR::Filter<float> kShelf[2], kHip[2];       // K-weighting (BS.1770), output meter
    juce::dsp::IIR::Filter<float> kShelfIn[2], kHipIn[2];   // K-weighting, input reference (gain match)
    float inRmsState = 0.0f;
    // Sliding loudness windows: 3 / 5 / 10 s maintained together, one selected
    std::vector<float> loudRms, loudK;   // rings sized to the longest window
    int   loudMax = 0, loudPos = 0, loudLen3 = 0, loudLen5 = 0, loudLen10 = 0;
    double rmsSum3=0, rmsSum5=0, rmsSum10=0, kSum3=0, kSum5=0, kSum10=0;
    std::vector<float> tpEnv;            // per-sample true-peak envelope estimate
    int lookaheadSamples = 0;
    float pokeFast = 0.0f, pokeSlow = 0.0f;             // transient detector envs
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> deltaDelay { 32768 };
    juce::AudioBuffer<float> deltaBuffer;
    void processFinal (juce::AudioBuffer<float>&);

    // ---- Master ----
    double currentSampleRate = 44100.0;
    juce::SmoothedValue<float> inGain, outGain, mixSmooth;
    juce::AudioBuffer<float> dryBuffer;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> dryDelay { 8192 };
    int latencySamples = 0;

    // ---- Gain match (loudness-compensated A/B) ----
    // Trims the FINAL output (post clipper/limiter) down to the pre-processing input
    // loudness using K-weighted (LUFS) mean-square. Attenuate-only, so the limiter's
    // ceiling can never be breached.
    juce::SmoothedValue<float> gmGain;   // 1.0 = no compensation
    float gmInLoudState  = 0.0f;         // ~400 ms K-weighted, channel-summed MS of trimmed input
    float gmOutLoudState = 0.0f;         // ~400 ms K-weighted, channel-summed MS of final output (pre-gm)

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HertzMagicAudioProcessor)
};
