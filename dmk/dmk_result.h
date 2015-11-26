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

#if defined( DMK_OS_WIN )
#include <windows.h>
#endif

#include <string>
#include <memory>
#include <tuple>
#include <iostream>
#include <exception>

namespace dmk
{

    enum system_error_t
    {
        system_error
    };

    inline std::string system_error_string( )
    {
#if defined( DMK_OS_WIN )
        DWORD ErrorCode = GetLastError( );
        TCHAR* pMsgBuf  = NULL;
        DWORD nMsgLen   = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                            FORMAT_MESSAGE_IGNORE_INSERTS,
                                        NULL,
                                        ErrorCode,
                                        MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ),
                                        reinterpret_cast<LPTSTR>( &pMsgBuf ),
                                        0,
                                        NULL );
        if ( !nMsgLen )
            return fmt::format( "Unknown system error (GetLastError()={})", ErrorCode );
        std::string msg( pMsgBuf, pMsgBuf + nMsgLen );
        LocalFree( pMsgBuf );
        return fmt::format( "{}; GetLastError={}", msg, ErrorCode );
#elif defines( DMK_OS_POSIX )
        return "unknown posix error";
#endif
    }

    class error : public std::runtime_error
    {
    public:
        typedef std::runtime_error inherited;

        explicit error( const std::string& message ) : inherited( message.c_str( ) )
        {
        }

        explicit error( const char* message ) : inherited( message )
        {
        }
        template <typename... Args>
        error( const std::string& message, const Args&... args )
            : inherited( fmt::format( message, args... ) )
        {
        }
        template <typename... Args>
        error( const char* message, const Args&... args )
            : inherited( fmt::format( message, args... ) )
        {
        }
        error( system_error_t, const std::string& message )
            : inherited( message + "\n" + system_error_string( ) )
        {
        }

        error( system_error_t, const char* message )
            : inherited( std::string( message ) + "\n" + system_error_string( ) )
        {
        }
        template <typename... Args>
        error( system_error_t, const std::string& message, const Args&... args )
            : inherited( fmt::format( message, args... ) + "\n" + system_error_string( ) )
        {
        }
        template <typename... Args>
        error( system_error_t, const char* message, const Args&... args )
            : inherited( fmt::format( message, args... ) + "\n" + system_error_string( ) )
        {
        }

        explicit error( const std::exception& err, const std::string& message )
            : inherited( message + "\n" + err.what( ) )
        {
        }

        explicit error( const std::exception& err, const char* message )
            : inherited( std::string( message ) + "\n" + err.what( ) )
        {
        }
        template <typename... Args>
        error( const std::exception& err, const std::string& message, const Args&... args )
            : inherited( fmt::format( message, args... ) + "\n" + err.what( ) )
        {
        }
        template <typename... Args>
        error( const std::exception& err, const char* message, const Args&... args )
            : inherited( fmt::format( message, args... ) + "\n" + err.what( ) )
        {
        }
    };
} // namespace dmk
