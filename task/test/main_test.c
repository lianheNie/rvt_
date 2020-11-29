/*
 * main_test.c
 *
 *  Created on: 2020Äê9ÔÂ7ÈÕ
 *      Author: admin
 */

#include <aw_config.h>
#ifdef IS_USE_C_TEST  // CÓïÑÔ²âÊÔ
#include <stdio.h>
#include <aw_mqtt_def.h>
typedef struct _test_t {
    char str1[10];
    char ch1;
    char str2[10];
    char ch2;
} test_str_t;
int                   a = 2, b = 2, c = 0, d = 0;
static aw_at_staist_t _statis;
static int            _sum1(int* __restrict p1, int* __restrict p2) {
    *p1 = 5;
    *p2 = 6;
    return *p1 + *p2;
}

static int _sum2(int* p1, int* p2) {
    *p1 = 5;
    *p2 = 6;
    return *p1 + *p2;
}
bool b_a = true;
#define _ID 12
int main_C_test() {
    char Host[15] = "\0";
    //    int        num     = 22;
    //    const char DevId[] = AW_firmware_version;
    //    num                = sizeof(_statis);
    //    if (num == 0 && _sum2(&a, &b) == 1) {
    //        c = 0;
    //    }
    //    if (2 < 1 + 3) {
    //        c = 0;
    //    } else {
    //        c = 4;
    //    }
    //
    //    test_str_t strs = {.str1 = {'\0'}, .ch1 = '\0', .str1 = {'\0'}, .ch2 = '\0'};
    //
    //    typedef int AW_INT_t;
    //
    //    typedef AW_INT_t* AW_INT_PTR_t;
    //
    //    typedef AW_INT_PTR_t* AW_INT_PTR_PTR_t;
    //
    //    //
    //    //
    //    //
    //    //    typedef int AW_INT_t;
    //    //
    //    //    typedef int* AW_INT_PTR_t;
    //    //
    //    //    typedef int** AW_INT_PTR_PTR_t;
    //    int               aa      = 0;     // 0
    //    AW_INT_t*         p_a     = NULL;  // 4
    //    AW_INT_PTR_t*     p_p_a   = NULL;  // 8
    //    AW_INT_PTR_PTR_t* p_p_p_a = NULL;  // 12
    //
    //    p_a     = &aa;
    //    p_p_a   = &p_a;
    //    p_p_p_a = &p_p_a;
    //
    //    u32 a_u32[10][3] = {0};
    //
    //    u8 a_u8[10] = {0};
    //
    //    u32* a_u32_ptr = a_u32;
    //
    //    // u32* a_u32_ptr = &a_u32[0];
    //
    //    int i = 0;
    //
    //    //    for (i = 0; i < 10; i++) {
    //    //        a_u32[i] = i;
    //    //    }
    //
    //    for (i = 0; i < 10; i++) {
    //        a_u8[i] = i;
    //    }
    //
    //    u8  c1 = 1;  // 1
    //    u32 c2 = 2;  // 2
    //    u32 c3 = 2;  // 6
    //
    //    int* p_b = NULL;
    //    p_b      = &b;
    //    if (b_a == true) {
    //        c = _sum1(p_a, p_b);
    //        d = _sum1(&b, &b);
    //    }

    s16 sss = -15666;
    snprintf(Host, 15, "%x", (u16)sss);
    return 0;
}

void test() {}
#endif
