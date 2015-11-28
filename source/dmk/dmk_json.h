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
#include "dmk_assert.h"
#include "dmk_result.h"

#include <string>
#include <vector>
#include <map>
#include <exception>

namespace dmk
{

    template <typename _Type>
    struct ordered_map : public std::vector<std::pair<std::string, _Type>>
    {
    public:
        typedef std::vector<std::pair<std::string, _Type>> inherited;
        typedef typename inherited::iterator iterator;
        typedef typename inherited::const_iterator const_iterator;

        const_iterator find( const std::string& key ) const
        {
            for ( const_iterator it = this->begin( ); it != this->end( ); ++it )
            {
                if ( it->first == key )
                {
                    return it;
                }
            }
            return this->end( );
        }
        iterator find( const std::string& key )
        {
            for ( iterator it = this->begin( ); it != this->end( ); ++it )
            {
                if ( it->first == key )
                {
                    return it;
                }
            }
            return this->end( );
        }
        void insert_or_assign( const std::string& key, const _Type& value )
        {
            iterator it = find( key );
            if ( it != this->end( ) )
            {
                it->second = value;
            }
            else
            {
                this->push_back( { key, value } );
            }
        }
        void insert_or_assign( const std::string& key, _Type&& value )
        {
            iterator it = find( key );
            if ( it != this->end( ) )
            {
                it->second = std::move( value );
            }
            else
            {
                this->push_back( { key, value } );
            }
        }
        _Type& operator[]( const std::string& key )
        {
            iterator it = find( key );
            if ( it != this->end( ) )
            {
                return it->second;
            }
            else
            {
                this->push_back( { key, _Type( ) } );
                return this->back( ).second;
            }
        }
        const _Type& operator[]( const std::string& key ) const
        {
            const_iterator it = find( key );
            if ( it != this->end( ) )
            {
                return it->second;
            }
            else
            {
                const _Type empty;
                return empty;
            }
        }
    };

    class json_error : public error
    {
    public:
        using error::error;
    };

    template <char _open>
    struct closed_bracket : public std::integral_constant<char, _open>
    {
    };

    template <>
    struct closed_bracket<'['> : public std::integral_constant<char, ']'>
    {
    };

    template <>
    struct closed_bracket<'('> : public std::integral_constant<char, ')'>
    {
    };

    template <>
    struct closed_bracket<'{'> : public std::integral_constant<char, '}'>
    {
    };

    template <>
    struct closed_bracket<'<'> : public std::integral_constant<char, '>'>
    {
    };

    template <char _key_separator  = ':',
              char _item_separator = ',',
              char _array_bracket  = '[',
              char _object_bracket = '{',
              bool _optional_quote = false>
    struct json_format
    {
    public:
        static const char key_separator  = _key_separator;
        static const char item_separator = _item_separator;
        static const char array_bracket  = _array_bracket;
        static const char object_bracket = _object_bracket;
        static const bool optional_quote = _optional_quote;
    };

    template <bool _pretty_print = true, typename _JsonFormat = json_format<>>
    struct json_printer;

    template <typename _JsonFormat = json_format<>>
    struct json_parser;

    struct json;

    template <typename _TargetType>
    inline _TargetType json_cast( const json& value );

    struct json // sizeof(json) = 12
    {
    public:
        template <bool _pretty_print, typename _JsonFormat>
        friend struct json_printer;
        template <typename _JsonFormat>
        friend struct json_parser;

        template <typename _JsonFormat = json_format<>>
        static json parse( const std::string& data )
        {
            json_parser<_JsonFormat> parser;
            return parser.parse_json( data.c_str( ) );
        }

        template <typename _JsonFormat = json_format<>>
        static json parse_object( const std::string& data )
        {
            json_parser<_JsonFormat> parser;
            return parser.parse_naked_object( data.c_str( ) );
        }

        template <bool _pretty_print = true, typename _JsonFormat = json_format<>>
        std::string stringify( ) const
        {
            json_printer<_pretty_print, _JsonFormat> printer;
            return printer.print_json( *this );
        }

        template <bool _pretty_print = true, typename _JsonFormat = json_format<>>
        std::string stringify_object( ) const
        {
            if ( is_object( ) )
            {
                json_printer<_pretty_print, _JsonFormat> printer;
                printer.print_naked_object( *m_obj );
                return printer.wr.str( );
            }
            else
                throw json_error( "Specified value is not object" );
        }

