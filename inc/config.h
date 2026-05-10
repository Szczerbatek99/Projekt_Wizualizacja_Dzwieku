#pragma once

#include <QString>

/**
 * @file
 * @brief Plik zawierający stałe wartości używane w całej aplikacji.
 */

// ==========================================
// USTAWIENIA KOMUNIKACJI (UART/USB)
// ==========================================

/**
 * @brief Nazwa portu szeregowego.
 */
const QString portName = "/dev/ttyACM0";

/**
 * @brief Zapis słowa startu ramki w hexadecymalnym.
 */
const char* const SYNC = "AA55AA55";

/**
 * @brief Szybkość transmisji danych w b/s (domyślnie 115200 b/s).
 * Wartość ta jest jedynie atrapą, w praktyce MCU jest wyposażone w NativeUSB,
 * które umożliwia przesyłanie z prędkością 12 Mb/s.
 */
constexpr int baudRate = 115200;


// ==========================================
// PARAMETRY SYGNAŁU AUDIO I PACZEK
// ==========================================

/**
 * @brief Częstotliwość próbkowania mikrofonu.
 */
constexpr int sampleRate = 16000;

/**
 * @brief Rozmiar liczby próbek wysyłanych w jednym pakiecie z MCU.
 */
constexpr size_t samplesPacketSize = 256;


// ==========================================
// PARAMETRY ANALIZY DSP I BUFORÓW
// ==========================================

/**
 * @brief Rozmiar okna analitycznego do obliczania FFT.
 */
constexpr int fftWindowSize = 2048;

/**
 * @brief Czas wyświetlanej historii dźwięku w oscyloskopie.
 */
constexpr int displaySeconds = 5;

/**
 * @brief Wielkość bufora oscyloskopu w liczbie próbek.
 */
constexpr int oscillogramBufferSize = sampleRate * displaySeconds;


// ==========================================
// USTAWIENIA INTERFEJSU UŻYTKOWNIKA (UI)
// ==========================================

/**
 * @brief Czas odświeżania widgetów w ms (np. 33 ms to ~30 FPS).
 */
constexpr int refreshRate = 16;
