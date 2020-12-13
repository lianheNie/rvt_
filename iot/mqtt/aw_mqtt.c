/*
 * aw_mqtt.c
 *
 *  Created on: 2020年9月25日
 *      Author: admin
 */
#include <iot/mqtt/aw_mqtt.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>

#include "aw_config.h"
#include "uart_driver.h"
#include "util_delay.h"
#include "util_other.h"
#include "util_printf.h"
#ifdef IS_USE_OLED
#include "aw_oled.h"
#endif
#define URC_RECV "+QMTRECV:"
#define URC_LOST "+QMTSTAT:"
static char Host[20] = AW_MQTT_HOST;
static const char DevName[] = AW_MQTT_DEV_NAME;
static const char DevId[] = AW_MQTT_DEV_ID;
static const int PortNum = 1883;
static u16 KeepAliveTime = AW_MQTT_KEEPALIVE;
static int aw_mqtt_set_alive(u32 alive_time);
static aw_mqtt_config_t _config = { .dev_id = DevId, .dev_name = DevName,
                                    .host = Host, .qos = QOS0_MOST_ONECE };
static u8 _is_sub = 0;  //是否订阅消息
static int aw_mqtt_sub_f(aw_qos_t qos, const char *__restrict _format, ...);
void aw_mqtt_set_host(char *host)
{
    if (host == NULL)
    {
        return;
    }
    Host[0] = '\0';

    strcpy(Host, host);
}
void aw_mqtt_set_sub(u8 is_sub)
{
    _is_sub = is_sub;
}
char* aw_mqtt_get_host()
{
    return Host;
}
void aw_mqtt_set_keepAlive(u16 keepAliveTime)
{
    KeepAliveTime = keepAliveTime;
    aw_mqtt_set_alive(KeepAliveTime);
}
const aw_mqtt_config_t* aw_mqtt_config_get()
{
    return &_config;
}
void aw_mqtt_config_set_qos(aw_qos_t qos)
{
    _config.qos = qos;
}

void aw_mqtt_config_set_host(const char *host)
{
    _config.host = host;
}
void aw_mqtt_config_set_dev_name(const char *dev_name)
{
    _config.dev_name = dev_name;
}
void aw_mqtt_config_set_dev_id(const char *dev_id)
{
    _config.dev_id = dev_id;
}
extern s8 at_uart_open();
extern s8 at_uart_close();
static Semaphore_Handle _respSem = NULL;
static bool _resp_sem_init()
{
    Semaphore_Params params;
    Error_Block eb;
    Semaphore_Params_init(&params);
    Error_init(&eb);
    _respSem = Semaphore_create(0, &params, &eb);
    if (_respSem == NULL)
    {
        // System_abort("Semaphore creation failed");
    }

    return true;
}
static void _resp_func(void)
{
    if (NULL != _respSem)
    {
        Semaphore_post(_respSem);
    }
}

static aw_at_response_t _at_resp = { .buf = { '\0' }, .key = { '\0' },
                                     .buf_size = AW_AT_RESPONSE_BUF_SIZE,
                                     .buf_len = 0, .end_sign = '\0', .is_set =
                                     false,
                                     .func = _resp_func, .status = AT_RESP_NULL,
                                     .line_cnt = 0, .line_num = 0 };

static void aw_at_response_init()
{
    _at_resp.buf[0] = '\0';
    _at_resp.key[0] = '\0';
    _at_resp.buf_len = 0;
    _at_resp.is_set = false;
    _at_resp.end_sign = '\0';
    _at_resp.status = AT_RESP_NULL;
    _at_resp.line_cnt = 0;
    _at_resp.line_num = 0;
}

aw_at_response_t* aw_at_response_get()
{
    return &_at_resp;
}

static void aw_at_response_set_line(u8 line_num)
{
    aw_at_response_init();
    _at_resp.is_set = true;
    _at_resp.line_num = line_num;
}

