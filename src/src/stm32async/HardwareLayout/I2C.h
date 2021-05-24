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

#ifndef HARDWARE_LAYOUT_I2C_H_
#define HARDWARE_LAYOUT_I2C_H_

#include "HardwareLayout.h"

#ifdef HAL_I2C_MODULE_ENABLED

namespace Stm32async
{
namespace HardwareLayout
{

/**
 * @brief Configuration of I2C interface.
 */
class I2C : public HalAfioDevice
{
    DECLARE_INSTANCE(I2C_TypeDef)

public:

    /**
     * @brief Pins from corresponding ports
     */
    Pins pins;

    /**
     * @brief Standard initialization constructor.
     */
    I2C (size_t _id,  I2C_TypeDef *_instance,
                  Port & _port, uint32_t _pins,
                  bool _remapped, Afio * _afio) :
        HalAfioDevice { _id, _remapped, _afio },
        instance { _instance },
        pins { _port, _pins }
    {
        // empty
    }
};

} // end of namespace HardwareLayout
} // end of namespace Stm32async

#endif
#endif
