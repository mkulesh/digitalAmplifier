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

#include "Hardware.h"
#include "MyApplication.h"
#include <cmath>

#define USART_DEBUG_MODULE "HRDW: "


/************************************************************************
 * Class ClockParameters
 ************************************************************************/
void ClockParameters::assign (const ClockParameters & f)
{
    M          = f.M;
    N          = f.N;
    Q          = f.Q;
    P          = f.P;
    PLLMout    = f.PLLMout;
    PLLNout    = f.PLLNout;
    clock48out = f.clock48out;
    SYSCLC     = f.SYSCLC;
    APB1       = f.APB1;
    APB2       = f.APB2;
}


void ClockParameters::print ()
{
    USART_DEBUG("HSE=" << HSE_VALUE << " M=" << M << " N=" << N << " Q=" << Q << " P=" << P <<
                " SYSCLC=" << SYSCLC << " APB1=" << APB1 << " APB2=" << APB2 << UsartLogger::ENDL);
}


void ClockParameters::searchBestParameters (uint32_t targetFreq)
{
    int maxM = 0;
    float bestDiff = 9999.0;
    float hse = HSE_VALUE / 1000000;
    ClockParameters bestFactors;
    for (int M = 2; M <= 63; ++M)
    {
        ClockParameters f;
        f.M = M;
        f.PLLMout = hse / (float)f.M;
        if (f.PLLMout >= 0.95 && f.PLLMout <= 2.1)
        {
            for (int N = 50; N <= 432; ++N)
            {
                f.N = N;
                f.PLLNout = hse * (float)f.N / (float)f.M;
                if (f.PLLNout >= 100.0 && f.PLLNout <= 432.0)
                {
                    for (int Q = 2; Q <= 15; ++Q)
                    {
                        f.Q = Q;
                        f.clock48out = f.PLLNout / (float)f.Q;
                        int clock48out = (int)f.clock48out;
                        if (f.clock48out - (float)clock48out == 0.0 && clock48out == 48)
                        {
                            for (int P = 2; P <= 8; P+=2)
                            {
                                f.P = P;
                                f.SYSCLC = f.PLLNout / (float)f.P;
                                if (f.SYSCLC >= 24.0 && f.SYSCLC <= 168.0)
                                {
                                    float diff = ::fabs(f.SYSCLC - targetFreq);
                                    if (diff < bestDiff || (diff == bestDiff && f.M > maxM))
                                    {
                                        maxM = f.M;
                                        bestDiff = diff;
                                        bestFactors.assign(f);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Adjust APB prescalers
    while (true)
    {
        float pclk1 = bestFactors.SYSCLC / (float) bestFactors.APB1;
        if (pclk1 <= 42 || bestFactors.APB1 == 16)
        {
            break;
        }
        bestFactors.APB1 *= 2;
    }
    while (true)
    {
        float pclk1 = bestFactors.SYSCLC / (float) bestFactors.APB2;
        if (pclk1 <= 84 || bestFactors.APB2 == 16)
        {
            break;
        }
        bestFactors.APB2 *= 2;
    }
    assign(bestFactors);
}


/************************************************************************
 * Class Hardware
 ************************************************************************/
Hardware::Hardware ():
    // System, RTC and MCO
    sysClock { HardwareLayout::Interrupt { SysTick_IRQn, 0 } },
    rtc { HardwareLayout::Interrupt { RTC_WKUP_IRQn, 15 } },

    // LEDs
    ledBlue { portC, GPIO_PIN_1, Drivers::Led::ConnectionType::CATHODE },

    // I2C (TDA7439)
    i2c2 { portB, GPIO_PIN_10 | GPIO_PIN_11, /*remapped=*/ true, NULL },
    i2cDsp { i2c2, GPIO_NOPULL },
    tda7439 { i2cDsp, I2C_DSP_ADDRESS },

    // Channels
    btn1 { portB, GPIO_PIN_15, GPIO_PULLUP },
    btn2 { portB, GPIO_PIN_14, GPIO_PULLUP },
    btn3 { portB, GPIO_PIN_13, GPIO_PULLUP },
    btn4 { portB, GPIO_PIN_12, GPIO_PULLUP },
    led1 { portA, GPIO_PIN_12, Drivers::Led::ConnectionType::ANODE },
    led2 { portA, GPIO_PIN_11, Drivers::Led::ConnectionType::ANODE },
    led3 { portA, GPIO_PIN_10, Drivers::Led::ConnectionType::ANODE },
    led4 { portC, GPIO_PIN_9, Drivers::Led::ConnectionType::ANODE },

    // Amp switches
    pinAmpMute { portC, GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL },
    pinAmpEnable { portC, GPIO_PIN_11, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL },
    pinAmpGain0 { portC, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL },
    pinAmpGain1 { portD, GPIO_PIN_2, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL },

    // Encoders
    timer1 { HardwareLayout::Interrupt { TIM1_UP_TIM10_IRQn, 8, 0 } },
    volumeEncoder { timer1, portA, GPIO_PIN_8, portA, GPIO_PIN_9, TIM_ENCODERMODE_TI12, /*filter=*/ 0xA, /*step=*/ 4 },
    timer2 { HardwareLayout::Interrupt { TIM2_IRQn, 8, 0 } },
    bassEncoder { timer2, portA, GPIO_PIN_15, portB, GPIO_PIN_3, TIM_ENCODERMODE_TI12, /*filter=*/ 0xA, /*step=*/ 4 },
    timer3 { HardwareLayout::Interrupt { TIM3_IRQn, 8, 0 } },
    trebleEncoder { timer3, portC, GPIO_PIN_6, portC, GPIO_PIN_7, TIM_ENCODERMODE_TI12, /*filter=*/ 0xA, /*step=*/ 4 },

    // SPI
    spi1 { portA, GPIO_PIN_5, portA, GPIO_PIN_7, portA, GPIO_PIN_6, /*remapped=*/ true, NULL,
             HardwareLayout::Interrupt { SPI1_IRQn, 1, 0 },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream5, DMA_CHANNEL_3,
                                         HardwareLayout::Interrupt { DMA2_Stream5_IRQn, 1, 1 } },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream2, DMA_CHANNEL_3,
                                         HardwareLayout::Interrupt { DMA2_Stream2_IRQn, 1, 2 } }
    },
    spi { spi1, GPIO_NOPULL },
    eepRom { spi, portC, GPIO_PIN_4 },

    // USART logger
    usart1 { portB, GPIO_PIN_6, portB, UNUSED_PIN, /*remapped=*/ true, NULL,
             HardwareLayout::Interrupt { USART1_IRQn, 13, 0 },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream7, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA2_Stream7_IRQn, 14 } },
             HardwareLayout::DmaStream { &dma2, DMA2_Stream2, DMA_CHANNEL_4,
                                         HardwareLayout::Interrupt { DMA2_Stream2_IRQn, 14 } }
    },
    usartLogger { usart1, 115200 }
{
    // External oscillators use system pins
    sysClock.setHSE(&portH, GPIO_PIN_0 | GPIO_PIN_1);
    sysClock.setLSE(&portC, GPIO_PIN_14 | GPIO_PIN_15);
}


void Hardware::abort ()
{
    while(1)
    {
        __NOP();
    }
}


void Hardware::initClock (uint32_t frequency, int APB1, int APB2)
{
    clockParameters.searchBestParameters(frequency);
    clockParameters.APB1 *= APB1;
    clockParameters.APB2 *= APB2;
    sysClock.setSysClockSource(RCC_SYSCLKSOURCE_PLLCLK);
    sysClock.getOscParameters().PLL.PLLState = RCC_PLL_ON;
    sysClock.getOscParameters().PLL.PLLSource = RCC_PLLSOURCE_HSE;
    sysClock.getOscParameters().PLL.PLLM = clockParameters.M;
    sysClock.getOscParameters().PLL.PLLN = clockParameters.N;
    sysClock.getOscParameters().PLL.PLLP = clockParameters.P;
    sysClock.getOscParameters().PLL.PLLQ = clockParameters.Q;
    sysClock.setAHB(RCC_SYSCLK_DIV1, clockParameters.APB1, clockParameters.APB2);
    sysClock.setLatency(FLASH_LATENCY_7);
    sysClock.setRTC();
    sysClock.start();
}


bool Hardware::start()
{
    DeviceStart::Status status;

    // LEDs
    ledBlue.start();
    ledBlue.turnOff();

    // Logger
    usartLogger.initInstance();
    USART_DEBUG("--------------------------------------------------------" << UsartLogger::ENDL);
    USART_DEBUG("MCU frequency: " << SystemClock::getInstance()->getMcuFreq() << UsartLogger::ENDL);
    clockParameters.print();

    // For RTC, it is necessary to reset the state since it will not be
    // automatically reset after MCU programming.
    rtc.stop();
    do
    {
        Rtc::Start::Status status = rtc.start(8 * 2047 + 7, RTC_WAKEUPCLOCK_RTCCLK_DIV2);
        USART_DEBUG("RTC status: " << Rtc::Start::asString(status) << " (" << rtc.getHalStatus() << ")" << UsartLogger::ENDL);
    }
    while (rtc.getHalStatus() != HAL_OK);

    // Channels
    btn1.start();
    btn2.start();
    btn3.start();
    btn4.start();
    led1.start();
    led2.start();
    led3.start();
    led4.start();

    // Amp switches
    pinAmpMute.setLow();
    pinAmpMute.start();
    pinAmpEnable.setHigh();
    pinAmpEnable.start();
    pinAmpGain0.setHigh();
    pinAmpGain0.start();
    pinAmpGain1.setHigh();
    pinAmpGain1.start();

    // start I2C
    i2cDsp.stop();
    status = i2cDsp.start(I2C_SPEED, I2C_MASTER_ADDRESS);
    USART_DEBUG("I2C status: " << DeviceStart::asString(status) << " (" << i2cDsp.getHalStatus() << ")" << UsartLogger::ENDL);
    if (status != DeviceStart::Status::OK)
    {
        return false;
    }
    
    // Encoders
    status = volumeEncoder.start(TIM_CHANNEL_1);
    USART_DEBUG("TIM(volume) status: " << DeviceStart::asString(status) << " (" << volumeEncoder.getHalStatus() << ")" << UsartLogger::ENDL);
    status = bassEncoder.start(TIM_CHANNEL_2);
    USART_DEBUG("TIM(bass) status: " << DeviceStart::asString(status) << " (" << bassEncoder.getHalStatus() << ")" << UsartLogger::ENDL);
    status = trebleEncoder.start(TIM_CHANNEL_3);
    USART_DEBUG("TIM(treble) status: " << DeviceStart::asString(status) << " (" << trebleEncoder.getHalStatus() << ")" << UsartLogger::ENDL);

    // SPI
    status = spi.start(SPI_DIRECTION_2LINES, SPI_BAUDRATEPRESCALER_256, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE);
    USART_DEBUG("SPI" << spi.getId() << " status: " << DeviceStart::asString(status) << " (" << spi.getHalStatus() << ")" << UsartLogger::ENDL);
    if (status != DeviceStart::Status::OK)
    {
        return false;
    }
    eepRom.start();

    USART_DEBUG("--------------------------------------------------------" << UsartLogger::ENDL);
    return true;
}


void Hardware::stop ()
{
    // Stop all devices
    eepRom.stop();
    spi.stop();
    volumeEncoder.stop();
    bassEncoder.stop();
    trebleEncoder.stop();
    i2cDsp.stop();
    btn1.stop();
    btn2.stop();
    btn3.stop();
    btn4.stop();
    led1.stop();
    led2.stop();
    led3.stop();
    led4.stop();
    pinAmpGain0.stop();
    pinAmpGain1.stop();
    pinAmpEnable.stop();
    pinAmpMute.stop();
    rtc.stop();
    ledBlue.stop();

    // Log resource occupations after all devices (except USART1 for logging, HSE, LSE) are stopped.
    // Desired: one at portB and DMA2 (USART1), one for portC (LSE), one for portH (HSE)
    printResourceOccupation();
    usartLogger.clearInstance();

    sysClock.stop();
}


void Hardware::printResourceOccupation ()
{
    USART_DEBUG("Resource occupations: " << UsartLogger::ENDL
                << UsartLogger::TAB << "portA=" << portA.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portB=" << portB.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portC=" << portC.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portD=" << portD.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "portH=" << portH.getObjectsCount() << UsartLogger::ENDL
                << UsartLogger::TAB << "dma2=" << dma2.getObjectsCount() << UsartLogger::ENDL);
}


/************************************************************************
 * External interrupts
 ************************************************************************/
extern MyApplication * appPtr;

extern "C"
{
    // Errors
    void HardFault_Handler (void)
    {
        appPtr->abort();
    }

    void MemManage_Handler (void)
    {
        appPtr->abort();
    }

    void BusFault_Handler (void)
    {
        appPtr->abort();
    }

    void UsageFault_Handler (void)
    {
        appPtr->abort();
    }

    // System
    void SysTick_Handler (void)
    {
        HAL_IncTick();
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->onMilliSecondInterrupt();
        }
    }

    void RTC_WKUP_IRQHandler ()
    {
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->processInterrupt();
        }
    }

    void HAL_RTCEx_WakeUpTimerEventCallback (RTC_HandleTypeDef * /*hrtc*/)
    {
        if (Rtc::getInstance() != NULL)
        {
            Rtc::getInstance()->processEventCallback();
            appPtr->onRtcSecond();
        }
    }

    // UARTs: uses both USART and DMA interrupts
    void DMA2_Stream7_IRQHandler (void)
    {
        appPtr->getLoggerUsart().processDmaTxInterrupt();
    }

    void USART1_IRQHandler (void)
    {
        appPtr->getLoggerUsart().processInterrupt();
    }

    void HAL_UART_TxCpltCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::TX_CMPL);
        }
    }

    void HAL_UART_RxCpltCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::RX_CMPL);
        }
    }

    void HAL_UART_ErrorCallback (UART_HandleTypeDef * channel)
    {
        if (channel->Instance == USART1)
        {
            appPtr->getLoggerUsart().processCallback(SharedDevice::State::ERROR);
        }
    }
}
