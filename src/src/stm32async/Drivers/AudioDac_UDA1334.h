/*******************************************************************************
 * StmPlusPlus: object-oriented library implementing device drivers for 
 * STM32F3 and STM32F4 MCU
 * *****************************************************************************
 * Copyright (C) 2016-2017 Mikhail Kulesh
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

#ifndef DRIVERS_AUDIO_DAC_UDA1334_H_
#define DRIVERS_AUDIO_DAC_UDA1334_H_

#include "../I2S.h"

namespace Stm32async
{
namespace Drivers
{

class AudioDac_UDA1334 : public SharedDevice::DeviceClient
{
public:
    
    static constexpr uint32_t BLOCK_SIZE = 2048;
    static constexpr uint32_t BLOCK_SIZE2 = BLOCK_SIZE / 2;
    static constexpr uint32_t MSB_OFFSET = 0xFFFF / 2 + 1;
    static constexpr uint32_t START_DELAY = 50;

    enum class SourceType
    {
        STREAM = 0, TEST_LIN = 1, TEST_SIN = 2
    };

    AudioDac_UDA1334 (AsyncI2S & _i2s,
                      const HardwareLayout::Port & _powerPort, uint32_t _powerPin,
                      const HardwareLayout::Port & _mutePort, uint32_t _mutePin,
                      const HardwareLayout::Port & _smplFreqPort, uint32_t _smplFreqPin);

    void powerOn ();
    bool start (SourceType s, uint32_t standard, uint32_t audioFreq, uint32_t dataFormat);
    void stop ();

    virtual bool onTransmissionFinished (SharedDevice::State state);
    
    inline bool isActive () const
    {
        return currDataBuffer != NULL;
    }
    
    inline SourceType getSourceType () const
    {
        return sourceType;
    }
    
    inline bool isBlockRequested () const
    {
        return blockRequested;
    }
    
    inline void confirmBlock ()
    {
        blockRequested = false;
    }
    
    inline uint16_t * getBlockPtr ()
    {
        return (currDataBuffer == dataPtr1) ? dataPtr2 : dataPtr1;
    }
    
    inline uint32_t getBlockSize () const
    {
        return BLOCK_SIZE2;
    }
    
private:
    
    AsyncI2S & i2s;
    IOPort power, mute, smplFreq;

    // Source
    SourceType sourceType;

    // Data containers
    uint16_t dataBuffer1[BLOCK_SIZE2];
    uint16_t dataBuffer2[BLOCK_SIZE2];
    uint16_t * dataPtr1;
    uint16_t * dataPtr2;

    // These variables are modified from interrupt service routine, therefore declare them as volatile
    volatile uint16_t * currDataBuffer;
    volatile bool blockRequested;

protected:
    
    void makeTestSignalLin ();
    void makeTestSignalSin ();
};

} // end of namespace Drivers
} // end of namespace Stm32async

#endif
