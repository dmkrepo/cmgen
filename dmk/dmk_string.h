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
#include "dmk_assert.h"
#include "dmk_memory.h"
#include <string>
#include <vector>
#include <iostream>
#include <tuple>
#include <type_traits>

#include <cppformat/format.h>

#if defined( DMK_COMPILER_MSVC )
#pragma execution_character_set( "utf-8" )
#endif

namespace dmk
{
#define REPL_CHAR ( 0xFFFD ) // �

    inline std::string asci_lowercase( const std::string& str )
    {
        std::string temp = str;
        std::transform( str.begin( ), str.end( ), temp.begin( ), ::tolower );
        return temp;
    }

    inline std::string asci_uppercase( const std::string& str )
    {
        std::string temp = str;
        std::transform( str.begin( ), str.end( ), temp.begin( ), ::toupper );
        return temp;
    }

    inline std::string asci_lowercase( std::string&& str )
    {
        std::string temp = std::move( str );
        std::transform( str.begin( ), str.end( ), temp.begin( ), ::tolower );
        return temp;
    }

    inline std::string asci_uppercase( std::string&& str )
    {
        std::string temp = std::move( str );
        std::transform( str.begin( ), str.end( ), temp.begin( ), ::toupper );
        return temp;
    }

    template <typename _T>
    inline std::string stringify( const _T& value )
    {
        std::stringstream mem;
        mem << value;
        return mem.str( );
    }

    template <typename uchar>
    struct utf_coder
    {
    public:
        size_t encode_length( char32_t codepoint ) const;
        size_t decode_length( const uchar* buffer ) const;
        void encode( size_t length, char32_t codepoint, uchar* buffer );
        void decode( size_t length, char32_t& codepoint, const char* buffer );
    };

    template <>
    struct utf_coder<char>
    {
    public:
        static const size_t number = 1;

        size_t encode_length( char32_t input ) const
        {
            if ( input < 0x80 )
                return 1;
            else if ( input < 0x800 )
                return 2;
            else if ( input < 0x10000 )
                return 3;
            else if ( input < 0x200000 )
                return 4;
            else if ( input < 0x4000000 )
                return 5;
            else if ( input < 0x80000000u )
                return 6;
            else
                return 0;
        }
        size_t decode_length( const char* input ) const
        {
            static const uint8_t trailing_bytes[128] = //
                {
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 80-8F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 90-9F
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A0-AF
                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B0-BF
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // C0-CF
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // D0-DF
                  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // E0-EF
                  3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5 //  F0-FF
                };
            //    F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF
            uint8_t first = static_cast<uint8_t>( *input );
            return 1 + ( first < 0x80 ? 0 : trailing_bytes[first - 0x80] );
        }
        void encode( size_t length, char32_t codepoint, char* buffer )
        {
            static const uint8_t first_bytes[7] = { 0xFF, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
            switch ( length )
            {
            case 7:
                buffer[3] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 6:
                buffer[3] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 5:
                buffer[3] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 4:
                buffer[3] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 3:
                buffer[2] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 2:
                buffer[1] = static_cast<uint8_t>( ( codepoint | 0x80 ) & 0xBF );
                codepoint >>= 6;
            case 1:
                buffer[0] = static_cast<uint8_t>( codepoint | first_bytes[length] );
            }
        }
        void decode( size_t length, char32_t& codepoint, const char* buffer )
        {
            static const uint32_t code_offsets[7] = { 0xFFFFFFFF, 0x00000000, 0x00003080, 0x000E2080,
                                                      0x03C82080, 0xFA082080, 0x82082080 };
            char32_t o = 0;
            switch ( length )
            {
            case 6:
                o += static_cast<uint8_t>( *buffer++ );
                o <<= 6;
            case 5:
                o += static_cast<uint8_t>( *buffer++ );
                o <<= 6;
            case 4:
                o += static_cast<uint8_t>( *buffer++ );
                o <<= 6;
            case 3:
                o += static_cast<uint8_t>( *buffer++ );
                o <<= 6;
            case 2:
                o += static_cast<uint8_t>( *buffer++ );
                o <<= 6;
            case 1:
                o += static_cast<uint8_t>( *buffer++ );
            }
            codepoint = o - code_offsets[length];
        }
    };

