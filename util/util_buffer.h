/*
 * util_buffer.h
 *
 *  Created on: 2020��9��7��
 *      Author: admin
 */

#ifndef UTIL_UTIL_BUFFER_H_
#define UTIL_UTIL_BUFFER_H_
#include "util_def.h"
#define BUF_SIZE 30
typedef s8 BufData_t;
typedef struct {
    BufData_t buf[BUF_SIZE];
    u8        head;  //��
    u8        tail;  //д
    u8        next;
} BufDatas_t;

AW_BOOL putBuf(BufDatas_t* buf, BufData_t* data);
AW_BOOL readBuf(BufDatas_t* buf, BufData_t* data);
u8      getBufCnt(BufDatas_t* buf);
AW_BOOL checkFullBuf(BufDatas_t* buf);
AW_BOOL checkNullBuf(BufDatas_t* buf);
AW_BOOL resetBuf(BufDatas_t* buf);

#endif /* UTIL_UTIL_BUFFER_H_ */
