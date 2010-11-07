/*
 *    Stack-less Just-In-Time compiler
 *
 *    Copyright 2009-2010 Zoltan Herczeg (hzmester@freemail.hu). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *      conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *      of conditions and the following disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SLJIT_CONFIG_INTERNAL_H_
#define _SLJIT_CONFIG_INTERNAL_H_

// SLJIT defines the following variables itself depending on the configuration
// sljit_b, sljit_ub : signed and unsigned byte
// sljit_w, sljit_uw : signed and unsigned machine word, enough to store a pointer (same as intptr_t)
// SLJIT_CALL : C calling convention for both calling JIT and C callbacks from JIT
// SLJIT_32BIT_ARCHITECTURE : 32 bit architecture
// SLJIT_64BIT_ARCHITECTURE : 64 bit architecture
// SLJIT_WORD_SHIFT : the shift required to apply when accessing a sljit_w/sljit_uw array by index
// SLJIT_FLOAT_SHIFT : the shift required to apply when accessing a double array by index
// SLJIT_BIG_ENDIAN : big endian architecture
// SLJIT_LITTLE_ENDIAN : little endian architecture
// SLJIT_INDIRECT_CALL : see SLJIT_FUNC_OFFSET()
// SLJIT_W : for defining 64 bit constants on 64 bit architectures (compiler workaround)
// SLJIT_UNALIGNED : allows unaligned memory accesses for integer arithmetic (only!)

// Auto select option (requires compiler support)
#ifdef SLJIT_CONFIG_AUTO
#ifndef _WIN32

#if defined(__i386__) || defined(__i386)
#define SLJIT_CONFIG_X86_32
#elif defined(__x86_64__)
#define SLJIT_CONFIG_X86_64
#elif defined(__arm__) || defined(__ARM__)
#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)
#define SLJIT_CONFIG_ARM_V7
#elif defined(__ARM_ARCH_7__)
#define SLJIT_CONFIG_ARM_THUMB2
#else
#define SLJIT_CONFIG_ARM_V5
#endif
#elif (__ppc64__) || (__powerpc64__)
#define SLJIT_CONFIG_PPC_64
#elif defined(__ppc__) || defined(__powerpc__)
#define SLJIT_CONFIG_PPC_32
#elif (__mips__)
#define SLJIT_CONFIG_MIPS_32
#else
/* Unsupported machine */
#define SLJIT_CONFIG_UNSUPPORTED
#endif

#else // ifndef _WIN32

#if defined(_M_X64) || defined(__x86_64__)
#define SLJIT_CONFIG_X86_64
#elif defined(_ARM_)
#define SLJIT_CONFIG_ARM_V5
#else
#define SLJIT_CONFIG_X86_32
#endif

#endif // ifndef WIN32
#endif // ifdef SLJIT_CONFIG_AUTO

#ifndef SLJIT_STD_MACROS_DEFINED

// General libraries
// SLJIT is designed to be independent from them as possible
// In release (SLJIT_DEBUG not defined) mode only
// the following macros are depending from them
#include <stdlib.h>
#include <string.h>

// General allocation
#define SLJIT_MALLOC(size) malloc(size)
#define SLJIT_FREE(ptr) free(ptr)
#define SLJIT_MEMMOVE(dest, src, len) memmove(dest, src, len)

#endif // SLJIT_NEED_STDLIB

#ifndef SLJIT_HAVE_LIKELY

#if defined(__GNUC__) && (__GNUC__ >= 3)
#define SLJIT_LIKELY(x)		__builtin_expect((x), 1)
#define SLJIT_UNLIKELY(x)	__builtin_expect((x), 0)
#else
#define SLJIT_LIKELY(x)		(x)
#define SLJIT_UNLIKELY(x)	(x)
#endif

#endif // SLJIT_HAVE_LIKELY

#ifndef SLJIT_HAVE_C_DEFINES

// Inline functions
#define SLJIT_INLINE __inline

// Const variables
#define SLJIT_CONST const

#endif // SLJIT_C_DEFINES

#ifndef SLJIT_HAVE_CACHE_FLUSH

#if !defined(SLJIT_CONFIG_X86_32) && !defined(SLJIT_CONFIG_X86_64)
	// Just call __ARM_NR_cacheflush on Linux
#define SLJIT_CACHE_FLUSH(from, to) \
	__clear_cache((char*)(from), (char*)(to))
#else
	// Not required to implement on archs with unified caches
#define SLJIT_CACHE_FLUSH(from, to)
#endif

#endif // SLJIT_HAVE_CACHE_FLUSH

