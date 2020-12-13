/*
 * aw_flash.h
 *
 *  Created on: 2020��12��7��
 *      Author: xiaoshi
 */

#ifndef MODULE_AW_FLASH_H_
#define MODULE_AW_FLASH_H_

#include"util_def.h"
typedef struct _aw_flash_data_t {
   u16 report_time;//�ϱ�����
   s16 temp_bias;//�¶Ȳ���
   char*host;    //��������ַ
} Aw_Flash_Data_t;
s8 aw_flash_init();
s8 aw_flash_write(Aw_Flash_Data_t*config);
s8 aw_flash_read(Aw_Flash_Data_t*config);
s8 aw_flash_erase();
#endif /* MODULE_AW_FLASH_H_ */
