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

#include "I2C.h"

#ifdef HAL_I2C_MODULE_ENABLED

using namespace Stm32async;

/************************************************************************
 * Class AsyncI2C
 ************************************************************************/

BaseI2C::BaseI2C (const HardwareLayout::I2C & _device, uint32_t _pull):
    IODevice { _device, {
              IOPort { _device.pins.port, _device.pins.pins, GPIO_MODE_AF_OD,
                  _pull, GPIO_SPEED_FREQ_HIGH } } }
{
    parameters.Instance = device.getInstance();
    parameters.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    parameters.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    parameters.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    parameters.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    parameters.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
}


DeviceStart::Status BaseI2C::start (uint32_t clockSpeed, uint32_t ownAddress)
{
    __HAL_I2C_ENABLE(&parameters);

    parameters.Init.ClockSpeed  = clockSpeed;
    parameters.Init.OwnAddress1 = ownAddress;

    device.enableClock();
    IODevice::enablePorts();

    halStatus = HAL_I2C_Init(&parameters);

    if (halStatus != HAL_OK)
    {
        return DeviceStart::DEVICE_INIT_ERROR;
    }

    return DeviceStart::OK;
}


void BaseI2C::stop ()
{
    HAL_I2C_DeInit(&parameters);
    device.disableClock();
    IODevice::disablePorts();
    __HAL_I2C_DISABLE(&parameters);
}

#endif
