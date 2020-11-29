/*
 * imu.c
 *
 *  Created on: 2020年8月1日
 *      Author: admin
 */
#include "bmi160.h"
#include "util_timer.h"
#include "util_printf.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <stdio.h>
#include "Board.h"
#include "driver_comm.h"
#include "bmi160_defs.h"
typedef bmi160_sensor_data_t imu_sensor_data_t;
static Clock_Struct          _clockS;
#define GYRO_MIN       20
#define GYRO_RATIO     16.4  //=>BMI160_GYRO_RANGE_2000_DEG_SEC
#define GYRO_RATIO_INV 1 / GYRO_RATIO

#define Period   20               // 20ms 50
#define Duration Period           // 20ms
#define Dt       Period / 1000.0  //

#define TimeOut_5_Deg (int)(5.0 / (Dt))   // 5s
#define TimeOut_Max   (int)(30.0 / (Dt))  // 30s
#define Open_Deg      10                  //开门报警角度
#define Exit_Deg      5                   //敲门振动角度
static Semaphore_Struct     _timeoutSemS; /* not static so you can see in ROV */
static Semaphore_Struct     _intSemS;
static Semaphore_Handle     _timeoutSem;
static Semaphore_Handle     _intSem;
static Semaphore_Struct     uartSemS;
static s8                   imu_sem_init();
static uint32_t             _timeCnt  = 0;
static u8                   _int_flag = 1;
static bmi160_sensor_data_t _gXYZ;
static float                _ang, _angLast, _angAbs;
static s8                   _rslt       = 0;
static AwCallBack_float_t   _imuAlarmCB = NULL;

static float getGyroAngle(const bmi160_sensor_data_t *gXYZ, const float dt, float *ang) {
    float aSum = 0, gX, gY, gZ;
    gX         = gXYZ->x;
    gY         = gXYZ->y;
    gZ         = gXYZ->z;
    if ((gX < GYRO_MIN) && (gX > -GYRO_MIN)) gX = 0;
    if ((gY < GYRO_MIN) && (gY > -GYRO_MIN)) gY = 0;
    if ((gZ < GYRO_MIN) && (gZ > -GYRO_MIN)) gZ = 0;
    gX   = -(gX)*GYRO_RATIO_INV * dt;  // x轴角微分
    gY   = -(gY)*GYRO_RATIO_INV * dt;  // y轴角微分
    gZ   = -(gZ)*GYRO_RATIO_INV * dt;  // z轴角微分
    aSum = gX + gY + gZ;
    *ang = *ang + aSum;
    return aSum;
}

static float imu_get_angle(const bmi160_sensor_data_t *gXYZ, float *ang) {
    return getGyroAngle(gXYZ, Dt, ang);
}

static s8 timer_init(AwCallback_u32_t timeOutCb) {
    Timer_construct(&_clockS, timeOutCb, Duration, Period, false, 0);
    return AW_OK;
}

static s8 _imu_init_(AwCallback_u32_t timeOutCb, AwCallBack_u8_t int1Cb) {
    s8 rslt = AW_OK;
    rslt    = imu_sem_init();
    rslt    = timer_init(timeOutCb);
    rslt    = bmi160_init(int1Cb);
    return rslt;
}

static void timeOutCB(uint32_t arg) {
    _timeCnt++;
    Semaphore_post(_timeoutSem);
}

static void int1CB(u8 arg) {
    if (_int_flag == 1) {
        Semaphore_post(_intSem);
    }
}

static s8 _imu_init(AwCallBack_float_t imuAlarmCb) {
    if (imuAlarmCb == NULL) {
        return AW_NULL_ERR;
    }
    _imuAlarmCB = imuAlarmCb;
    return _imu_init_(&timeOutCB, &int1CB);
}

static void imu_timer_stop() {
    Timer_stop(&_clockS);  //关定时器
}

static void imu_timer_start() {
    Timer_start(&_clockS);  //开定时器
}

s8 imu_read_accel(imu_sensor_data_t *accel) { return bmi160_read_accel(accel); }

s8 imu_read_gyro(imu_sensor_data_t *gyro) { return bmi160_read_gyro(gyro); }

static s8 imu_gyro_enable(u8 enable) { return bmi160_gyro_enable(enable); }

static s8 imu_int_enable(u8 enable) { return bmi160_acc_any_motion_int_enable(enable); }

static s8 _imu_close() {
    imu_int_enable(AW_FALSE);
    imu_gyro_enable(AW_FALSE);
    return 0;
}
static bool _imu_enable = true;
s8          aw_imu_close() {
    _imu_enable = false;
    return 0;
}

static s8 imu_sem_init() {
    Semaphore_Params semParams;
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&_timeoutSemS, 0, &semParams);
    _timeoutSem = Semaphore_handle(&_timeoutSemS);
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&_intSemS, 0, &semParams);
    _intSem = Semaphore_handle(&_intSemS);
    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&uartSemS, 0, &semParams);
    return AW_OK;
}

static void _imu_process() {
    while (1) {
        Semaphore_pend(_intSem, BIOS_WAIT_FOREVER);
        if (false == _imu_enable) {
            _imu_close();
            break;
        }
        _int_flag = 0;
        imu_timer_start();                 //开定时器
        _rslt = imu_int_enable(AW_FALSE);  //关中断
        if (_rslt != 0) {
            util_printf("imu int disable err=%d\r\n", _rslt);
        }
        _rslt = imu_gyro_enable(AW_TRUE);
        if (_rslt != 0) {
            util_printf("imu gryo enable err=%d\r\n", _rslt);
        }
        while (_timeCnt < TimeOut_Max) {
            Semaphore_pend(_timeoutSem, BIOS_WAIT_FOREVER);
            imu_read_gyro(&_gXYZ);
            imu_get_angle(&_gXYZ, &_ang);
            _angAbs = _ang;
            if (_angAbs < 0) {
                _angAbs = -_angAbs;
            }
            if (_timeCnt > TimeOut_5_Deg) {
                if (_angAbs < Exit_Deg) {
                    break;
                }
            }
            if ((_ang * _angLast) > 0.00001) {
                break;
            }
            if (_angAbs > Open_Deg) {
                if ((_ang * _angLast) < 0.00001) {
                    _angLast = _ang;
                    if (_imuAlarmCB != NULL) {
                        _imuAlarmCB(_ang);
                    }
                    break;
                }
            }
        }
        _ang    = 0;
        _angAbs = 0;
        imu_timer_stop();  //关定时器
        _timeCnt  = 0;
        _int_flag = 1;
        _rslt     = imu_int_enable(AW_TRUE);  //开中断
        if (_rslt != 0) {
            util_printf("imu int enable err=%d\r\n", _rslt);
        }
        _rslt = imu_gyro_enable(AW_FALSE);
        if (_rslt != 0) {
            util_printf("imu gryo disable err=%d\r\n", _rslt);
        }
    }
}
extern void imuAlarmCB(float ang);
extern void imuAlarmOldCB(float ang);
static Void _imuThread(UArg arg0, UArg arg1) {
    _imu_init(imuAlarmOldCB);
    _imu_process();
}
#include <ti/sysbios/knl/Task.h>
#define _TASK_STACK_SIZE 512
static Task_Struct _task_struct;
static Char        _task_stack[_TASK_STACK_SIZE];
void               aw_imu_task_init() {
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = _TASK_STACK_SIZE;
    taskParams.stack     = &_task_stack;
    taskParams.priority  = 1;
    Task_construct(&_task_struct, (Task_FuncPtr)_imuThread, &taskParams, NULL);
}
