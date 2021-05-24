/*******************************************************************************
 * stm32async: Asynchronous I/O C++ library for STM32
 * *****************************************************************************
 * Copyright (C) 2018 Mikhail Kulesh
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

#include "Dsp_TDA7439.h"
#include "../UsartLogger.h"

using namespace Stm32async::Drivers;

#define USART_DEBUG_MODULE "DSP: "

Dsp_TDA7439::Dsp_TDA7439 (BaseI2C & _i2c, uint16_t _address) :
    i2c { _i2c },
    address { _address },
    input { 0 },
    inputGain { 0 },
    volume { 0 },
    bass { 0 },
    middle { 0 },
    trebble { 0 }
{
    // empty
}

void Dsp_TDA7439::setInput(uint8_t _input)
{
    input = _input;
    switch (input)
    {
        case 1: i2Cwrite(INPUT_CMD, INPUT_1); break;
        case 2: i2Cwrite(INPUT_CMD, INPUT_2); break;
        case 3: i2Cwrite(INPUT_CMD, INPUT_3); break;
        case 4: i2Cwrite(INPUT_CMD, INPUT_4); break;
    }
}


void Dsp_TDA7439::setInputGain(uint8_t _inputGain)
{
    inputGain = std::min(_inputGain, INPUT_GAIN_MAX);
    i2Cwrite(INPUT_GAIN_CMD, inputGain);
}


void Dsp_TDA7439::setVolume(uint8_t _volume)
{
    volume = _volume;
    i2Cwrite(VOLUME_CMD, VOLUME_MAX - volume);
}


void Dsp_TDA7439::setSpeakerAttenuation (uint8_t left, uint8_t right)
{
    i2Cwrite(ATT_LEFT_CMD, left);
    i2Cwrite(ATT_RIGHT_CMD, right);
}


void Dsp_TDA7439::setTone (ToneRange range, uint8_t val)
{
    uint8_t cmd;
    switch (range)
    {
      case ToneRange::BASS:    cmd = BASS_CMD;    bass = val;    break;
      case ToneRange::MIDDLE:  cmd = MIDDLE_CMD;  middle = val;  break;
      case ToneRange::TREBBLE: cmd = TREBBLE_CMD; trebble = val; break;
      default:                 cmd = MIDDLE_CMD;  middle = val;  break;
    }
    switch (val)
    {
      case 0 : val = 0;  break;
      case 1 : val = 1;  break;
      case 2 : val = 2;  break;
      case 3 : val = 3;  break;
      case 4 : val = 4;  break;
      case 5 : val = 5;  break;
      case 6 : val = 6;  break;
      case 7 : val = 7;  break;
      case 8 : val = 14; break;
      case 9 : val = 13; break;
      case 10: val = 12; break;
      case 11: val = 11; break;
      case 12: val = 10; break;
      case 13: val = 9;  break;
      case 14: val = 8;  break;
    }
    i2Cwrite(cmd, val);
}


void Dsp_TDA7439::i2Cwrite (uint8_t reg, uint8_t data)
{
    buffer[0] = reg;
    buffer[1] = data;
    HAL_StatusTypeDef st = i2c.masterTransmitBlocking(address, buffer, 2);
    if (st == HAL_OK)
    {
        USART_DEBUG("I2C(" << reg << "," << data << ") -> OK" << UsartLogger::ENDL);
    }
    else
    {
        USART_DEBUG("I2C(" << reg << "," << data << ") -> ERROR: " << HAL_I2C_GetError(&(i2c.getParameters())) << UsartLogger::ENDL);
    }
}
