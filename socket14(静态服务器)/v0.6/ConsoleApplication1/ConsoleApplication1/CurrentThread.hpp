#pragma once
/*stdint.h中定义了一些整数类型，规则如下(其中N可以为8，16，32，64)

intN_t, int_leastN_t, int_fastN_t表示长度为N位的整型数；

uintN_t, uint_leastN_t, uint_fastN_t表示长度为N位的无符号整型数 ；

stdint.h中的常量，定义以上各类型数的最大最小值(其中N可以为8，16，32，64)

INTN_MIN, UINTN_MIN, INTN_MAX, UINTN_MAX;

INT_LEASEN_MIN, INT_LEASEN_MAX;

INT_FASTN_MIN, INT_FASTN_MAX;

以上类型的C++类型定义等

三、大数输出

int64_t数的输出：%lld;
uint64_t数的输出：%llu;
uint64_t数十六进制输出：%llx;
uint64_t数八进制输出：%llo;*/
#include <stdint.h>