static s8 aw_at_response_check_key(const char *__restrict _format, ...)
{
    _at_resp.key[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_at_resp.key, AW_AT_RESPONSE_KEY_SIZE, _format, args);
    va_end(args);
    if (strstr(_at_resp.buf, _at_resp.key))
    {
        return AW_OK;
    }
    return AW_ERR;
}

static void aw_at_response_set_end_sign(const char end_sign)
{
    _at_resp.is_set = true;
    _at_resp.end_sign = end_sign;
}
static aw_at_urc_t _urc_table[Urc_CNT] =
        { { URC_RECV, "\r\n", NULL }, {
        URC_LOST,
                                        "\r\n", NULL }, };

#define _IS_USE_STATIS
#ifdef _IS_USE_STATIS  //统计信息
static aw_at_staist_t _statis = { .at_rsp_err_cnt = 0, .connect_ok_cnt = 0,
                                  .connect_cnt = 0, .at_timeout_cnt = 0,
                                  .pub_err_cnt = 0, .reset_cnt = 0,
                                  .lost_urc_cnt = 0, .recv_urc_cnt = 0,
                                  .sub_err_cnt = 0, .open_err_cnt = 0,
                                  .connect_err_cnt = 0 };

#endif
const aw_at_staist_t* aw_mqtt_statis_get()
{
#ifdef _IS_USE_STATIS  //统计信息
    return &_statis;
#else
  return NULL;
#endif
}
#define _IS_USE_STATUS
#ifdef _IS_USE_STATUS
static aw_at_status_t _status = { .mqtt_sts = MQTT_STS_FREE, .is_building =
false };
#endif
const aw_at_status_t* aw_mqtt_status_get()
{
#ifdef _IS_USE_STATUS
    return &_status;
#else
  return NULL
#endif
}

bool aw_mqtt_status_free()
{
#ifdef _IS_USE_STATUS
    return _status.mqtt_sts == MQTT_STS_FREE;
#else
  return true;
#endif
}
bool aw_mqtt_status_busy()
{
#ifdef _IS_USE_STATUS
    return _status.mqtt_sts != MQTT_STS_FREE;
#else
  return false;
#endif
}
bool aw_mqtt_status_building()
{
#ifdef _IS_USE_STATUS
    return _status.is_building;
#else
  return false;
#endif
}
static void aw_at_urc_table_set(AwCallBack_char_t funcRecv,
                                AwCallBack_char_t funcStat)
{
    _urc_table[Urc_RECV].func = funcRecv;
    _urc_table[Urc_STAT].func = funcStat;
}

const aw_at_urc_t* aw_at_check_urc(const aw_uart_rx_t *uart_buf)
{
    u8 i = 0, prefix_len = 0, suffix_len = 0;
    u8 bufsz = 0;
    const char *buffer = NULL;
    const aw_at_urc_t *urc = NULL;
    buffer = uart_buf->buf;
    bufsz = uart_buf->len;
    for (i = 0; i < Urc_CNT; i++)
    {
        urc = &_urc_table[i];
        prefix_len = strlen(urc->cmd_prefix);
        suffix_len = strlen(urc->cmd_suffix);
        if (bufsz < prefix_len + suffix_len)
        {
            continue;
        }
        if ((prefix_len ? !strncmp(buffer, urc->cmd_prefix, prefix_len) : 1)
                && (suffix_len ?
                        !strncmp(buffer + bufsz - suffix_len, urc->cmd_suffix,
                                 suffix_len) :
                        1))
        {
#ifdef _IS_USE_STATIS
            if (strstr(urc->cmd_prefix, URC_LOST))
            {
                _statis.lost_urc_cnt++;
            }
            else if (strstr(urc->cmd_prefix, URC_RECV))
            {
                _statis.recv_urc_cnt++;
            }
#endif
            return urc;
        }
    }
    return NULL;
}

