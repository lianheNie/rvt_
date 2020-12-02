/*
 * task_app.c
 *
 *  Created on: 2020��9��25��
 *      Author: admin
 */
#include <aon_batmon.h>
#include <aw_config.h>
#include <stdio.h>
#include <string.h>
#include <sys_ctrl.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>

#include "uart_driver.h"
#include "util_def.h"
#include "util_delay.h"
#include "util_other.h"
#include "util_printf.h"
#define IMU_EVENT 0x0001
#define MQTT_LOST_EVENT 0x0002
#define MQTT_RECV_EVENT 0x0004
#define EASYLINK_RX_EVENT 0x0008

#define BATTERY_TIMER_EVENT 0x0010
#define MQTT_TIMER_EVENT 0x0020
#define POWER_SWITCH_EVENT 0x0040
#define POWER_ADC_EVENT 0x0080
#define PIR_ALARM_EVENT 0x0100
#define RVT_TIMER_EVENT 0x0200  //
#define KEY_EVENT 0x0400        //
static u16 _events = 0;
static Semaphore_Handle _appSem = NULL;

static bool _app_sem_init() {
  Semaphore_Params params;
  Error_Block eb;
  Semaphore_Params_init(&params);
  Error_init(&eb);
  _appSem = Semaphore_create(0, &params, &eb);
  if (_appSem == NULL) {
  }
  return true;
}
#ifdef IS_USE_POWER_CTL
#include "aw_power.h"
#endif

#ifdef IS_USE_MQTT
#include <aw_mqtt.h>
#endif

#ifdef IS_USE_LED
#include "aw_led.h"
#endif
#ifdef IS_USE_RV
#include <vibration/aw_vibration.h>
#endif

#ifdef IS_USE_OLED
#include "aw_oled.h"
#endif
#if defined(IS_USE_LS)
#include "aw_switch.h"
#endif

#if defined(IS_USE_NVS)
#include "nvs_driver.h"
#endif
#ifdef IS_USE_RT
#include "aw_temper.h"
#endif
#if defined(IS_USE_ULTRASONIC)
#include "hc-sr04.h"
#endif

#ifdef IS_USE_WATCHDOG
#include "aw_watchdog.h"
#endif
#ifdef IS_USE_MQTT
extern s8 at_uart_open();
extern s8 at_uart_close();
static s8 _mqtt_pub_sensor(Aw_Sensor_t *sensor) {
  s8 res = 0;
  const char *topic = NULL;
  if (AW_devtype_rd == sensor->type || AW_devtype_gateway_rd == sensor->type) {
    topic = aw_mqtt_topic_get_f("" AW_DEV_RD "/%d/" AW_NOTIFY "", sensor->id);
  } else if (AW_devtype_rp == sensor->type ||
             AW_devtype_gateway_rp == sensor->type) {
    topic = aw_mqtt_topic_get_f("" AW_DEV_RP "/%d/" AW_NOTIFY "", sensor->id);
  } else if (AW_devtype_rt == sensor->type || AW_devtype_rvt == sensor->type) {
    topic = aw_mqtt_topic_get_f("" AW_DEV_RT "/%d/" AW_NOTIFY "", sensor->id);
  } else {
    topic = aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_NOTIFY "",
                                AW_MQTT_DEV_ID);
  }
  res = aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN, topic,
                      "{i:%d,t:%d,y:%d,v:%d,b:%d,r:%d,c:%d,csq:%d}",
                      AW_DEV_SELF_ADDRESS, sensor->src_id, sensor->type,
                      sensor->val, sensor->bat, sensor->rssi, sensor->cnt,
                      aw_mqtt_get_csq());

  return res;
}
#endif
#if defined(IS_USE_IMU) || defined(IS_USE_PIR) || defined(IS_USE_RT)
static u16 _sensor_cnt = 0;
static s8 _process_sensor(u8 type, s16 val, u16 cnt) {
  Aw_Sensor_t _sensor;
  _sensor.bat = 0;
#ifdef IS_USE_POWER_CTL
  _sensor.bat = aw_power_adc_get();
#endif
  _sensor.id = AW_DEV_SELF_ADDRESS;
  _sensor.src_id = _sensor.id;
  _sensor.type = type;
  _sensor.cnt = cnt;
  _sensor.val = val;

#ifdef IS_USE_MQTT
  _mqtt_pub_sensor(&_sensor);
#endif

#ifdef IS_USE_LED
  aw_toggle_led();
#endif
  return 0;
}
#endif

