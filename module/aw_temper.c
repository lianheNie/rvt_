/*
 * aw_temper.c
 *
 *  Created on: 2020Äê11ÔÂ6ÈÕ
 *      Author: admin
 */

#include "aw_temper.h"

#include "ads1120.h"
#include "util_delay.h"
static s16 _temp_bias = 0;
s8 aw_temper_init() {
  s8 res = 0;
  res = ads1120_init();
  return res;
}
s8 aw_temper_read(s16* val) {
  s8 res = 0;
  res = ads1120_set_internal_source(val);
  if (res == -1) {
    return -1;
  }

  res = ads1120_set_external_source();
  res = ads1120_read_temp(val);
  *val = *val - _temp_bias;
  return res;
}

void aw_temper_set_bias(s16 val) { _temp_bias = val; }
s16 aw_temper_get_bias() { return _temp_bias; }
s8 aw_temper_test() {
  s8 res = 0;
  res = aw_temper_init();
  u8 cnt = 0;
  s16 temp = 0;
  while (cnt++ < 10000) {
    aw_delay_ms(1000);
    res = aw_temper_read(&temp);
  }
  return res;
}
s8 aw_temper_close() {
  s8 res = 0;
  res = ads1120_close();
  return res;
}
