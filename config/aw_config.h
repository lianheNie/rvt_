/*
 * aw_config.h
 *
 *  Created on: 2020年9月21日
 *      Author: admin
 */

#ifndef CONFIG_AW_CONFIG_H_
#define CONFIG_AW_CONFIG_H_
//#define IS_USE_SENSOR_TEST
////////////////////////////////////固件版本///////////////////////////////////////////////
#define AW_version_alphal 0   //内部测试版 已实现功能，但有bug
#define AW_version_beta 1     //外部测试版 基本无bug,但其他相关配置未完善
#define AW_version_release 2  //发布版

#define AW_main_version 1   //主板本号     最大为16
#define AW_sub_version 0    //子版本号     最大为16
#define AW_stage_version 3  //阶段版本号  最大为16

#define AW_version_status AW_version_release  //版本状态

#define AW_version_num                           \
  (AW_main_version << 4 | AW_sub_version) << 8 | \
      (AW_stage_version << 4 | AW_version_status)

#define AW_version_num_str                                          \
  "" AW_STR(AW_main_version) "." AW_STR(AW_sub_version) "." AW_STR( \
      AW_stage_version) ""

#define AW_firmware_version \
  "v" AW_version_num_str "_" AW_STR(AW_version_status) ""  //
//////////////////////////////////固件版本///////////////////////////////////////////////

//////////////////////////////设备类型定义/////////////////////////////////////////////////
#define AW_devtype_gateway_lowpower 0  //传感器的网关  低功耗
#define AW_devtype_gateway_rd 1        //网关+开门       低功耗
#define AW_devtype_gateway_rp 2        //网关+红外       低功耗
#define AW_devtype_gateway_normal 3    //控制设备的网关

#define AW_devtype_gateway_rs_np_1 4       //网关+单路正负开关
#define AW_devtype_gateway_rs_3_cur_bat 5  //网关+三路开关+电流+电池
#define AW_devtype_rs_3 6                  //三路开关
#define AW_devtype_rs_1 7                  //单路开关
#define AW_devtype_rs_np_1 8               //单路正负开关

#define AW_devtype_rd 9             //开门  低功耗
#define AW_devtype_rp 10            //红外  低功耗
#define AW_devtype_rv 11            //振动  低功耗
#define AW_devtype_rt 12            //温度 低功耗
#define AW_devtype_rvt 13           //振动+温度 低功耗
#define AW_DEV_TYPE AW_devtype_rvt  //设备类型
///////////////////////////////////////////////////////////////////////////
#if AW_devtype_rt == AW_DEV_TYPE
#define IS_USE_7X7_PACKAGE  // 7X7封裝
#else
#define IS_USE_4X4_PACKAGE  // 4X4封裝
#endif
///////////////////////////////////////////////////////////////////////////
#define AW_DEV_SELF_ADDRESS 2102       // 1103        //   36 设备地址
#define AW_DEV_PAIR_DEFAULT_ADDRESS 0  //设备配对地址
#define AW_MQTT_HOST "106.14.123.177"  //"106.14.192.13"
#define AW_MQTT_DEV_NAME "GAT"
#define AW_MQTT_DEV_ID AW_STR(AW_DEV_SELF_ADDRESS)

#if AW_devtype_rv == AW_DEV_TYPE || AW_DEV_TYPE == AW_devtype_rvt
#define IS_USE_RV
//#define IS_USE_BMI160
#define IS_USE_LSM6DSM
#define IS_USE_ACC_FIFO
#define IS_USE_LSM6DSM_SPI
#ifndef IS_USE_RVT_TIMER
#define IS_USE_RVT_TIMER
#endif
#endif

#if AW_DEV_TYPE <= AW_devtype_gateway_rp
#define AW_MQTT_KEEPALIVE 300  // 180  //低功耗时心跳包较长
#else
#if AW_devtype_rv == AW_DEV_TYPE || AW_DEV_TYPE == AW_devtype_rvt || \
    AW_DEV_TYPE == AW_devtype_rt
#define AW_MQTT_KEEPALIVE 600  //
#else
#define AW_MQTT_KEEPALIVE 30  //
#endif
#endif

#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat
#define AW_RX_TIMEOUT_MS (2 * 1000)  // 2s
#else
#define AW_RX_TIMEOUT_MS (3 * 1000)  // 2s
#endif

#define IS_USE_EASYLINK_ACK  //启用433MHz通信应答机制

#define IS_USE_RETRANSIMIT  //启用网关转发机制

#define IS_USE_NVS  //启用内部储存flash
//#define IS_USE_OLED           //启用OLED显示屏
//#define IS_USE_ULTRASONIC     //启用超声波模块

