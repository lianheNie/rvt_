/*
 * aw_config.h
 *
 *  Created on: 2020��9��21��
 *      Author: admin
 */

#ifndef CONFIG_AW_CONFIG_H_
#define CONFIG_AW_CONFIG_H_
//#define IS_USE_SENSOR_TEST
////////////////////////////////////�̼��汾///////////////////////////////////////////////
#define AW_version_alphal  0  //�ڲ����԰� ��ʵ�ֹ��ܣ�����bug
#define AW_version_beta    1  //�ⲿ���԰� ������bug,�������������δ����
#define AW_version_release 2  //������

#define AW_main_version  1  //���屾��     ���Ϊ16
#define AW_sub_version   0  //�Ӱ汾��     ���Ϊ16
#define AW_stage_version 3  //�׶ΰ汾��  ���Ϊ16

#define AW_version_status AW_version_release  //�汾״̬

#define AW_version_num \
    (AW_main_version << 4 | AW_sub_version) << 8 | (AW_stage_version << 4 | AW_version_status)

#define AW_version_num_str \
    "" AW_STR(AW_main_version) "." AW_STR(AW_sub_version) "." AW_STR(AW_stage_version) ""

#define AW_firmware_version "v" AW_version_num_str "_" AW_STR(AW_version_status) ""  //
//////////////////////////////////�̼��汾///////////////////////////////////////////////

//////////////////////////////�豸���Ͷ���/////////////////////////////////////////////////
#define AW_devtype_gateway_lowpower 0  //������������  �͹���
#define AW_devtype_gateway_rd       1  //����+����       �͹���
#define AW_devtype_gateway_rp       2  //����+����       �͹���
#define AW_devtype_gateway_normal   3  //�����豸������

#define AW_devtype_gateway_rs_np_1      4  //����+��·��������
#define AW_devtype_gateway_rs_3_cur_bat 5  //����+��·����+����+���
#define AW_devtype_rs_3                 6  //��·����
#define AW_devtype_rs_1                 7  //��·����
#define AW_devtype_rs_np_1              8  //��·��������

#define AW_devtype_rd  9               //����  �͹���
#define AW_devtype_rp  10              //����  �͹���
#define AW_devtype_rv  11              //��  �͹���
#define AW_devtype_rt  12              //�¶� �͹���
#define AW_devtype_rvt 13              //��+�¶� �͹���
#define AW_DEV_TYPE    AW_devtype_rvt  //�豸����
///////////////////////////////////////////////////////////////////////////
#if AW_devtype_rt == AW_DEV_TYPE
#define IS_USE_7X7_PACKAGE  // 7X7���b
#else
#define IS_USE_4X4_PACKAGE  // 4X4���b
#endif
///////////////////////////////////////////////////////////////////////////
#define AW_DEV_SELF_ADDRESS         2106              // 1103        //   36 �豸��ַ
#define AW_DEV_PAIR_DEFAULT_ADDRESS 0                 //�豸��Ե�ַ
#define AW_MQTT_HOST                "106.14.123.177"  //"106.14.192.13"
#define AW_MQTT_DEV_NAME            "GAT"
#define AW_MQTT_DEV_ID              AW_STR(AW_EASYLINK_SELF_ADDRESS)

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
#define AW_MQTT_KEEPALIVE 300  // 180  //�͹���ʱ�������ϳ�
#else
#if AW_devtype_rv == AW_DEV_TYPE || AW_DEV_TYPE == AW_devtype_rvt || AW_DEV_TYPE == AW_devtype_rt
#define AW_MQTT_KEEPALIVE 300  //
#else
#define AW_MQTT_KEEPALIVE 30  //
#endif
#endif

#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat
#define AW_RX_TIMEOUT_MS (2 * 1000)  // 2s
#else
#define AW_RX_TIMEOUT_MS (3 * 1000)  // 2s
#endif

#define IS_USE_EASYLINK_ACK  //����433MHzͨ��Ӧ�����

#define IS_USE_RETRANSIMIT  //��������ת������

#define IS_USE_NVS  //�����ڲ�����flash
//#define IS_USE_OLED           //����OLED��ʾ��
//#define IS_USE_ULTRASONIC     //���ó�����ģ��

#if AW_DEV_TYPE == AW_devtype_rp || AW_DEV_TYPE == AW_devtype_gateway_rp
#define IS_USE_PIR  //���ú��⴫����
#endif

