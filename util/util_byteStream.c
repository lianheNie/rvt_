/*
 * util_byteStream.c
 *
 *  Created on: 2020��9��8��
 *      Author: admin
 */
#include "util_byteStream.h"
#include <stdlib.h>
static s32 readOffset(struct _ByteStream_t* stream, u8* rBuffer, s32 rOffset, s32 rSize) {
    s32 avaLen = stream->ops.baseOps.availableRead(stream);
    //�ж϶�ȡ�Ƿ�Խ�磬��¼�ɶ�ȡ����
    s32 rLen = 0;
    avaLen   = rSize <= avaLen ? rSize : avaLen;
    while (avaLen != 0) {
        s32 avaCopySize = (s32)(stream->rBuf.size - stream->rBuf.sp);
        s32 copySize    = avaLen <= avaCopySize ? avaLen : avaCopySize;
        //�����ֽ�����
        memcpy(rBuffer + rOffset, (void*)(stream->rBuf.buf + stream->rBuf.sp), copySize);
        //ƫ�ƶ�ȡָ��
        stream->rBuf.sp = (stream->rBuf.sp + copySize) % stream->rBuf.size;
        //�ƶ��Ѷ�ȡָ��
        rOffset += copySize;
        rLen += copySize;
        //���ȼ�ȥ�Ѷ�ȡ����
        avaLen -= copySize;
    }
    return rLen;
}
static s32 read(struct _ByteStream_t* stream, u8* rBuffer, s32 rSize) {
    return stream->ops.baseOps.readOffset(stream, rBuffer, 0, rSize);
}
static s32 readAll(struct _ByteStream_t* stream, u8* rBuffer, s32 maxSize) {
    int32_t ava = stream->ops.baseOps.availableRead(stream);
    return stream->ops.baseOps.read(stream, rBuffer, ava <= maxSize ? ava : maxSize);
}

static u8 readByte(struct _ByteStream_t* stream) {
    uint8_t byte = 0;
    stream->ops.baseOps.read(stream, &byte, 1);
    return byte;
}
static s8 readStr(struct _ByteStream_t* stream, char* str, const char* splitStr, s32* size,
                  bool keepSplitStr) {
    int32_t sizeSp   = stream->ops.peekStrPos(stream, splitStr);
    int32_t splitLen = strlen(splitStr);
    if (sizeSp == -1) {
        return -1;
    }
    if (keepSplitStr) {
        sizeSp += splitLen;
    }
    //�жϴ�С�Ƿ�����ַ�������+\0
    if ((sizeSp + 1) >= *size) {
        *size = sizeSp;
        return -2;
    }

    if (stream->ops.baseOps.read(stream, (uint8_t*)str, sizeSp) != sizeSp) {
        return -3;
    }
    str[sizeSp] = '\0';
    *size       = sizeSp;
    return 0;
}

static s32 writeOffset(struct _ByteStream_t* stream, u8* wBuffer, s32 wOffset, s32 wSize) {
    s32 wLen = 0;
    // int64_t last = uTick.Millis();
    while (wSize != 0) {
        s32 singleTouch = stream->ops.touchOffset(stream, wBuffer, wOffset, wSize);
        if (singleTouch == -1) {
            return -1;
        }
        if (singleTouch == 0) {
            s32 buffered = stream->ops.buffededWrite(stream);
            if (buffered == 0) {
                //                if ((uTick.Millis() - last) > _writeTimeoutSpan) {
                //                    //д�볬ʱ
                //                    _writeTimoutFlag = true;
                //                    return -1;
                //                }
                //
                continue;
            } else {
                //��֮ǰ����õ����ݣ�����Flush
                // ULOG_D("Write: there has buffered data->%d", int(buffered));
            }
        }

        if (!stream->ops.flushWrite(stream)) {
            // ULOG_W("Write: Flush timeout or error");
            //���䳬ʱ�����˴���
            return -1;
        }

        // last = uTick.Millis();
        wLen += singleTouch;
        wOffset += singleTouch;
        wSize -= singleTouch;
    }
    return wLen;
}
static s32 write(struct _ByteStream_t* stream, u8* wBuffer, s32 wSize) {
    return stream->ops.baseOps.writeOffset(stream, wBuffer, 0, wSize);
}
static s32 writeByte(struct _ByteStream_t* stream, u8 wData) {
    return stream->ops.baseOps.writeOffset(stream, &wData, 0, 1);
}
static s32 writeStr(struct _ByteStream_t* stream, char* str) {
    return stream->ops.baseOps.writeOffset(stream, (uint8_t*)str, 0, strlen(str));
}
static s32 Printf(struct _ByteStream_t* stream, const char* format, ...) {
    s32     wLen;
    va_list va;
    va_start(va, format);
    wLen = stream->ops.baseOps.vPrintf(stream, format, va);
    va_end(va);
    return wLen;
}