    template <>
    struct utf_coder<char16_t>
    {
    public:
        static const size_t number = 3;

        size_t encode_length( char32_t input ) const
        {
            if ( input < 0x10000 )
                return 1;
            else
                return 2;
        }
        size_t decode_length( const char16_t* input ) const
        {
            char16_t first = *input;
            return first >= 0xD800 && first < 0xDC00 ? 2 : 1;
        }
        void encode( size_t length, char32_t codepoint, char16_t* buffer )
        {
            switch ( length )
            {
            case 1:
                buffer[0] = codepoint;
                break;
            case 2:
                codepoint -= 0x10000;
                buffer[0] = 0xD800 | ( ( codepoint >> 10 ) & 0x3FF );
                buffer[1] = 0xDC00 | ( codepoint & 0x3FF );
                break;
            }
        }
        void decode( size_t length, char32_t& codepoint, const char16_t* buffer )
        {
            switch ( length )
            {
            case 1:
                codepoint = buffer[0];
                break;
            case 2:
                codepoint = ( ( buffer[0] & 0x3FF ) << 10 ) | ( buffer[1] & 0x3FF );
                break;
            }
        }
    };

    template <>
    struct utf_coder<char32_t>
    {
    public:
        static const size_t number = 6;

        size_t encode_length( char32_t input ) const
        {
            return 1;
        }
        size_t decode_length( const char32_t* input ) const
        {
            return 1;
        }
        void encode( size_t, char32_t codepoint, char32_t* buffer )
        {
            *buffer = codepoint;
        }
        void decode( size_t, char32_t& codepoint, const char32_t* buffer )
        {
            codepoint = *buffer;
        }
    };

    using utf_wchar_t = std::conditional_t<sizeof( wchar_t ) == sizeof( char16_t ), char16_t, char32_t>;

    using utf_coder_wchar = utf_coder<utf_wchar_t>;

    template <>
    struct utf_coder<wchar_t> : public utf_coder_wchar
    {
    public:
        static const size_t number = utf_coder_wchar::number;

        size_t encode_length( char32_t input ) const
        {
            return utf_coder_wchar::encode_length( input );
        }
        size_t decode_length( const wchar_t* input ) const
        {
            return utf_coder_wchar::decode_length( reinterpret_cast<const utf_wchar_t*>( input ) );
        }
        void encode( size_t length, char32_t codepoint, wchar_t* buffer )
        {
            return utf_coder_wchar::encode( length, codepoint, reinterpret_cast<utf_wchar_t*>( buffer ) );
        }
        void decode( size_t length, char32_t& codepoint, const wchar_t* buffer )
        {
            return utf_coder_wchar::decode(
                length, codepoint, reinterpret_cast<const utf_wchar_t*>( buffer ) );
        }
    };

    template <typename uchar>
    inline const uchar* advance( size_t pos, const uchar* begin, const uchar* end )
    {
        utf_coder<uchar> coder;
        size_t len = coder.decode_length( begin );
        return std::min( begin + len, end );
    }

    template <typename uchar>
    inline const uchar* advance( size_t pos, const uchar* begin )
    {
        utf_coder<uchar> coder;
        size_t len = coder.decode_length( begin );
        return begin + len;
    }

    template <typename ochar, typename ichar>
    inline size_t recode( const ichar* input_begin,
                          const ichar* input_end,
                          ochar* output_begin,
                          ochar* output_end )
    {
        utf_coder<ichar> decoder;
        utf_coder<ochar> encoder;
        size_t count;
        if ( input_begin == input_end )
        {
            return 0;
        }
        for ( count = 0;; )
        {
            size_t dlen = decoder.decode_length( input_begin );
            if ( input_begin + dlen > input_end )
                break;
            char32_t cp;
            decoder.decode( dlen, cp, input_begin );
            input_begin += dlen;
            size_t elen = encoder.encode_length( cp );
            if ( output_begin + elen > output_end )
                break;
            encoder.encode( elen, cp, output_begin );
            output_begin += elen;
            count += elen;
        }
        return count;
    }

