/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief
 */

#ifndef ZEPHYR_ARCH_ARM_INCLUDE_AARCH32_CORTEX_M_WRAP_FUNC_H_
#define ZEPHYR_ARCH_ARM_INCLUDE_AARCH32_CORTEX_M_WRAP_FUNC_H_

#ifdef _ASMLANGUAGE

/* nothing */

#else

/**
 * @brief Macro for "sandwiching" a function call (@p name) in two other calls
 *
 * This macro should be called via @ref Z_ARM_WRAP_FUNC or
 * @ref Z_ARM_WRAP_FUNC_STACK_ARGS.
 *
 * This macro creates the function body of an "outer" function which behaves
 * exactly like the wrapped function (@p name), except that the preface function
 * is called before, and the postface function afterwards.
 *
 * @param preface   The function to call first. Must have no parameters and no
 *                  return value.
 * @param name      The main function, i.e. the function to wrap. This function
 *                  will receive the arguments, and its return value will be
 *                  returned.
 * @param postface  The function to call last. Must have no parameters and no
 *                  return value.
 * @param store_lr  The assembly instruction for storing away the LR value
 *                  before the functions are called.
 * @param load_lr   The assembly instruction for restoring the LR value after
 *                  the functions have been called.
 */
#define Z_ARM_WRAP_FUNC_RAW(preface, name, postface, store_lr, load_lr) \
	do { \
		__asm(".global "#preface"; .type "#preface", %function"); \
		__asm(".global "#name"; .type "#name", %function"); \
		__asm(".global "#postface"; .type "#postface", %function"); \
		\
		__asm(store_lr); \
		__asm("push {r0-r3}"); \
		__asm("bl "#preface); \
		__asm("pop {r0-r3}"); \
		__asm("bl "#name); \
		__asm("push {r0-r3}"); \
		__asm("bl "#postface); \
		__asm("pop {r0-r3}"); \
		__asm(load_lr); \
	} while (0)

/**
 * @brief Macro for "sandwiching" a function call (@p name) in two other calls
 *
 * This is for functions which do not pass arguments or return values via the
 * stack. I.e. the arguments and return values must each fit within 4 words,
 * after accounting for alignment.
 *
 * If nothing is passed on the stack, the stack can be used to store LR without
 * messing with offsets.
 *
 *	int foo(char *arg); // Implemented elsewhere.
 *	int __attribute__((naked)) foo_wrapped(char *arg)
 *		{Z_ARM_WRAP_FUNC(bar, foo, baz)}
 *
 * is equivalent to
 *
 *	int foo(char *arg); // Implemented elsewhere.
 *	int foo_wrapped(char *arg)
 *	{
 *		bar();
 *		int res = foo(arg);
 *		baz();
 *		return res;
 *	}
 *
 * @note __attribute__((naked)) is not mandatory, but without it, GCC gives a
 *       warning for functions with a return value. It also saves a bit of space
 *       since it removes a little code that is not necessary.
 *
 * See @ref Z_ARM_WRAP_FUNC_RAW for more information.
 */
#define Z_ARM_WRAP_FUNC(preface, name, postface) \
	Z_ARM_WRAP_FUNC_RAW(preface, name, postface, "push {r4, lr}", \
			"pop {r4, pc}")

/**
 * @brief Macro for "sandwiching" a function call (@p name) in two other calls
 *
 * This is for functions which pass arguments or return values via the stack.
 * I.e. the arguments or return values exceed 4 words, after accounting for
 * alignment.
 *
 * This means LR cannot be stored on the stack, so it must be stored in an extra
 * argument called lr_backup, see example below.
 *
 *	int foo(char *arg1, char *arg2, int arg3, uint64 arg4);
 *	int __attribute__((naked)) foo_wrapped(char *arg1, char *arg2, int arg3,
 *						uint64 arg4, uint32_t lr_backup)
 *		{Z_ARM_WRAP_FUNC_STACK_ARGS(bar, foo, baz)}
 *
 * is equivalent to
 *
 *	int foo(char *arg1, char *arg2, int arg3, uint64 arg4);
 *	int foo_wrapped(char *arg1, char *arg2, int arg3, uint64 arg4,
 *			uint32_t lr_backup)
 *	{
 *		bar();
 *		int res = foo(arg1, arg2, arg3, arg4);
 *		baz();
 *		return res;
 *	}
 *
 * @note __attribute__((naked)) is not mandatory, but without it, GCC gives a
 *       warning for functions with a return value. It also saves a bit of space
 *       since it removes a little code that is not necessary.
 *
 * @note This macro is not available on ARMV6-m and ARMv8-m Baseline, because
 *       str/ldr on high registers is not supported.
 *
 * See @ref Z_ARM_WRAP_FUNC_RAW for more information.
 */
#ifndef CONFIG_ARMV6_M_ARMV8_M_BASELINE
#define Z_ARM_WRAP_FUNC_STACK_ARGS(preface, name, postface) \
	Z_ARM_WRAP_FUNC_RAW(preface, name, postface, \
			"str lr, %0" :: "m" (lr_backup), \
			"ldr pc, %0" :: "m" (lr_backup))
#endif /* CONFIG_ARMV6_M_ARMV8_M_BASELINE */
#endif /* _ASMLANGUAGE */
#endif /* ZEPHYR_ARCH_ARM_INCLUDE_AARCH32_CORTEX_M_WRAP_FUNC_H_ */