// static s32 vPrintf(struct _ByteStream_t* stream, const char* format, va_list va) {
//    s32    wLen;
//    char   buf[64];                    //Ĭ��ʹ��64�ֽڵ�������Ϊ���ͻ���
//    size_t bufSize = sizeof(buf) - 1;  //����\0�ռ�
//    int    cnt     = vsnprintf(buf, bufSize, format, va);
//    wLen           = stream->ops.baseOps.write(stream, (uint8_t*)buf, bufSize);
//    return wLen;
//}

static s32 vPrintf(struct _ByteStream_t* stream, const char* format, va_list va) {
    int32_t wLen;
    char*   buf = (char*)&stream->tBuf.buf[stream->tBuf.ep];  //ֱ��ʹ��txBuf��Ϊ���壬����Ч�����
    size_t  bufSize =
        stream->tBuf.size - stream->tBuf.ep - 1;  //���嵽����ĩβ�Ŀ��ÿռ��С,����\0�ռ�
    int cnt = vsnprintf(buf, bufSize, format, va);
    stream->tBuf.ep += cnt;
    wLen = cnt;
    stream->ops.flushWrite(stream);
    return wLen;
}

static s32 availableRead(struct _ByteStream_t* stream) {
    return stream->rBuf.ops.getUsed(&stream->rBuf);
}
static s32 availableWrite(struct _ByteStream_t* stream) {
    return stream->tBuf.ops.getUsed(&stream->tBuf);
}

static s32 discardRead(struct _ByteStream_t* stream, s32 num) {
    s32 avaLen = stream->rBuf.ops.getUsed(&stream->rBuf);
    if ((num != 0) && (num < avaLen)) {
        //����������ֽ�С�ڻ���ʣ���ֽ�����������Ӧ���ֽڲ��˳�
        stream->rBuf.sp = (stream->rBuf.sp + num) % stream->rBuf.size;
        return num;
    } else {
        //������������
        stream->rBuf.sp = stream->rBuf.ep;
        return avaLen;
    }
}
static s32 discardWrite(struct _ByteStream_t* stream, s32 num) {
    int32_t avaLen = stream->tBuf.ops.getUsed(&stream->tBuf);
    if ((num != 0) && (num < avaLen)) {
        //����������ֽ�С�ڻ���ʣ���ֽ�����������Ӧ���ֽڲ��˳�
        stream->tBuf.sp = (stream->tBuf.sp + num) % stream->tBuf.size;
        return num;
    } else {
        //������������
        stream->tBuf.sp = stream->tBuf.ep;
        return avaLen;
    }
}
static bool isWriteIdle(struct _ByteStream_t* stream) { return stream->tBuf.ep == stream->tBuf.sp; }
static bool waitWriteIdle(struct _ByteStream_t* stream, int64_t timeoutMs) { return true; }
static s8   readLine(struct _ByteStream_t* stream, char* line, s32* size) {
    return stream->ops.baseOps.readStr(stream, line, "\r\n", size, true);
}
static u8 peekWithSp(struct _ByteStream_t* stream, int32_t sp) { return stream->rBuf.buf[sp]; }
static u8 peek(struct _ByteStream_t* stream) {
    return stream->ops.peekWithSp(stream, stream->rBuf.sp);
}
static u8 peekNextDigital(struct _ByteStream_t* stream, s32 sp, u8 ignore) {
    //�鿴��ǰ�����ֽ�
    u8 data = stream->ops.peekWithSp(stream, sp);
    //���������ַ�Ϊ '-','+','0'-'9','.'��detectDecimalΪtrue��ʱ����
    if ((data == '-') || (data == '+') || ((data >= '0') && (data <= '9')) || (data == '.') ||
        (data == ignore)) {
        return data;
    }
    return 0;
}
static s32 peekStrPos(struct _ByteStream_t* stream, const char* str) {
    int32_t i           = 0;
    int32_t machEp      = 0;
    bool    firsttMatch = false;
    bool    mach        = false;
    s32     ep          = 0;
    for (ep = stream->rBuf.sp; ep != stream->rBuf.ep; ep = (ep + 1) % stream->rBuf.size) {
        if (!firsttMatch) {
            //����ƥ��ĵ�һ���ֽ�
            if (stream->rBuf.buf[ep] == str[0]) {
                //��һ���ֽ�ƥ��
                firsttMatch = true;
                i           = 1;
                machEp      = ep;

                if (str[i] == '\0') {
                    //�ַ���ֻ��һ���ֽڣ�����ƥ��
                    //ȫ�ַ���ƥ��
                    mach = true;
                    break;
                }
            }
        } else {
            if (stream->rBuf.buf[ep] == str[i++]) {
                if (str[i] == '\0') {
                    //��ǰƥ����ֽ������һ���ֽ�
                    //ȫ�ַ���ƥ��
                    mach = true;
                    break;
                }
            } else {
                if (stream->rBuf.buf[ep] == str[0]) {
                    //�����µ��ֽ�ͷ����λָ��
                    i      = 1;
                    machEp = ep;
                } else {
                    firsttMatch = false;
                }
            }
        }
    }
    if (mach) {
        return stream->rBuf.ops.getUsedWithEp(&stream->rBuf, machEp);
    } else {
        return -1;
    }
}

