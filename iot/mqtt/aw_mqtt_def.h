/*
 * aw_mqtt_def.h
 *
 *  Created on: 2020年9月25日
 *      Author: admin
 */

#ifndef STACK_AW_MQTT_DEF_H_
#define STACK_AW_MQTT_DEF_H_
#include "util_def.h"
#include "aw_config.h"
#define AT_OK                  "OK"
#define AT_ERROR               "ERROR"
#define AT_RESP_END_FAIL       "FAIL"
#define AT_END_CR_LF           "\r\n"
#define AT_TEST                "AT"
#define AT_ECHO_OFF            "ATE0"
#define AT_ECHO_ON             "ATE1"
#define AT_QREGSWT_2           "AT+QREGSWT=2"
#define AT_AUTOCONNECT_DISABLE "AT+NCONFIG=AUTOCONNECT,FALSE"
#define AT_REBOOT              "AT+NRB"
#define AT_QRST                "AT+QRST=1"

#define AT_NBAND                    "AT+NBAND=%d"
#define AT_FUN_ON                   "AT+CFUN=1"
#define AT_LED_ON                   "AT+QLEDMODE=1"
#define AT_EDRX_OFF                 "AT+CEDRXS=0,5"
#define AT_PSM_OFF                  "AT+CPSMS=0"
#define AT_RECV_AUTO                "AT+NSONMI=2"
#define AT_UE_ATTACH                "AT+CGATT=1"
#define AT_UE_DEATTACH              "AT+CGATT=0"
#define AT_QUERY_IMEI               "AT+CGSN=1"
#define AT_QUERY_CSQ                "AT+CSQ"
#define AT_QUERY_CSQ_SUCC           "+CSQ:%d,%d"
#define AT_QUERY_IMSI               "AT+CIMI"
#define AT_QUERY_STATUS             "AT+NUESTATS"
#define AT_QUERY_REG                "AT+CEREG?"
#define AT_QUERY_IPADDR             "AT+CGPADDR"
#define AT_QUERY_ATTACH             "AT+CGATT?"
#define AT_UE_ATTACH_SUCC           "+CGATT:1"
#define AT_MQTT_WILL                "AT+QMTCFG=\"will\",%d,1,%d,%d,\"%s\",\"%s\""
#define AT_MQTT_AUTH                "AT+QMTCFG=\"aliauth\",%d,\"%s\",\"%s\",\"%s\""
#define AT_MQTT_ALIVE               "AT+QMTCFG=\"keepalive\",%d,%d"
#define AT_MQTT_OPEN                "AT+QMTOPEN=%d,\"%s\",%d"
#define AT_MQTT_OPEN_SUCC           "+QMTOPEN: %d,%d"
#define AT_MQTT_CLOSE               "AT+QMTCLOSE=%d"
#define AT_MQTT_CLOSE_SUCC          "+QMTCLOSE: %d,%d"
#define AT_MQTT_CONNECT_USER_PASSWD "AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\""
#define AT_MQTT_CONNECT             "AT+QMTCONN=%d,\"%s\""
#define AT_MQTT_CONNECT_SUCC        "+QMTCONN: %d,%d,%d"
#define AT_MQTT_DISCONNECT          "AT+QMTDISC=%d"
#define AT_MQTT_DISCONNECT_SUCC     "+QMTDISC:%d,%d"

#define AT_MQTT_SUB        "AT+QMTSUB=%d,%d,\"%s\",%d"
#define AT_MQTT_SUB_SUCC   "+QMTSUB: %d,%d,%d,%d"
#define AT_MQTT_UNSUB      "AT+QMTUNS=%d,%d,\"%s\""
#define AT_MQTT_UNSUB_SUCC "+QMTUNS: %d,%d,%d"
#define AT_MQTT_PUB        "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%s"
#define AT_MQTT_PUB_LEN    "AT+QMTPUB=%d,%d,%d,%d,\"%s\",%d"
#define AT_MQTT_PUB_SUCC   "+QMTPUB: %d,%d,%d"

