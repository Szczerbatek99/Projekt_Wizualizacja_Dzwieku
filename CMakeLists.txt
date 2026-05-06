cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0100 NEW)
project(SoundVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_path(FFTW_INCLUDE_DIR NAMES fftw3.h)
find_library(FFTW_LIBRARY NAMES fftw3)

if(FFTW_INCLUDE_DIR AND FFTW_LIBRARY)
    message(STATUS "Znaleziono FFTW: ${FFTW_LIBRARY}")
else()
    message(FATAL_ERROR "Nie znaleziono biblioteki FFTW! Sprawdź ścieżki.")
endif()


find_package(Qt6 COMPONENTS Core Widgets SerialPort REQUIRED)


include_directories(inc)

set(SOURCES
    src/main.cpp
    src/McuAudioReceiver.cpp
    src/OscillogramWidget.cpp
    src/FFTWidget.cpp
    src/DataProcessor.cpp
)

set(HEADERS
    inc/McuAudioReceiver.h
    inc/OscillogramWidget.h
    inc/FFTWidget.h
    inc/DataProcessor.h
    inc/config.h
)

add_executable(SoundVisualizer
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(SoundVisualizer PRIVATE Qt6::Core Qt6::Widgets Qt6::SerialPort)
target_include_directories(SoundVisualizer PRIVATE ${FFTW_INCLUDE_DIR})
target_link_libraries(SoundVisualizer PRIVATE ${FFTW_LIBRARY})