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

#include "MyApplication.h"

#define USART_DEBUG_MODULE "APP: "

/************************************************************************
 * Class MyApplication
 ************************************************************************/
const char * MyApplication::modeStr[] = {
    "INPUT",
    "INPUT_GAIN",
    "OUTPUT_GAIN",
    "VOLUME",
    "MUTE",
    "BASS",
    "MIDDLE",
    "TREBLE"
};


MyApplication::MyApplication () :
    Hardware {},
    mode { Mode::INPUT },
    inputBtns { &btn1, &btn2, &btn3, &btn4 },
    inputLeds { &led1, &led2, &led3, &led4 },
    inputChannel { eepRom, "channel", 1 },
    volume { eepRom, "volume", 2 },
    bass { eepRom, "bass", 3 },
    trebble { eepRom, "treble", 4 },
	inputGain { eepRom, "inputGain", 5 },
	outputGain { eepRom, "outputGain", 6 },
    outputGainVal { 0 }
{
    // empty
}


void MyApplication::init ()
{
    ledBlue.turnOn();
    eepRom.getMode(true);
    uint8_t inp = readWithDef(inputChannel, 1);
    tda7439.setInput(inp);
    updateLeds(inp);
    tda7439.setVolume(readWithDef(volume, 10));
    tda7439.setSpeakerAttenuation(0, 0);
    tda7439.setTone(Drivers::Dsp_TDA7439::ToneRange::BASS, readWithDef(bass, 7));
    tda7439.setTone(Drivers::Dsp_TDA7439::ToneRange::MIDDLE, 0);
    tda7439.setTone(Drivers::Dsp_TDA7439::ToneRange::TREBBLE, readWithDef(trebble, 7));
    tda7439.setInputGain(readWithDef(inputGain, 0));
    setOutputGain(readWithDef(outputGain, 0));
    ledBlue.turnOff();
}


void MyApplication::setInput(uint8_t input)
{
    ledBlue.turnOn();
    USART_DEBUG("input channel = " << input << UsartLogger::ENDL);
    tda7439.setInput(input);
    inputChannel.write(input);
    updateLeds(input);
    ledBlue.turnOff();
}


void MyApplication::updateLeds(uint8_t input)
{
    for (auto l : inputLeds)
    {
        l->turnOff();
    }
    size_t idx = input - 1;
    if (idx < BTN_COUNT)
    {
        inputLeds[idx]->turnOn();
    }
}


void MyApplication::updateActiveLed(int mode)
{
	uint8_t input = tda7439.getInput() - 1;
	if (input < BTN_COUNT)
	{
		switch (mode)
		{
		case 0: inputLeds[input]->turnOff(); break;
		case 1: inputLeds[input]->turnOn(); break;
		case 2: inputLeds[input]->toggle(); break;
		}
	}
}


void MyApplication::setUp (Mode _mode)
{
    ledBlue.turnOn();
    switch (_mode)
    {
        case Mode::INPUT: tda7439.inputUp(); break;
        case Mode::INPUT_GAIN: tda7439.inputGainUp(); break;
        case Mode::OUTPUT_GAIN:
            {
                outputGainVal = std::min(OUTPUT_GAIN_MAX, outputGainVal + 1);
                setOutputGain(outputGainVal);
            }
            break;
        case Mode::VOLUME:
            if (tda7439.getVolume() == Drivers::Dsp_TDA7439::VOLUME_MIN)
            {
                unmute(MUTE_DELAY);
            }
            tda7439.volumeUp();
            break;
        case Mode::MUTE: tda7439.mute(); break;
        case Mode::BASS: tda7439.bassUp(); break;
        case Mode::MIDDLE: tda7439.middleUp(); break;
        case Mode::TREBLE: tda7439.trebbleUp(); break;
    }
    ledBlue.turnOff();
}


void MyApplication::setDown (Mode _mode)
{
    ledBlue.turnOn();
    switch (_mode)
    {
        case Mode::INPUT: tda7439.inputDown(); break;
        case Mode::INPUT_GAIN: tda7439.inputGainDown(); break;
        case Mode::OUTPUT_GAIN:
            {
                outputGainVal = std::max(OUTPUT_GAIN_MIN, outputGainVal - 1);
                setOutputGain(outputGainVal);
            }
            break;
        case Mode::VOLUME:
            tda7439.volumeDown();
            if (tda7439.getVolume() == Drivers::Dsp_TDA7439::VOLUME_MIN)
            {
                mute(MUTE_DELAY);
            }
            break;
        case Mode::MUTE: tda7439.unmute(); break;
        case Mode::BASS: tda7439.bassDown(); break;
        case Mode::MIDDLE: tda7439.middleDown(); break;
        case Mode::TREBLE: tda7439.trebbleDown(); break;
    }
    ledBlue.turnOff();
}


void MyApplication::unmute (int delay)
{
    USART_DEBUG("unmute amp" << UsartLogger::ENDL);
    pinAmpEnable.setLow();
    HAL_Delay(delay);
    pinAmpMute.setHigh();
	updateActiveLed(1);
}