#ifdef IS_USE_POWER_CTL
static void _power_switch_cb(void) {
  _events |= POWER_SWITCH_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static void _proc_power_switch_event(void) {
  u8 cnt = 30;
  while (aw_power_switch_get()) {
    aw_delay_ms(100);
    cnt--;
    if (0 == cnt) {
      break;
    }
  }
  if (0 == cnt) {
#ifdef IS_USE_LED
    aw_blink_led(1, 100);
#endif
    aw_power_switch_set(AW_POWER_OFF);

#ifdef IS_USE_PAIR  //�������

#ifdef IS_USE_IMU
    extern s8 aw_imu_close();
    aw_imu_close();  //�ر�IMU������
#endif

#ifdef IS_USE_PIR
    extern void aw_pir_close();
    aw_pir_close();  //�ر�PIR������
#endif

#endif
  }
}

static void _power_adc_cb(void) {
  _events |= POWER_ADC_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
#define _POWER_ADC_LOW true
static void _proc_power_adc_event(void) {
  if (_POWER_ADC_LOW == aw_power_adc_update(NULL)) {
    _process_sensor(AW_DEV_TYPE, AW_DEV_LOWPOWER, ++_sensor_cnt);
#ifdef IS_USE_LED
    aw_blink_led(1, 100);
#endif
    aw_power_switch_set(AW_POWER_OFF);
  }
}
#endif

#ifdef IS_USE_AMMETER
#include "aw_ammeter.h"
#endif

#ifdef IS_USE_BAT_CHARGE
#include "aw_battery.h"
#ifdef IS_USE_BAT_CHARGE_TIMER
static void _aw_battery_timeout_cb(void) {
  _events |= BATTERY_TIMER_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static void _proc_battery_timeout_event() { aw_batttey_update(); }
#endif
#endif

#ifdef IS_USE_MQTT
#if defined(IS_USE_RT) || defined(IS_USE_RV)

#ifdef IS_USE_RVT_TIMER
static int _rvt_report_time = 1800;  //�ϴ�����
#endif

static bool _is_key_set = false;
#endif
#include <aw_mqtt.h>
#if defined(IS_USE_MQTT_TIMER)
static s8 _mqtt_lost_cb(const char *str) {
  if (aw_mqtt_status_building() || aw_mqtt_status_busy()) {
    return -1;
  }
  _events |= MQTT_LOST_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
  return 0;
}
static void _proc_mqtt_lost_event() { aw_mqtt_net_building(); }
#endif

#include "aw_json.h"
static JsonBuf_t _json_buf = {
    .head = 0, .tail = 0, .next = 1, .size = AW_JSON_BUF_SIZE};
static char _json_str[AW_JSON_STR_SIZE] = {'\0'};

static s8 _mqtt_recv_cb(const char *str) {
  _events |= MQTT_RECV_EVENT;
  parse_json(&_json_buf, str);
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
  return 0;
}
static const aw_at_staist_t *_mqtt_statis = NULL;

static void _mqtt_pub_flush_res() {
  aw_mqtt_pub_add_f("}");
  aw_mqtt_pub_flush(AW_PUB_MODAL, aw_mqtt_config_get()->qos, NO_RETAIN,
                    aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_RESULT "",
                                        AW_MQTT_DEV_ID));  //��������ִ�н��
}
static void _mqtt_pub_status_data() {
#ifdef IS_USE_LS  //ʹ�ñ��ؿ���
  const E_sw_sts_t *chs = NULL;
  chs = sw_get_chs();
#endif

#if AW_devtype_gateway_rs_3_cur_bat == AW_DEV_TYPE
  u16 cur_4v = 0, cur_5v = 0, cur_12v = 0;
  aw_ammeter_get(&cur_4v, &cur_5v, &cur_12v);
  const aw_battery_status_t *_bat_sts = NULL;
  _bat_sts = aw_battery_get_status();
  u16 bat = _bat_sts->v_bat;
  u16 bat_c = _bat_sts->cur;
  u16 bus_v = _bat_sts->v_bus;
  u16 bat_v = _bat_sts->bat_max;
  u16 sys_v = _bat_sts->v_sys;
  aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN,
                aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_STATUS "",
                                    AW_MQTT_DEV_ID),
                "{i:%d,y:%d,b:%d,bc:%d,bv:%d,byv:%d,bs:%d,cc1:%d,cc2:%d,cc3:%d,"
                "c1:%d,c2:%d,c3:%d,"
                "csq:%d,ver:%d}",
                AW_DEV_SELF_ADDRESS, AW_DEV_TYPE, bat, bat_c, bat_v, sys_v,
                bus_v, cur_12v, cur_5v, cur_4v, chs[AW_SW_CH1], chs[AW_SW_CH2],
                chs[AW_SW_CH3], aw_mqtt_get_csq(), AW_version_num);
#elif AW_devtype_rs_3 == AW_DEV_TYPE
  aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN,
                aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_STATUS "",
                                    AW_MQTT_DEV_ID),
                "{i:%d,y:%d,c1:%d,c2:%d,c3:%d,csq:%d,ver:%d}",
                AW_DEV_SELF_ADDRESS, AW_DEV_TYPE, chs[AW_SW_CH1],
                chs[AW_SW_CH2], chs[AW_SW_CH3], aw_mqtt_get_csq(),
                AW_version_num);