    template <typename ochar, typename ichar>
    inline constexpr size_t recode_factor( )
    {
        return std::max( ( size_t )1, utf_coder<ichar>::number / utf_coder<ochar>::number );
    }

    template <typename ochar, typename ichar>
    inline std::basic_string<ochar> recode( const ichar* input_begin, const ichar* input_end )
    {
        size_t length = recode_factor<ochar, ichar>( ) * ( input_end - input_begin );
        std::basic_string<ochar> result( length, 0 );
        length = recode<ochar, ichar>(
            input_begin, input_end, &*result.begin( ), &*result.begin( ) + result.size( ) );
        result.resize( length );
        return result;
    }

    template <typename ochar, typename ichar>
    inline std::basic_string<ochar> recode( const ichar* input_begin )
    {
        const ichar* input_end = input_begin;
        while ( *input_end )
            input_end++;
        return recode<ochar, ichar>( input_begin, input_end );
    }

    template <typename ochar, typename ichar>
    inline std::basic_string<ochar> recode( const std::basic_string<ichar>& string )
    {
        return recode<ochar, ichar>( &*string.begin( ), &*string.begin( ) + string.size( ) );
    }

    inline const std::string& make_string( const std::string& s )
    {
        return s;
    }

    inline std::string&& make_string( std::string&& s )
    {
        return std::move( s );
    }

    inline std::string make_string( const char* s )
    {
        return std::string( s );
    }

    template <size_t N>
    inline std::string make_string( const char( &s )[N] )
    {
        return std::string( s, s + N - 1 );
    }

    inline std::string make_string( const std::wstring& s )
    {
        return recode<char>( s );
    }

    inline std::string make_string( const wchar_t* s )
    {
        return recode<char>( s );
    }

    template <size_t N>
    inline std::string make_string( const wchar_t( &s )[N] )
    {
        return recode<char>( s, s + N - 1 );
    }

    inline std::string make_string( const std::u16string& s )
    {
        return recode<char>( s );
    }

    inline std::string make_string( const char16_t* s )
    {
        return recode<char>( s );
    }

    template <size_t N>
    inline std::string make_string( const char16_t( &s )[N] )
    {
        return recode<char>( s, s + N - 1 );
    }

    inline std::string make_string( const std::u32string& s )
    {
        return recode<char>( s );
    }

    inline std::string make_string( const char32_t* s )
    {
        return recode<char>( s );
    }

    template <size_t N>
    inline std::string make_string( const char32_t( &s )[N] )
    {
        return recode<char>( s, s + N - 1 );
    }

    inline std::string hex( const std::string& str )
    {
        std::string result = "";
        for ( char c : str )
        {
            if ( c < 0x20 || c >= 0x7F )
            {
                result += fmt::format( "\\x{:02X}", uint8_t( c ) );
            }
            else
            {
                result += c;
            }
        }
        return result;
    }

    inline std::vector<std::string> tokenize( const std::string& str )
    {
        size_t param_end = 0;
        std::vector<std::string> args;
        while ( param_end < str.size( ) )
        {
            size_t param_start = str.find_first_not_of( ' ', param_end );
            if ( param_start == std::string::npos )
                break;
            param_end = str.find_first_of( ' ', param_start );
            if ( param_end == std::string::npos )
                param_end = str.size( );
            args.push_back( str.substr( param_start, param_end - param_start ) );
        }
        return args;
    }

