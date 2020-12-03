/*
 * aw_mqtt.h
 *
 *  Created on: 2020Äê9ÔÂ25ÈÕ
 *      Author: admin
 */

#ifndef STACK_AW_MQTT_H_
#define STACK_AW_MQTT_H_
#include <iot/mqtt/aw_mqtt_def.h>
#include <stdio.h>
#include <string.h>

#include "aw_config.h"
#include "util_def.h"

const aw_at_urc_t* aw_at_check_urc(const aw_uart_rx_t* uart_buf);
aw_at_response_t* aw_at_response_get();

int aw_mqtt_pub_add_f(const char* __restrict _format, ...);
int aw_mqtt_pub_flush(aw_pub_type_t pub_type, aw_qos_t qos, u8 retain,
                      const char* topic);
int aw_mqtt_pub_f(aw_pub_type_t pub_type, aw_qos_t qos, u8 retain,
                  const char* topic, const char* __restrict _format, ...);
int aw_mqtt_init(AwCallBack_char_t funcRecv, AwCallBack_char_t funcStat);
int aw_mqtt_pub_status();
int aw_mqtt_net_building(aw_mqtt_mode_t mode, u8 isPub, char* dev_name);
int aw_mqtt_get_csq();
int aw_mqtt_set_ate(aw_at_ate_t ate);
s8 aw_mqtt_nb_close();
s8 aw_mqtt_nb_open();
const aw_at_staist_t* aw_mqtt_statis_get();
const aw_at_status_t* aw_mqtt_status_get();
bool aw_mqtt_status_free();
bool aw_mqtt_status_busy();
bool aw_mqtt_status_building();
void aw_mqtt_set_host(char* host);
const char* aw_mqtt_get_host();
void aw_mqtt_set_sub(u8 is_sub);
void aw_mqtt_set_keepAlive(u16 keepAliveTime);
const aw_mqtt_config_t* aw_mqtt_config_get();
void aw_mqtt_config_set_qos(aw_qos_t qos);
int aw_mqtt_reset(u8 is_connect);
int aw_mqtt_net_led(u8 on);
void aw_mqtt_config_set_host(const char* host);
void aw_mqtt_config_set_dev_name(const char* dev_name);
void aw_mqtt_config_set_dev_id(const char* dev_id);
const char* aw_mqtt_topic_get_f(const char* __restrict _format, ...);
#endif /* STACK_AW_MQTT_H_ */
