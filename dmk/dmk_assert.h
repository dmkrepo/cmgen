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

#include "dmk.h"
#include "dmk_string.h"

// Configuration

#include <string>
#include <iostream>
#include <sstream>

DMK_ALWAYS_INLINE std::string _dmk_object( )
{
    return std::string( );
}

namespace dmk
{

#if !defined( DMK_ERROR_STREAM )
#define DMK_ERROR_STREAM std::cerr
#endif

#define DMK_ASSERTION_BREAKPOINT

//

#define DMK_ASSUME( _Expression ) DMK_NOOP

#ifdef NDEBUG

#define DMK_ASSERT( _Expression ) DMK_ASSUME( _Expression )

#define DMK_ASSERT_EQ( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 == _Expression2 )

#define DMK_ASSERT_NE( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 != _Expression2 )

#define DMK_ASSERT_GE( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 >= _Expression2 )

#define DMK_ASSERT_LE( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 <= _Expression2 )

#define DMK_ASSERT_GT( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 > _Expression2 )

#define DMK_ASSERT_LT( _Expression1, _Expression2 ) DMK_ASSUME( _Expression1 < _Expression2 )

#define DMK_ASSERT_NULL( _Expression ) DMK_ASSUME( _Expression == nullptr )

#define DMK_ASSERT_NOTNULL( _Expression ) DMK_ASSUME( _Expression != nullptr )

#else

    DMK_ALWAYS_INLINE const char* get_filename( const char* path )
    {
        if ( !path )
            return path;
        const char* filename = path + strlen( path );
        while ( filename > path && *filename != '/' && *filename != '\\' )
        {
            filename--;
        }
        if ( filename > path )
            filename++;
        return filename;
    }

    inline void assertion_failed( const char* comparison,
                                  const char* expression1,
                                  const char* expression2,
                                  const std::string& value1,
                                  const std::string& value2,
                                  const char* file,
                                  const char* func,
                                  int line,
                                  const std::string& object,
                                  bool fixed = false )
    {
        auto& os = DMK_ERROR_STREAM;
        os << "Assertion failed" << std::endl;
        os << "Expected: " << expression1 << " " << comparison << " " << expression2 << std::endl;
        os << "Got:      " << expression1 << " == " << value1 << std::endl;
        if ( !fixed )
            os << "          " << expression2 << " == " << value2 << std::endl;
        if ( func )
        {
            os << "In func:  " << func << " (" << get_filename( file ) << ", line " << line << ")"
               << std::endl;
        }
        if ( !object.empty( ) )
        {
            os << "Object:   " << object << std::endl;
        }
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_eq( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 == value2 ) )
        {
            assertion_failed( "==",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_ne( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 != value2 ) )
        {
            assertion_failed( "!=",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_ge( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 >= value2 ) )
        {
            assertion_failed( ">=",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_le( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 <= value2 ) )
        {
            assertion_failed( "<=",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_gt( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 > value2 ) )
        {
            assertion_failed( ">",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T1, typename _T2>
    DMK_ALWAYS_INLINE bool assert_lt( const char* expression1,
                                      const char* expression2,
                                      _T1&& value1,
                                      _T2&& value2,
                                      const char* file,
                                      const char* func,
                                      int line,
                                      const std::string& comment )
    {
        if ( !( value1 < value2 ) )
        {
            assertion_failed( "<",
                              expression1,
                              expression2,
                              stringify( value1 ),
                              stringify( value2 ),
                              file,
                              func,
                              line,
                              comment );
            return false;
        }
        return true;
    }

    template <typename _T>
    DMK_ALWAYS_INLINE bool assert_notnull( const char* expression,
                                           _T&& value,
                                           const char* file,
                                           const char* func,
                                           int line,
                                           const std::string& comment )
    {
        if ( !( value != nullptr ) )
        {
            assertion_failed(
                "!=", expression, "nullptr", stringify( value ), "", file, func, line, comment, true );
            return false;
        }
        return true;
    }

    template <typename _T>
    DMK_ALWAYS_INLINE bool assert_null( const char* expression,
                                        _T&& value,
                                        const char* file,
                                        const char* func,
                                        int line,
                                        const std::string& comment )
    {
        if ( !( value == nullptr ) )
        {
            assertion_failed(
                "==", expression, "nullptr", stringify( value ), "", file, func, line, comment, true );
            return false;
        }
        return true;
    }

    template <typename _T>
    DMK_ALWAYS_INLINE bool assert_true( const char* expression,
                                        _T&& value,
                                        const char* file,
                                        const char* func,
                                        int line,
                                        const std::string& comment )
    {
        if ( !( value ) )
        {
            assertion_failed(
                "==", expression, "true", stringify( value ), "", file, func, line, comment, true );
            return false;
        }
        return true;
    }

    template <typename _T>
    DMK_ALWAYS_INLINE bool assert_false( const char* expression,
                                         _T&& value,
                                         const char* file,
                                         const char* func,
                                         int line,
                                         const std::string& object )
    {
        if ( !( !value ) )
        {
            assertion_failed(
                "==", expression, "false", stringify( value ), "", file, func, line, object, true );
            return false;
        }
        return true;
    }

#define DMK_ASSERT( _Expression, ... )                                                                       \
    ( void )(                                                                                                \
        ::dmk::assert_true(                                                                                  \
            #_Expression, _Expression, __FILE__, DMK_FUNC_NAME, __LINE__, _dmk_object( ), __VA_ARGS__ ) ||   \
        ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_TRUE( _Expression, ... )                                                                  \
    ( void )(                                                                                                \
        ::dmk::assert_true(                                                                                  \
            #_Expression, _Expression, __FILE__, DMK_FUNC_NAME, __LINE__, _dmk_object( ), __VA_ARGS__ ) ||   \
        ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_FALSE( _Expression, ... )                                                                 \
    ( void )(                                                                                                \
        ::dmk::assert_false(                                                                                 \
            #_Expression, _Expression, __FILE__, DMK_FUNC_NAME, __LINE__, _dmk_object( ), __VA_ARGS__ ) ||   \
        ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_NULL( _Expression, ... )                                                                  \
    ( void )(                                                                                                \
        ::dmk::assert_null(                                                                                  \
            #_Expression, _Expression, __FILE__, DMK_FUNC_NAME, __LINE__, _dmk_object( ), __VA_ARGS__ ) ||   \
        ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_NOTNULL( _Expression, ... )                                                               \
    ( void )(                                                                                                \
        ::dmk::assert_notnull(                                                                               \
            #_Expression, _Expression, __FILE__, DMK_FUNC_NAME, __LINE__, _dmk_object( ), __VA_ARGS__ ) ||   \
        ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_EQ( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_eq( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_NE( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_ne( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_LT( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_lt( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_GT( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_gt( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_LE( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_le( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_GE( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_ge( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#define DMK_ASSERT_NE( _Expression1, _Expression2, ... )                                                     \
    ( void )(::dmk::assert_ne( #_Expression1,                                                                \
                               #_Expression2,                                                                \
                               _Expression1,                                                                 \
                               _Expression2,                                                                 \
                               __FILE__,                                                                     \
                               DMK_FUNC_NAME,                                                                \
                               __LINE__,                                                                     \
                               _dmk_object( ),                                                               \
                               __VA_ARGS__ ) ||                                                              \
             ( DMK_DEBUG_BREAK, true ) )

#endif
} // namespace dmk