    inline std::vector<std::string> tokenize_params( const std::string& str )
    {
        size_t param_end = 0;
        std::vector<std::string> args;
        while ( param_end < str.size( ) )
        {
            size_t param_start = str.find_first_not_of( "\t ", param_end );
            if ( param_start == std::string::npos )
                break;
            if ( str[param_start] == '"' )
                param_end = str.find( '"', param_start ) + 1;
            else
                param_end = str.find_first_of( " \t", param_start );
            if ( param_end == std::string::npos )
                param_end = str.size( );
            args.push_back( str.substr( param_start, param_end - param_start ) );
        }
        return args;
    }

    inline std::string replace_one( const std::string& str, const std::string& from, const std::string& to )
    {
        std::string r    = str;
        size_t start_pos = 0;
        if ( ( start_pos = r.find( from, start_pos ) ) != std::string::npos )
        {
            r.replace( start_pos, from.size( ), to );
        }
        return r;
    }

    inline std::string replace_one( const std::string& str, char from, char to )
    {
        std::string r    = str;
        size_t start_pos = 0;
        if ( ( start_pos = r.find( from, start_pos ) ) != std::string::npos )
        {
            r[start_pos] = to;
        }
        return r;
    }

    inline std::string replace_all( const std::string& str, const std::string& from, const std::string& to )
    {
        std::string r    = str;
        size_t start_pos = 0;
        while ( ( start_pos = r.find( from, start_pos ) ) != std::string::npos )
        {
            r.replace( start_pos, from.size( ), to );
            start_pos += to.size( );
        }
        return r;
    }

    inline std::string replace_all( const std::string& str, char from, char to )
    {
        std::string r    = str;
        size_t start_pos = 0;
        while ( ( start_pos = r.find( from, start_pos ) ) != std::string::npos )
        {
            r[start_pos] = to;
            start_pos++;
        }
        return r;
    }

    inline std::string q( const std::string& str )
    {
        return '"' + str + '"';
    }

    inline std::string q( const char* str )
    {
        return q( std::string( str ) );
    }

    inline std::string qo( const std::string& str )
    {
        if ( !str.empty( ) && str.find_first_of( " \"'<>&|" ) == std::string::npos )
            return str;
        return '"' + str + '"';
    }

    inline std::string qo( const char* str )
    {
        return qo( std::string( str ) );
    }

#define _S_ ( std::string )

    enum class line_ending
    {
        CRLF,
        LF,
        Unknown,
#if defined DMK_OS_WIN
        Native = CRLF
#else
        Native = LF
#endif
    };

    static const char* line_endings[3] = { "\r\n", "\n", "\n" };

    inline line_ending detect_line_ending( const std::string& str )
    {
        size_t p = str.find_first_of( "\r\n" );
        if ( p == std::string::npos )
        {
            return line_ending::Unknown;
        }
        return str[p] == '\n' ? line_ending::LF : line_ending::CRLF;
    }

    inline std::string append_line( const std::string& text, const std::string& line )
    {
        line_ending le = detect_line_ending( text );
        return text + line_endings[static_cast<int>( le )] + line;
    }

    inline std::string prepend_line( const std::string& text, const std::string& line )
    {
        line_ending le = detect_line_ending( text );
        return line + line_endings[static_cast<int>( le )] + text;
    }

    inline bool contains( const std::string& text, const std::string& substr )
    {
        return text.find( substr ) != std::string::npos;
    }

    inline bool ends_with( const std::string& text, const std::string& end )
    {
        return text.size( ) >= end.size( ) && text.substr( text.size( ) - end.size( ), end.size( ) ) == end;
    }

    inline bool begins_with( const std::string& text, const std::string& begin )
    {
        return text.size( ) >= begin.size( ) && text.substr( 0, begin.size( ) ) == begin;
    }

    inline bool contains( const std::string& text, char substr )
    {
        return text.find( substr ) != std::string::npos;
    }

    inline bool ends_with( const std::string& text, char end )
    {
        return text.size( ) && text.back( ) == end;
    }

    inline bool begins_with( const std::string& text, char begin )
    {
        return text.size( ) && text.front( ) == begin;
    }

