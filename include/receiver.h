/*
    Created by Robert Sale, January 2024
*/

#ifndef CR_RECEIVER
#define CR_RECEIVER
#include "common.h"

namespace CraneRemote {
    namespace DigitalOutputs {
        constexpr int RELAY_UP = 2;
        constexpr int RELAY_DOWN = 3;
    }

    namespace EventHandling {
        constexpr unsigned long commandTimeout = 100;

        void setRelayState(uint8_t to) {
            if (!!(to & Radio::Payloads::STOP)) {
                digitalWrite(DigitalOutputs::RELAY_UP, LOW);
                digitalWrite(DigitalOutputs::RELAY_DOWN, LOW);
            } else if (!!(to & Radio::Payloads::GO_UP)) {
                digitalWrite(DigitalOutputs::RELAY_UP, HIGH);
                digitalWrite(DigitalOutputs::RELAY_DOWN, LOW);
            } else if (!!(to & Radio::Payloads::GO_DOWN)) {
                digitalWrite(DigitalOutputs::RELAY_UP, LOW);
                digitalWrite(DigitalOutputs::RELAY_DOWN, HIGH);
            } else { // If an invalid command was received, recurse to STOP
                setRelayState(Radio::Payloads::STOP);
            }
        }

        uint8_t handleRadioCommands() {
            static unsigned long lastCommandTime = millis();
            static uint8_t lastReceivedCommand = Radio::Payloads::STOP;
            
            // if commands in radio buffer, process them
            if (Radio::radio().available()) {
                Radio::radio().read(&lastReceivedCommand, sizeof(lastReceivedCommand));
                lastCommandTime = millis();
            }

            // If no response from radio for a while go ahead and STOP
            if (millis() - lastCommandTime > commandTimeout) {
                lastReceivedCommand = Radio::Payloads::STOP;
                lastCommandTime = millis();
                setRelayState(Radio::Payloads::STOP);
            } else {
                setRelayState(lastReceivedCommand);
            }

            return lastReceivedCommand;
        }
    }
    
    void initialize() {

        pinMode(DigitalOutputs::RELAY_DOWN, OUTPUT);
        pinMode(DigitalOutputs::RELAY_UP, OUTPUT);
        
        commonInitialize();

        Radio::radio().openReadingPipe(1, Radio::address);
        Radio::radio().startListening();
        OLED::display().setTextSize(2);
    }

    constexpr unsigned long refreshMillis = 42; // ~24 FPS

    void handleRun() {
        static unsigned long previousRefreshMillis = millis();
        handleCommonRun();
        uint8_t currentCommand = EventHandling::handleRadioCommands();

        if ((millis() - previousRefreshMillis) >= refreshMillis) {
            OLED::display().clearDisplay();
            UI::drawCommand(currentCommand);
            UI::drawChannel();
            OLED::display().display();
            previousRefreshMillis = millis();
        }
    }
}

#endif