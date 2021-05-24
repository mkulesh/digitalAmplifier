/*******************************************************************************
 * Digital Amplifier based on STM32F405RGT6
 * *****************************************************************************
 * Copyright (C) 2021 Mikhail Kulesh
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef MYAPPLICATION_H_
#define MYAPPLICATION_H_

#include "Hardware.h"

class MyApplication : public Hardware
{
public:

    enum class Mode
    {
        INPUT,
        INPUT_GAIN,
        OUTPUT_GAIN,
        VOLUME,
        MUTE,
        BASS,
        MIDDLE,
        TREBLE
    };

    MyApplication ();

    virtual ~MyApplication () = default;

    void run (uint32_t frequency);
    void onRtcSecond();

private:

    static const char * modeStr[];
    static const uint32_t BTN_COUNT = 4;
    static const uint32_t MUTE_DELAY = 250;
    static const int32_t OUTPUT_GAIN_MIN = 0;
    static const int32_t OUTPUT_GAIN_MAX = 3;
    static const int32_t EEPROM_DERLAY = 2000;

    Mode mode;
    Drivers::Button * inputBtns[BTN_COUNT];
    Drivers::Led * inputLeds[BTN_COUNT];
    Drivers::EepRomByte inputChannel, volume, bass, trebble, inputGain, outputGain;
    int32_t outputGainVal;

    void init ();
    void setInput(uint8_t input);
    void updateLeds(uint8_t input);
    void updateActiveLed(int mode);
    void unmute (int delay);
    void mute (int delay);
    void setUp (Mode _mode);
    void setDown (Mode _mode);
    void setOutputGain(uint32_t g);
    void processButton(uint8_t num, int numOccured);
    uint8_t readWithDef(Drivers::EepRomByte & reg, uint8_t def);
};

#endif
