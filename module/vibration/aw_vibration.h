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
void aw_vibration_process(s16 tempt);
void aw_fft_test();
s16* aw_vibration_calibration();

#endif /* MODULE_VIBRATION_AW_VIBRATION_H_ */