// Byte type
typedef unsigned char sljit_ub;
typedef signed char sljit_b;

// Machine word type. Can encapsulate a pointer.
//   32 bit for 32 bit machines
//   64 bit for 64 bit machines
#if !defined(SLJIT_CONFIG_X86_64) && !defined(SLJIT_CONFIG_PPC_64)
#define SLJIT_32BIT_ARCHITECTURE	1
#define SLJIT_WORD_SHIFT		2
typedef unsigned int sljit_uw;
typedef int sljit_w;
#else
#define SLJIT_64BIT_ARCHITECTURE	1
#define SLJIT_WORD_SHIFT		3
#ifdef _WIN32
typedef unsigned __int64 sljit_uw;
typedef __int64 sljit_w;
#else
typedef unsigned long int sljit_uw;
typedef long int sljit_w;
#endif
#endif

// Double precision
#define SLJIT_FLOAT_SHIFT		3

// Defining long constants
#ifdef SLJIT_64BIT_ARCHITECTURE
#define SLJIT_W(w)	(w##ll)
#else
#define SLJIT_W(w)	(w)
#endif

// ABI (Application Binary Interface) types
#ifdef SLJIT_CONFIG_X86_32

#ifdef __GNUC__
#define SLJIT_CALL __attribute__ ((fastcall))
#define SLJIT_X86_32_FASTCALL
#elif defined(_WIN32)
#define SLJIT_CALL __fastcall
#define SLJIT_X86_32_FASTCALL
#else
#define SLJIT_CALL __stdcall
#endif

#else // Other architectures

#define SLJIT_CALL

#endif // SLJIT_CONFIG_X86_32

#if defined(SLJIT_CONFIG_PPC_32) || defined(SLJIT_CONFIG_PPC_64)
#define SLJIT_BIG_ENDIAN		1
#else
#define SLJIT_LITTLE_ENDIAN		1
#endif

#if defined(SLJIT_CONFIG_PPC_64)
// It seems ppc64 compilers use an indirect addressing for functions
// I don't know why... It just makes things complicated
#define SLJIT_INDIRECT_CALL		1
#endif

#if defined(SLJIT_CONFIG_X86_32) || defined(SLJIT_CONFIG_X86_64)
// Turn on SSE2 support on x86 (operating on doubles)
// (If you want better performance than legacy fpu instructions)
#define SLJIT_SSE2			1

#ifdef SLJIT_CONFIG_X86_32
// Auto detect SSE2 support using CPUID.
// On 64 bit x86 cpus, sse2 must be present
#define SLJIT_SSE2_AUTO			1
#endif

#endif // defined(SLJIT_CONFIG_X86_32) || defined(SLJIT_CONFIG_X86_64)

#if defined(SLJIT_CONFIG_X86_32) || defined(SLJIT_CONFIG_X86_64) \
	|| defined(SLJIT_CONFIG_ARM_V7) || defined(SLJIT_CONFIG_ARM_THUMB2) \
	|| defined(SLJIT_CONFIG_PPC_32) || defined(SLJIT_CONFIG_PPC_64)
#define SLJIT_UNALIGNED
#endif

#ifdef SLJIT_EXECUTABLE_ALLOCATOR
void* sljit_malloc_exec(sljit_uw size);
void sljit_free_exec(void* ptr);
#define SLJIT_MALLOC_EXEC(size) sljit_malloc_exec(size)
#define SLJIT_FREE_EXEC(ptr) sljit_free_exec(ptr)
#endif

#if defined(SLJIT_DEBUG) || defined(SLJIT_VERBOSE)
#include <stdio.h>
#endif

#ifdef SLJIT_DEBUG

// Feel free to overwrite these two macros
#ifndef SLJIT_ASSERT

#define SLJIT_ASSERT(x) \
	do { \
		if (SLJIT_UNLIKELY(!(x))) { \
			printf("Assertion failed at " __FILE__ ":%d\n", __LINE__); \
			*((int*)0) = 0; \
		} \
	} while (0)

#endif // SLJIT_ASSERT

#ifndef SLJIT_ASSERT_STOP

#define SLJIT_ASSERT_STOP() \
	do { \
		printf("Should never been reached " __FILE__ ":%d\n", __LINE__); \
		*((int*)0) = 0; \
	} while (0)

#endif // SLJIT_ASSERT_STOP

#else // SLJIT_DEBUG

#define SLJIT_ASSERT(x) \
	do { } while (0)
#define SLJIT_ASSERT_STOP() \
	do { } while (0)

#endif // SLJIT_DEBUG

#endif
