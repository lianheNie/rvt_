/*
 * aw_ammeter.h
 *������
 *  Created on: 2020��9��27��
 *      Author: admin
 */

#ifndef MODULE_AW_AMMETER_H_
#define MODULE_AW_AMMETER_H_
#include "util_def.h"
s8 aw_ammeter_init();
s8 aw_ammeter_get(u16* cur_4v, u16* cur_5v, u16* cur_12v);
#endif /* MODULE_AW_AMMETER_H_ */
