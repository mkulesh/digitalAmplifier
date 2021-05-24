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

#ifndef HARDWARE_LAYOUT_I2C2_H_
#define HARDWARE_LAYOUT_I2C2_H_

#include "I2C.h"

#ifdef HAL_I2C_MODULE_ENABLED
#ifdef I2C2

namespace Stm32async
{
namespace HardwareLayout
{

/**
 * @brief Wrapper class for I2C module that is hosted on I2C2
 */
class I2c2 : public HardwareLayout::I2C
{
public:
    I2c2 (HardwareLayout::Port & _port, uint32_t _pins,
                   bool _remapped, HardwareLayout::Afio * _afio) :
        I2C { 2, I2C2, _port, _pins, _remapped, _afio }
    {
        // empty
    }

    virtual void enableClock () const
    {
        // https://electronics.stackexchange.com/questions/272427/stm32-busy-flag-is-set-after-i2c-initialization
        // Use reset to fix a bug: bus is considered permanently busy.
        __HAL_RCC_I2C2_FORCE_RESET();
        __HAL_RCC_I2C2_CLK_ENABLE();
        __HAL_RCC_I2C2_RELEASE_RESET();
    }

    virtual void disableClock () const
    {
        __HAL_RCC_I2C2_CLK_DISABLE();
    }

    virtual void remapPins (GPIO_InitTypeDef & gpioParameters) const
    {
        if (remapped)
        {
            #if defined(STM32F4)
                gpioParameters.Alternate = GPIO_AF4_I2C2;
            #endif
        }
    }

    virtual void unremapPins (GPIO_InitTypeDef & gpioParameters) const
    {
        if (remapped)
        {
            UNUSED(gpioParameters);
        }
    }
};

} // end of namespace HardwareLayout
} // end of namespace Stm32async

#endif
#endif
#endif
