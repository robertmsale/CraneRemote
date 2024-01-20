/*
    Created by Robert Sale, January 2024

    -- DESCRIPTION

    This is the entry point for the two Arduino Nano modules, transmitter and receiver. If you are
    compiling for the transmitter, set the TRANSMITTER macro to 1, and if you are compiling for a
    receiver set the TRANSMITTER macro to 0.

    Many different SSD1306 OLED displays exist on the market, so a macro representing the address
    is included here incase the ones bought online have a different address. May require
    experimentation, but if the display does not work try adjusting that macro until the display
    kicks on. ONLY WORKS ON 128x64 DISPLAYS!

    main.cpp is very basic. It just imports the correct header, then runs their initialization
    and loop. All global state exists inside the headers and are Meyer's Singletons so they can
    exist inside the headers without being defined here. 

    -- MODULES

    * include/common.h
    This file holds common logic between the transmitter and receiver. It sets up the radio, EEPROM
    storage, OLED display, bitmaps for displaying commands, and channel up/down pins. This file is
    included in both the transmitter and receiver.

    * include/transmitter.h
    For the handheld remote control. This file contains logic for issuing commands to the receiver
    and drawing the battery and charging symbol on the OLED display.

    * include/receiver.h
    This file contains logic for receiving commands over radio and switching the relays.
*/

#include <Arduino.h>

#define TRANSMITTER 1
#define OLED_ADDRESS 0x3D

#ifdef TRANSMITTER
#include "transmitter.h"
#else
#include "receiver.h"
#endif

void setup() {
    CraneRemote::initialize();
}

void loop() {
    CraneRemote::handleRun();
}
