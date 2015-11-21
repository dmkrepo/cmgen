/**
 * DMK
 * Copyright (C) 2015  Dmitriy Ka
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

// Architecture

#if defined( _M_X64 ) || defined( __x86_64__ )
#define DMK_ARCH_X64 1
#else
#define DMK_ARCH_X32 1
#define DMK_ARCH_X86 1
#endif

#if defined( __AVX__ )
#define DMK_ARCH_AVX 1
#else
#define DMK_ARCH_SSE 1
#endif

// OS

#if defined( _WIN32 )
#define DMK_OS_WIN 1
#else
#define DMK_OS_UNIX 1
#define DMK_OS_POSIX 1
#if defined( __APPLE__ )
#define DMK_OS_MAC 1
#define DMK_OS_OSX 1
#endif
#endif

// Compiler

#if defined( _MSC_VER ) // Visual C++

#define DMK_COMPILER_MSVC 1

#define DMK_ALWAYS_INLINE inline __forceinline
#define DMK_ALWAYS_INLINE_STATIC inline __forceinline static
#define DMK_NOINLINE __declspec( noinline )
#define DMK_FLATTEN

#endif

#if defined( __GNUC__ ) // GCC, Clang

#define DMK_COMPILER_GNU 1

#define DMK_ALWAYS_INLINE __inline__ __attribute__( ( always_inline ) )
#define DMK_ALWAYS_INLINE_STATIC __inline__ __attribute__( ( always_inline ) ) static
#define DMK_NOINLINE __attribute__( ( noinline ) )
#define DMK_FLATTEN __attribute__( ( flatten ) )
#define DMK_RESTRICT __restrict__

#endif

#if defined( __INTEL_COMPILER ) // Intel Compiler
#define DMK_COMPILER_INTEL 1
#endif

#if defined( __clang__ ) // Clang
#define DMK_COMPILER_CLANG 1
#endif

// Compatibility with compilers other than clang
#ifndef __has_feature
#define __has_feature( feature ) 0
#endif

// Useful macro

#ifdef __cplusplus
#define DMK_EXTERN_C extern "C"
#else
#define DMK_EXTERN_C
#endif

#if defined( DMK_OS_WIN )
#define DMK_IF_WIN( t, f ) t
#else
#define DMK_IF_WIN( t, f ) f
#endif

#if defined( DMK_ARCH_X64 )
#define DMK_IF_X64( t, f ) t
#else
#define DMK_IF_X64( t, f ) f
#endif

#if defined( DMK_ARCH_AVX )
#define DMK_IF_AVX( t, f ) t
#else
#define DMK_IF_AVX( t, f ) f
#endif

#define DMK_NOOP ( ( void )0 )

#if defined( DMK_COMPILER_MSVC )
#define DMK_FUNC_NAME __FUNCSIG__
#else
#define DMK_FUNC_NAME __PRETTY_FUNCTION__
#endif

#if defined( DMK_COMPILER_MSVC )
#define DMK_INLINE_BREAKPOINT ( void )( __debugbreak( ) )
#else
#define DMK_INLINE_BREAKPOINT ( void )( __asm__ volatile( "int $0x03" ) )
#endif

#if defined( NDEBUG )
#define DMK_DEBUG_BREAK DMK_NOOP
#else
#define DMK_DEBUG_BREAK DMK_INLINE_BREAKPOINT
#endif

#if defined( DMK_COMPILER_MSVC )
#include <yvals.h>
#endif

#if defined( DMK_COMPILER_MSVC )
#define DMK_NOEXCEPT _NOEXCEPT
#define DMK_NOEXCEPT_OP( x ) _NOEXCEPT_OP( x )
#define DMK_CONSTEXPR_DATA _CONST_DATA
#define DMK_CONSTEXPR_FUNC _CONST_FUN
#else
#define DMK_NOEXCEPT noexcept
#define DMK_NOEXCEPT_OP( x ) noexcept( x )
#define DMK_CONSTEXPR_DATA constexpr
#define DMK_CONSTEXPR_FUNC constexpr
#endif

#if defined( DMK_COMPILER_GNU )
#define DMK_ALIGNED_ALLOCATOR( alignment )                                                                   \
    __attribute__( ( assume_aligned( alignment ) ) ) __attribute__( ( malloc ) )
#else
#define DMK_ALIGNED_ALLOCATOR( alignment ) __declspec( noalias ) __declspec( restrict )
#endif

#include <cstddef>
#include <cstdint>
#include <cinttypes>

namespace dmk
{
    template <typename T, std::size_t N>
    DMK_CONSTEXPR_FUNC std::size_t countof( T const( & )[N] ) DMK_NOEXCEPT
    {
        return N;
    }

    template <typename T>
    DMK_ALWAYS_INLINE void zeroize( T& value )
    {
        std::memset( &value, 0, sizeof( value ) );
    }

#if _MSC_VER < 1900 && !defined( snprintf )

    DMK_ALWAYS_INLINE int c99_vsnprintf( char* str, size_t size, const char* format, va_list ap )
    {
        int count = -1;

        if ( size != 0 )
        {
            count = _vsnprintf_s( str, size, _TRUNCATE, format, ap );
        }
        if ( count == -1 )
        {
            count = _vscprintf( format, ap );
        }

        return count;
    }

    DMK_ALWAYS_INLINE int c99_snprintf( char* str, size_t size, const char* format, ... )
    {
        int count;
        va_list ap;

        va_start( ap, format );
        count = c99_vsnprintf( str, size, format, ap );
        va_end( ap );

        return count;
    }

#define snprintf ::dmk::c99_snprintf

#endif

    template <size_t bits>
    struct bits_tpl
    {
    };

    template <>
    struct bits_tpl<8>
    {
        typedef uint8_t u;
        typedef int8_t i;
    };

    template <>
    struct bits_tpl<16>
    {
        typedef uint16_t u;
        typedef int16_t i;
    };

    template <>
    struct bits_tpl<32>
    {
        typedef uint32_t u;
        typedef int32_t i;
    };

    template <>
    struct bits_tpl<64>
    {
        typedef uint64_t u;
        typedef int64_t i;
    };

    template <size_t bits>
    using uintbits_t = typename bits_tpl<bits>::u;

    template <size_t bits>
    using intbits_t = typename bits_tpl<bits>::i;

    template <size_t bits>
    using uintbytes_t = typename bits_tpl<bits * 8>::u;

    template <size_t bits>
    using intbytes_t = typename bits_tpl<bits * 8>::i;

    template <typename _Type>
    using uintsized_t = typename bits_tpl<sizeof( _Type ) * 8>::u;

    template <typename _Type>
    using intsized_t = typename bits_tpl<sizeof( _Type ) * 8>::i;

    typedef uint8_t byte_t;

    template <size_t _N>
    struct struct_padding
    {
    private:
        byte_t __hidden_padding[_N];
    };

    template <>
    struct struct_padding<0>
    {
    };

} // namespace dmk