static u32 _aw_ms_to_tick(u32 ms)
{
    return (ms * 1000 / Clock_tickPeriod);
}

#define _AW_AT_TOPIC_SZIE 32
static char _topic[_AW_AT_TOPIC_SZIE] = { '\0' };

const char* aw_mqtt_topic_get_f(const char *__restrict _format, ...)
{
    _topic[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_topic, _AW_AT_TOPIC_SZIE, _format, args);
    va_end(args);
    return _topic;
}
#ifdef IS_USE_RV
#define _AW_AT_PAYLOAD_SIZE 1105
#else
#define _AW_AT_PAYLOAD_SIZE 128
#endif
static char _payload[_AW_AT_PAYLOAD_SIZE] = { '\0' };  //字符串缓存,用于数据发送和参数化配置
static at_resp_status_t aw_at_exec_cmd(u32 timeout,
                                       const char *__restrict _format, ...)
{
    va_list args;
    va_start(args, _format);
    char *cmd = uart_vsprintf(_format, args);
    va_end(args);
    if (AW_OK != uart_write_str(cmd))
    {
        return AT_RESP_ERROR;
    }
    if (NULL != _respSem)
    {
        bool isok = Semaphore_pend(_respSem, _aw_ms_to_tick(timeout));
        if (isok)
        {
#ifdef _IS_USE_STATIS
            if (AT_RESP_OK != _at_resp.status)
            {
                _statis.at_rsp_err_cnt++;
            }
#endif
            return _at_resp.status;
        }
        else
        {
#ifdef _IS_USE_STATIS
            _statis.at_timeout_cnt++;
#endif
            return AT_RESP_TIMEOUT;
        }
    }
    else
    {
        return AT_RESP_ERROR;
    }
}

static int at_resp_parse_line_by_key(const char *key, const char *resp_expr,
                                     ...)
{
    va_list args;
    int resp_args_num = 0;
    char *pStr = NULL;
    if (pStr = strstr(_at_resp.buf, key))
    {
        va_start(args, resp_expr);
        resp_args_num = vsscanf(pStr, resp_expr, args);
        va_end(args);
    }
    return resp_args_num;
}

int aw_mqtt_get_csq()
{
//    if (!uart_is_open()) {
//        s8 res = at_uart_open();
//    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_QUERY_CSQ;
#endif
    int csq = -1;
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT+CSQ\r\n");
    if (AT_RESP_OK == rsp_sts)
    {
        if (at_resp_parse_line_by_key("+CSQ:", "+CSQ:%d", &csq) <= 0)
        {
            return -1;
        }
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return csq;
}
int aw_mqtt_set_ate(aw_at_ate_t ate)
{
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "ATE%d\r\n",
                                              (int) ate);
    return rsp_sts;
}
static int mqtt_check_net(u8 cnt)
{
    u8 i = 0;
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CHECK_NET;
#endif
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_SHORT_TIMEOUT,
                                              "AT+CEREG?\r\n");
    if (AT_RESP_OK == rsp_sts)
    {
        if (AW_OK != aw_at_response_check_key("+CEREG:0,1"))
        {
            rsp_sts = AT_RESP_ERROR;
        }
    }
    u8 csq = 0;
    while (i++ < cnt && AT_RESP_OK != rsp_sts)
    {
        aw_delay_ms(1000);
        aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
        rsp_sts = aw_at_exec_cmd(AT_SHORT_TIMEOUT, "AT+CEREG?\r\n");
        if (AT_RESP_OK == rsp_sts)
        {
#ifdef IS_USE_BC26
      if (AW_OK != aw_at_response_check_key("+CEREG: 0,1")) {
        rsp_sts = AT_RESP_ERROR;
      }
#else
            if (AW_OK != aw_at_response_check_key("+CEREG:0,1"))
            {
                rsp_sts = AT_RESP_ERROR;
                csq = aw_mqtt_get_csq();
                if (csq > 10)
                {
                    rsp_sts = AT_RESP_OK;
                }
            }
#endif
        }
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
#ifdef _IS_USE_STATIS
    if (AT_RESP_OK != rsp_sts)
        _statis.net_err_cnt++;
#endif
    return rsp_sts;
}

