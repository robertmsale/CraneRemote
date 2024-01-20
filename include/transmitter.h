/*
    Created by Robert Sale, January 2024
*/

#ifndef CR_TRANSMITTER
#define CR_TRANSMITTER
#include "common.h"

namespace CraneRemote {
    namespace AnalogInputs {
        constexpr int BATTERY_LEVEL = 7;
    }

    namespace DigitalInputs {
        constexpr int BATTERY_CHARGER_ENABLED = 2;
    }

    namespace UI {
        /*
        000001111111111111111111111111111111111111111
        000001111111111111111111111111111111111111111
        000001111111111111111111111111111111111111111
        000001111000000000000000000000000000000001111
        0000011110┌───┐0┌───┐0┌───┐0┌───┐0┌────┐01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        1111111110│   │0│   │0│   │0│   │0│    │01111
        0000011110└───┘0└───┘0└───┘0└───┘0└────┘01111
        000001111000000000000000000000000000000001111
        000001111111111111111111111111111111111111111
        000001111111111111111111111111111111111111111
        000001111111111111111111111111111111111111111
        */
       constexpr int16_t BatteryX = (CraneRemote::OLED::SCREEN.WIDTH - 46);
       constexpr int16_t BatteryY = 0;
       constexpr uint16_t Color = 0xFFFF;
       void drawBattery(int withBars) {
            OLED::display().fillRect(
                BatteryX, 
                BatteryY + 5,
                5, 6, Color
            );
            OLED::display().fillRect(
                BatteryX + 5, 
                BatteryY,
                4, OLED::HUD_AREA.HEIGHT, Color
            );
            OLED::display().fillRect(
                BatteryX + 5, 
                BatteryY,
                4, OLED::HUD_AREA.HEIGHT, Color
            );
            OLED::display().fillRect(
                BatteryX + 9, 
                BatteryY,
                32, 3, Color
            );
            OLED::display().fillRect(
                BatteryX + 9, 
                BatteryY + 13,
                32, 3, Color
            );
            if (withBars >= 1) OLED::display().fillRect(
                    BatteryX + 34, 
                    BatteryY + 4,
                    6, 8, Color
                );
            if (withBars >= 2) OLED::display().fillRect(
                    BatteryX + 28, 
                    BatteryY + 4,
                    5, 8, Color
                );
            if (withBars >= 3) OLED::display().fillRect(
                    BatteryX + 22, 
                    BatteryY + 4,
                    5, 8, Color
                );
            if (withBars >= 4) OLED::display().fillRect(
                    BatteryX + 16, 
                    BatteryY + 4,
                    5, 8, Color
                );
            if (withBars >= 5) OLED::display().fillRect(
                    BatteryX + 10, 
                    BatteryY + 4,
                    5, 8, Color
                );
        }
        constexpr uint8_t LIGHTNING_BOLT[] = {
            0b00000011, 0b11111111, // 0000001111111111
            0b00000111, 0b11111110, // 0000011111111110
            0b00001111, 0b11111100, // 0000111111111100
            0b00011111, 0b11111000, // 0001111111111000
            0b00111111, 0b11110000, // 0011111111110000
            0b01111111, 0b11111111, // 0111111111111111
            0b00000000, 0b01111110, // 0000000001111110
            0b00000000, 0b11111100, // 0000000011111100
            0b00000001, 0b11111000, // 0000000111111000
            0b00000111, 0b11100000, // 0000011111100000
            0b00001111, 0b10000000, // 0000111110000000
            0b00001110, 0b00000000, // 0000111000000000
            0b00011100, 0b00000000, // 0001110000000000
            0b00011000, 0b00000000, // 0001100000000000
            0b00110000, 0b00000000, // 0011000000000000
            0b00100000, 0b00000000, // 0010000000000000
        };
        void drawChargingSymbol() {
            OLED::display().drawBitmap(BatteryX - 17, 0, LIGHTNING_BOLT, 16, 16, Color);
        }
    }

    namespace InputStates {
        Bounce2::Button& upButton() {static Bounce2::Button btn; return btn;}
        Bounce2::Button& downButton() {static Bounce2::Button btn; return btn;}
        float& batteryPercent() {static float batpcnt; return batpcnt;}
    }

    namespace EventHandling {
        inline void handleBatteryPercentage() {
            // 10-bit value coerced into 0-5
            InputStates::batteryPercent() = ((float)analogRead(AnalogInputs::BATTERY_LEVEL) / BATTERY_MAX_VALUE * 5.0f);
            UI::drawBattery((int)InputStates::batteryPercent);
        }
        inline uint8_t handleCommandButtons() {
            static unsigned long previousMillis{millis()};
            if (previousMillis - millis() < debounceInterval) return;
            InputStates::upButton().update();
            InputStates::downButton().update();
            bool upPressed = InputStates::upButton().pressed();
            bool downPressed = InputStates::downButton().pressed();
            if ((upPressed && downPressed) || (!upPressed && !downPressed)) {
                while (!Radio::radio().write(&Radio::Payloads::STOP, 1));
                return Radio::Payloads::STOP;
            }
            if (upPressed) {
                Radio::radio().write(&Radio::Payloads::GO_UP, 1);
                return Radio::Payloads::GO_UP;
            }
            if (downPressed) {
                Radio::radio().write(&Radio::Payloads::GO_DOWN, 1);
                return Radio::Payloads::GO_DOWN;
            }
        }
    }

    void initialize() {
        pinMode(AnalogInputs::BATTERY_LEVEL, INPUT);
        pinMode(DigitalInputs::BATTERY_CHARGER_ENABLED, INPUT);

        commonInitialize();

        Radio::radio().openWritingPipe(Radio::address);
        Radio::radio().stopListening();
        OLED::display().setTextSize(2);
    }

    constexpr unsigned long refreshMillis = 42; // ~24 FPS
    void handleRun() {
        static unsigned long previousRefreshMillis = millis();
        handleCommonRun();
        // handle input events first
        uint8_t currentCommand = EventHandling::handleCommandButtons();
        // now draw screen
        if ((millis() - previousRefreshMillis) >= refreshMillis) {
            OLED::display().clearDisplay();
            EventHandling::handleBatteryPercentage();
            UI::drawCommand(currentCommand);
            UI::drawChannel();
            OLED::display().display();

            if (digitalRead(DigitalInputs::BATTERY_CHARGER_ENABLED) & 1) {
                UI::drawChargingSymbol();
            }

            previousRefreshMillis = millis();
        }
    }
}


#endif