#if AW_DEV_TYPE == AW_devtype_rd || AW_DEV_TYPE == AW_devtype_gateway_rd
#define IS_USE_IMU  //����IMU������
#endif

#define IS_USE_PAIR  //������Ի���

#if defined(IS_USE_IMU) || defined(IS_USE_PIR)
#define IS_USE_POWER_CTL  //���õ�Դ����(��Դ����+��ԴADC)
#endif

#if AW_DEV_TYPE >= AW_devtype_gateway_rs_np_1 && AW_DEV_TYPE <= AW_devtype_rs_np_1
#define IS_USE_SWITCH_SENSOR  //���ÿ���ģ��
#ifdef IS_USE_SWITCH_SENSOR
#if AW_DEV_TYPE == AW_devtype_rs_3
#define IS_USE_SWITCH3_SENSOR
#elif AW_DEV_TYPE == AW_devtype_rs_np_1 || AW_DEV_TYPE == AW_devtype_gateway_rs_np_1
#define IS_USE_SERVO_SWITCH  //�ŷ�ģ�鼴��������ģ��
#else
#endif
#endif
#endif

#define IS_USE_EASYLINK  //����EASYLINK(433MHz)

#ifdef IS_USE_EASYLINK
#if AW_DEV_TYPE != AW_devtype_rd && AW_DEV_TYPE != AW_devtype_rp
#define IS_USE_RX  //����433MHzһֱ����ģʽ
#endif
#endif

#if AW_DEV_TYPE == AW_devtype_rv || AW_DEV_TYPE == AW_devtype_rvt || AW_DEV_TYPE == AW_devtype_rt
#undef IS_USE_EASYLINK
#undef IS_USE_RX
#endif

#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat || AW_devtype_rv == AW_DEV_TYPE || \
    AW_DEV_TYPE == AW_devtype_rvt || AW_DEV_TYPE == AW_devtype_rt
//#define IS_USE_LED       //����LED
//#define IS_USE_WATCHDOG  //���ÿ��Ź�
#define IS_USE_MQTT  //����MQTT
#ifdef IS_USE_MQTT
#define IS_USE_AT_CLIENT
#define IS_USE_MQTT_TIMER
//#define IS_USE_NB_POWERDOWN
//#define IS_USE_BC26
#endif
#endif

#if (defined(IS_USE_EASYLINK) && defined(IS_USE_RX)) || defined(IS_USE_MQTT)
#define IS_USE_RS  //Զ�̿���
#endif

#if AW_DEV_TYPE == AW_devtype_rs_np_1 || AW_DEV_TYPE == AW_devtype_gateway_rs_np_1
#undef IS_USE_LED
#endif

#ifdef IS_USE_SWITCH_SENSOR
#define IS_USE_LS  //���ñ��ؿ���
#endif

#if defined(IS_USE_EASYLINK)
#if AW_DEV_TYPE <= AW_devtype_gateway_rs_3_cur_bat
//#define IS_USE_PA_LNA  //ʹ�ù��ź͸߷�
#endif
#endif

#if AW_DEV_TYPE == AW_devtype_gateway_rs_3_cur_bat
#define IS_USE_BAT_CHARGE  //���õ�س�����
#ifdef IS_USE_BAT_CHARGE
#define IS_USE_BAT_CHARGE_TIMER
#endif
#define IS_USE_AMMETER  //���õ�����
#endif

#if AW_devtype_rt == AW_DEV_TYPE || \
    AW_devtype_rvt == AW_DEV_TYPE  //�¶� �͹���   //��+�¶� �͹���
#define IS_USE_RT
#undef IS_USE_PAIR  //������Ի���
#ifndef IS_USE_RVT_TIMER
#define IS_USE_RVT_TIMER
#endif
#undef IS_USE_LED
#undef IS_USE_MQTT_TIMER
#ifdef IS_USE_7X7_PACKAGE

#undef IS_USE_MQTT
#undef IS_USE_AT_CLIENT
#define IS_USE_EASYLINK  //����EASYLINK(433MHz)
#endif

#endif
//#define IS_USE_DEV//�����豸��(ռ���ڴ�)
//#define IS_USE_C_TEST  //����C���Բ���
#endif /* CONFIG_AW_CONFIG_H_ */
