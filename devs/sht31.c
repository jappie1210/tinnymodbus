/*********************************************************************
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2018
 *
 * Balint Cristian <cristian dot balint at gmail dot com>
 *
 * TinnyModbus
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *   * Neither the name of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/

/*

  sht21.c (SHT21 temperature and humidity sensor)

*/

#include "sht31.h"
#include <util/delay.h>
#include "crc8.h"
#include "softi2c.h"






// stap 1, haal de data uit de sht31ReadValue

int32_t sht31ReadValue(uint8_t TYPE)
  {

    uint8_t buffer[6];
    uint32_t V;

    i2c_init();

    i2c_start(SHT31_ADDR<<1);
    //send command
    i2c_write(SHT31_MEAS_HIGHREP);
    //wait for measurements to complete
    _delay_ms(20);
    //read buffers
    i2c_rep_start((SHT31_ADDR<<1)|0x1);

    buffer[0] = i2c_read(0); i2c_read(0); //temp msb
    buffer[1] = i2c_read(0); i2c_read(0); //temp lsb
    buffer[2] = i2c_read(0); i2c_read(0); //tenp crc
    buffer[3] = i2c_read(0); i2c_read(0); //hum msb
    buffer[4] = i2c_read(0); i2c_read(0); //hum lsb
    buffer[5] = i2c_read(0); i2c_read(1); //hum crc

    i2c_stop();


    switch ( TYPE )
    {
        case SHT31_TEMP:;
          // raw temperature
          uint16_t valuet = ((uint16_t)buffer[0]) << 8;
          valuet |= buffer[1];
          //from specsheet
          // t[c]  = -45 + 175 * Sr / (2^16 -1)
          // 100*t = 17500 / 2 ^ 16 * Sr - 4500
          // 100*t = (17500/4 * Sr) / 2 ^ 14  - 4500
          V = ((4375 * (uint32_t)valuet) >> 14) - 4500;
          break;
        case SHT31_HUMI:;
          // raw humidity
          uint16_t valueh = ((uint16_t)buffer[3]) << 8;
          valueh |= buffer[4];
          //Rh = 100 * value / (2^16-1)
          // 100*Rh = 10000 * value / (2^16-1)
          //100*Rh = (10000/16) * value / (2^12)

          V = (625 * (uint32_t)valueh) >> 12;
          break;
    } // end switch

  return V;



}








/*
 * read sensor serial number
 */
void sht31ReadSerial( uint8_t *sn )
{
  i2c_init();

  // low
  i2c_start(0x80);

  i2c_write(0xFA);
  i2c_write(0x0F);

  i2c_rep_start(0x81);

  sn[5] = i2c_read(0); i2c_read(0);
  sn[4] = i2c_read(0); i2c_read(0);
  sn[3] = i2c_read(0); i2c_read(0);
  sn[2] = i2c_read(0); i2c_read(1);

  i2c_stop();

  // high
  i2c_start(0x80);

  i2c_write(0xFC);
  i2c_write(0xC9);

  i2c_rep_start(0x81);

  sn[1] = i2c_read(0); sn[0] = i2c_read(0); i2c_read(0);
  sn[7] = i2c_read(0); sn[6] = i2c_read(0); i2c_read(1);

  i2c_stop();

}
