/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <../arch/arm/include/aarch32/cortex_m/wrap_func.h>

static bool preface_called;
static bool postface_called;
static bool foo1_called;
static uint32_t foo1_retval;
static uint32_t foo1_arg1;
static uint32_t foo1_arg2;
static uint32_t foo1_arg3;
static uint32_t foo1_arg4;
static bool foo2_called;
static uint64_t foo2_retval;
static uint32_t foo2_arg1;
static uint32_t foo2_arg2;
static uint32_t foo2_arg3;
static uint64_t foo2_arg4;

void reset_mocks(void)
{
	preface_called = 0;
	postface_called = 0;
	foo1_called = 0;
	foo1_retval = 0;
	foo1_arg1 = 0;
	foo1_arg2 = 0;
	foo1_arg3 = 0;
	foo1_arg4 = 0;
	foo2_called = 0;
	foo2_retval = 0;
	foo2_arg1 = 0;
	foo2_arg2 = 0;
	foo2_arg3 = 0;
	foo2_arg4 = 0;
}

void preface(void)
{
	zassert_false(preface_called, "%s already called", __func__);
	preface_called = true;
}
void postface(void)
{
	zassert_false(postface_called, "%s already called", __func__);
	postface_called = true;
}
uint32_t foo1(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4)
{
	zassert_false(foo1_called, "%s already called", __func__);
	zassert_equal(arg1, foo1_arg1, "Was 0x%"PRIx32", expected 0x%"PRIx32, arg1,
		foo1_arg1);
	zassert_equal(arg2, foo1_arg2, "2");
	zassert_equal(arg3, foo1_arg3, "3");
	zassert_equal(arg4, foo1_arg4, "4");
	foo1_called = true;
	return foo1_retval;
}

#ifndef CONFIG_ARMV6_M_ARMV8_M_BASELINE
uint64_t foo2(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint64_t arg4)
{
	zassert_false(foo2_called, "%s already called", __func__);
	zassert_equal(arg1, foo2_arg1, "5");
	zassert_equal(arg2, foo2_arg2, "6");
	zassert_equal(arg3, foo2_arg3, "7");
	zassert_equal(arg4, foo2_arg4, "Was 0x%"PRIx64", expected 0x%"PRIx64, arg4,
		foo2_arg4);
	foo2_called = true;
	return foo2_retval;
}
#endif

WRAP_4(uint32_t, preface, foo1, postface,
	wrap_foo1, uint32_t, arg1, uint32_t, arg2, uint32_t, arg3, uint32_t, arg4)

WRAP_4(uint64_t, preface, foo2, postface,
	wrap_foo2, uint32_t, arg1, uint32_t, arg2, uint32_t, arg3, uint64_t, arg4)

#ifdef CONFIG_ARMV6_M_ARMV8_M_BASELINE
#define GET_MSP(dst) __asm("mov %0,MSP" : "=rm" (dst))
#define GET_PSP(dst) __asm("mov %0,PSP" : "=rm" (dst))
#else
#define GET_MSP(dst) dst = __get_MSP()
#define GET_PSP(dst) dst = __get_PSP()
#endif

void test_arm_wrap_func(void)
{
	reset_mocks();
	foo1_retval = 0x01234567;
	foo1_arg1 = 0x12345678;
	foo1_arg2 = 0x23456789;
	foo1_arg3 = 0x3456789a;
	foo1_arg4 = 0x456789ab;

	uint32_t msp1, msp2, psp1, psp2;
	GET_MSP(msp1);
	GET_PSP(psp1);

	zassert_equal(foo1_retval,
		wrap_foo1(foo1_arg1, foo1_arg2, foo1_arg3, foo1_arg4), "9");

	GET_MSP(msp2);
	GET_PSP(psp2);

	zassert_equal(msp1, msp2, NULL);
	zassert_equal(psp1, psp2, NULL);

	zassert_true(preface_called, NULL);
	zassert_true(foo1_called, NULL);
	zassert_true(postface_called, NULL);

#ifndef CONFIG_ARMV6_M_ARMV8_M_BASELINE
	reset_mocks();
	foo2_retval = 0x0123456789UL;
	foo2_arg1 = 0x12345679;
	foo2_arg2 = 0x2345678a;
	foo2_arg3 = 0x3456789b;
	foo2_arg4 = 0x456789abc;

	uint64_t ret2 = wrap_foo2(foo2_arg1, foo2_arg2, foo2_arg3, foo2_arg4);
	zassert_equal(foo2_retval, ret2,
		"wrong retval. Was 0x%"PRIx64", expected 0x%"PRIx64, ret2,
		foo2_retval);

	GET_MSP(msp2);
	GET_PSP(psp2);

	zassert_equal(msp1, msp2, NULL);
	zassert_equal(psp1, psp2, NULL);

	zassert_true(preface_called, NULL);
	zassert_true(foo2_called, NULL);
	zassert_true(postface_called, NULL);
#endif
}

void test_main(void)
{
	ztest_test_suite(arm_wrap_func,
		ztest_unit_test(test_arm_wrap_func));
	ztest_run_test_suite(arm_wrap_func);
}
