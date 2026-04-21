cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)
project(SoundVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets SerialPort REQUIRED)

include_directories(inc)

set(SOURCES
    src/main.cpp
    src/McuAudioReceiver.cpp
    src/OscillogramWidget.cpp
    src/FFTWidget.cpp
)

set(HEADERS
    inc/McuAudioReceiver.hh
    inc/OscillogramWidget.hh
    inc/FFTWidget.hh
)

add_executable(SoundVisualizer
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(SoundVisualizer PRIVATE Qt6::Core Qt6::Widgets Qt6::SerialPort)