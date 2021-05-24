/*******************************************************************************
 * stm32async: Asynchronous I/O C++ library for STM32
 * *****************************************************************************
 * Copyright (C) 2018 Mikhail Kulesh, Denis Makarov
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

#include "Timer.h"

#ifdef HAL_TIM_MODULE_ENABLED

using namespace Stm32async;

/************************************************************************
 * Class BaseTimer
 ************************************************************************/

BaseTimer::BaseTimer (const HardwareLayout::Timer & _device) :
    device { _device },
    halStatus { HAL_ERROR }
{
    parameters.Instance = device.getInstance();
}

DeviceStart::Status BaseTimer::startCounter (uint32_t counterMode, uint32_t prescaler,
                                             uint32_t period,
                                             uint32_t clockDivision/* = TIM_CLOCKDIVISION_DIV1*/)
{
    device.enableClock();
    parameters.Init.CounterMode = counterMode;
    parameters.Init.Prescaler = prescaler;
    parameters.Init.Period = period;
    parameters.Init.ClockDivision = clockDivision;
    __HAL_TIM_SET_COUNTER(&parameters, 0);

    halStatus = HAL_TIM_Base_Init(&parameters);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::DEVICE_INIT_ERROR;
    }

    halStatus = HAL_TIM_Base_Start(&parameters);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::TIMER_START_ERROR;
    }

    return DeviceStart::Status::OK;
}

void BaseTimer::stopCounter ()
{
    HAL_TIM_Base_Stop(&parameters);
    HAL_TIM_Base_DeInit(&parameters);
    device.disableClock();
}

/************************************************************************
 * Class InterruptTimer
 ************************************************************************/

InterruptTimer::InterruptTimer (const HardwareLayout::Timer & _device) :
    BaseTimer { _device }
{
    // empty
}

DeviceStart::Status InterruptTimer::start (uint32_t counterMode, uint32_t prescaler,
                                           uint32_t period,
                                           uint32_t clockDivision/* = TIM_CLOCKDIVISION_DIV1*/,
                                           uint32_t repetitionCounter/* = 1*/)
{
    device.enableClock();
    parameters.Init.CounterMode = counterMode;
    parameters.Init.Prescaler = prescaler;
    parameters.Init.Period = period;
    parameters.Init.ClockDivision = clockDivision;
    parameters.Init.RepetitionCounter = repetitionCounter;
    __HAL_TIM_SET_COUNTER(&parameters, 0);

    halStatus = HAL_TIM_Base_Init(&parameters);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::DEVICE_INIT_ERROR;
    }

    halStatus = HAL_TIM_Base_Start_IT(&parameters);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::TIMER_START_ERROR;
    }

    device.timerIrq.enable();
    return DeviceStart::Status::OK;
}

void InterruptTimer::stop ()
{
    device.timerIrq.disable();
    HAL_TIM_Base_Stop_IT(&parameters);
    HAL_TIM_Base_DeInit(&parameters);
    device.disableClock();
}


/************************************************************************
 * Class EncoderTimer
 ************************************************************************/
EncoderTimer::EncoderTimer(const HardwareLayout::Timer & _device,
                           const HardwareLayout::Port & _channelAPort, uint32_t _channelAPin,
                           const HardwareLayout::Port & _channelBPort, uint32_t _channelBPin,
                           uint32_t _encoderMode, uint32_t _filter, uint32_t _prescale) :
    BaseTimer { _device },
    channelA { _channelAPort, _channelAPin, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH },
    channelB { _channelBPort, _channelBPin, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_HIGH },
    encoderVal { 0 }
{
    _device.remapPins(channelA.getParameters());
    _device.remapPins(channelB.getParameters());

    parameters.Init.Period = 0xFFFF;
    parameters.Init.CounterMode = TIM_COUNTERMODE_UP;
    parameters.Init.Prescaler = _prescale;
    parameters.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    encoder.EncoderMode = _encoderMode;
    encoder.IC1Filter = _filter;
    encoder.IC1Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC1Prescaler = TIM_ICPSC_DIV1;
    encoder.IC2Filter = _filter;
    encoder.IC2Polarity = TIM_ICPOLARITY_RISING;
    encoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    encoder.IC2Prescaler = TIM_ICPSC_DIV1;
}

DeviceStart::Status EncoderTimer::start (uint32_t _channel)
{
    channelA.start();
    channelB.start();

    device.enableClock();
    halStatus = HAL_TIM_Encoder_Init(&parameters, &encoder);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::DEVICE_INIT_ERROR;
    }

    halStatus = HAL_TIM_Encoder_Start(&parameters, _channel);
    if (halStatus != HAL_OK)
    {
        return DeviceStart::Status::TIMER_START_ERROR;
    }

    encoderVal = getValue();
    return DeviceStart::Status::OK;
}

void EncoderTimer::stop ()
{
    device.disableClock();
    channelA.stop();
    channelB.stop();
}

#endif