    public:
        typedef std::vector<json> array;
        typedef ordered_map<json> object;
        typedef std::pair<std::string, json> objectpair;
        static json readonly_array_item;
        static json readonly_object_item;
        void check_readonly( ) const
        {
            if ( this == &readonly_array_item )
            {
                throw json_error( "json value is not array" );
            }
            if ( this == &readonly_object_item )
            {
                throw json_error( "json value is not object" );
            }
        }
        enum type : uint32_t
        {
            /*0*/ Null,
            /*1*/ Bool,
            /*2*/ Int,
            /*3*/ Double,
            /*4*/ String,
            /*5*/ Array,
            /*6*/ Object
        };
        json( )
        {
            assign_null( );
        }
        json( std::nullptr_t )
        {
            assign_null( );
        }
        json( const json& value )
        {
            assign( value );
        }
        json( json&& value )
        {
            assign_null( );
            swap( value );
        }
        json( const std::initializer_list<json>& list )
        {
            assign( list );
        }
        json( size_t count, const json& value )
        {
            assign( count, value );
        }
        json( bool value )
        {
            assign( value );
        }
        json( int8_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( uint8_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( int16_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( uint16_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( int32_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( uint32_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( int64_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( uint64_t value )
        {
            assign( static_cast<int64_t>( value ) );
        }
        json( float value )
        {
            assign( static_cast<double>( value ) );
        }
        json( double value )
        {
            assign( value );
        }
        json( std::string&& value )
        {
            assign( std::move( value ) );
        }
        json( const char* value )
        {
            assign( std::string( value ) );
        }
        template <size_t N>
        json( const char( &value )[N] )
        {
            assign( make_string( value ) );
        }
        json( const std::string& value )
        {
            assign( value );
        }
        json( const std::wstring& value )
        {
            assign( make_string( value ) );
        }
        json( const wchar_t* value )
        {
            assign( make_string( value ) );
        }
        template <size_t N>
        json( const wchar_t( &value )[N] )
        {
            assign( make_string( value ) );
        }
        json( const std::u16string& value )
        {
            assign( make_string( value ) );
        }
        json( const char16_t* value )
        {
            assign( make_string( value ) );
        }
        template <size_t N>
        json( const char16_t( &value )[N] )
        {
            assign( make_string( value ) );
        }
        json( const std::u32string& value )
        {
            assign( make_string( value ) );
        }
        json( const char32_t* value )
        {
            assign( make_string( value ) );
        }
        template <size_t N>
        json( const char32_t( &value )[N] )
        {
            assign( make_string( value ) );
        }
        json( const array& value )
        {
            assign( value );
        }
        json( array&& value )
        {
            assign( std::move( value ) );
        }
        json( const object& value )
        {
            assign( value );
        }
        json( object&& value )
        {
            assign( std::move( value ) );
        }
        json( const std::vector<std::string>& value )
        {
            assign( value );
        }
        json& operator=( json value )
        {
            return assign( value ), *this;
        }
        json& operator=( std::nullptr_t )
        {
            return assign_null( ), *this;
        }
        json& operator=( bool value )
        {
            return assign( value ), *this;
        }
        json& operator=( int8_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( uint8_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( int16_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( uint16_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( int32_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( uint32_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( int64_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( uint64_t value )
        {
            return assign( static_cast<int64_t>( value ) ), *this;
        }
        json& operator=( double value )
        {
            return assign( static_cast<double>( value ) ), *this;
        }
        json& operator=( float value )
        {
            return assign( static_cast<float>( value ) ), *this;
        }
        json& operator=( std::string&& value )
        {
            return assign( std::move( value ) ), *this;
        }
        json& operator=( const std::string& value )
        {
            return assign( value ), *this;
        }

        json& operator=( const std::wstring& value )
        {
            return assign( make_string( value ) ), *this;
        }
        json& operator=( const wchar_t* value )
        {
            return assign( make_string( value ) ), *this;
        }
        template <size_t N>
        json& operator=( const wchar_t( &value )[N] )
        {
            return assign( make_string( value ) ), *this;
        }

        json& operator=( const std::u16string& value )
        {
            return assign( make_string( value ) ), *this;
        }
        json& operator=( const char16_t* value )
        {
            return assign( make_string( value ) ), *this;
        }
        template <size_t N>
        json& operator=( const char16_t( &value )[N] )
        {
            return assign( make_string( value ) ), *this;
        }

        json& operator=( const std::u32string& value )
        {
            return assign( make_string( value ) ), *this;
        }
        json& operator=( const char32_t* value )
        {
            return assign( make_string( value ) ), *this;
        }
        template <size_t N>
        json& operator=( const char32_t( &value )[N] )
        {
            return assign( make_string( value ) ), *this;
        }

        json& operator=( const array& value )
        {
            return assign( value ), *this;
        }
        json& operator=( array&& value )
        {
            return assign( std::move( value ) ), *this;
        }
        json& operator=( const object& value )
        {
            return assign( value ), *this;
        }
        json& operator=( object&& value )
        {
            return assign( std::move( value ) ), *this;
        }
        json& operator=( const std::vector<std::string>& value )
        {
            assign( value );
            return *this;
        }

        template <typename _TargetType>
        _TargetType cast( ) const
        {
            return json_cast<_TargetType>( *this );
        }

        void swap( json& value )
        {
            std::swap( m_type, value.m_type );
            std::swap( m_i64, value.m_i64 );
        }

        bool is_null( ) const
        {
            return m_type == Null;
        }
        bool is_bool( ) const
        {
            return m_type == Bool;
        }
        bool is_int( ) const
        {
            return m_type == Int;
        }
        bool is_double( ) const
        {
            return m_type == Double;
        }
        bool is_string( ) const
        {
            return m_type == String;
        }
        bool is_array( ) const
        {
            return m_type == Array;
        }
        bool is_object( ) const
        {
            return m_type == Object;
        }
        type get_type( ) const
        {
            return m_type;
        }
        bool is_compound( ) const
        {
            return ( m_type == Array ) || ( m_type == Object );
        }
        bool is_number( ) const
        {
            return ( m_type == Int ) || ( m_type == Double );
        }
        double as_number( double default_value = 0.0 ) const
        {
            return as_double( default_value );
        }
        int64_t as_int( int64_t default_value = 0 ) const
        {
            return is_int( ) ? m_i64 : ( is_double( ) ? std::llround( m_f64 ) : is_bool( ) ? m_i64 != 0
                                                                                           : default_value );
        }
        double as_double( double default_value = 0 ) const
        {
            return is_double( ) ? m_f64 : ( is_int( ) ? m_i64 : default_value );
        }
        template <typename _Enum>
        std::enable_if_t<std::is_enum<_Enum>::value, _Enum> as_enum( _Enum default_value = _Enum( ) ) const
        {
            return is_number( ) ? static_cast<_Enum>( static_cast<int64_t>( as_number( ) ) ) : default_value;
        }
        std::string as_string( const std::string& default_value = "" ) const
        {
            return is_string( ) ? *m_str : default_value;
        }
        const array& as_array( ) const
        {
            static const array empty;
            return is_array( ) ? *m_arr : empty;
        }
        const object& as_object( ) const
        {
            static const object empty = object();
            return is_object( ) ? *m_obj : empty;
        }
        bool has_index( size_t index ) const
        {
            return is_array( ) && index < m_arr->size( );
        }
        bool has_key( const std::string& key ) const
        {
            return is_object( ) && m_obj->find( key ) != m_obj->end( );
        }
        std::vector<json> flatten( ) const
        {
            std::vector<json> result;
            switch ( m_type )
            {
            case json::Object:
                for ( const objectpair& o : *m_obj )
                {
                    result.push_back( o.second );
                }
                break;
            case json::Array:
                for ( const json& a : *m_arr )
                {
                    result.push_back( a );
                }
                break;
            case json::Null:
                break;
            default:
                result.push_back( *this );
                break;
            }
            return result;
        }
        std::vector<json> flatten_array( ) const
        {
            std::vector<json> result;
            switch ( m_type )
            {
            case json::Array:
                for ( const json& a : *m_arr )
                {
                    result.push_back( a );
                }
                break;
            case json::Null:
                break;
            default:
                result.push_back( *this );
                break;
            }
            return result;
        }
        std::vector<json> flatten_object( ) const
        {
            std::vector<json> result;
            switch ( m_type )
            {
            case json::Object:
                for ( const objectpair& o : *m_obj )
                {
                    result.push_back( o.second );
                }
                break;
            case json::Null:
                break;
            default:
                result.push_back( *this );
                break;
            }
            return result;
        }
        template <typename _Visitor>
        void for_each( _Visitor&& visitor )
        {
            switch ( m_type )
            {
            case json::Array:
                for ( json& a : *m_arr )
                {
                    visitor( "", a );
                }
                break;
            case json::Object:
                for ( objectpair& o : *m_obj )
                {
                    visitor( o.first, o.second );
                }
                break;
            }
        }
        size_t size( ) const
        {
            switch ( m_type )
            {
            case json::Array:
                return m_arr->size( );
            case json::Object:
                return m_obj->size( );
            }
            return 0;
        }
        std::string to_string( ) const
        {
            switch ( m_type )
            {
            case json::Null:
                return "null";
            case json::Bool:
                return m_i64 ? "true" : "false";
            case json::Int:
                return fmt::FormatInt( m_i64 ).c_str( );
            case json::Double:
                return std::to_string( m_f64 );
            case json::String:
                return *m_str;
            case json::Array:
                return array_str( *m_arr );
            case json::Object:
                return object_str( *m_obj );
            }
            return "null";
        }
        bool as_bool( ) const
        {
            switch ( m_type )
            {
            case json::Null:
                return false;
            case json::Bool:
                return m_i64 != 0;
            case json::Int:
                return m_i64 != 0;
            case json::Double:
                return m_f64 != 0.0;
            case json::String:
                return !m_str->empty( );
            case json::Array:
                return !m_arr->empty( );
            case json::Object:
                return !m_obj->empty( );
            }
            return false;
        }
        int64_t to_int( ) const
        {
            switch ( m_type )
            {
            case json::Null:
                return 0;
            case json::Bool:
                return m_i64 ? 1 : 0;
            case json::Int:
                return m_i64;
            case json::Double:
                return static_cast<int64_t>( m_f64 );
            case json::String:
                return 0;
            case json::Array:
                return 0;
            case json::Object:
                return 0;
            }
            return 0;
        }
        double to_double( ) const
        {
            switch ( m_type )
            {
            case json::Double:
                return m_f64;
            default:
                return ( double )to_int( );
            }
            return 0.0;
        }
        friend std::ostream& operator<<( std::ostream& os, const json& value )
        {
            os << value.to_string( );
            return os;
        }
        static std::string array_str( const array& arr )
        {
            std::string str;
            for ( const json& item : arr )
            {
                str += str.empty( ) ? item.to_string( ) : ", " + item.to_string( );
            }
            return "(" + str + ")";
        }
        static std::string object_str( const object& obj )
        {
            std::string str;
            for ( const objectpair& item : obj )
            {
                str += str.empty( ) ? item.first + ": " + item.second.to_string( )
                                    : ", " + item.first + ": " + item.second.to_string( );
            }
            return "(" + str + ")";
        }
        const json& operator[]( size_t index ) const
        {
            switch ( m_type )
            {
            case json::Array:
                return m_arr->operator[]( index );
            }
            return readonly_array_item;
        }
        json& operator[]( size_t index )
        {
            switch ( m_type )
            {
            case json::Array:
                if ( index >= m_arr->size( ) )
                {
                    m_arr->resize( index + 1 );
                    return m_arr->operator[]( index );
                }
                else
                {
                    return m_arr->operator[]( index );
                }
            }
            return readonly_array_item;
        }
        const json& operator[]( const std::string& key ) const
        {
            switch ( m_type )
            {
            case json::Object:
                return m_obj->operator[]( key );
            }
            return readonly_object_item;
        }
        json& operator[]( const std::string& key )
        {
            switch ( m_type )
            {
            case json::Object:
                return m_obj->operator[]( key );
            }
            return readonly_object_item;
        }
        void push_back( const json& value )
        {
            switch ( m_type )
            {
            case json::Array:
                return m_arr->push_back( value );
            }
        }
        void push_back( json&& value )
        {
            switch ( m_type )
            {
            case json::Array:
                return m_arr->push_back( std::move( value ) );
            }
        }
        array& get_array( )
        {
            switch ( m_type )
            {
            case json::Array:
                return *m_arr;
            }
            throw std::out_of_range( "json::operator[](size_t): type is not array" );
        }
        object& get_object( )
        {
            switch ( m_type )
            {
            case json::Object:
                return *m_obj;
            }
            throw std::out_of_range( "json::operator[](size_t): type is not object" );
        }
        const array& get_array( ) const
        {
            switch ( m_type )
            {
            case json::Array:
                return *m_arr;
            }
            throw std::out_of_range( "json::operator[](size_t): type is not array" );
        }
        const object& get_object( ) const
        {
            switch ( m_type )
            {
            case json::Object:
                return *m_obj;
            }
            throw std::out_of_range( "json::operator[](size_t): type is not object" );
        }
        void clear( )
        {
            json v;
            swap( v );
        }
        ~json( )
        {
            finalize( );
            m_type = Null;
            m_ptr  = nullptr;
        }

        bool operator==( const json& rhs ) const
        {
            if ( m_type != rhs.m_type )
                return false;
            switch ( m_type )
            {
            // case json::Null: // ok
            case json::Bool:
                return m_i64 == rhs.m_i64;
            case json::Int:
                return m_i64 == rhs.m_i64;
            case json::Double:
                return m_f64 == rhs.m_f64;
            case json::String:
                return *m_str == *rhs.m_str;
            case json::Array:
                return *m_arr == *rhs.m_arr;
            case json::Object:
                return *m_obj == *rhs.m_obj;
            }
            return false;
        }

        bool operator!=( const json& rhs ) const
        {
            return !operator==( rhs );
        }

    private:
        inline void finalize( )
        {
            if ( m_type >= String )
            {
                finalize_heap( );
            }
        }
        void finalize_heap( )
        {
            switch ( m_type )
            {
            case json::Null:
            case json::Bool:
            case json::Int:
            case json::Double:
                break;
            case json::String:
                delete m_str;
                break;
            case json::Array:
                delete m_arr;
                break;
            case json::Object:
                delete m_obj;
                break;
            }
        }
        void assign( json&& value )
        {
            check_readonly( );
            m_type = Null;
            m_i64 = 0;
            swap( value );
        }
        void assign( const std::initializer_list<json>& list )
        {
            check_readonly( );
            m_type = Array;
            m_arr  = new array( list );
        }
        void assign( const json& value )
        {
            check_readonly( );
            m_type = value.m_type;
            switch ( m_type )
            {
            case json::String:
                m_str = new std::string( *value.m_str );
                break;
            case json::Array:
                m_arr = new array( *value.m_arr );
                break;
            case json::Object:
                m_obj = new object( *value.m_obj );
                break;
            default:
                m_i64 = value.m_i64;
            }
        }
        void assign( size_t count, const json& value )
        {
            check_readonly( );
            m_type = Array;
            m_arr  = new array( count, value );
        }
        void assign_null( )
        {
            // check_readonly();
            m_type = Null;
            m_i64  = 0;
        }
        void assign( bool value )
        {
            check_readonly( );
            m_type = Bool;
            m_i64  = value ? 1 : 0;
        }
        void assign( int64_t value )
        {
            check_readonly( );
            m_type = Int;
            m_i64  = value;
        }
        void assign( double value )
        {
            check_readonly( );
            m_type = Double;
            m_f64  = value;
        }
        void assign( const std::string& value )
        {
            check_readonly( );
            m_type = String;
            m_str  = new std::string( value );
        }
        void assign( std::string&& value )
        {
            check_readonly( );
            m_type = String;
            m_str  = new std::string( std::move( value ) );
        }
        void assign( const array& value )
        {
            check_readonly( );
            m_type = Array;
            m_arr  = new array( value );
        }
        void assign( array&& value )
        {
            check_readonly( );
            m_type = Array;
            m_arr  = new array( std::move( value ) );
        }
        void assign( const object& value )
        {
            check_readonly( );
            m_type = Object;
            m_obj  = new object( value );
        }
        void assign( object&& value )
        {
            check_readonly( );
            m_type = Object;
            m_obj  = new object( std::move( value ) );
        }
        void assign( const std::vector<std::string>& value )
        {
            check_readonly( );
            m_type = Array;
            m_arr = new array( );
            m_arr->reserve( value.size( ) );
            for ( const std::string& s : value )
            {
                m_arr->push_back( s );
            }
        }

    private:
        union
        {
            int64_t m_i64;
            double m_f64;
            void* m_ptr;
            std::string* m_str;
            array* m_arr;
            object* m_obj;
        };
        type m_type;
    };

    template <>
    inline int json_cast<int>( const json& value )
    {
        return static_cast<int>( value.as_int( ) );
    }

    template <>
    inline unsigned int json_cast<unsigned int>( const json& value )
    {
        return static_cast<unsigned int>( value.as_int( ) );
    }

    template <>
    inline uint64_t json_cast<uint64_t>( const json& value )
    {
        return static_cast<uint64_t>( value.as_int( ) );
    }

    template <>
    inline int64_t json_cast<int64_t>( const json& value )
    {
        return static_cast<int64_t>( value.as_int( ) );
    }

    template <>
    inline double json_cast<double>( const json& value )
    {
        return value.as_double( );
    }

    template <>
    inline float json_cast<float>( const json& value )
    {
        return static_cast<float>( value.as_double( ) );
    }

    template <>
    inline bool json_cast<bool>( const json& value )
    {
        return value.as_bool( );
    }

    template <>
    inline std::vector<std::string> json_cast<std::vector<std::string>>( const json& value )
    {
        std::vector<std::string> result;
        const json::array& array = value.as_array( );
        result.reserve( array.size( ) );
        for ( const auto& a : array )
        {
            result.push_back( a.as_string( ) );
        }
        return result;
    }

    inline std::string operator||( const json& left, const std::string& right )
    {
        return left.as_string( right );
    }

    inline double operator||( const json& left, double right )
    {
        return left.as_double( right );
    }

    inline int64_t operator||( const json& left, int64_t right )
    {
        return left.as_int( right );
    }

    inline int operator||( const json& left, int right )
    {
        return left.as_int( right );
    }

    inline const json::array& operator||( const json& left, const json::array& right )
    {
        return left.is_array( ) ? left.as_array( ) : right;
    }

    inline const json::object& operator||( const json& left, const json::object& right )
    {
        return left.is_object( ) ? left.as_object( ) : right;
    }

    inline json::array operator+( const json::array& left, const json::array& right )
    {
        json::array a( left );
        a.insert( a.end( ), right.begin( ), right.end( ) );
        return a;
    }

    inline json::object operator+( const json::object& left, const json::object& right )
    {
        json::object o( left );
        for ( auto p : right )
        {
            o.insert_or_assign( p.first, p.second );
        }
        return o;
    }

    inline std::string q( const json& value )
    {
        return q( value.to_string( ) );
    }

    inline std::string qo( const json& value )
    {
        return qo( value.to_string( ) );
    }

    inline std::string join( const std::vector<json>& list, const std::string& delimeter )
    {
        std::string text;
        for ( const json& item : list )
        {
            if ( !text.empty( ) )
            {
                text += delimeter;
            }
            text += item.to_string( );
        }
        return text;
    }

    static const int json_max_depth = 200;

    template <typename _JsonFormat>
    struct json_parser
    {
    public:
        static const char key_separator  = _JsonFormat::key_separator;
        static const char item_separator = _JsonFormat::item_separator;
        static const char array_bracket  = _JsonFormat::array_bracket;
        static const char object_bracket = _JsonFormat::object_bracket;
        static const bool optional_quote = _JsonFormat::optional_quote;

        bool skip_comment( const char*& text )
        {
            if ( *text == '/' )
            {
                if ( text[1] == '/' ) // single-line
                {

                    text += 2;
                    while ( *text && *text != '\n' )
                    {
                        text++;
                    }
                    if ( *text == '\n' )
                    {
                        return true;
                    }
                }
                else if ( text[1] == '*' ) // multi-line
                {
                    text += 2;
                    while ( *text && ( *text != '*' || text[1] != '/' ) )
                    {
                        text++;
                    }
                    if ( *text == '*' && text[1] == '/' )
                    {
                        text++;
                        return true;
                    }
                }
            }
            return false;
        }

        bool skip_whitespace( const char*& text )
        {
            if ( *text == ' ' || *text == '\r' || *text == '\n' || *text == '\t' || skip_comment( text ) )
            {
                text++;
                while ( *text == ' ' || *text == '\r' || *text == '\n' || *text == '\t' ||
                        skip_comment( text ) )
                {
                    text++;
                }
                return true;
            }
            return false;
        }

        bool in_range( int32_t value, int32_t min, int32_t max )
        {
            return ( value >= min && value <= max );
        }
        std::string esc( char c )
        {
            char buffer[10];
            if ( byte_t( c ) >= 0x20 && byte_t( c ) <= 0x7f )
            {
                snprintf( buffer, sizeof( buffer ), "'%c' (%d)", c, c );
            }
            else
            {
                snprintf( buffer, sizeof( buffer ), "(%d)", c );
            }
            return buffer;
        }

        json parse_number( const char*& text )
        {
            char buffer[32];
            char* buf = buffer;
            bool real = false;
            for ( ;; )
            {
                switch ( *text )
                {
                case '.':
                case 'e':
                case 'E':
                    *buf++ = *text++;
                    real   = true;
                    continue;
                case '-':
                case '+':
                    *buf++ = *text++;
                    continue;
                default:
                    if ( *text >= '0' && *text <= '9' )
                    {
                        *buf++ = *text++;
                        continue;
                    }
                }
                break;
            }
            *buf = '\0';
            json value;
            if ( real )
            {
                char* end;
                value = std::strtod( buffer, &end );
                if ( end != buf )
                {
                    throw json_error( "Invalid number value: {}", buffer );
                }
            }
            else
            {
                char* end;
                value = std::strtoll( buffer, &end, 10 );
                if ( end != buf )
                {
                    throw json_error( "Invalid number value: {}", buffer );
                }
            }
            return value;
        }

        std::string parse_string( const char*& text )
        {
            fmt::MemoryWriter wr;
            if ( optional_quote )
            {
                if ( *text != '"' )
                {
                    while ( ( byte_t )*text > ' ' && *text != array_bracket && *text != object_bracket &&
                            *text != item_separator && *text != key_separator )
                    {
                        wr << *text++;
                    }
                    return wr.str( );
                }
            }
            if ( *text != '"' )
            {
                throw json_error( "Expected {}, got {}", esc( '"' ), esc( *text ) );
            }
            text++;

            for ( ;; )
            {
                switch ( *text )
                {
                case '\0':
                    throw json_error( "Unexpected end of string" );
                case '"':
                    text++;
                    return wr.str( );
                case '\\':
                    text++;
                    switch ( *text )
                    {
                    case '\0':
                        throw json_error( "Unexpected end of string" );
                    case 'n':
                        wr << '\n';
                        break;
                    case 'r':
                        wr << '\r';
                        break;
                    case 't':
                        wr << '\t';
                        break;
                    case 'b':
                        wr << '\b';
                        break;
                    case 'f':
                        wr << '\f';
                        break;
                    case '0':
                        wr << '\0';
                        break;
                    case '/':
                    case '\\':
                    case '"':
                        wr << *text;
                        break;
                    case 'u':
                        text++;
                        {
                            uint32_t codepoint = 0;
                            for ( size_t j = 0; j < 4; j++ )
                            {
                                codepoint <<= 4;
                                if ( text[j] >= '0' && text[j] <= '9' )
                                {
                                    codepoint += text[j] - '0';
                                }
                                else if ( text[j] >= 'a' && text[j] <= 'f' )
                                {
                                    codepoint += text[j] - 'a' + 10;
                                }
                                else if ( text[j] >= 'A' && text[j] <= 'F' )
                                {
                                    codepoint += text[j] - 'A' + 10;
                                }
                                else
                                {
                                    throw json_error( "Bad \\u escape" );
                                }
                            }
                            char utf8char[7];
                            utf_coder<char> encoder;
                            size_t len = encoder.encode_length( codepoint );
                            encoder.encode( len, codepoint, utf8char );
                            utf8char[len] = '\0';
                            wr << utf8char;
                        }
                        break;
                    default:
                        throw json_error( "Invalid escape character {}", esc( *text ) );
                    }
                    break;
                default:
                    if ( ( byte_t )*text < ' ' )
                    {
                        throw json_error( "Unescaped \\x{:02x} in string", *text );
                    }
                    else
                    {
                        wr << *text;
                    }
                }
                text++;
            }
        }

        json parse_array( const char*& text )
        {
            text++;
            skip_whitespace( text );
            if ( *text == closed_bracket<array_bracket>::value )
            {
                return text++, json::array( );
            }
            else
            {
                json::array temp;

                for ( ;; )
                {
                    json item = parse( text );
                    temp.push_back( item );
                    if ( item_separator == ' ' )
                    {
                        bool ws = skip_whitespace( text );
                        if ( *text == closed_bracket<array_bracket>::value )
                        {
                            return text++, temp;
                        }
                        if ( *text == '\0' || !ws )
                        {
                            throw json_error( "Unexpected end of array" );
                        }
                    }
                    else
                    {
                        skip_whitespace( text );
                        if ( *text == closed_bracket<array_bracket>::value )
                        {
                            return text++, temp;
                        }
                        if ( *text != item_separator )
                        {
                            throw json_error( "Unexpected end of array" );
                        }
                        text++;
                        skip_whitespace( text );
                    }
                }
            }
        }

        template <char stop>
        json _parse_naked_object( const char*& text )
        {
            json::object temp;

            for ( ;; )
            {
                json item;
                std::string key = parse_string( text );
                skip_whitespace( text );
                if ( *text != key_separator )
                {
                    throw json_error( "Expected {}", esc( key_separator ) );
                }
                text++;

                item = parse( text );
                temp[std::move( key )] = item;
                if ( item_separator == ' ' )
                {
                    bool ws = skip_whitespace( text );
                    if ( *text == stop )
                    {
                        return text++, temp;
                    }
                    if ( *text == '\0' || !ws )
                    {
                        throw json_error( "Unexpected end of object" );
                    }
                }
                else
                {
                    skip_whitespace( text );
                    if ( *text == stop )
                    {
                        return text++, temp;
                    }
                    if ( *text != item_separator )
                    {
                        throw json_error( "Unexpected end of object" );
                    }
                    text++;
                    skip_whitespace( text );
                }
            }
        }

        json parse_naked_object( const char* text )
        {
            return _parse_naked_object<0>( text );
        }

        json parse_object( const char*& text )
        {
            text++;
            skip_whitespace( text );
            if ( *text == closed_bracket<object_bracket>::value )
                return text++, json::object( );
            else
            {
                return _parse_naked_object<closed_bracket<object_bracket>::value>( text );
            }
        }

        json parse( const char*& text )
        {
            skip_whitespace( text );
            switch ( *text )
            {
            case '-':
                return parse_number( text );
            case 't':
                if ( text[1] == 'r' && text[2] == 'u' && text[3] == 'e' )
                {
                    return text += 4, true;
                }
                if ( optional_quote )
                {
                    return parse_string( text );
                }
                else
                    throw json_error( "Invalid constant, expected true" );

            case 'f':
                if ( text[1] == 'a' && text[2] == 'l' && text[3] == 's' && text[4] == 'e' )
                {
                    return text += 5, false;
                }
                if ( optional_quote )
                {
                    return parse_string( text );
                }
                else
                    throw json_error( "Invalid constant, expected false" );

            case 'n':
                if ( text[1] == 'u' && text[2] == 'l' && text[3] == 'l' )
                {
                    return text += 4, nullptr;
                }
                if ( optional_quote )
                {
                    return parse_string( text );
                }
                else
                    throw json_error( "Invalid constant, expected null" );

            case '"':
                return parse_string( text );

            case array_bracket:
                return parse_array( text );

            case object_bracket:
                return parse_object( text );

            default:
                if ( *text >= '0' && *text <= '9' )
                    return parse_number( text );
                else if ( optional_quote && ( *text >= 'a' && *text <= 'z' || *text >= 'A' && *text <= 'Z' ) )
                {
                    return parse_string( text );
                }
                else
                {
                    throw json_error( "Invalid character: {}", esc( *text ) );
                }
            }
        }

        json parse_json( const char* text )
        {
            const char* text_begin = text;
            const char* text_end   = text_begin + std::strlen( text );
            try
            {
                return parse( text );
            }
            catch ( json_error& e )
            {
                const char* begin = std::max( text_begin, text - 16 );
                const char* end = std::min( text_end, text + 16 );
                std::string str( begin, end );
                std::string pad( text - begin, '~' );
                for ( char& c : str )
                {
                    if ( ( byte_t )c < ' ' )
                    {
                        c = ' ';
                    }
                }
                throw json_error( std::string( e.what( ) ) + '\n' + str + '\n' + pad + '^' + '\n' );
            }
            return nullptr;
        }

    private:
    };

    template <bool _pretty_print, typename _JsonFormat>
    struct json_printer
    {
    public:
        static const bool pretty_print   = _pretty_print;
        static const char key_separator  = _JsonFormat::key_separator;
        static const char item_separator = _JsonFormat::item_separator;
        static const char array_bracket  = _JsonFormat::array_bracket;
        static const char object_bracket = _JsonFormat::object_bracket;
        static const bool optional_quote = _JsonFormat::optional_quote;

        void padding( int depth )
        {
            const char tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"; // 16
            while ( depth > 0 )
            {
                wr << fmt::BasicStringRef<char>( tabs, std::min( 16, depth ) );
                depth -= 16;
            }
        }
        void print_string( const std::string& value )
        {
            if ( optional_quote )
            {
                bool omit_quotes = true;
                if ( value.size( ) && value[0] >= 'a' && value[0] <= 'z' ||
                     value[0] >= 'A' && value[0] <= 'Z' )
                {
                    for ( char c : value )
                    {
                        if ( ( byte_t )c <= ' ' || c == key_separator || c == item_separator ||
                             c == array_bracket || c == object_bracket )
                        {
                            omit_quotes = false;
                            break;
                        }
                    }
                }
                else
                {
                    omit_quotes = false;
                }
                if ( value == "true" || value == "false" || value == "null" )
                    omit_quotes = false;
                if ( omit_quotes )
                {
                    wr << value;
                    return;
                }
            }
            wr << '"';
            for ( size_t i = 0; i < value.size( ); i++ )
            {
                const char ch = value[i];
                if ( ch == '\\' )
                {
                    wr << "\\\\";
                }
                else if ( ch == '"' )
                {
                    wr << "\\\"";
                }
                else if ( ch == '\b' )
                {
                    wr << "\\b";
                }
                else if ( ch == '\f' )
                {
                    wr << "\\f";
                }
                else if ( ch == '\n' )
                {
                    wr << "\\n";
                }
                else if ( ch == '\r' )
                {
                    wr << "\\r";
                }
                else if ( ch == '\t' )
                {
                    wr << "\\t";
                }
                else if ( byte_t( ch ) <= 0x1f )
                {
                    wr.write( "\\u{:04x}", ch );
                }
                else if ( value.size( ) - i >= 3 && byte_t( ch ) == 0xe2 && byte_t( value[i + 1] ) == 0x80 &&
                          byte_t( value[i + 2] ) == 0xa8 )
                {
                    wr << "\\u2028";
                    i += 2;
                }
                else if ( value.size( ) - i >= 3 && byte_t( ch ) == 0xe2 && byte_t( value[i + 1] ) == 0x80 &&
                          byte_t( value[i + 2] ) == 0xa9 )
                {
                    wr << "\\u2029";
                    i += 2;
                }
                else
                {
                    wr << ch;
                }
            }
            wr << '"';
        }

        void print_array( const json::array& values, int depth )
        {
            if ( values.empty( ) )
            {
                wr << array_bracket;
                wr << closed_bracket<array_bracket>::value;
                return;
            }

            wr << array_bracket;
            if ( pretty_print )
            {
                wr << '\n';
            }

            bool first = true;
            for ( const json& value : values )
            {
                if ( !first )
                {
                    wr << item_separator;
                    if ( pretty_print )
                    {
                        wr << '\n';
                    }
                }
                if ( pretty_print )
                {
                    padding( depth + 1 );
                    print( value, depth + 1 );
                }
                else
                {
                    print( value, depth );
                }
                first = false;
            }
            if ( pretty_print )
            {
                wr << '\n';
                padding( depth );
            }
            wr << closed_bracket<array_bracket>::value;
        }

        void print_naked_object( const json::object& values, int depth = 0 )
        {
            bool first = true;
            for ( const json::objectpair& kv : values )
            {
                if ( !first )
                {
                    wr << item_separator;
                    if ( pretty_print )
                    {
                        wr << '\n';
                    }
                }

                if ( pretty_print )
                {
                    padding( depth );
                    print_string( kv.first );
                    wr << key_separator;
                    if ( kv.second.is_compound( ) && kv.second.size( ) )
                    {
                        wr << '\n';
                        padding( depth );
                    }
                    else
                    {
                        if ( key_separator == ':' ) // ": "
                            wr << ' ';
                    }
                    print( kv.second, depth );
                }
                else
                {
                    print_string( kv.first );
                    wr << key_separator;
                    print( kv.second, depth );
                }
                first = false;
            }
        }
        void print_object( const json::object& values, int depth )
        {
            if ( values.empty( ) )
            {
                wr << object_bracket;
                wr << closed_bracket<object_bracket>::value;
                return;
            }

            wr << object_bracket;
            if ( pretty_print )
            {
                wr << '\n';
            }
            print_naked_object( values, depth + 1 );

            if ( pretty_print )
            {
                wr << '\n';
                padding( depth );
            }
            wr << closed_bracket<object_bracket>::value;
        }

        void print( const json& value, int depth = 0 )
        {
            switch ( value.m_type )
            {
            case json::Null:
                wr << "null";
                break;
            case json::Bool:
                wr << ( value.m_i64 == 0 ? "false" : "true" );
                break;
            case json::Int:
                wr << value.m_i64;
                break;
            case json::Double:
                wr << value.m_f64;
                break;
            case json::String:
                print_string( *value.m_str );
                break;
            case json::Array:
                print_array( *value.m_arr, depth );
                break;
            case json::Object:
                print_object( *value.m_obj, depth );
                break;
            default:
                wr << "null";
                break;
            }
        }

        std::string print_json( const json& value )
        {
            print( value );
            return wr.str( );
        }

    public:
        fmt::MemoryWriter wr;
    };

} // namespace dmk
