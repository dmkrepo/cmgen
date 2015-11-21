/**
 * CMGen
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

#include <string.h>
#include <map>
#include <dmk_string.h>
#include <dmk_json.h>
#include <ctime>

namespace dmk
{

    // Replace variables (in angle brackets: <vars>) in strings and json values
    // Usage: variable_list vars; ... string = vars( string ) or json = vars( json )
    struct variable_list : public std::map<std::string, std::string>
    {
    public:
        using std::map<std::string, std::string>::map;
        variable_list( ) = default;
        variable_list( const variable_list& ) = default;
        variable_list( variable_list&& ) = default;
        variable_list& operator=( const variable_list& ) = default;
        variable_list& operator=( variable_list&& ) = default;

        void print( )
        {
            for ( const auto& var : *this )
            {
                fmt::print( "{:30} = {}\n", var.first, var.second );
            }
        }

        std::string operator( )( const char* value ) const
        {
            return operator( )( std::string( value ) );
        }

        json::object operator( )( const json::object& obj ) const
        {
            json::object result;
            for ( const json::objectpair& o : obj )
            {
                merge( result, process_key( o.first ), operator( )( o.second ) );
            }
            return result;
        }
        json::array operator( )( const json::array& arr ) const
        {
            json::array result;
            for ( const json& a : arr )
            {
                result.push_back( operator( )( a ) );
            }
            return result;
        }

        std::string operator( )( const std::string& value ) const
        {
            if ( value.find_first_of( "<" ) == std::string::npos &&
                 value.find_first_of( ">" ) == std::string::npos )
                return value;
            std::string result = value;

            size_t pos = 0;
            while ( ( pos = result.find_first_of( "<>", pos ) ) != std::string::npos )
            {
                if ( pos == result.size( ) - 1 )
                {
                    throw error( "Invalid variable format: \"{}\"", value );
                }
                if ( result[pos] == '>' && result[pos + 1] == '>' ||
                     result[pos] == '<' && result[pos + 1] == '<' )
                {
                    result.erase( pos );
                    pos++;
                    continue;
                }
                if ( result[pos] == '>' )
                {
                    throw error( "Invalid variable format: \"{}\"", value );
                }
                size_t end = result.find( '>', pos + 1 );
                if ( end == std::string::npos )
                {
                    throw error( "Invalid variable format: \"{}\"", value );
                }
                std::string key   = result.substr( pos + 1, end - pos - 1 );
                key               = asci_lowercase( key );
                bool no_exception = erase_leading( key, '.' );

                auto it = find( key );
                if ( it != this->end( ) )
                {
                    result.replace( pos, end - pos + 1, it->second );
                    pos += it->second.size( );
                }
                else if ( no_exception )
                {
                    pos = end + 1;
                }
                else
                {
                    throw error( "Undefined variable: \"{}\"", key );
                }
            }
            return result;
        }

        json operator( )( const json& value ) const
        {
            switch ( value.get_type( ) )
            {
            case json::String:
                return operator( )( value.as_string( ) );
            case json::Array:
                return operator( )( value.as_array( ) );
            case json::Object:
                return operator( )( value.as_object( ) );
            default:
                return value;
            }
        }

        variable_list transform( const std::string& prefix,
                                 const std::string& postfix,
                                 text_case newcase = text_case::nochange ) const
        {
            variable_list list;
            for ( auto m : *this )
            {
                std::string k = m.first;
                list[prefix + transform_case( k, newcase ) + postfix] = m.second;
            }
            return list;
        }

    private:
        //  key|condition
        //  key|!not-condition
        //  key|first-condition|second-condition
        std::string process_key( const std::string& key ) const
        {
            size_t p = key.find_last_of( '|' );
            if ( p != std::string::npos )
            {
                std::string test = key.substr( p + 1 );
                bool invert = false;
                if ( !test.empty( ) && test.front( ) == '!' )
                {
                    test   = test.substr( 1 );
                    invert = true;
                }
                bool result = find( test ) != end( );
                if ( result != invert ) // logical xor
                {
                    return process_key( key.substr( 0, p ) );
                }
                else
                {
                    return std::string( );
                }
            }
            else
            {
                return key;
            }
        }
        void merge( json::object& object, const std::string& key, const json& value ) const
        {
            if ( key.empty( ) ) // remove empty keys
            {
                return;
            }
            json& item = object[key];
            if ( item.is_object( ) && value.is_object( ) )
            {
                item = item.as_object( ) + value.as_object( );
            }
            else if ( item.is_array( ) && value.is_array( ) )
            {
                item = item.as_array( ) + value.as_array( );
            }
            else
            {
                item = value;
            }
        }
    };

    inline variable_list dynamic_variables( )
    {
        variable_list vars;
        vars["random"]   = std::to_string( rand( ) );
        char buff[32]    = { 0 };
        std::time_t time = std::time( NULL );
        std::tm* tm = std::gmtime( &time );
        std::strftime( buff, countof( buff ), "%F", tm );
        vars["date"] = buff;
        std::strftime( buff, countof( buff ), "%T", tm );
        vars["time"] = buff;
        return vars;
    }

    inline variable_list operator+( const variable_list& left, const variable_list& right )
    {
        variable_list result = left;
        result.insert( right.begin( ), right.end( ) );
        return result;
    }
    inline variable_list& operator+=( variable_list& left, const variable_list& right )
    {
        left.insert( right.begin( ), right.end( ) );
        return left;
    }
}
