/*
 * util_other.h
 *
 *  Created on: 2020��8��1��
 *      Author: admin
 */

#ifndef UTIL_OTHER_H_
#define UTIL_OTHER_H_

/******************************************************************************
 Includes
 *****************************************************************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


#if !defined(STATIC)
#if defined(UNIT_TEST)
/*! Allow access to the local variables to test code by making them public*/
#define STATIC
#else
/*! Define STATIC as static for local variables */
#define STATIC static
#endif
#endif

#if !defined(CONST)
#if defined(UNIT_TEST)
/*! Allow constant to be used in different context*/
#define CONST
#else
/*! Define CONST as const for local variables */
#define CONST const
#endif
#endif
/*!
 * \ingroup UtilMisc
 * @{
 */

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief      Converts from a uint16 to ascii hex string.
 *             The # will be exactly 4 hex digits (e.g. 0x0000 or 0x1E3F).
 *             NULL terminates the string.
 *
 * @param      u - Number to be converted
 * @param      string - pointer to coverted string
 */
extern void Util_uint16toa(uint16_t u, char *string);

/*!
 * @brief      Convert a 16bit number to ASCII
 *
 * @param      num - number to convert
 * @param      buf - buffer to write ASCII
 * @param      radix - base to convert to (ie. 10 or 16)
 */
extern void Util_itoa(uint16_t num, uint8_t *buf, uint8_t radix);
extern int  Util_itoa_cnt(uint16_t num, uint8_t *buf, uint8_t radix);
/*!
 * @brief      Convert a long unsigned int to a string.
 *
 * @param      l - long to convert
 * @param      buf - buffer to convert to
 * @param      radix - 10 dec, 16 hex
 *
 * @return     pointer to buffer
 */
extern unsigned char *Util_ltoa(uint32_t l, uint8_t *buf, uint8_t radix);

/*!
 * @brief      Get the high byte of a uint16_t variable
 *
 * @param      a - uint16_t variable
 *
 * @return     high byte
 */
extern uint8_t Util_hiUint16(uint16_t a);

/*!
 * @brief      Get the low byte of a uint16_t variable
 *
 * @param      a - uint16_t variable
 *
 * @return     low byte
 */
extern uint8_t Util_loUint16(uint16_t a);

/*!
 * @brief      Build a uint16_t out of 2 uint8_t variables
 *
 * @param      loByte - low byte
 * @param      hiByte - high byte
 *
 * @return     combined uint16_t
 */
extern uint16_t Util_buildUint16(uint8_t loByte, uint8_t hiByte);

/*!
 * @brief      Build a uint32_t out of 4 uint8_t variables
 *
 * @param      byte0 - byte - 0
 * @param      byte1 - byte - 1
 * @param      byte2 - byte - 2
 * @param      byte3 - byte - 3
 *
 * @return     combined uint32_t
 */
extern uint32_t Util_buildUint32(uint8_t byte0, uint8_t byte1, uint8_t byte2,
                                 uint8_t byte3);

/*!
 * @brief      Pulls 1 uint8_t out of a uint32_t
 *
 * @param      var - uint32_t variable
 * @param      byteNum - what byte to pull out (0-3)
 *
 * @return     uint8_t
 */
extern uint8_t Util_breakUint32(uint32_t var, int byteNum);

/*!
 * @brief      Build a uint16_t from a uint8_t array
 *
 * @param      pArray - pointer to uint8_t array
 *
 * @return     combined uint16_t
 */

extern uint16_t Util_parseUint16(uint8_t *pArray);

/*!
 * @brief      Build a uint32_t from a uint8_t array
 *
 * @param      pArray - pointer to uint8_t array
 *
 * @return     combined uint32_t
 */
extern uint32_t Util_parseUint32(uint8_t *pArray);

/*!
 * @brief      Break and buffer a uint16 value - LSB first
 *
 * @param      pBuf - ptr to next available buffer location
 * @param      val  - 16-bit value to break/buffer
 *
 * @return     pBuf - ptr to next available buffer location
 */
extern uint8_t *Util_bufferUint16(uint8_t *pBuf, uint16_t val);

/*!
 * @brief      Break and buffer a uint32 value - LSB first
 *
 * @param      pBuf - ptr to next available buffer location
 * @param      val  - 32-bit value to break/buffer
 *
 * @return     pBuf - ptr to next available buffer location
 */
extern uint8_t *Util_bufferUint32(uint8_t *pBuf, uint32_t val);

/*!
 * @brief       Utility function to clear an event
 *
 * @param       pEvent - pointer to event variable
 * @param       event - event(s) to clear
 */
extern void Util_clearEvent(uint16_t *pEvent, uint16_t event);

/*!
 * @brief       Utility function to set an event
 *
 * @param       pEvent - pointer to event variable
 * @param       event - event(s) to clear
 */
extern void Util_setEvent(uint16_t *pEvent, uint16_t event);

/*!
 * @brief       Utility function to copy the extended address
 *
 * @param       pSrcAddr - pointer to source from which to be copied
 * @param       pDstAddr - pointer to destination to copy to
 */
extern void Util_copyExtAddr(void *pSrcAddr, void *pDstAddr);

/*! @} end group UtilMisc */

#ifdef __cplusplus
}
#endif


#endif /* UTIL_OTHER_H_ */
