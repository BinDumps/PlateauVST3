cmake_minimum_required(VERSION 3.20)
project(PlateauVST VERSION 1.0.0)

set(JUCE_INCLUDE_ZLIB_CODE 1)
set(JUCE_INCLUDE_JPEGLIB_CODE 1)
set(JUCE_INCLUDE_PNGLIB_CODE 1)

find_package(JUCE REQUIRED)

juce_add_plugin(Plateau
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    FORMATS VST3
    PRODUCT_NAME "Plateau"
    COMPANY_NAME "Valley Audio"
    BUNDLE_ID "com.valley.plateau"
    PLUGIN_MANUFACTURER_CODE "Vlly"
    PLUGIN_CODE "Plat"
    VST3_AUTO_MANIFEST FALSE
)

target_compile_definitions(Plateau
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_INCLUDE_ZLIB_CODE=1
        JUCE_INCLUDE_JPEGLIB_CODE=1
        JUCE_INCLUDE_PNGLIB_CODE=1
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Plateau
    PRIVATE
        juce::juce_audio_processors
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
)

target_sources(Plateau
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Dattorro.cpp
        Source/BinaryData.cpp
)

target_include_directories(Plateau PRIVATE Source)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR NOT CMAKE_CROSSCOMPILING)
    add_custom_command(TARGET Plateau_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:Plateau_VST3>/../Resources"
        COMMAND ${CMAKE_COMMAND} -E echo "{\"Version\":\"1.0.0\",\"VST3ID\":\"com.valley.plateau\",\"VST3Categories\":\"Fx\"}"
            > "$<TARGET_FILE_DIR:Plateau_VST3>/../Resources/moduleinfo.json"
        COMMENT "Generating VST3 moduleinfo.json"
        VERBATIM)
endif()
