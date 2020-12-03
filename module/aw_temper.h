/*
 * aw_temper.h
 *
 *  Created on: 2020��11��6��
 *      Author: admin
 */

#ifndef MODULE_AW_TEMPER_H_
#define MODULE_AW_TEMPER_H_

#include "aw_config.h"
#include "util_def.h"
#define AW_TEMPER_SCALE 100
s8 aw_temper_init();
s8 aw_temper_test();
s8 aw_temper_close();
s8 aw_temper_read(s16* val);
void aw_temper_set_bias(s16 val);
s16 aw_temper_get_bias();
#endif /* MODULE_AW_TEMPER_H_ */