static int aw_mqtt_set_alive(u32 alive_time)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CONFIG;
#endif
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(
            AT_DEFAULT_TIMEOUT, "" AT_MQTT_ALIVE "" AT_END_CR_LF "",
            ConnectID_0, alive_time);
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_reboot()
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_REBOOT;
#endif
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT,
                                              "" AT_REBOOT "" AT_END_CR_LF "");
#ifdef _IS_USE_STATIS
    _statis.reset_cnt++;
#endif
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_set_will(aw_qos_t qos, u8 retain, const char *msg,
                            const char *topic)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CONFIG;
#endif
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(
            AT_DEFAULT_TIMEOUT, "" AT_MQTT_WILL "" AT_END_CR_LF "", ConnectID_0,
            qos, retain, topic, msg);
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

#define _MQTT_CONFIG_STR_SIZE 32
static char _config_str[_MQTT_CONFIG_STR_SIZE] = { '\0' };
static int aw_mqtt_set_will_f(aw_qos_t qos, u8 retain, const char *msg,
                              const char *__restrict _format, ...)
{
    int res = 0;
    _config_str[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_config_str, _MQTT_CONFIG_STR_SIZE, _format, args);
    va_end(args);
    res = aw_mqtt_set_will(qos, retain, msg, _config_str);
    _config_str[0] = '\0';
    return res;
}

static int aw_mqtt_close()
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CLOSE;
#endif
    aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT,
                                              "AT+QMTCLOSE=%d\r\n",
                                              ConnectID_0);
    if (AT_RESP_OK == rsp_sts)
    {
        if (AW_OK != aw_at_response_check_key(AT_MQTT_CLOSE_SUCC, ConnectID_0,
        AT_MQTT_RES_CODE_OK))
        {
            rsp_sts = AT_RESP_ERROR;
        }
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_open(const char *hostName, u32 port)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_OPEN;
#endif
    aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT,
                                              "AT+QMTOPEN=%d,\"%s\",%d\r\n",
                                              ConnectID_0, hostName, port);

    if (AT_RESP_OK == rsp_sts)
    {
        if (AW_OK != aw_at_response_check_key(AT_MQTT_OPEN_SUCC, ConnectID_0,
        AT_MQTT_RES_OK))
        {
            rsp_sts = AT_RESP_ERROR;
        }
    }

#ifdef _IS_USE_STATIS
    if (AT_RESP_OK != rsp_sts)
        _statis.open_err_cnt++;
#endif
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_connect(const char *clientID)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CONNECT;
#endif
    aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(
            AT_LONG_TIMEOUT, "" AT_MQTT_CONNECT "" AT_END_CR_LF "", ConnectID_0,
            clientID);

    if (AT_RESP_OK == rsp_sts)
    {
        if (AW_OK != aw_at_response_check_key(AT_MQTT_CONNECT_SUCC, ConnectID_0,
        AT_MQTT_RES_OK,
                                              AT_MQTT_RES_CODE_OK))
        {
            rsp_sts = AT_RESP_ERROR;
        }
    }
#ifdef _IS_USE_STATIS
    _statis.connect_cnt++;
    if (AT_RESP_OK != rsp_sts)
        _statis.connect_err_cnt++;
