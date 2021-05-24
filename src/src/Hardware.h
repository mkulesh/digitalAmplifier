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

#ifndef HARDWARE_H_
#define HARDWARE_H_

// Peripherie used in this projects
#include "stm32async/HardwareLayout/Dma2.h"
#include "stm32async/HardwareLayout/PortA.h"
#include "stm32async/HardwareLayout/PortB.h"
#include "stm32async/HardwareLayout/PortC.h"
#include "stm32async/HardwareLayout/PortD.h"
#include "stm32async/HardwareLayout/PortH.h"
#include "stm32async/HardwareLayout/Spi1.h"
#include "stm32async/HardwareLayout/I2C2.h"
#include "stm32async/HardwareLayout/Usart1.h"
#include "stm32async/HardwareLayout/Timer1.h"
#include "stm32async/HardwareLayout/Timer2.h"
#include "stm32async/HardwareLayout/Timer3.h"

#include "stm32async/SystemClock.h"
#include "stm32async/Rtc.h"
#include "stm32async/IOPort.h"
#include "stm32async/UsartLogger.h"
#include "stm32async/Timer.h"
#include "stm32async/Spi.h"

#include "stm32async/Drivers/Button.h"
#include "stm32async/Drivers/Led.h"
#include "stm32async/Drivers/Dsp_TDA7439.h"
#include "stm32async/Drivers/EepRom_25AA040A.h"

using namespace Stm32async;

/**
 * @brief A class collecting clock parameters for STM32F405
 */
class ClockParameters
{
public:
    int M, N, Q, P, APB1, APB2;
    float PLLMout, PLLNout, clock48out, SYSCLC;
    ClockParameters ():
        M {0}, N {0}, Q {0}, P {0}, APB1{1}, APB2 {1},
        PLLMout {0.0},
        PLLNout {0.0},
        clock48out {0.0},
        SYSCLC {0.0}
    {
        // empty
    }

    void assign (const ClockParameters & f);
    void print ();
    void searchBestParameters (uint32_t targetFreq);
};


/**
 * @brief A class providing the map of used hardware
 */
class Hardware
{
public:

    static const uint32_t I2C_SPEED = 100000;
    static const uint16_t I2C_MASTER_ADDRESS = 0x01;
    static const uint16_t I2C_DSP_ADDRESS = 0x88;

    // Used ports
    HardwareLayout::PortA portA;
    HardwareLayout::PortB portB;
    HardwareLayout::PortC portC;
    HardwareLayout::PortD portD;
    HardwareLayout::PortH portH;

    // System and MCO
    ClockParameters clockParameters;
    SystemClock sysClock;
    Rtc rtc;

    // LEDs
    Drivers::Led ledBlue;

    // I2C
    HardwareLayout::I2c2 i2c2;
    BaseI2C i2cDsp;
    Drivers::Dsp_TDA7439 tda7439;

    // Channels
    Drivers::Button btn1, btn2, btn3, btn4;
    Drivers::Led led1, led2, led3, led4;

    // Amp switches
    IOPort pinAmpMute, pinAmpEnable, pinAmpGain0, pinAmpGain1;

    // Encoders
    HardwareLayout::Timer1 timer1;
    EncoderTimer volumeEncoder;
    HardwareLayout::Timer2 timer2;
    EncoderTimer bassEncoder;
    HardwareLayout::Timer3 timer3;
    EncoderTimer trebleEncoder;

    // SPI
    HardwareLayout::Spi1 spi1;
    BaseSpi spi;
    Drivers::EepRom_25AA040A eepRom;

    // USART logger
    HardwareLayout::Dma2 dma2;
    HardwareLayout::Usart1 usart1;
    UsartLogger usartLogger;

    Hardware ();

    void abort ();
    void initClock (uint32_t frequency, int APB1, int APB2);
    bool start ();
    void stop ();
    void printResourceOccupation ();

    inline AsyncUsart & getLoggerUsart ()
    {
        return usartLogger.getUsart();
    }
};

#endif
