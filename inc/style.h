#pragma once

#include <QString>

/**
 * @brief Styl aplikacji w QSS.
 */
const QString myTheme = R"(
/* === GŁÓWNE OKNO (Powrót do eleganckiego Dark Mode) === */
QMainWindow, #MainWindow, #MainLayout {
    /* Głęboki, ciemny gradient - powrót do pierwotnej, mrocznej palety */
    background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, 
                                stop:0 #1A1B26, stop:1 #24283B);
}

/* === STYLIZACJA GŁÓWNEGO TYTUŁU (Tytul_l) - Wersja Neon-Kapsuła === */
#Tytul_l {
    color: #FFFFFF;
    font-size: 34px;              /* Odrobinę większy */
    font-weight: 900;             /* Maksymalny Bold */
    letter-spacing: 6px;          /* Jeszcze nowocześniejszy rozstaw */
    padding: 12px 30px;
    qproperty-alignment: 'AlignCenter'; /* Wyśrodkowanie napisu (wymaga Qt5/6) */

    /* 1. FAJNY GRADIENT: Centralny, eliptyczny. 
       Ciemniejszy środek, rozjaśniający się ku brzegom, co daje efekt 3D. */
    background: qradialgradient(cx:0.5, cy:0.5, radius:0.8, fx:0.5, fy:0.5,
                                stop:0 rgba(26, 27, 38, 0.6),  /* Środek ciemny jak tło */
                                stop:0.7 rgba(61, 89, 161, 0.5), /* Przejście w niebieski */
                                stop:1 rgba(122, 162, 247, 0.2)); /* Jasne krawędzie */

    /* 2. KRAWDĘDZIE: Pełna ramka, ale boki grubsze i świecące */
    border: 2px solid rgba(122, 162, 247, 0.3); /* Delikatna ramka góra/dół */
    border-left: 6px solid #7AA2F7;             /* Mocny akcent lewo */
    border-right: 6px solid #7AA2F7;            /* Mocny akcent prawo */
    
    /* 3. KSZTAŁT: Zaokrąglamy rogi, tworząc kapsułę */
    border-radius: 15px;

    /* 4. CIEŃ/BLASK: Delikatny wewnetrzny blask (tylko góra/dół) */
    /* Niestety QSS nie ma box-shadow, więc symulujemy to grubym, przezroczystym paddingiem góra/dół w ramce, 
       ale najlepszy efekt daje gradient radialny powyżej. */

    margin: 10px auto 25px auto; /* Centrowanie belki w layoutcie i margines na dole */
}

#Opcje_b,
#WidokAn_b,
#WidokMuz_b,
#Wyjscie_b {
font-size: 24px;
}

/* === "SZKLANE" WARSTWY (Ostateczna Lista) === */
#Widoki_sw, 
#MainMenu_w, 
#Opcje_w, 
#WidokAnalityczny_w, 
#WidokMuzyczny_w,
#Widoki_an_sw,
#Widoki_muz_sw,
#FFT_w,
#Osc_Spekt_w,
#Osc_w,
#Spekt_w,
#Pianino_w,
#Pieciolinia_w {
    background: transparent;
    border: none; /* Dodatkowo usuwamy ewentualne domyślne ramki */
}

/* === DOMYŚLNY TEKST === */
QWidget {
    color: #C0CAF5; 
    font-family: "Segoe UI", "Ubuntu", Arial, sans-serif;
}

/* === ZWYKŁE NAPISY (QLabel) === */
QLabel {
    color: #C0CAF5;
    font-size: 18px; /* Zostawiamy duże napisy */
    font-weight: bold;
    background: transparent; 
}

/* === STYL SLIDERA === */
QSlider {
    background: transparent;
}
QSlider::groove:horizontal {
    border: 1px solid #414868;
    height: 10px;
    background: #16161E; 
    border-radius: 5px;
}
QSlider::sub-page:horizontal {
    background: #3D59A1; 
    border: 1px solid #274472;
    height: 10px;
    border-radius: 5px;
}
QSlider::add-page:horizontal {
    background: #16161E;
    border: 1px solid #414868;
    height: 10px;
    border-radius: 5px;
}
QSlider::handle:horizontal {
    background: #7AA2F7; 
    border: 2px solid #C0CAF5;
    width: 22px; 
    height: 22px; 
    margin: -6px 0;
    border-radius: 11px;
}
QSlider::handle:horizontal:hover {
    background: #89B4FA;
    border: 2px solid #FFFFFF;
}

/* === STYL SPINBOXA & DOUBLE SPINBOXA === */
QSpinBox, QDoubleSpinBox {
    border: 2px solid #414868;
    border-radius: 6px;
    padding: 6px 10px;
    background-color: #16161E; /* Bardzo ciemne tło dla kontrastu */
    color: #7AA2F7; 
    font-size: 18px; /* Duże liczby */
    font-weight: bold;
    selection-background-color: #3D59A1;
    selection-color: white;
}

QSpinBox:focus, QDoubleSpinBox:focus {
    border: 2px solid #7AA2F7;
    background-color: #1A1B26;
}

/* Przyciski wewnątrz Spinboxa */
QSpinBox::up-button, QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 28px;
    border-left: 2px solid #414868;
    border-top-right-radius: 4px;
    background-color: #24283B;
}
QSpinBox::down-button, QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 28px;
    border-left: 2px solid #414868;
    border-top: 1px solid #414868;
    border-bottom-right-radius: 4px;
    background-color: #24283B;
}

/* === STRZAŁKI SVG === */
QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
    image: url("/home/jakub/studia/programowanie/C++/WDS/graphics/w_gore.svg");
    width: 14px;
    height: 14px;
}
QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
    image: url("/home/jakub/studia/programowanie/C++/WDS/graphics/w_dol.svg");
    width: 14px;
    height: 14px;
}

/* === PRZYCISKI (QPushButton) === */
QPushButton {
    background-color: #3D59A1;
    color: #FFFFFF;
    border: 1px solid #274472;
    border-radius: 6px;
    padding: 8px 18px;
    font-size: 18px; /* Duży tekst na przycisku */
    font-weight: bold;
}
QPushButton:hover {
    background-color: #7AA2F7;
    color: #16161E; 
}

/* === RAMKI Z TYTUŁEM (QGroupBox) === */
QGroupBox {
    border: 2px solid #414868;
    border-radius: 6px;
    margin-top: 16px;
    background-color: rgba(36, 40, 59, 0.4); /* Lekko przezroczyste wnętrze */
}
QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 10px;
    padding: 0 5px;
    color: #7AA2F7;
    font-size: 16px; 
    font-weight: bold;
}
    )";