#endif
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_connect_f(const char *__restrict _format, ...)
{
    int res = 0;
    _config_str[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_config_str, _MQTT_CONFIG_STR_SIZE, _format, args);
    va_end(args);
    res = aw_mqtt_connect(_config_str);
    _config_str[0] = '\0';
    return res;
}
static int aw_mqtt_sub(aw_qos_t qos, const char *topic)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_SUB;
#endif
    aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
    at_resp_status_t rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT,
                                              "" AT_MQTT_SUB "" AT_END_CR_LF "",
                                              ConnectID_0, MsgID_1, topic, qos);

    if (AT_RESP_OK == rsp_sts)
    {
        if (AW_OK
                != aw_at_response_check_key(AT_MQTT_SUB_SUCC, ConnectID_0,
                                            MsgID_1, AT_MQTT_RES_OK, qos))
        {
            rsp_sts = AT_RESP_ERROR;
        }
    }

#ifdef _IS_USE_STATIS
    if (AT_RESP_OK != rsp_sts)
        _statis.sub_err_cnt++;
#endif
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

static int aw_mqtt_sub_f(aw_qos_t qos, const char *__restrict _format, ...)
{
    int res = 0;
    _config_str[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_config_str, _MQTT_CONFIG_STR_SIZE, _format, args);
    va_end(args);
    res = aw_mqtt_sub(qos, _config_str);
    _config_str[0] = '\0';
    return res;
}

