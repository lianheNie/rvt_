
#include <aw_config.h>
#include <aw_mqtt.h>
#include <stdio.h>
#include <string.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Error.h>

#include "uart_driver.h"
#include "util_def.h"
#include "util_delay.h"
#include "util_other.h"
#include "util_printf.h"
// static Semaphore_Struct _appSemS;

static Semaphore_Handle _appSem = NULL;

static bool _app_sem_init() {
  Semaphore_Params params;
  Error_Block eb;
  Semaphore_Params_init(&params);
  Error_init(&eb);
  _appSem = Semaphore_create(0, &params, &eb);
  if (_appSem == NULL) {
    // System_abort("Semaphore creation failed");
  }
  return true;
}

static void _uart_read_cb(void* _buf, size_t _size) {
  if (NULL != _appSem) {
    Semaphore_post(_appSem);
  }
}

static aw_uart_rx_t _uart_rx = {.buf = {'\0'},
                                .size = AW_UART_RX_LINE_BUF,
                                .len = 0,
                                .ch = '\0',
                                .last_ch = '\0'};

static void _uart_rx_reset() {
  _uart_rx.buf[0] = '\0';
  _uart_rx.ch = '\0';
  _uart_rx.last_ch = '\0';
  _uart_rx.len = 0;
}

static const aw_at_urc_t* _urc = NULL;

static bool _check_urc() {
  _urc = aw_at_check_urc(&_uart_rx);
  if (NULL != _urc) {
    if (NULL != _urc->func) {
      _urc->func(_uart_rx.buf);
    }
    _uart_rx_reset();
  }
  _urc = NULL;
  return true;
}

static aw_at_response_t* _resp = NULL;

static bool _check_resp() {
  char last_ch = _uart_rx.last_ch;
  char ch = _uart_rx.ch;
  bool is_end_sign = (0 != _resp->end_sign) && (ch == _resp->end_sign);
  bool is_new_line = (ch == '\n' && last_ch == '\r');
  if (is_new_line || is_end_sign) {
    if (is_new_line) {
      if (_resp->is_set) {
        bool is_ok = false;
        int free_len = AW_AT_RESPONSE_BUF_SIZE - _resp->buf_len - 1;
        int len = (_uart_rx.len < free_len) ? _uart_rx.len : free_len;
        memcpy(_resp->buf + _resp->buf_len, _uart_rx.buf, len);
        _resp->buf_len += len;
        _resp->buf[_resp->buf_len] = '\0';
        _resp->line_cnt++;
        if (_resp->line_num == 0 &&
            memcmp(_uart_rx.buf, AT_RESP_END_OK, strlen(AT_RESP_END_OK)) == 0) {
          _resp->status = AT_RESP_OK;
          is_ok = true;
        } else if (strstr(_uart_rx.buf, AT_RESP_END_ERROR) ||
                   (memcmp(_uart_rx.buf, AT_RESP_END_FAIL,
                           strlen(AT_RESP_END_FAIL)) == 0)) {
          _resp->status = AT_RESP_ERROR;
          is_ok = true;
        } else if (_resp->line_num && _resp->line_cnt == _resp->line_num) {
          _resp->status = AT_RESP_OK;
          is_ok = true;
        } else {
        }
        if (is_ok) {
          _resp->is_set = false;
          if (NULL != _resp->func) {
            _resp->func();
          }
        }
      }
    } else if (is_end_sign) {
      if (_resp->is_set) {
        _resp->is_set = false;
        _resp->end_sign = '\0';
        _resp->status = AT_RESP_OK;
        if (NULL != _resp->func) {
          _resp->func();
        }
      }
    }
    _uart_rx_reset();
  }
  return true;
}

static void _proc_uart_rx_event() {
  char ch = _uart_rx.ch;
  if (_uart_rx.len < _uart_rx.size - 1) {  //防止越界
    _uart_rx.buf[_uart_rx.len++] = ch;
    _uart_rx.buf[_uart_rx.len] = '\0';
    _check_urc();   //检测URC
    _check_resp();  //检测应答
  } else {
    _uart_rx_reset();
  }
  _uart_rx.last_ch = ch;
  uart_read(&_uart_rx.ch, 1);
}
s8 at_uart_open() {
  s8 res = 0;
  if (!uart_is_open()) {
    res = uart_open();
  } else {
    return -1;
  }
  _uart_rx_reset();
  uart_read(&_uart_rx.ch, 1);  //串口读取一个字
  return res;
}
s8 at_uart_close() {
  s8 res = 0;
  res = uart_close();
  return res;
}

static void _uart_init() {
  uart_init(9600, _uart_read_cb);  //串口初始化
  _uart_rx_reset();
  _resp = aw_at_response_get();
#if defined(IS_USE_NB_POWERDOWN)
  at_uart_close();
#else
  uart_read(&_uart_rx.ch, 1);  //串口读取一个字
#endif
}
static void _app_init() {
  _app_sem_init();
  _uart_init();
}
static void _app_process() {
  while (1) {
    Semaphore_pend(_appSem, BIOS_WAIT_FOREVER);
    _proc_uart_rx_event();
  }
}

static void _app_thread(UArg arg0, UArg arg1) {
  _app_init();
  _app_process();
}

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#define _TASK_STACK_SIZE 512
static Task_Struct _task_struct;
static Char _task_stack[_TASK_STACK_SIZE];
void aw_at_client_task_init() {
  Task_Params taskParams;
  Task_Params_init(&taskParams);
  taskParams.stackSize = _TASK_STACK_SIZE;
  taskParams.stack = &_task_stack;
  taskParams.priority = 1;
  Task_construct(&_task_struct, (Task_FuncPtr)_app_thread, &taskParams, NULL);
}
