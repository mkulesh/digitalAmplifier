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

#include "EepRom_25AA040A.h"
#include "../UsartLogger.h"

using namespace Stm32async::Drivers;

#define USART_DEBUG_MODULE "ROM: "

/************************************************************************
 * Class EepRom_25AA040A
 ************************************************************************/
EepRom_25AA040A::EepRom_25AA040A(BaseSpi & _spi, const HardwareLayout::Port & _csPort, uint32_t _csPin):
    spi { _spi },
    csPin { _csPort, _csPin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL }
{
    // empty
}


Stm32async::DeviceStart::Status EepRom_25AA040A::start ()
{
    csPin.start();
    csPin.setHigh();
    return Stm32async::DeviceStart::Status::OK;
}


void EepRom_25AA040A::stop ()
{
    csPin.stop();
}


uint8_t EepRom_25AA040A::getMode(bool showLog)
{
    uint8_t txBuffer[2] = { 0b00000101, 0 };
    uint8_t rxBuffer[2] = { 0, 0 };
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 1);
    spi.receiveBlocking(&rxBuffer[0], 1);
    csPin.setHigh();
    if (showLog)
    {
        USART_DEBUG("Mode = " << getMode() << UsartLogger::ENDL);
    }
    return rxBuffer[0];
}


uint8_t EepRom_25AA040A::readByte(uint8_t addr)
{
    uint8_t txBuffer[3] = { 0b00000011, addr, 0 };
    uint8_t rxBuffer[3] = { 0, 0, 0 };
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 2);
    spi.receiveBlocking(&rxBuffer[0], 2);
    csPin.setHigh();
    uint8_t result = rxBuffer[0];
    return result;
}


void EepRom_25AA040A::readPage(uint8_t addr, uint8_t *val, size_t count)
{
    uint8_t txBuffer[3] = { 0b00000011, addr, 0 };
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 2);
    spi.receiveBlocking(val, count);
    csPin.setHigh();
}


void EepRom_25AA040A::enableWrite()
{
    uint8_t txBuffer[2] = { 0b00000110, 0 };
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 1);
    csPin.setHigh();
}


void EepRom_25AA040A::writeByte(uint8_t addr, uint8_t val)
{
    uint8_t write[4] = { 0b00000010, addr, val, 0 };
    csPin.setLow();
    spi.transmitBlocking(&write[0], 3);
    csPin.setHigh();
}


void EepRom_25AA040A::writePage(uint8_t addr, uint8_t *val, size_t count)
{
    size_t const maxCount = 18;
    uint8_t txBuffer[maxCount];
    txBuffer[0] = 0b00000010;
    txBuffer[1] = addr;
    for(size_t k = 0; k < std::min(count, maxCount); k++)
    {
        txBuffer[2 + k] = val[k];
    }
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 2 + count);
    csPin.setHigh();
}


void EepRom_25AA040A::disableWrite()
{
    uint8_t txBuffer[2] = { 0b00000100, 0 };
    csPin.setLow();
    spi.transmitBlocking(&txBuffer[0], 1);
    csPin.setHigh();
}

/************************************************************************
 * Class EepRomByte
 ************************************************************************/
EepRomByte::EepRomByte (EepRom_25AA040A & _eepRom, const char * _name, const uint8_t _address):
    eepRom { _eepRom },
    name { _name },
    address { _address },
    data { 0 },
    refTime { 0 },
    delay { 0 }
{

}

uint8_t EepRomByte::read ()
{
    data = eepRom.readByte(address);
    USART_DEBUG(name << ": read[" << address << "] -> " << data << UsartLogger::ENDL);
    return data;
}


void EepRomByte::write (uint8_t _data)
{
    data = _data;
    USART_DEBUG(name << ": " << data << " -> write[" << address << "]" << UsartLogger::ENDL);
    eepRom.enableWrite();
    eepRom.writeByte(address, data);
    eepRom.disableWrite();
}


void EepRomByte::delayedWrite (uint8_t _data, uint32_t _delay)
{
    data = _data;
    refTime = HAL_GetTick();
    delay = _delay;
}


void EepRomByte::periodic ()
{
    if (delay > 0 && HAL_GetTick() > refTime + delay)
    {
        write(data);
        refTime = delay = 0;
    }
}