/////write fuc///////
static s32 touchOffset(struct _ByteStream_t* stream, u8* wBuffer, s32 wOffset, s32 wSize) {
    int32_t wLen     = 0;
    int32_t freeSize = stream->tBuf.ops.getFree(&stream->tBuf);
    //������Է��͵��ֽڴ�С
    int32_t copySize   = wSize <= freeSize ? wSize : freeSize;
    int32_t singleSize = 0;
    //�����ֽڵ�������
    while (copySize != 0) {
        int32_t tail = stream->tBuf.size - stream->tBuf.ep;
        // sp��ȡ˲����Ҫ�浵(epֻ���������޸ģ���ȫ)����ֹ�жϰ�ȫ���⡣����ֻ��ȡ��һ�Σ����Բ���Ҫ����
        singleSize =
            (stream->tBuf.sp <= stream->tBuf.ep) ? (copySize <= tail ? copySize : tail) : copySize;
        memcpy((void*)(stream->tBuf.buf + stream->tBuf.ep), wBuffer + wOffset, singleSize);
        stream->tBuf.ep = (stream->tBuf.ep + singleSize) % stream->tBuf.size;
        //ƫ�Ƶ��ѷ����ֽ�
        wOffset += singleSize;
        wLen += singleSize;
        //���ȼ�ȥ��������䳤��
        copySize -= singleSize;
    }

    return wLen;
}
static s32 touch(struct _ByteStream_t* stream, u8* wBuffer, s32 wSize) {
    return stream->ops.touchOffset(stream, wBuffer, 0, wSize);
}
static s32 touchByte(struct _ByteStream_t* stream, u8 wData) {
    return stream->ops.touchOffset(stream, &wData, 0, 1);
}
static uart_writeCb_t _writeCb = NULL;
//�������������ݷ��ͳ�ȥ �����ͷ�æʱӦ����true ֻ�е����ͳ�ʱʱ��Ӧ�÷���false
static bool flushWrite(struct _ByteStream_t* stream) {
    if (!stream->txBusy) {
        s32 _singleSend = stream->tBuf.sp <= stream->tBuf.ep ? stream->tBuf.ep - stream->tBuf.sp
                                                             : stream->tBuf.size - stream->tBuf.sp;
        if (_singleSend != 0) {
            if (_writeCb != NULL) {
                stream->txBusy = true;
                _writeCb((void*)(stream->tBuf.buf + stream->tBuf.sp), _singleSend);
                stream->tBuf.sp = (stream->tBuf.sp + _singleSend) % stream->tBuf.size;
                stream->txBusy  = false;
            }
        }
    }
    return true;
}
/////////////////////
static s32 buffededWrite(struct _ByteStream_t* stream) {
    return stream->tBuf.ops.getUsed(&stream->tBuf);
}
static bool nextInt(struct _ByteStream_t* stream, s64* num, u8 ignore) {
    bool    status    = false;
    bool    firstChar = true;
    bool    isNeg     = false;
    uint8_t c         = 0;
    int64_t n         = 0;
    int32_t sp        = stream->rBuf.sp;
    while (sp != stream->rBuf.ep) {
        c = stream->ops.peekNextDigital(stream, sp, ignore);
        if (c != 0) {
            //�����������
            if (c == '-') {
                if (firstChar) {
                    //��⵽һ��'-'
                    isNeg     = true;
                    firstChar = false;
                    sp        = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //'-'���ǵ�һ����
                    break;
                }
            } else if (c == '+') {
                if (firstChar) {
                    //��⵽һ��'+'
                    firstChar = false;
                    sp        = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //'+'���ǵ�һ����
                    break;
                }
            } else if (c == '.') {
                //�����в�����.
                break;
            } else if ((c == ignore) && (ignore != 0)) {
                if (firstChar) {
                    sp = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //���ݵ��м���ֺ����ַ�
                    break;
                }
            } else {
                n         = n * 10 + c - '0';
                sp        = (sp + 1) % stream->rBuf.size;
                firstChar = false;
                status    = true;
            }
        } else {
            //����ѭ��
            break;
        }
    }
    if (status) {
        stream->rBuf.sp = sp;
        //�ж�ȡ����
        n = isNeg ? -n : n;
    }
    *num = n;
    return status;
}
static bool nextFloat(struct _ByteStream_t* stream, double* flo, u8 ignore) {
    double  f         = 0;
    double  frac      = 1.0;
    bool    status    = false;
    bool    isNeg     = false;
    bool    isFra     = false;
    bool    firstChar = true;
    int32_t sp        = stream->rBuf.sp;
    uint8_t c         = 0;
    while (sp != stream->rBuf.ep) {
        c = stream->ops.peekNextDigital(stream, sp, ignore);
        if (c != 0) {
            if (c == '-') {
                if (firstChar) {
                    //��⵽һ��'-'
                    isNeg = true;
                    sp    = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //'-'���ǵ�һ����
                    break;
                }
            } else if (c == '+') {
                if (firstChar) {
                    //��⵽һ��'+'
                    sp = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //'-'���ǵ�һ����
                    break;
                }
            } else if ((c == ignore) && (ignore != 0)) {
                if (firstChar) {
                    sp = (sp + 1) % stream->rBuf.size;
                    continue;
                } else {
                    //���ݵ��м���ֺ����ַ�
                    break;
                }
            } else if (c == '.') {
                if (isFra) {  //��Ӧ��������'-'
                    break;
                } else {
                    if (!firstChar) {
                        isFra = true;
                        sp    = (sp + 1) % stream->rBuf.size;
                        continue;
                    } else {
                        //��һ���ַ�Ϊ���ʱ��
                        break;
                    }
                }
            }
            if (isFra) {
                frac *= 0.1;
            }
            f         = f * 10 + c - '0';
            status    = true;
            firstChar = false;
            sp        = (sp + 1) % stream->rBuf.size;
        } else {
            break;
        }
    }
    if (status) {
        //�ж�ȡ����
        stream->rBuf.sp = sp;
        f               = isNeg ? -f : f;
        f               = isFra ? f * frac : f;
    }

    *flo = f;
    return status;
}
static void doSendedCallBack(struct _ByteStream_t* stream) {}
static void doReceivedCallBack(struct _ByteStream_t* stream) {}

