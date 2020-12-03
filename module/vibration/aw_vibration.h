/*
 * aw_fft.h
 *
 *  Created on: 2020Äê10ÔÂ19ÈÕ
 *      Author: admin
 */

#ifndef MODULE_VIBRATION_AW_VIBRATION_H_
#define MODULE_VIBRATION_AW_VIBRATION_H_
#include <vibration/DSPLib.h>

#include "util_def.h"
void aw_vibration_init();
void aw_vibration_process(E_xyz_axis_t axis, s16 tempt, u16 bat);
void aw_fft_test();
s16* aw_vibration_calibration();
s16* aw_vibration_get_bias();
#endif /* MODULE_VIBRATION_AW_VIBRATION_H_ */
