#pragma once

#include <QString>

/**
 * @file
 * @brief Plik zawierający stałe wartości używane w całej aplikacji.
 */

/**
 * @brief Nazwa portu szeregowego.
 */
const QString portName = "/dev/ttyACM0";

/**
 * @brief Szybkość transmisji danych w b/s (domyślnie 115200 b/s).
 * Wartość ta jest jedynie atrapą, w praktyce MCU jest wyposażone w NativeUSB,
 * które umożliwia przesyłanie z prędkością 12 Mb/s.
 */
constexpr int baudRate = 115200;

/**
 * @brief Rozmiar liczby próbek wysyłanych w jednym pakiecie z MCU.
 */
constexpr size_t samplesPacketSize = 2048;

/**
 * @brief Częstotliwość próbkowania mikrofonu.
 */
constexpr int sampleRate = 16000;

/**
 * @brief Czas wyświetlanej historii dźwięku.
 */
constexpr int displaySeconds = 5;

/**
 * @brief Czas odświeżania w ms.
 */
constexpr int refreshRate = 33;