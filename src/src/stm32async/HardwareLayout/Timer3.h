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

#ifndef HARDWARE_LAYOUT_TIM3_H_
#define HARDWARE_LAYOUT_TIM3_H_

#include "Timer.h"

#ifdef HAL_TIM_MODULE_ENABLED
#ifdef TIM3

namespace Stm32async
{
namespace HardwareLayout
{

/**
 * @brief Wrapper class for TIM3 module.
 *
 * Implementation shall provide wrappers for TIM3 clock enable/disable macros
 */
class Timer3 : public HardwareLayout::Timer
{
public:
    Timer3 (Interrupt && _timerIrq) :
        Timer { 3, TIM3, std::move(_timerIrq) }
    {
        // empty
    }

    virtual void enableClock () const
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }

    virtual void disableClock () const
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }

    virtual void remapPins (GPIO_InitTypeDef & gpioParameters) const
    {
        gpioParameters.Alternate = GPIO_AF2_TIM3;
    }
};

} // end of namespace HardwareLayout
} // end of namespace Stm32async

#endif
#endif
#endif
