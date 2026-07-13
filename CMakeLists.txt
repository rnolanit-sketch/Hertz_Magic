cmake_minimum_required(VERSION 3.22)

project(HertzMagic VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use a local JUCE checkout if present, otherwise fetch it automatically.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(HertzMagic
    COMPANY_NAME "Ric Hertz"
    COMPANY_WEBSITE "https://richertz.com"
    BUNDLE_ID com.richertz.hertzmagic
    PLUGIN_MANUFACTURER_CODE RicH
    PLUGIN_CODE HzMg
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Hertz Magic"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx Dynamics EQ Mastering)

target_sources(HertzMagic PRIVATE
    # DSP workspace — one class split across per-module translation units
    Source/dsp/PluginProcessor.cpp
    Source/dsp/Parameters.cpp
    Source/dsp/Eq.cpp
    Source/dsp/MultibandComp.cpp
    Source/dsp/Saturation.cpp
    Source/dsp/SpectralTame.cpp
    Source/dsp/Master.cpp
    # UI workspace — editor + one file per widget group
    Source/ui/PluginEditor.cpp
    Source/ui/LookAndFeel.cpp
    Source/ui/EqCurveDisplay.cpp
    Source/ui/MultibandGRDisplay.cpp
    Source/ui/SaturationDisplays.cpp
    Source/ui/Meters.cpp
    Source/ui/SpectralTameDisplay.cpp
    Source/ui/PresetBar.cpp)

# So bare #include "PluginProcessor.h" / "PluginEditor.h" resolve from either workspace
target_include_directories(HertzMagic PRIVATE Source/dsp Source/ui)

target_compile_definitions(HertzMagic PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(HertzMagic
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