#define AT_QMTSTAT_CLOSED          1
#define AT_QMTSTAT_PINGREQ_TIMEOUT 2
#define AT_QMTSTAT_CONNECT_TIMEOUT 3
#define AT_QMTSTAT_CONNACK_TIMEOUT 4
#define AT_QMTSTAT_DISCONNECT      5
#define AT_QMTSTAT_WORNG_CLOSE     6
#define AT_QMTSTAT_INACTIVATED     7

#define AT_QMTRECV_DATA "+QMTRECV: %d,%d,\"%s\",\"%s\""

#define AT_MQTT_RES_OK          0
#define AT_MQTT_RES_CODE_OK     0
#define AT_CLIENT_RECV_BUFF_LEN 256
#define AT_DEFAULT_TIMEOUT      10000
#define AT_SHORT_TIMEOUT        5000
#define AT_LONG_TIMEOUT         20000
#define AW_MQTT_RETAIN          1
#define AW_MQTT_NO_RETAIN       0
#define AW_MQTT_TOPIC_MAX_LEN   32
#define AW_MQTT_MESSAGE_MAX_LEN 128

#define AW_MQTT_SUCC_LINE_NUM 4
#define AW_MQTT_OK_LINE_NUM   0
#define AT_RESP_END_OK        "OK"
#define AT_RESP_END_ERROR     "ERROR"
#define AT_RESP_END_FAIL      "FAIL"
#define AT_END_CR_LF          "\r\n"

#define AW_TOPIC_CMD "cmd"
#define AW_RESULT    "res"
#define AW_STATUS    "sts"
#define AW_NOTIFY    "nfy"
#define AW_ACK       "ack"
#define AW_DEV_RS    "RS"  //开关
#define AW_DEV_RD    "RD"  //门磁
#define AW_DEV_RT    "RT"  //溫度
#define AW_DEV_RV    "RV"  //震動
#define AW_DEV_RP    "RP"  //红外
#define AW_DEV_RB    "RB"  //电源

//#define AW_MQTT_TOPIC_CMD    AW_MQTT_DEV_NAME / AW_DEV_SELF_ADDRESS / AW_TOPIC_ACK
//#define AW_MQTT_TOPIC_RESULT AW_MQTT_DEV_NAME / AW_DEV_SELF_ADDRESS / AW_TOPIC_RESULT
//#define AW_MQTT_TOPIC_STATUS AW_MQTT_DEV_NAME / AW_DEV_SELF_ADDRESS / AW_TOPIC_STATUS
//#define AW_MQTT_TOPIC_NOTIFY AW_MQTT_DEV_NAME / AW_DEV_SELF_ADDRESS / AW_NOTIFY
//#define AW_MQTT_TOPIC_ACK    AW_MQTT_DEV_NAME / AW_DEV_SELF_ADDRESS / AW_TOPIC_ACK

#define CH1 "c1"
#define CH2 "c2"
#define CH3 "c3"
#define QOS "qos"
#define ATE "ate"

#define USC         "usc"
#define CSQ         "csq"
#define STS         "sts"
#define BAT         "bat"
#define BAT_CUR     "bsc"
#define BAT_VOL     "bsv"
#define SELECT_ID   "i"
#define RS_DELAY    "dt"
#define RESET_NB    "rsn"
#define RESET_CPU   "rsc"
#define SET_LED     "led"
#define SET_NET_LED "ned"
#define GATE_PAIR   "par"
#define SENSOR_PAIR "sar"
#define CONNECT_CNT "con"
#define DEV_TYPE    "y"
#define AW_VERSION  "ver"
#define SERVER_IP   "sip"

#define RVT_REPORT_TIME "rpt"
#define GET_VIBR        "grv"
#define GET_TEMP        "grt"
#define VIBR_BIAS       "vbs"
#define TEMP_BIAS       "tbs"
enum at_resp_status {
    AT_RESP_OK        = 0,  /* AT response end is OK */
    AT_RESP_ERROR     = -1, /* AT response end is ERROR */
    AT_RESP_TIMEOUT   = -2, /* AT response is timeout */
    AT_RESP_BUFF_FULL = -3, /* AT response buffer is full */
    AT_RESP_NULL      = -4, /* AT response buffer is full */
};
typedef enum at_resp_status at_resp_status_t;