#elif AW_devtype_gateway_rs_np_1 == AW_DEV_TYPE || \
    AW_devtype_rs_np_1 == AW_DEV_TYPE || AW_devtype_rs_1 == AW_DEV_TYPE
  aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN,
                aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_STATUS "",
                                    AW_MQTT_DEV_ID),
                "{i:%d,y:%d,c1:%d,csq:%d,ver:%d}", AW_DEV_SELF_ADDRESS,
                AW_DEV_TYPE, chs[AW_SW_CH1], aw_mqtt_get_csq(), AW_version_num);
#else
  aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN,
                aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_STATUS "",
                                    AW_MQTT_DEV_ID),
                "{i:%d,y:%d,csq:%d,rpt:%d,sip:%s,ver:%d}", AW_DEV_SELF_ADDRESS,
                AW_DEV_TYPE, aw_mqtt_get_csq(), _rvt_report_time,
                aw_mqtt_get_host(), AW_version_num);
#endif
}
static void _proc_mqtt_recv_event() {                 // mqtt ���ݽ��մ����¼�
  while (true == read_json(&_json_buf, _json_str)) {  //��json����
    int len = strlen(_json_str);
    if (len > 0) {
      aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN,
                    aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_ACK "",
                                        AW_MQTT_DEV_ID),
                    "{ack:%s,cnt:%d,csq:%d,y:%d,ver:%d}", _json_str,
                    (NULL != _mqtt_statis) ? _mqtt_statis->recv_urc_cnt : 0,
                    aw_mqtt_get_csq(), AW_DEV_TYPE,
                    AW_version_num);  //��������Ӧ��

      char *pStr = NULL;
      int val = 0;
      int select_id = -1;
      if (pStr = strstr(_json_str, SELECT_ID)) {  //�豸ID
        if (sscanf(pStr - 1, "\"" SELECT_ID "\":%d", &val) == 1) {
          select_id = val;
        }
      }
      int cnt = 0;
      aw_mqtt_pub_add_f("{i:%d",
                        select_id > 0 ? select_id : AW_DEV_SELF_ADDRESS);
      cnt++;
      if (pStr = strstr(_json_str, QOS)) {  //�ź���������
        if (sscanf(pStr - 1, "\"" QOS "\":%d", &val) == 1) {
          aw_mqtt_config_set_qos((aw_qos_t)val);
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" QOS "\":%d", val);
          cnt++;
        }
      }
#ifdef IS_USE_RV
      if (pStr = strstr(_json_str, GET_VIBR)) {  //��ȡ������
        if (sscanf(pStr - 1, "\"" GET_VIBR "\":%d", &val) == 1) {
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" GET_VIBR "\":%d", val);
          _mqtt_pub_flush_res();    //��������ִ�н��
          aw_vibration_process(0);  //��ȡ�����ݲ�����
        }
      }
      if (pStr = strstr(_json_str, VIBR_BIAS)) {  //�񶯴�����У׼
        if (sscanf(pStr - 1, "\"" VIBR_BIAS "\":%d", &val) == 1) {
          s16 *cal = NULL;
          cal = aw_vibration_calibration();
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          if (cal) {
            aw_mqtt_pub_add_f("\"" VIBR_BIAS "\":[%d,%d,%d]", cal[0], cal[1],
                              cal[2]);
          }
          cnt++;
        }
      }