#if AW_DEV_TYPE == AW_devtype_rp || AW_DEV_TYPE == AW_devtype_gateway_rp
#define IS_USE_PIR  //启用红外传感器
#endif

#if AW_DEV_TYPE == AW_devtype_rd || AW_DEV_TYPE == AW_devtype_gateway_rd
#define IS_USE_IMU  //启用IMU传感器
#endif

#define IS_USE_PAIR  //启用配对机制

#if defined(IS_USE_IMU) || defined(IS_USE_PIR)
#define IS_USE_POWER_CTL  //启用电源控制(电源开关+电源ADC)
#endif

#if AW_DEV_TYPE >= AW_devtype_gateway_rs_np_1 && \
    AW_DEV_TYPE <= AW_devtype_rs_np_1
#define IS_USE_SWITCH_SENSOR  //启用开关模块
#ifdef IS_USE_SWITCH_SENSOR
#if AW_DEV_TYPE == AW_devtype_rs_3
#define IS_USE_SWITCH3_SENSOR
#elif AW_DEV_TYPE == AW_devtype_rs_np_1 || \
    AW_DEV_TYPE == AW_devtype_gateway_rs_np_1
#define IS_USE_SERVO_SWITCH  //伺服模块即正负开关模块
#else
#endif
#endif
#endif

#define IS_USE_EASYLINK  //启用EASYLINK(433MHz)

#ifdef IS_USE_EASYLINK
#if AW_DEV_TYPE != AW_devtype_rd && AW_DEV_TYPE != AW_devtype_rp
#define IS_USE_RX  //启用433MHz一直接收模式
#endif
#endif

#if AW_DEV_TYPE == AW_devtype_rv || AW_DEV_TYPE == AW_devtype_rvt || \
    AW_DEV_TYPE == AW_devtype_rt
#define IS_USE_POWER_CTL  //启用电源控制(电源开关+电源ADC)

#ifdef IS_USE_POWER_CTL
//#define IS_USE_POWER_SWITCH  //启用电源控制(电源开关+电源ADC)
#define IS_USE_POWER_ADC  //启用电源控制(电源开关+电源ADC)
#endif
#undef IS_USE_EASYLINK
#undef IS_USE_RX

#endif

#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat ||                \
    AW_devtype_rv == AW_DEV_TYPE || AW_DEV_TYPE == AW_devtype_rvt || \
    AW_DEV_TYPE == AW_devtype_rt
//#define IS_USE_LED       //启用LED
//#define IS_USE_WATCHDOG  //启用看门狗
#define IS_USE_MQTT  //启用MQTT
#ifdef IS_USE_MQTT
#define IS_USE_AT_CLIENT
#define IS_USE_MQTT_TIMER
#define IS_USE_NB_POWERDOWN
//#define IS_USE_BC26
#endif
#endif

#if (defined(IS_USE_EASYLINK) && defined(IS_USE_RX)) || defined(IS_USE_MQTT)
#define IS_USE_RS  //远程开关
#endif

#if AW_DEV_TYPE == AW_devtype_rs_np_1 || \
    AW_DEV_TYPE == AW_devtype_gateway_rs_np_1
#undef IS_USE_LED
#endif

#ifdef IS_USE_SWITCH_SENSOR
#define IS_USE_LS  //启用本地开关
#endif

#if defined(IS_USE_EASYLINK)
#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat
//#define IS_USE_PA_LNA  //使用功放和高放
#endif
#endif

#if AW_DEV_TYPE == AW_devtype_gateway_rs_3_cur_bat
#define IS_USE_BAT_CHARGE  //启用电池充电管理
#ifdef IS_USE_BAT_CHARGE
#define IS_USE_BAT_CHARGE_TIMER
#endif
#define IS_USE_AMMETER  //启用电流计
#endif

#if AW_devtype_rt == AW_DEV_TYPE || \
    AW_devtype_rvt == AW_DEV_TYPE  //温度 低功耗   //振动+温度 低功耗
#define IS_USE_RT
#undef IS_USE_PAIR  //启用配对机制
#ifndef IS_USE_RVT_TIMER
#define IS_USE_RVT_TIMER
#endif
#undef IS_USE_LED
#undef IS_USE_MQTT_TIMER
#ifdef IS_USE_7X7_PACKAGE

#undef IS_USE_MQTT
#undef IS_USE_AT_CLIENT
#define IS_USE_EASYLINK  //启用EASYLINK(433MHz)
#endif

#endif
//#define IS_USE_DEV//启用设备层(占用内存)
//#define IS_USE_C_TEST  //启用C语言测试
#endif /* CONFIG_AW_CONFIG_H_ */
