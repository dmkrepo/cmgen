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
#include "dmk_result.h"
#include "dmk_json.h"
#if defined( DMK_OS_WIN )
#include <windows.h>
#endif
#include <iostream>
#include <vector>
#include <string>
#include <system_error>
#include <chrono>
#include <iostream>
#include <ostream>
#include <istream>

#if defined( DMK_OS_WIN )
#include <filesystem>
namespace dmk
{
    using namespace std::experimental::filesystem;
#define DMK_COPY_OVERWRITE std::experimental::filesystem::copy_options::overwrite_existing
}
#else
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
namespace dmk
{
    using namespace boost::filesystem;
    using copy_options = copy_option;
#define DMK_COPY_OVERWRITE boost::filesystem::copy_option::overwrite_if_exists
}
#endif

namespace dmk
{

    class file_error : public error
    {
    public:
        using error::error;
    };

    enum class open_mode : int
    {
        None   = 0,
        Read   = 1,
        Write  = 2,
        Binary = 4,
        Append = 8
    };

    inline void fix_write_rights( const path& entry )
    {
#if defined( DMK_OS_WIN )
        std::wstring pathw = entry.wstring( );
        DWORD attr = GetFileAttributesW( pathw.c_str( ) );
        if ( attr & FILE_ATTRIBUTE_READONLY )
        {
            attr &= ~( DWORD )FILE_ATTRIBUTE_READONLY;
            SetFileAttributesW( pathw.c_str( ), attr );
        }
#endif
    }

    inline open_mode operator|( open_mode x, open_mode y )
    {
        return static_cast<open_mode>( static_cast<int>( x ) | static_cast<int>( y ) );
    }

    inline open_mode operator&( open_mode x, open_mode y )
    {
        return static_cast<open_mode>( static_cast<int>( x ) & static_cast<int>( y ) );
    }

    inline bool operator!( open_mode x )
    {
        return !static_cast<bool>( x );
    }

    inline FILE* open_file( const path& file, open_mode mode )
    {
#if defined( DMK_OS_WIN )
        wchar_t mode_s[8] = { 0 };
        int i = 0;
        if ( !!( mode & open_mode::Read ) )
        {
            mode_s[i++] = 'r';
        }
        if ( !!( mode & open_mode::Write ) )
        {
            mode_s[i++] = 'w';
        }
        if ( !!( mode & open_mode::Binary ) )
        {
            mode_s[i++] = 'b';
        }
        if ( !!( mode & open_mode::Append ) )
        {
            mode_s[i++] = 'a';
        }
        return _wfopen( file.wstring( ).c_str( ), mode_s );
#elif defined( DMK_OS_POSIX )
        char mode_s[8] = { 0 };
        int i = 0;
        if ( !!( mode & open_mode::Read ) )
        {
            mode_s[i++] = 'r';
        }
        if ( !!( mode & open_mode::Write ) )
        {
            mode_s[i++] = 'w';
        }
        if ( !!( mode & open_mode::Binary ) )
        {
            mode_s[i++] = 'b';
        }
        if ( !!( mode & open_mode::Append ) )
        {
            mode_s[i++] = 'a';
        }
        return fopen( file.string( ).c_str( ), mode_s );
#endif
    }

    inline std::string q( const path& str )
    {
        return q( str.string( ) );
    }

    inline std::string qo( const path& str )
    {
        return qo( str.string( ) );
    }

    inline bool is_file( const path& p )
    {
        return is_regular_file( p ) || ( is_symlink( p ) && is_regular_file( read_symlink( p ) ) );
    }

    inline path find_in_path( const path& bin )
    {
        std::vector<std::string> dirs = split( std::getenv( "PATH" ), DMK_IF_WIN( ';', ':' ) );
        for ( auto dir : dirs )
        {
            if ( is_file( path( dir ) / bin ) )
            {
                return path( dir ) / bin;
            }
        }
        throw error( "Can't find {} in PATH", bin.string( ) );
    }

    template <typename _Type>
    inline void file_get_bytes( const path& filename, _Type& bytes, open_mode mode = open_mode::None )
    {
        typedef typename _Type::pointer pointer;
        static_assert( sizeof( typename _Type::value_type ) == 1, "file_get_bytes: incorrect element size" );
        FILE* f = open_file( filename, open_mode::Read | mode );
        if ( !f )
        {
            throw file_error( system_error, "file_get_bytes: Can't open file for read {}", filename );
        }

        static const size_t bufsize = 1024;

        bytes = _Type( );
        byte_t buffer[bufsize];
        for ( ;; )
        {
            size_t rsize = fread( buffer, 1, bufsize, f );
            if ( !rsize )
            {
                break;
            }
            bytes.insert( bytes.end( ), ( pointer )buffer, ( pointer )buffer + rsize );
        }

        fclose( f );
    }