#endif
#ifdef IS_USE_RT
      if (pStr = strstr(_json_str, GET_TEMP)) {  //��ȡ�¶�����
        if (sscanf(pStr - 1, "\"" GET_TEMP "\":%d", &val) == 1) {
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" GET_TEMP "\":%d", val);
          _mqtt_pub_flush_res();  //��������ִ�н��
          s16 tempt = 0;
          aw_temper_read(&tempt);  //���¶�
          _process_sensor(AW_devtype_rt, tempt, 0);
        }
      }
      if (pStr = strstr(_json_str, TEMP_BIAS))  //�¶�����
      {
        int temp_bias = 0;
        if (sscanf(pStr - 1, "\"" TEMP_BIAS "\":%d", &temp_bias) == 1) {
          aw_temper_set_bias((s16)temp_bias);
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" TEMP_BIAS "\":%d", temp_bias);
          cnt++;
        }
      }
#endif

#ifdef IS_USE_RVT_TIMER
      if (pStr = strstr(_json_str, RVT_REPORT_TIME))  //�ϱ���������
      {
        int report_time = 0;
        if (sscanf(pStr - 1, "\"" RVT_REPORT_TIME "\":%d", &report_time) == 1) {
          _rvt_report_time = report_time;
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" RVT_REPORT_TIME "\":%d", report_time);
          cnt++;
        }
      }
#endif
      if (pStr = strstr(_json_str, SERVER_IP))  //����������
      {
        char host[20] = {'\0'};
        if (sscanf(pStr - 1, "\"" SERVER_IP "\":\"%31[^\"]\"", host) == 1) {
          aw_mqtt_set_host(host);
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" SERVER_IP "\":%s", host);
          _mqtt_pub_flush_res();  //��������ִ�н��
          if (_is_key_set) {
            aw_mqtt_set_keepAlive(600);  //���ó������������ڵ͹���
            aw_mqtt_set_sub(0);          //��������Ϣ
            aw_mqtt_net_building();      //����������
            at_uart_close();             //�رմ���
            _is_key_set = false;
          }
        }
      }
#ifdef IS_USE_BAT_CHARGE
      if (pStr = strstr(_json_str, BAT_CUR)) {  //��س���������
        if (sscanf(pStr - 1, "\"" BAT_CUR "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            aw_battery_set_current((u16)val);
          } else {
          }

          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" BAT_CUR "\":%d", val);
          cnt++;
        }
      }

      if (pStr = strstr(_json_str, BAT_VOL)) {  //��س���������
        if (sscanf(pStr - 1, "\"" BAT_VOL "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            aw_battery_set_voltage((u16)val);
          } else {
          }
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" BAT_VOL "\":%d", val);
          cnt++;
        }
      }
#endif

#ifdef IS_USE_LED
      if (pStr = strstr(_json_str, SET_LED)) {  // led����ʾ����
        if (sscanf(pStr - 1, "\"" SET_LED "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            aw_led_enable((bool)val);

          } else if (select_id > 0) {
          }

          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" SET_LED "\":%d", val);
          cnt++;
        }
      }
#endif
      if (pStr = strstr(_json_str, SET_NET_LED)) {  // NBģ�����������
        if (sscanf(pStr - 1, "\"" SET_NET_LED "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            aw_mqtt_net_led(val);
          } else if (select_id > 0) {
          }
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" SET_NET_LED "\":%d", val);
          cnt++;
        }
      }
      if (pStr = strstr(_json_str, ATE)) {  // NBģ���������
        if (sscanf(pStr - 1, "\"" ATE "\":%d", &val) == 1) {
          aw_mqtt_set_ate((aw_at_ate_t)val);
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" ATE "\":%d", val);
          cnt++;
        }
      }
#if defined(IS_USE_PAIR)  //�������
      if (pStr = strstr(_json_str, GATE_PAIR)) {
        if (sscanf(pStr - 1, "\"" GATE_PAIR "\":%d", &val) == 1) {
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" GATE_PAIR "\":%d", val);
          cnt++;
        }
      }

#endif

#ifdef IS_USE_ULTRASONIC
      if (pStr = strstr(_json_str, USC)) {  //��ȡ������
        if (sscanf(pStr - 1, "\"" USC "\":\"%d\"", &val) == 1) {
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" USC "\":\"%d\"", hc_sr04_get());
          cnt++;
        }
      }
#endif