    inline std::vector<std::string> split( const std::string& text, const std::string& delimeter )
    {
        std::string r    = text;
        size_t prev_pos  = 0;
        size_t start_pos = 0;
        std::vector<std::string> list;
        while ( ( start_pos = r.find( delimeter, prev_pos ) ) != std::string::npos )
        {
            list.push_back( text.substr( prev_pos, start_pos - prev_pos ) );
            prev_pos = start_pos + delimeter.size( );
        }
        list.push_back( text.substr( prev_pos ) );
        return list;
    }

    inline std::vector<std::string> split( const std::string& text, char delimeter )
    {
        std::string r    = text;
        size_t prev_pos  = 0;
        size_t start_pos = 0;
        std::vector<std::string> list;
        while ( ( start_pos = r.find( delimeter, prev_pos ) ) != std::string::npos )
        {
            list.push_back( text.substr( prev_pos, start_pos - prev_pos ) );
            prev_pos = start_pos + 1;
        }
        list.push_back( text.substr( prev_pos ) );
        return list;
    }

    inline std::string join( const std::vector<std::string>& list, const std::string& delimeter )
    {
        std::string text;
        for ( const std::string& item : list )
        {
            if ( !text.empty( ) )
            {
                text += delimeter;
            }
            text += item;
        }
        return text;
    }

    inline std::string join( const std::vector<std::string>& list, char delimeter )
    {
        std::string text;
        for ( const std::string& item : list )
        {
            if ( !text.empty( ) )
            {
                text += delimeter;
            }
            text += item;
        }
        return text;
    }

    inline std::vector<std::string> split_lines( const std::string& text, line_ending& le )
    {
        le = detect_line_ending( text );
        return split( text, line_endings[( int )le] );
    }

    inline std::vector<std::string> split_lines( const std::string& text )
    {
        line_ending le;
        return split_lines( text, le );
    }

    inline std::string join_lines( const std::vector<std::string>& lines, line_ending le = line_ending::LF )
    {
        return join( lines, line_endings[( int )le] );
    }

    inline size_t count_substr( const std::string& text, const std::string& substr )
    {
        if ( substr.length( ) == 0 )
            return 0;
        size_t count = 0;
        for ( size_t offset = text.find( substr ); offset != std::string::npos;
              offset = text.find( substr, offset + substr.length( ) ) )
        {
            ++count;
        }
        return count;
    }

    inline size_t count_substr( const std::string& text, char substr )
    {
        size_t count = 0;
        for ( size_t offset = text.find( substr ); offset != std::string::npos;
              offset = text.find( substr, offset + 1 ) )
        {
            ++count;
        }
        return count;
    }

    inline std::string exclude_trailing( const std::string& text, const std::string& end )
    {
        if ( ends_with( text, end ) )
        {
            return text.substr( 0, text.size( ) - end.size( ) );
        }
        return text;
    }

    inline std::string exclude_leading( const std::string& text, const std::string& begin )
    {
        if ( begins_with( text, begin ) )
        {
            return text.substr( begin.size( ) );
        }
        return text;
    }

    inline std::string exclude_trailing( const std::string& text, char end )
    {
        if ( ends_with( text, end ) )
        {
            return text.substr( 0, text.size( ) - 1 );
        }
        return text;
    }

    inline std::string exclude_leading( const std::string& text, char begin )
    {
        if ( begins_with( text, begin ) )
        {
            return text.substr( 1 );
        }
        return text;
    }

    inline bool erase_trailing( std::string& text, const std::string& end )
    {
        if ( ends_with( text, end ) )
        {
            text.erase( text.size( ) - end.size( ), end.size( ) );
            return true;
        }
        return false;
    }

    inline bool erase_leading( std::string& text, const std::string& begin )
    {
        if ( begins_with( text, begin ) )
        {
            text.erase( 0, begin.size( ) );
            return true;
        }
        return false;
    }

    inline bool erase_trailing( std::string& text, char end )
    {
        if ( ends_with( text, end ) )
        {
            text.erase( text.size( ) - 1, 1 );
            return true;
        }
        return false;
    }