static void baseOpsRegister(struct _ByteStream_t* stream) {
    stream->ops.baseOps.readOffset     = readOffset;
    stream->ops.baseOps.read           = read;
    stream->ops.baseOps.readAll        = readAll;
    stream->ops.baseOps.readByte       = readByte;
    stream->ops.baseOps.readStr        = readStr;
    stream->ops.baseOps.writeOffset    = writeOffset;
    stream->ops.baseOps.write          = write;
    stream->ops.baseOps.writeByte      = writeByte;
    stream->ops.baseOps.writeStr       = writeStr;
    stream->ops.baseOps.Printf         = Printf;
    stream->ops.baseOps.vPrintf        = vPrintf;
    stream->ops.baseOps.availableRead  = availableRead;
    stream->ops.baseOps.availableWrite = availableWrite;
    stream->ops.baseOps.discardRead    = discardRead;
    stream->ops.baseOps.discardWrite   = discardWrite;
    stream->ops.baseOps.isWriteIdle    = isWriteIdle;
    stream->ops.baseOps.waitWriteIdle  = waitWriteIdle;
}
static void opsRegister(struct _ByteStream_t* stream) {
    stream->ops.readLine           = readLine;
    stream->ops.peekWithSp         = peekWithSp;
    stream->ops.peek               = peek;
    stream->ops.peekNextDigital    = peekNextDigital;
    stream->ops.peekStrPos         = peekStrPos;
    stream->ops.touchOffset        = touchOffset;
    stream->ops.touch              = touch;
    stream->ops.touchByte          = touchByte;
    stream->ops.flushWrite         = flushWrite;
    stream->ops.buffededWrite      = buffededWrite;
    stream->ops.nextInt            = nextInt;
    stream->ops.nextFloat          = nextFloat;
    stream->ops.doSendedCallBack   = doSendedCallBack;
    stream->ops.doReceivedCallBack = doReceivedCallBack;
}
s8 byteStreamInit(struct _ByteStream_t* stream, uart_writeCb_t writeCb) {
    byteBufInit(&stream->rBuf);
    byteBufInit(&stream->tBuf);
    baseOpsRegister(stream);
    opsRegister(stream);
    stream->txBusy = false;
    _writeCb       = writeCb;
    return 0;
}
