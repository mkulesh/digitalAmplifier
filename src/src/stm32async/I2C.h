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

#ifndef STM32ASYNC_I2C_H_
#define STM32ASYNC_I2C_H_

#include "HardwareLayout/I2C.h"

#ifdef HAL_I2C_MODULE_ENABLED

#include "IODevice.h"
#include "SharedDevice.h"

namespace Stm32async
{

/**
 * @brief Base I2C class that holds the I2C parameters and implements the communication in
 *        a blocking mode.
 */
class BaseI2C : public IODevice<HardwareLayout::I2C, I2C_HandleTypeDef, 1>
{
public:

    /**
     * @brief Default constructor.
     */
    BaseI2C (const HardwareLayout::I2C & _device, uint32_t _pull = GPIO_PULLUP);

    /**
     * @brief Open transmission session with given parameters.
     */
    DeviceStart::Status start (uint32_t clockSpeed, uint32_t ownAddress);

    /**
     * @brief Close the transmission session.
     */
    void stop ();

    /**
     * @brief Send an amount of data in blocking mode from master to slave.
     */
    inline HAL_StatusTypeDef masterTransmitBlocking (uint16_t address, uint8_t * buffer, uint16_t n, uint32_t timeout = __UINT32_MAX__)
    {
        halStatus = HAL_I2C_Master_Transmit(&parameters, address, buffer, n, timeout);
        return halStatus;
    }

    /**
     * @brief Receive an amount of data in blocking mode from master to slave.
     */
    inline HAL_StatusTypeDef slaveReceiveBlocking (uint8_t * buffer, uint16_t n, uint32_t timeout = __UINT32_MAX__)
    {
        halStatus = HAL_I2C_Slave_Receive(&parameters, buffer, n, timeout);
        return halStatus;
    }
};

} // end namespace

#endif
#endif