static int aw_mqtt_nb_psm(u8 enable)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_SET_PSM;
#endif
    at_resp_status_t rsp_sts = AT_RESP_ERROR;
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT+CPSMS=%\r\n", enable);
    u8 cnt = 0;
    while (AT_RESP_OK != rsp_sts && cnt++ < 10)
    {
        aw_delay_ms(100);
        aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
        rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT+CPSMS=%\r\n", enable);
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}
static int aw_mqtt_waiting_at()
{
    if (!uart_is_open())
    {
        return -1;
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_CHECK_AT;
#endif
    at_resp_status_t rsp_sts = AT_RESP_ERROR;
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT\r\n");
    int cnt = 0;
    while (AT_RESP_OK != rsp_sts && (cnt++ < 1000))
    {
        aw_delay_ms(1000);
        aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
        rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT\r\n");
    }
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_FREE;
#endif
    return rsp_sts;
}

int aw_mqtt_net_led(u8 on)
{
    at_resp_status_t rsp_sts = AT_RESP_ERROR;
    aw_at_response_set_line(AW_MQTT_OK_LINE_NUM);
    rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT+QLEDMODE=%d\r\n", on);
    return rsp_sts;
}

static int aw_mqtt_build_net()
{
    int res = AW_ERR;
    res = aw_mqtt_waiting_at();
    res = aw_mqtt_nb_psm(1);
    res = aw_mqtt_set_ate(ATE0);
    res = aw_mqtt_net_led(0);
    res = aw_mqtt_set_alive(KeepAliveTime);

#ifndef IS_USE_NB_POWERDOWN
  char data_str[23] = {'\0'};
  snprintf(data_str, sizeof(data_str), "{i:%d,con:0,y:%d}", AW_DEV_SELF_ADDRESS,
           AW_DEV_TYPE);
  res = aw_mqtt_set_will_f(QOS1_LEASET_ONCE, NO_RETAIN, data_str,
                           "%s/%s/" AW_NOTIFY "", DevName, DevId);
#endif

    res = mqtt_check_net(100);
    if (AW_OK != res)
    {
        return AW_ERR;
    }

    res = aw_mqtt_close();
    res = aw_mqtt_open(Host, PortNum);
    if (AW_OK != res)
    {
        return AW_ERR;
    }
    res = aw_mqtt_connect_f("%s_%s", DevName, DevId);
    if (AW_OK != res)
    {
        return AW_ERR;
    }
    if (_is_sub == 1)
    {
        res = aw_mqtt_sub_f(QOS1_LEASET_ONCE, "%s/%s/" AW_TOPIC_CMD "", DevName,
                            DevId);
        if (AW_OK != res)
        {
            return AW_ERR;
        }
    }
    return AW_OK;
}

static int aw_mqtt_rebuild_net()
{
    int res = AW_ERR;
    res = mqtt_check_net(100);
    if (AW_OK != res)
    {
        return AW_ERR;
    }
    res = aw_mqtt_close();
    res = aw_mqtt_open(Host, PortNum);
    if (AW_OK != res)
    {
        return AW_ERR;
    }
    res = aw_mqtt_connect_f("%s_%s", DevName, DevId);
    if (AW_OK != res)
    {
        return AW_ERR;
    }
    if (_is_sub == 1)
    {
        res = aw_mqtt_sub_f(QOS1_LEASET_ONCE, "%s/%s/cmd", DevName, DevId);
        if (AW_OK != res)
        {
            return AW_ERR;
        }
    }
    return AW_OK;
}

static int aw_reboot_waiting()
{
    if (!uart_is_open())
    {
        return -1;
    }
    if (AW_ERR == aw_mqtt_reboot())
    {
        return AW_ERR;
    }
    return aw_mqtt_waiting_at();
}
static int aw_mqtt_connect_cnt(u8 cnt)
{
    u8 i = 0;
    while (i < cnt)
    {
        if (0 == i)
        {
            if (AW_OK == aw_mqtt_build_net())
            {
                return AW_OK;
            }
        }
        else
        {
            if (AW_OK == aw_mqtt_rebuild_net())
            {
                return AW_OK;
            }
        }
        i++;
    }
    if (AW_ERR == aw_reboot_waiting())
    {
        return AW_ERR;
    }
    if (AW_OK == aw_mqtt_build_net())
    {
        return AW_OK;
    }
    if (AW_OK == aw_mqtt_rebuild_net())
    {
        return AW_OK;
    }
    return AW_ERR;
}
#include <sys_ctrl.h>
static int aw_mqtt_connecting()
{
#ifdef _IS_USE_STATUS
    _status.is_building = true;
#endif
    if (!uart_is_open())
    {
        return -1;
    }
    u8 cnt = 0;
    while (AW_OK != aw_mqtt_connect_cnt(5) && cnt < 5)
    {
        aw_delay_ms(300);
        cnt++;
    }
    if (cnt > 5)
    {
        SysCtrlSystemReset();
    }
#ifdef _IS_USE_STATUS
    _status.is_building = false;
#endif
#ifdef _IS_USE_STATIS
    _statis.connect_ok_cnt++;
#endif
    return 0;
}

static s8 aw_mqtt_pub(aw_qos_t qos, u8 retain, const char *topic,
                      const char *msg)
{
#ifdef _IS_USE_STATUS
    _status.mqtt_sts = MQTT_STS_PUB;
#endif
#ifdef IS_USE_BC26
  int msgId = MsgID_1;
  if (QOS0_MOST_ONECE == qos) {
    msgId = MsgID_0;
  }
  aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
  at_resp_status_t rsp_sts =
      aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%s\r\n",
                     ConnectID_0, msgId, (int)qos, (int)retain, topic, msg);
  if (AT_RESP_OK == rsp_sts) {
    if (AW_OK != aw_at_response_check_key(AT_MQTT_PUB_SUCC, ConnectID_0, msgId,
                                          AT_MQTT_RES_OK)) {
      rsp_sts = AT_RESP_ERROR;
    }
  }
  if (AT_RESP_OK == rsp_sts) {
    return AW_OK;
  } else {
#ifdef _IS_USE_STATIS
    _statis.pub_err_cnt++;
#endif
    return AW_ERR;
  }
#else
    int msg_len = strlen(msg);
    aw_at_response_set_end_sign('>');
    int msgId = MsgID_1;
    if (QOS0_MOST_ONECE == qos)
    {
        msgId = MsgID_0;
    }
    at_resp_status_t rsp_sts = aw_at_exec_cmd(
            AT_DEFAULT_TIMEOUT, "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d\r\n",
            ConnectID_0, msgId, (int) qos, (int) retain, topic, msg_len);
    if (AT_RESP_OK == rsp_sts)
    {
        aw_at_response_set_line(AW_MQTT_SUCC_LINE_NUM);
        rsp_sts = aw_at_exec_cmd(AT_DEFAULT_TIMEOUT, msg);
        if (AT_RESP_OK == rsp_sts)
        {
            if (AW_OK
                    != aw_at_response_check_key(AT_MQTT_PUB_SUCC, ConnectID_0,
                                                msgId, AT_MQTT_RES_OK))
            {
                rsp_sts = AT_RESP_ERROR;
            }
        }
        if (AT_RESP_OK == rsp_sts)
        {
#ifdef IS_USE_NB_POWERDOWN
            // aw_mqtt_nb_close();
#endif
            return AW_OK;
        }
        else
        {
#ifdef _IS_USE_STATIS
            _statis.pub_err_cnt++;
#endif
#ifdef IS_USE_NB_POWERDOWN
            // aw_mqtt_nb_close();
#endif
            return AW_ERR;
        }
    }
    else
    {
#ifdef _IS_USE_STATIS
        _statis.pub_err_cnt++;
#endif
#ifdef _IS_USE_STATUS
        _status.mqtt_sts = MQTT_STS_FREE;
#endif
        return AW_ERR;
    }
#endif
}
#define _MQTT_PUB_TRY_CNT 3
static s8 aw_mqtt_pubing(aw_qos_t qos, u8 retain, const char *topic,
                         const char *msg)
{
    if (!uart_is_open())
    {
        return -1;
    }
    s8 isOK = aw_mqtt_pub(qos, retain, topic, msg);
    u8 cnt = 0;
    while (AW_OK != isOK)
    {
        aw_delay_ms(400);
        isOK = aw_mqtt_pub(qos, retain, topic, msg);
        if (AW_OK != isOK)
        {
            if (cnt > 3)
            {
                aw_reboot_waiting();
            }
            if (cnt > 5)
            {
                return -5;
            }
            aw_mqtt_connecting();
            isOK = aw_mqtt_pub(qos, retain, topic, msg);
            cnt++;
        }
    }
    return isOK;
}