    inline bool erase_leading( std::string& text, char begin )
    {
        if ( begins_with( text, begin ) )
        {
            text.erase( 0, 1 );
            return true;
        }
        return false;
    }

    template <typename Tuple, std::size_t... I>
    inline std::string fmt_impl( fmt::CStringRef str, Tuple&& t, std::index_sequence<I...> )
    {
        return fmt::format( str, std::get<I>( t )... );
    }

    template <typename Tuple, std::size_t... I>
    inline std::wstring fmt_impl( fmt::WCStringRef str, Tuple&& t, std::index_sequence<I...> )
    {
        return fmt::format( str, std::get<I>( t )... );
    }

    namespace percentop
    {
        template <typename... _Types>
        inline auto tup( _Types&&... args ) -> decltype( std::make_tuple( std::forward<_Types>( args )... ) )
        {
            return std::make_tuple( std::forward<_Types>( args )... );
        }

        template <typename... _Types>
        inline std::string operator%( fmt::CStringRef str, const std::tuple<_Types...>& tuple )
        {
            return fmt_impl( str, tuple, std::index_sequence_for<_Types...>( ) );
        }

        template <typename... _Types>
        inline std::wstring operator%( fmt::WCStringRef str, const std::tuple<_Types...>& tuple )
        {
            return fmt_impl( str, tuple, std::index_sequence_for<_Types...>( ) );
        }
    }

    using namespace percentop;

    enum class text_case : int
    {
        nochange,
        upper,
        lower
    };

    inline std::string transform_case( const std::string& text, text_case newcase )
    {
        std::string result;
        switch ( newcase )
        {
        case text_case::upper:
            result = asci_uppercase( text );
            break;
        case text_case::lower:
            result = asci_lowercase( text );
            break;
        case text_case::nochange:
            result = text;
            break;
        }
        return result;
    }

    inline std::string transform_case( std::string&& text, text_case newcase )
    {
        std::string result;
        switch ( newcase )
        {
        case text_case::upper:
            result = asci_uppercase( std::move( text ) );
            break;
        case text_case::lower:
            result = asci_lowercase( std::move( text ) );
            break;
        case text_case::nochange:
            result = std::move( text );
            break;
        }
        return result;
    }

    // abc*
    // *xyz
    // *def*
    // !*ijk*
    // pattern1,pattern2
    inline bool matches( const std::string& pattern, const std::string& text, char delimeter = ',' )
    {
        if ( pattern == "*" )
            return true;
        if ( pattern.empty( ) )
            return text.empty( );
        std::vector<std::string> patterns = split( pattern, delimeter );
        if ( patterns.size( ) > 1 )
        {
            for ( const std::string& p : patterns )
            {
                if ( matches( p, text ) )
                    return true;
            }
            return false;
        }
        if ( pattern[0] == '!' )
        {
            return matches( pattern.substr( 1 ), text );
        }
        std::string lpattern = asci_lowercase( pattern );
        std::string ltext    = asci_lowercase( text );
        if ( erase_leading( lpattern, '*' ) )
        {
            if ( erase_trailing( lpattern, '*' ) )
            {
                return contains( ltext, lpattern );
            }
            else
            {
                return ends_with( ltext, lpattern );
            }
        }
        else
        {
            if ( erase_trailing( lpattern, '*' ) )
            {
                return begins_with( ltext, lpattern );
            }
            else
            {
                return lpattern == ltext;
            }
        }
    }

    template <typename _Type>
    inline std::vector<_Type> reversed( const std::vector<_Type>& list )
    {
        std::vector<_Type> result = list;
        std::reverse( result.begin( ), result.end( ) );
        return result;
    }

    template <typename _Type>
    inline std::vector<_Type> reversed( std::vector<_Type>&& list )
    {
        std::vector<_Type> result = std::move( list );
        std::reverse( result.begin( ), result.end( ) );
        return result;
    }

} // namespace dmk
