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

#ifndef DRIVERS_EepRom_25AA040A_H_
#define DRIVERS_EepRom_25AA040A_H_

#include "../Spi.h"

namespace Stm32async
{
namespace Drivers
{

/** 
 * @brief Driver for the EEPROM 25AA040A series by Microchip.
 *        This driver uses SPI connection method.
 */
class EepRom_25AA040A
{
public:
    
    /** 
     * @brief Default constructor
     */ 
    EepRom_25AA040A (BaseSpi & _spi, const HardwareLayout::Port & _csPort, uint32_t _csPin);
    
    DeviceStart::Status start ();
    void stop ();

    uint8_t getMode (bool showLog = false);
    uint8_t readByte (uint8_t addr);
    void readPage (uint8_t addr, uint8_t *val, size_t count);
    void enableWrite ();
    void writeByte (uint8_t addr, uint8_t val);
    void writePage (uint8_t addr, uint8_t *val, size_t count);
    void disableWrite ();

private:

    BaseSpi & spi;
    IOPort csPin;
};


class EepRomByte
{
public:

    EepRomByte (EepRom_25AA040A & _eepRom, const char * _name, const uint8_t _address);
    uint8_t read ();
    void write (uint8_t _data);
    void delayedWrite (uint8_t _data, uint32_t _delay);
    void periodic ();

private:

    EepRom_25AA040A & eepRom;
    const char * name;
    uint8_t address, data;
    uint32_t refTime, delay;
};


} // end of namespace Drivers
} // end of namespace Stm32async

#endif