    template <typename _Type>
    inline void file_put_bytes( const path& filename, const _Type& bytes, open_mode mode = open_mode::None )
    {
        FILE* f = open_file( filename, open_mode::Write | mode );
        if ( !f )
        {
            throw file_error( system_error, "file_get_bytes: Can't open file for write {}", filename );
        }
        if ( bytes.size( ) )
        {
            fwrite( ( const void* )bytes.data( ), 1, bytes.size( ), f );
        }
        fclose( f );
    }

    inline std::string file_get_string( const path& filename )
    {
        std::string result;
        file_get_bytes( filename, result );
        return result;
    }

    inline std::vector<byte_t> file_get_data( const path& filename )
    {
        std::vector<byte_t> result;
        file_get_bytes( filename, result, open_mode::Binary );
        return result;
    }

    template <typename _JsonFormat = json_format<>>
    inline json file_get_json( const path& filename )
    {
        return json::parse<_JsonFormat>( file_get_string( filename ) );
    }

    template <typename _JsonFormat = json_format<>>
    inline json file_get_json_object( const path& filename )
    {
        return json::parse_object<_JsonFormat>( file_get_string( filename ) );
    }

    inline void file_put_string( const path& filename, const std::string& string )
    {
        file_put_bytes( filename, string );
    }

    inline void file_put_data( const path& filename, const std::vector<byte_t>& data )
    {
        file_put_bytes( filename, data, open_mode::Binary );
    }

    template <bool _pretty_print = true, typename _JsonFormat = json_format<>>
    inline void file_put_json( const path& filename, const json& json )
    {
        file_put_string( filename, json.stringify<_pretty_print, _JsonFormat>( ) );
    }

    template <bool _pretty_print = true, typename _JsonFormat = json_format<>>
    inline void file_put_json_object( const path& filename, const json& json )
    {
        file_put_string( filename, json.stringify_object<_pretty_print, _JsonFormat>( ) );
    }

    inline void file_append_string( const path& filename, const std::string& string )
    {
        file_put_bytes( filename, string, open_mode::Append );
    }

    inline void file_append_data( const path& filename, const std::vector<byte_t>& data )
    {
        file_put_bytes( filename, data, open_mode::Append | open_mode::Binary );
    }

    inline std::vector<path> entries( const path& dir, const std::string& mask = "*" )
    {
        std::vector<path> list;
        for ( auto p : directory_iterator( dir ) )
        {
            if ( matches( mask, p.path( ).filename( ).string( ) ) )
            {
                list.push_back( p.path( ) );
            }
        }
        return list;
    }

    inline void remove_if_exists( const path& file )
    {
        if ( exists( file ) )
            remove( file );
    }

    inline bool is_empty_directory( const path& dir )
    {
        return is_directory( dir ) && is_empty( dir );
    }

    inline bool is_nonempty_directory( const path& dir )
    {
        return is_directory( dir ) && !is_empty( dir );
    }

    inline path unique_path( const path& dir,
                             const std::string& suffix  = ".tmp",
                             const std::string& pattern = "file%04d" )
    {
        std::string full_pattern = pattern + suffix;
        int n = 1;
        while ( exists( dir / fmt::sprintf( full_pattern, n ) ) )
        {
            n++;
        }
        return dir / fmt::sprintf( full_pattern, n );
    }

    inline void touch_file( const path& p )
    {
#if defined( DMK_OS_WIN )
        std::wstring wstr = p.wstring( );
        HANDLE h =
            CreateFileW( wstr.c_str( ), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
        if ( h == INVALID_HANDLE_VALUE )
        {
            throw error( system_error, "Can't touch file {}", p.string( ) );
        }
        CloseHandle( h );
#elif defined( DMK_OS_POSIX )
        FILE* fp = fopen( p.string( ).c_str( ), "w" );
        if ( !fp )
        {
            throw error( system_error, "Can't touch file {}", p.string( ) );
        }
        fclose( fp );
#endif
    }

    typedef std::vector<path> path_list;

    inline path_list operator+( const path_list& left, const path_list& right )
    {
        path_list result = left;
        result.insert( result.end( ), right.begin( ), right.end( ) );
        return result;
    }
    inline path_list& operator+=( path_list& left, const path_list& right )
    {
        left.insert( left.end( ), right.begin( ), right.end( ) );
        return left;
    }
}