#if defined(IS_USE_LS) || \
    (defined(IS_USE_RS) && defined(IS_USE_EASYLINK))  //���ÿ���ģ��
      int _rs_delay = 0;
      E_sw_sts_t _rs_sts[AW_SW_CNT] = {AW_SW_NULL, AW_SW_NULL, AW_SW_NULL};
      u8 _rs_cnt = 0;
      if (pStr = strstr(_json_str, RS_DELAY)) {  //���ص㶯��ʱ
        if (sscanf(pStr - 1, "\"" RS_DELAY "\":%d", &val) == 1) {
          _rs_delay = val;
        }
      }
      if (pStr = strstr(_json_str, CH1)) {  //����ͨ��1
        if (sscanf(pStr - 1, "\"" CH1 "\":%d", &val) == 1) {
          _rs_sts[AW_SW_CH1] = (E_sw_sts_t)val;
          _rs_cnt++;
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" CH1 "\":%d", val);
          cnt++;
        }
      }

      if (pStr = strstr(_json_str, CH2)) {  //����ͨ��2
        if (sscanf(pStr - 1, "\"" CH2 "\":%d", &val) == 1) {
          _rs_sts[AW_SW_CH2] = (E_sw_sts_t)val;
          _rs_cnt++;
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" CH2 "\":%d", val);
          cnt++;
        }
      }

      if (pStr = strstr(_json_str, CH3)) {  //����ͨ��3
        if (sscanf(pStr - 1, "\"" CH3 "\":%d", &val) == 1) {
          _rs_sts[AW_SW_CH3] = (E_sw_sts_t)val;
          _rs_cnt++;
          if (cnt > 0) {
            aw_mqtt_pub_add_f(",");
          } else {
            aw_mqtt_pub_add_f("{");
          }
          aw_mqtt_pub_add_f("\"" CH3 "\":%d", val);
          cnt++;
        }
      }

      if (_rs_cnt > 0) {
        if (_rs_delay > 0) {
          if (cnt > 1) {
            cnt = 0;
            _mqtt_pub_flush_res();  //��������ִ�н��
          }
        }
        if (select_id == AW_DEV_SELF_ADDRESS) {
#if defined(IS_USE_LS)
          sw_set_chs_sts(_rs_sts, _rs_delay);
#endif
        } else {
        }
        if (_rs_delay > 0) {
          u8 i = 0;
          u8 j = 0;
          aw_mqtt_pub_add_f("{i:%d",
                            select_id > 0 ? select_id : AW_DEV_SELF_ADDRESS);
          j++;
          for (i = 0; i < AW_SW_CNT; i++) {
            if (AW_SW_NULL != _rs_sts[i]) {
              if (j > 0) {
                aw_mqtt_pub_add_f(",");
              } else {
                aw_mqtt_pub_add_f("{");
              }
              aw_mqtt_pub_add_f("\"c%d\":%d", i + 1, AW_SW_OFF);
              j++;
            }
          }
          if (j > 1) {
            _mqtt_pub_flush_res();
          }
        } else {
          if (cnt > 1) {
            cnt = 0;
            _mqtt_pub_flush_res();  //��������ִ�н��
          }
        }
      } else {
        if (cnt > 1) {
          cnt = 0;
          _mqtt_pub_flush_res();  //��������ִ�н��
        }
      }

#else
      if (cnt > 1) {
        _mqtt_pub_flush_res();  //��������ִ�н��
      }
#endif
      if (pStr = strstr(_json_str, STS)) {  //״̬���ݲ�ѯ
        if (sscanf(pStr - 1, "\"" STS "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            _mqtt_pub_status_data();
          } else if (select_id > 0) {
          }
        }
      }
      if (pStr = strstr(_json_str, RESET_NB)) {  //��λNBģ��
        if (sscanf(pStr - 1, "\"" RESET_NB "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            aw_mqtt_reset((u8)val);
          } else if (select_id > 0) {
          }
        }
      }
      if (pStr = strstr(_json_str, RESET_CPU)) {  //��λ��Ƭ��
        if (sscanf(pStr - 1, "\"" RESET_CPU "\":%d", &val) == 1) {
          if (select_id == AW_DEV_SELF_ADDRESS) {
            SysCtrlSystemReset();
          } else if (select_id > 0) {
          }
        }
      }
    }
  }
}
#if defined(IS_USE_MQTT_TIMER)
#include "util_timer.h"
#define _MQTT_TIMEOUT 300 * 1000  // 300s
static Clock_Struct _mqtt_clockS;
static void _mqtt_timeout_cb(u32 arg) {
  _events |= MQTT_TIMER_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static void _mqtt_timer_init() {
  Timer_construct(&_mqtt_clockS, _mqtt_timeout_cb, _MQTT_TIMEOUT, _MQTT_TIMEOUT,
                  false, 0);
  Timer_start(&_mqtt_clockS);  //����ʱ��
}

static void _proc_mqtt_timeout_event() {
  aw_mqtt_pub_status();
#if defined(IS_USE_ULTRASONIC)
#ifdef IS_USE_OLED
  aw_oled_single_printf(AW_OLED_F8X16, "usc:%d", hc_sr04_get());
  aw_mqtt_pub_f(QOS0_MOST_ONECE, NO_RETAIN,
                aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_NOTIFY "",
                                    AW_MQTT_DEV_ID),
                "{usc:%d}", hc_sr04_get());
#endif
#endif
#ifdef IS_USE_LED
  aw_toggle_led();
#endif
}
#endif