int aw_mqtt_pub_f(aw_pub_type_t pub_type, aw_qos_t qos, u8 retain,
                  const char *topic, const char *__restrict _format, ...)
{
    int res = 0;
    _payload[0] = '\0';
    va_list args;
    va_start(args, _format);
    vsnprintf(_payload, _AW_AT_PAYLOAD_SIZE, _format, args);
    va_end(args);
    if (AW_PUB_MODAL == pub_type)
    {
        res = aw_mqtt_pubing(qos, retain, topic, _payload);
    }
    else
    {
        res = aw_mqtt_pub(qos, retain, topic, _payload);
    }

    _payload[0] = '\0';
    return res;
}

int aw_mqtt_pub_add_f(const char *__restrict _format, ...)
{
    int res = 0;
    int len = strlen(_payload);
    int size = _AW_AT_PAYLOAD_SIZE;
    va_list args;
    va_start(args, _format);
    int cnt = vsnprintf(&_payload[len], size - len, _format, args);
    va_end(args);
    if ((cnt + len) > size)
    {
        res = -1;
    }
    return res;
}

int aw_mqtt_pub_flush(aw_pub_type_t pub_type, aw_qos_t qos, u8 retain,
                      const char *topic)
{
    int res = 0;
    int len = strlen(_payload);
    if (len < 1)
    {
        return -2;
    }
    if (AW_PUB_MODAL == pub_type)
    {
        res = aw_mqtt_pubing(qos, retain, topic, _payload);
    }
    else
    {
        res = aw_mqtt_pub(qos, retain, topic, _payload);
    }
    _payload[0] = '\0';
    return res;
}

