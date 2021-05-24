/*******************************************************************************
 * stm32async: Asynchronous I/O C++ library for STM32
 * *****************************************************************************
 * Copyright (C) 2020 Mikhail Kulesh
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

#ifndef DRIVERS_DSP_H_
#define DRIVERS_DSP_H_

#include "../I2C.h"

namespace Stm32async
{
namespace Drivers
{

#define _valueUp(val, limit) (val < limit ? val + 1 : val)
#define _valueDown(val, limit) (val > limit ? val - 1 : val)

class Dsp_TDA7439
{
public:

    Dsp_TDA7439 (BaseI2C & _i2c, uint16_t _address);

    // Input selection
    static const uint8_t INPUT_CMD = 0x00;
    static const uint8_t INPUT_1 = 0x03;
    static const uint8_t INPUT_2 = 0x02;
    static const uint8_t INPUT_3 = 0x01;
    static const uint8_t INPUT_4 = 0x00;
    static const uint8_t INPUT_MIN = 1;
    static const uint8_t INPUT_MAX = 4;

    inline uint8_t getInput ()
    {
        return input;
    }

    void setInput (uint8_t _input);

    inline void inputDown ()
    {
        setInput(_valueDown(input, INPUT_MIN));
    }
    inline void inputUp ()
    {
        setInput(_valueUp(input, INPUT_MAX));
    }

    // Input gain
    static const uint8_t INPUT_GAIN_CMD = 0x01;
    static const uint8_t INPUT_GAIN_MIN = 0x0; // 0dB
    static const uint8_t INPUT_GAIN_MAX = 0xF; // 30 dB with 2 dB step

    inline uint8_t getInputGain ()
    {
        return inputGain;
    }

    void setInputGain (uint8_t _inputGain);

    inline void inputGainDown ()
    {
        setInputGain(_valueDown(inputGain, INPUT_GAIN_MIN));
    }

    inline void inputGainUp ()
    {
        setInputGain(_valueUp(inputGain, INPUT_GAIN_MAX));
    }

    // Master volume
    static const uint8_t VOLUME_CMD = 0x02;
    static const uint8_t VOLUME_MIN = 0x00; // 0dB
    static const uint8_t VOLUME_MAX = 0x28; // -40 dB with 1 dB step

    inline uint8_t getVolume ()
    {
        return volume;
    }

    void setVolume (uint8_t _volume);

    inline void volumeDown ()
    {
        setVolume(_valueDown(volume, VOLUME_MIN));
    }

    inline void volumeUp ()
    {
        setVolume(_valueUp(volume, VOLUME_MAX));
    }

    // Mute / unmute
    static const uint8_t MUTE = 0x38;

    inline void mute ()
    {
        i2Cwrite(VOLUME_CMD, MUTE);
    }

    inline void unmute ()
    {
        setVolume(volume);
    }

    // Speaker attenuation value
    static const uint8_t ATT_RIGHT_CMD = 0x06;
    static const uint8_t ATT_LEFT_CMD = 0x07;

    void setSpeakerAttenuation (uint8_t left, uint8_t right);

    // Tone control
    enum class ToneRange
    {
        BASS,
        MIDDLE,
        TREBBLE
    };

    static const uint8_t BASS_CMD = 0x03;
    static const uint8_t MIDDLE_CMD = 0x04;
    static const uint8_t TREBBLE_CMD = 0x05;
    static const uint8_t TONE_MIN = 0;
    static const uint8_t TONE_MAX = 14;

    void setTone (ToneRange range, uint8_t val);

    inline uint8_t getBass()
    {
        return bass;
    }

    inline void bassDown ()
    {
        setTone(ToneRange::BASS, _valueDown(bass, TONE_MIN));
    }

    inline void bassUp ()
    {
        setTone(ToneRange::BASS, _valueUp(bass, TONE_MAX));
    }

    inline void middleDown ()
    {
        setTone(ToneRange::MIDDLE, _valueDown(middle, TONE_MIN));
    }

    inline void middleUp ()
    {
        setTone(ToneRange::MIDDLE, _valueUp(middle, TONE_MAX));
    }

    inline uint8_t getTrebble()
    {
        return trebble;
    }

    inline void trebbleDown ()
    {
        setTone(ToneRange::TREBBLE, _valueDown(trebble, TONE_MIN));
    }

    inline void trebbleUp ()
    {
        setTone(ToneRange::TREBBLE, _valueUp(trebble, TONE_MAX));
    }

private:

    BaseI2C & i2c;
    uint16_t address;
    uint8_t input, inputGain, volume;
    uint8_t bass, middle, trebble;
    uint8_t buffer[4];

    void i2Cwrite (uint8_t reg, uint8_t data);
};

#undef _valueUp
#undef _valueDown

} // end of namespace Drivers
} // end of namespace Stm32async

#endif