#if defined(IS_USE_RT) || defined(IS_USE_RV)
#include "util_timer.h"
static Clock_Struct _rvt_clockS;
#include "key_driver.h"
#define _KEY_ID IOID_0
static void _key_cb(u8 arg) {
  _events |= KEY_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}

static void _proc_key_event() {
  int cnt = 0;
  u32 val = key_read(_KEY_ID);
  while (val == 0 && cnt < 500) {  //���5�밴��
    aw_delay_ms(10);
    cnt++;
    val = key_read(_KEY_ID);
    if (val > 0) {
      return;
    }
  }
  Timer_stop(&_rvt_clockS);  //����ʱ��
  aw_delay_ms(200);
  Timer_start(&_rvt_clockS);  //����ʱ��
  _is_key_set = true;

  at_uart_open();
  aw_mqtt_set_keepAlive(30);
  aw_mqtt_set_sub(1);  //������Ϣ����������
  aw_mqtt_net_building();
}

static void _key_init() { key_init(_key_cb); }
#endif
static void _mqtt_init() {
#if !defined(IS_USE_NB_POWERDOWN) && !defined(IS_USE_RT) && !defined(IS_USE_RV)
  json_init(&_json_buf);
  aw_mqtt_init(_mqtt_recv_cb, _mqtt_lost_cb);
  _mqtt_statis = aw_mqtt_statis_get();
#else
  json_init(&_json_buf);
  aw_mqtt_init(_mqtt_recv_cb, NULL);
  _mqtt_statis = aw_mqtt_statis_get();
#endif

#ifndef IS_USE_NB_POWERDOWN
  aw_mqtt_net_building();  //���ӷ�����
#if !defined(IS_USE_RT) && !defined(IS_USE_RV)
  _mqtt_timer_init();
#endif
#endif

#ifdef IS_USE_OLED
  aw_oled_single_printf(AW_OLED_F8X16, "csq:%d", aw_mqtt_get_csq());
#endif
}
#endif  // end mqtt