int aw_mqtt_pub_status()
{
    int res = 0;
#ifdef _IS_USE_STATIS
    res =
            aw_mqtt_pub_f(
                    AW_PUB_MODAL,
                    QOS0_MOST_ONECE,
                    NO_RETAIN,
                    aw_mqtt_topic_get_f("" AW_MQTT_DEV_NAME "/%s/" AW_STATUS "",
                    AW_MQTT_DEV_ID),
                    "{" CONNECT_CNT ":%d," RESET_NB ":%d,cnc:%d,pbe:%d," CSQ ":%d," DEV_TYPE
                    ":%d}",
                    _statis.connect_ok_cnt, _statis.reset_cnt,
                    _statis.connect_cnt, _statis.pub_err_cnt, aw_mqtt_get_csq(),
                    AW_DEV_TYPE);
//#ifdef IS_USE_OLED
//    aw_oled_printf(0, 2, AW_OLED_F6x8,
//                   "CON:%d,RST:%d,CNC:%d,RVC:%d,LOC:%d,SBE:%d,OPE:%d,CNE:%d,"
//                   "CSQ:%d",
//                   _statis.connect_ok_cnt, _statis.reset_cnt,
//                   _statis.connect_cnt, _statis.recv_urc_cnt,
//                   _statis.lost_urc_cnt, _statis.sub_err_cnt,
//                   _statis.open_err_cnt, _statis.connect_err_cnt,
//                   aw_mqtt_get_csq());
//#endif
#else
  res = aw_mqtt_pub_f(AW_PUB_MODAL, QOS0_MOST_ONECE, NO_RETAIN, "sts",
                      "{CSQ:%d}", aw_mqtt_get_csq());
//#ifdef IS_USE_OLED
//    aw_oled_printf(0, 2, AW_OLED_F6x8, "Csq:%d", aw_mqtt_get_csq());
//#endif
#endif
    return res;
}

int aw_mqtt_net_building(aw_mqtt_mode_t mode, u8 isPub, char *dev_name)
{
    if (!uart_is_open())
    {
        return -1;
    }
    aw_mqtt_connecting();
    if (isPub > 0)
    {
#ifdef _IS_USE_STATIS
        return aw_mqtt_pub_f(
                AW_PUB_MODAL, QOS1_LEASET_ONCE, NO_RETAIN,
                aw_mqtt_topic_get_f("%s/%s/" AW_NOTIFY "", dev_name,
                AW_MQTT_DEV_ID),
                "{i:%d," CONNECT_CNT ":%d," RESET_NB ":%d," CSQ ":%d," DEV_TYPE
                ":%d,mod:%d," AW_VERSION ":\"%s\"}",
                AW_DEV_SELF_ADDRESS, _statis.connect_ok_cnt, _statis.reset_cnt,
                aw_mqtt_get_csq(), AW_DEV_TYPE, mode, AW_firmware_version);
#else
    return aw_mqtt_pub_f(AW_PUB_MODAL, QOS1_LEASET_ONCE, NO_RETAIN, "nty",
                         "{\"CON\":\"%d\"}", 1);
#endif
    }
    return 0;
}

int aw_mqtt_init(AwCallBack_char_t funcRecv, AwCallBack_char_t funcStat)
{
    _resp_sem_init();
    aw_at_urc_table_set(funcRecv, funcStat);
    return 0;
}

int aw_mqtt_reset(u8 is_connect)
{
    aw_mqtt_reboot();
    if (is_connect)
    {
        aw_mqtt_net_building(AW_MQTT_NORMAL, 1, AW_MQTT_DEV_NAME);
    }
    return 0;
}

s8 aw_mqtt_nb_close()
{
    s8 res = 0;
    res = aw_mqtt_close();
    res = at_uart_close();
    return res;
}

s8 aw_mqtt_nb_open()
{
    s8 res = 0;
    if (!uart_is_open())
    {
        res = at_uart_open();
    }
    res = aw_mqtt_connecting();
    return res;
}
