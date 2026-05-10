cmake_minimum_required(VERSION 3.16)
project(SoundVisualizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatyzacja Qt - to musi być ON
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# To naprawia Twój błąd! Mówimy CMake, gdzie leżą pliki .ui, 
# żeby AUTOUIC wiedział gdzie ich szukać, gdy robisz include w folderze src/
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

# Znajdowanie FFTW
find_path(FFTW_INCLUDE_DIR NAMES fftw3.h)
find_library(FFTW_LIBRARY NAMES fftw3)

if(NOT FFTW_LIBRARY)
    message(FATAL_ERROR "Nie znaleziono biblioteki FFTW!")
endif()

# Znajdowanie Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

# Lista plików - najlepiej podawać pełne ścieżki
set(SOURCES
    src/main.cpp
    src/McuAudioReceiver.cpp
    src/OscillogramWidget.cpp
    src/FFTWidget.cpp
    src/DataProcessor.cpp
    src/mainwindow.cpp
    ui/mainwindow.ui  # Plik UI jest tutaj
)

set(HEADERS
    inc/McuAudioReceiver.h
    inc/OscillogramWidget.h
    inc/FFTWidget.h
    inc/DataProcessor.h
    inc/config.h
    inc/mainwindow.h
    inc/style.h
)

add_executable(SoundVisualizer ${SOURCES} ${HEADERS})

# Dodajemy ścieżki do nagłówków
target_include_directories(SoundVisualizer PRIVATE 
    inc 
    ${FFTW_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # Tu powstają pliki ui_*.h
    ${CMAKE_CURRENT_BINARY_DIR}/SoundVisualizer_autogen/include # Czasem Qt tu chowa nagłówki
)

# Linkowanie wszystkiego
target_link_libraries(SoundVisualizer PRIVATE 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::SerialPort
    ${FFTW_LIBRARY}
)