#define AW_AT_RESPONSE_BUF_SIZE 128
#define AW_AT_RESPONSE_KEY_SIZE 32
typedef struct _aw_at_response_t {
    char              buf[AW_AT_RESPONSE_BUF_SIZE];
    char              key[AW_AT_RESPONSE_KEY_SIZE];
    u8                buf_size;
    u8                buf_len;
    u8                line_num;  //设定的应答行数
    u8                line_cnt;  //当前接收应答行数
    char              end_sign;
    bool              is_set;
    AwCallBack_void_t func;
    at_resp_status_t  status;
} aw_at_response_t;

typedef enum { Urc_RECV, Urc_STAT, Urc_CNT } E_Urc;
typedef struct _aw_at_urc {
    const char*       cmd_prefix;
    const char*       cmd_suffix;
    AwCallBack_char_t func;
} aw_at_urc_t;

#define AW_UART_RX_LINE_BUF 128
typedef struct aw_uart_rx {
    char ch;
    char last_ch;
    char buf[AW_UART_RX_LINE_BUF];
    u8   len;
    u8   size;
} aw_uart_rx_t;

typedef enum {
    QOS0_MOST_ONECE,    // At most once
    QOS1_LEASET_ONCE,   // At least once
    QOS2_EXACTLY_ONECE  // Exactly once
} aw_qos_t;

typedef enum {
    AW_PUB_NOMODAL,  // 非模态
    AW_PUB_MODAL     // 模态
} aw_pub_type_t;

typedef enum {
    NO_RETAIN,    // At most once
    MQTT_RETAIN,  // At least once
} aw_retain_t;
typedef enum {
    ATE0,  // At most once
    ATE1,  // At least once
} aw_at_ate_t;

typedef enum {
    ConnectID_0,
    ConnectID_1,
    ConnectID_2,
    ConnectID_3,
    ConnectID_4,
    ConnectID_5
} E_ConnectID;
typedef enum { MsgID_0, MsgID_1, MsgID_2, MsgID_3, MsgID_4, MsgID_5 } E_MsgID;

typedef struct _aw_at_statistics_t {
    u16 connect_ok_cnt;
    u16 connect_cnt;
    u16 reset_cnt;
    u16 pub_err_cnt;
    u16 at_rsp_err_cnt;
    u16 at_timeout_cnt;
    u16 recv_urc_cnt;
    u16 lost_urc_cnt;
    u16 sub_err_cnt;
    u16 open_err_cnt;
    u16 connect_err_cnt;
    u16 net_err_cnt;
} aw_at_staist_t;

typedef enum {
    MQTT_STS_BUILDING,   //网络建立
    MQTT_STS_PUB,        //发布数据
    MQTT_STS_SUB,        //订阅消息
    MQTT_STS_OPEN,       //订阅消息
    MQTT_STS_CLOSE,      //订阅消息
    MQTT_STS_CHECK_AT,   //订阅消息
    MQTT_STS_QUERY_CSQ,  //订阅消息
    MQTT_STS_CONNECT,    //连接服务器
    MQTT_STS_CONFIG,     //配置信息
    MQTT_STS_REBOOT,     //配置信息
    MQTT_STS_CHECK_NET,  //查询网络
    MQTT_STS_SET_PSM,    //订阅消息
    MQTT_STS_FREE
} aw_mqtt_status_t;

typedef struct _aw_at_status_t {
    aw_mqtt_status_t mqtt_sts;
    bool             is_building;
} aw_at_status_t;

typedef struct _aw_mqtt_config_t {
    const char* host;
    const char* dev_name;
    const char* dev_id;
    aw_qos_t    qos;
} aw_mqtt_config_t;
#endif /* STACK_AW_MQTT_DEF_H_ */