void MyApplication::mute (int delay)
{
    USART_DEBUG("mute amp" << UsartLogger::ENDL);
	updateActiveLed(0);
    pinAmpMute.setLow();
    HAL_Delay(delay);
    pinAmpEnable.setHigh();
}


void MyApplication::setOutputGain(uint32_t g)
{
    USART_DEBUG("output gain: " << g << UsartLogger::ENDL);
    switch (g)
    {
    case 0:
        pinAmpGain0.setHigh();
        pinAmpGain1.setHigh();
        break;
    case 1:
        pinAmpGain0.setLow();
        pinAmpGain1.setHigh();
        break;
    case 2:
        pinAmpGain0.setHigh();
        pinAmpGain1.setLow();
        break;
    case 3:
        pinAmpGain0.setLow();
        pinAmpGain1.setLow();
        break;
    }
}


void MyApplication::processButton(uint8_t num, int numOccured)
{
	if (numOccured > 0)
	{
		return;
	}
    int btnPressed = 0;
    for (auto & b : inputBtns)
    {
    	if (b->isPressed())
    	{
    		btnPressed++;
    	}
    }
    if (btnPressed == 1)
    {
        USART_DEBUG("Single button pressed: " << num << ", mode=" << modeStr[size_t(mode)] << UsartLogger::ENDL);
        if (mode == Mode::INPUT_GAIN && num == 1)
        {
        	setDown(Mode::INPUT_GAIN);
        	inputGain.write(tda7439.getInputGain());
        }
        else if (mode == Mode::INPUT_GAIN && num == 2)
        {
        	setUp(Mode::INPUT_GAIN);
        	inputGain.write(tda7439.getInputGain());
        }
        else if (mode == Mode::OUTPUT_GAIN && num == 3)
        {
        	setDown(Mode::OUTPUT_GAIN);
        	outputGain.write(outputGainVal);
        }
        else if (mode == Mode::OUTPUT_GAIN && num == 4)
        {
        	setUp(Mode::OUTPUT_GAIN);
        	outputGain.write(outputGainVal);
        }
        else
        {
            mode = Mode::INPUT;
            setInput(num);
        }
    }
    else if (btn1.isPressed() && btn2.isPressed())
    {
    	mode = Mode::INPUT_GAIN;
        USART_DEBUG("Two buttons pressed: " << modeStr[size_t(mode)] << UsartLogger::ENDL);
        updateLeds(1);
        led2.setHigh();
    }
    else if (btn3.isPressed() && btn4.isPressed())
    {
    	mode = Mode::OUTPUT_GAIN;
        USART_DEBUG("Two buttons pressed: " << modeStr[size_t(mode)] << UsartLogger::ENDL);
        updateLeds(3);
        led4.setHigh();
    }
}


uint8_t MyApplication::readWithDef(Drivers::EepRomByte & reg, uint8_t def)
{
    uint8_t data = reg.read();
    return (data == 0xFF) ? def : data;
}


void MyApplication::run (uint32_t frequency)
{
    initClock(frequency, 2, 4);
    if (!start())
    {
        abort();
    }

    init();
    if (tda7439.getVolume() != Drivers::Dsp_TDA7439::VOLUME_MIN)
    {
    	unmute(MUTE_DELAY);
    }

    // Start main loop
    while (true)
    {
        volumeEncoder.periodic([&](int change)
        {
            if (change > 0)
            {
                setUp(Mode::VOLUME);
            }
            else
            {
                setDown(Mode::VOLUME);
            }
            volume.delayedWrite(tda7439.getVolume(), EEPROM_DERLAY);
        });

        bassEncoder.periodic([&](int change)
        {
            if (change > 0)
            {
                setUp(Mode::BASS);
            }
            else
            {
                setDown(Mode::BASS);
            }
            bass.delayedWrite(tda7439.getBass(), EEPROM_DERLAY);
        });

        trebleEncoder.periodic([&](int change)
        {
            if (change > 0)
            {
                setUp(Mode::TREBLE);
            }
            else
            {
                setDown(Mode::TREBLE);
            }
            trebble.delayedWrite(tda7439.getTrebble(), EEPROM_DERLAY);

        });

        btn1.periodic([&](uint32_t numOccured)
        {
        	processButton(1, numOccured);
        });

        btn2.periodic([&](uint32_t numOccured)
        {
        	processButton(2, numOccured);
        });

        btn3.periodic([&](uint32_t numOccured)
        {
        	processButton(3, numOccured);
        });

        btn4.periodic([&](uint32_t numOccured)
        {
        	processButton(4, numOccured);
        });

        volume.periodic();
        bass.periodic();
        trebble.periodic();
    }

    stop ();
}

void MyApplication::onRtcSecond()
{
	if (!pinAmpMute.getBit())
	{
		updateActiveLed(2);
	}
}