#ifdef IS_USE_IMU
#include "util_buffer.h"
static BufDatas_t _imuAlarmBuf = {.head = 0, .tail = 0, .next = 1, .buf = {0}};
void imuAlarmOldCB(float ang) {
  _events |= IMU_EVENT;
  BufData_t data;
  if (ang > 0.00001f) {
    data = 1;
  } else if (ang < 0.00001f) {
    data = -1;
  }
  putBuf(&_imuAlarmBuf, &data);
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static u16 _imu_cnt = 0;
static void _proc_imu_event() {
  BufData_t val = 0;
  while (AW_TRUE == readBuf(&_imuAlarmBuf, &val)) {
    _process_sensor(AW_DEV_TYPE, val, ++_imu_cnt);
  }
}
#endif  // end imu

#ifdef IS_USE_PIR
#include "aw_pir.h"
#include "util_buffer.h"
static BufDatas_t _pirAlarmBuf = {.head = 0, .tail = 0, .next = 1, .buf = {0}};
static void _pir_cb(s8 val) {
  _events |= PIR_ALARM_EVENT;
  putBuf(&_pirAlarmBuf, &val);
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static u16 _pir_cnt = 0;
static void _proc_pir_event() {
  s8 val = 0;
  while (AW_TRUE == readBuf(&_pirAlarmBuf, &val)) {
    _process_sensor(AW_DEV_TYPE, val, ++_pir_cnt);
  }
}
#endif  // end pir

#ifdef IS_USE_RVT_TIMER
#include "util_timer.h"
#define _RVT_TIME_BASE 60                   //��׼����60s
#define _RVT_TIMEOUT _RVT_TIME_BASE * 1000  // 300s
#define _RV_TIMEOUT _RVT_TIME_BASE * 1000   // 300s

static void _rvt_timeout_cb(u32 arg) {
  _events |= RVT_TIMER_EVENT;
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}
static void _rvt_timer_init() {
  Timer_construct(&_rvt_clockS, _rvt_timeout_cb, _RVT_TIMEOUT, _RVT_TIMEOUT,
                  false, 0);
  Timer_start(&_rvt_clockS);  //����ʱ��
}
static u16 _rvt_timer_cnt = 0;
static u16 _rvt_cnt = 0;

#define _AW_TEMPER_LOST_VAL 23000
static bool _is_temper_lost = false;
static void _proc_rvt_timer_event() {
  int cnt = (_rvt_report_time) / (_RVT_TIME_BASE);
  _rvt_cnt++;

  if (_is_key_set && uart_is_open() &&
      _rvt_cnt > (AW_MIN(5, cnt)))  //���ð����Ƿ���
  {
    aw_mqtt_set_keepAlive(600);
    aw_mqtt_set_sub(0);      //��������Ϣ
    aw_mqtt_net_building();  //����������
    at_uart_close();         //�رմ���
    _is_key_set = false;
  }

  if (_rvt_cnt < cnt) {
    return;
  } else {
    _rvt_cnt = 0;
  }

  s16 tempt = 0;
  s8 res = aw_temper_read(&tempt);  //���¶�
  if (res == -1)                    //��ȡʧ��
  {
    if (!_is_temper_lost) {
#ifdef IS_USE_MQTT
#ifdef IS_USE_NB_POWERDOWN
      aw_mqtt_nb_open();
#else
      at_uart_open();   //�򿪴���
#endif
#endif
      _process_sensor(AW_devtype_rt, tempt, ++_rvt_timer_cnt);
#ifdef IS_USE_MQTT
#ifdef IS_USE_NB_POWERDOWN
      aw_mqtt_nb_close();
#else
      at_uart_close();  //�رմ���
#endif
#endif
      _is_temper_lost = true;
    }
    return;
  } else {
    _is_temper_lost = false;
  }
#ifdef IS_USE_MQTT
#ifdef IS_USE_NB_POWERDOWN
  aw_mqtt_nb_open();
#else
  at_uart_open();       //�򿪴���
#endif
#endif
  // read temperatuer
  //_process_sensor(AW_devtype_rt, tempt,
  //++_rvt_timer_cnt);  //��ȡ�¶����ݣ�������

  // read acc sensor
  aw_vibration_process(tempt);  //��ȡ�����ݣ�������

#ifdef IS_USE_MQTT
#ifdef IS_USE_NB_POWERDOWN
  aw_mqtt_nb_close();
#else
  at_uart_close();      //�رմ���
#endif
#endif
}
#endif

static void _proc_events() {  //�¼�����
#ifdef IS_USE_MQTT
#if !defined(IS_USE_NB_POWERDOWN) && !defined(IS_USE_RT) && !defined(IS_USE_RV)
  if (_events & MQTT_LOST_EVENT) {
    _proc_mqtt_lost_event();
    Util_clearEvent(&_events, MQTT_LOST_EVENT);
  }
  if (_events & MQTT_TIMER_EVENT) {
    _proc_mqtt_timeout_event();
    Util_clearEvent(&_events, MQTT_TIMER_EVENT);
  }
#endif
  if (_events & MQTT_RECV_EVENT) {  // mqtt ���ݽ����¼�
    _proc_mqtt_recv_event();
    Util_clearEvent(&_events, MQTT_RECV_EVENT);
  }
#endif

#ifdef IS_USE_BAT_CHARGE
#ifdef IS_USE_BAT_CHARGE_TIMER
  if (_events & BATTERY_TIMER_EVENT) {
    _proc_battery_timeout_event();
    Util_clearEvent(&_events, BATTERY_TIMER_EVENT);
  }
#endif
#endif

#ifdef IS_USE_EASYLINK
#if defined(IS_USE_RX) | defined(IS_USE_PAIR)
  if (_events & EASYLINK_RX_EVENT) {
    _proc_easyLink_rx_event();
    Util_clearEvent(&_events, EASYLINK_RX_EVENT);
  }
#endif
#endif

#ifdef IS_USE_POWER_CTL
  if (_events & POWER_SWITCH_EVENT) {
    _proc_power_switch_event();
    Util_clearEvent(&_events, POWER_SWITCH_EVENT);
  }
  if (_events & POWER_ADC_EVENT) {
    _proc_power_adc_event();
    Util_clearEvent(&_events, POWER_ADC_EVENT);
  }
#endif

#ifdef IS_USE_IMU
  if (_events & IMU_EVENT) {
    _proc_imu_event();
    Util_clearEvent(&_events, IMU_EVENT);
  }
#endif

#ifdef IS_USE_PIR
  if (_events & PIR_ALARM_EVENT) {
    _proc_pir_event();
    Util_clearEvent(&_events, PIR_ALARM_EVENT);
  }
#endif

#if defined(IS_USE_RT) || defined(IS_USE_RV)
#ifdef IS_USE_RVT_TIMER
  if (_events & RVT_TIMER_EVENT) {  //�¶��񶯴�������ʱ�¼�
    _proc_rvt_timer_event();
    Util_clearEvent(&_events, RVT_TIMER_EVENT);
  }

  if (_events & KEY_EVENT) {  //�����¼�
    _proc_key_event();
    Util_clearEvent(&_events, KEY_EVENT);
  }
#endif
#endif
}

static void _app_init() {
  _app_sem_init();  //�źų�ʼ��

#ifdef IS_USE_POWER_CTL
  aw_power_init(_power_switch_cb, _power_adc_cb);  //��ѹ���
#endif

#ifdef IS_USE_OLED
  aw_oled_init();
#endif

#ifdef IS_USE_LED
  aw_led_init();
  aw_blink_led(1, 100);
#endif

#ifdef IS_USE_RT
  aw_temper_init();  //�¶ȴ�������ʼ��
  s16 val = 0;
  aw_temper_read(&val);
#endif

#ifdef IS_USE_RV
  aw_vibration_init();  //�𶯴�������ʼ��
#endif

#if defined(IS_USE_LS)
#if AW_devtype_gateway_rs_np_1 == AW_DEV_TYPE || \
    AW_devtype_rs_np_1 == AW_DEV_TYPE
  aw_sw_init(AW_SW_OFF);
#else
  aw_sw_init(AW_SW_ON);
#endif
#endif

#if defined(IS_USE_ULTRASONIC)
  hc_sr04_init();
#endif

#ifdef IS_USE_PIR
  aw_pir_init(_pir_cb);
#endif

#ifdef IS_USE_AMMETER
  aw_ammeter_init();
#endif

#ifdef IS_USE_BAT_CHARGE
#ifdef IS_USE_BAT_CHARGE_TIMER
  aw_battery_init(_aw_battery_timeout_cb);
#else
  aw_battery_init(NULL);
#endif
#endif

#if defined(IS_USE_EASYLINK)
  _easylink_init();
#endif

#ifdef IS_USE_MQTT
  _mqtt_init();  // mqtt��ʼ��
#if defined(IS_USE_RT) || defined(IS_USE_RV)
  _key_init();  //������ʼ��
#if !defined(IS_USE_NB_POWERDOWN)
  at_uart_close();  //�رմ���
#endif
#endif
#endif

#ifdef IS_USE_WATCHDOG
  aw_watchdog_init();
#endif

#ifdef IS_USE_LED
  aw_toggle_led();
#endif

  aw_delay_ms(1000);
#if defined(IS_USE_RT) || defined(IS_USE_RV)
#ifdef IS_USE_RVT_TIMER
  _rvt_timer_init();  //���¶����ڶ�ʱ����ʼ��
#endif
#endif
}

static void _app_process() {
  while (1) {
    Semaphore_pend(_appSem, BIOS_WAIT_FOREVER);  //���ź�
    _proc_events();                              //�����¼�
  }
}

static void _app_thread(UArg arg0, UArg arg1) {
  _app_init();
  _app_process();
}

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#define _TASK_STACK_SIZE 1024
static Task_Struct _task_struct;
static Char _task_stack[_TASK_STACK_SIZE];
void aw_app_task_init() {
  Task_Params taskParams;
  Task_Params_init(&taskParams);
  taskParams.stackSize = _TASK_STACK_SIZE;
  taskParams.stack = &_task_stack;
  taskParams.priority = 2;
  Task_construct(&_task_struct, (Task_FuncPtr)_app_thread, &taskParams, NULL);
}
