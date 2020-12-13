/*
 * aw_flash.c
 *
 *  Created on: 2020Äê12ÔÂ7ÈÕ
 *      Author: xiaoshi
 */

#ifndef MODULE_AW_FLASH_C_
#define MODULE_AW_FLASH_C_
#include"aw_flash.h"
#include"nvs_driver.h"
#include<string.h>
static u8 _data_buf[30] = { 0 };

static void _to_buf(Aw_Flash_Data_t *config)
{
    memset(_data_buf,0,sizeof(_data_buf));
    _data_buf[0] = config->report_time & 0xFF;
    _data_buf[1] = (config->report_time >> 8) & 0xFF;
    _data_buf[2] = config->temp_bias & 0xFF;
    _data_buf[3] = (config->temp_bias >> 8) & 0xFF;
    memcpy(&_data_buf[4], (u8*) config->host, strlen(config->host));
}
static void _to_config(Aw_Flash_Data_t *config)
{
    config->report_time = _data_buf[1] << 8 | _data_buf[0];
    config->temp_bias = (s16) (_data_buf[3] << 8 | _data_buf[2]);
    config->host = (char*) (&_data_buf[4]);
}
s8 aw_flash_init()
{
    return nvs_init();
}
s8 aw_flash_write(Aw_Flash_Data_t *config)
{
    _to_buf(config);
    return nvs_write(_data_buf, sizeof(_data_buf));
}
s8 aw_flash_read(Aw_Flash_Data_t *config)
{
    s8 res = 0;
    memset(_data_buf,0,sizeof(_data_buf));
    res=nvs_read(_data_buf, sizeof(_data_buf));
    _to_config(config);
    return res;
}
s8 aw_flash_erase(){
   return nvs_erase();
}
#endif /* MODULE_AW_FLASH_C_ */
