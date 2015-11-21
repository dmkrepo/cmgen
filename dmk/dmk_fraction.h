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
#include <iostream>

namespace dmk
{

    class fraction
    {
    public:
        // Calculates the greatest common divisor
        static int64_t gcd( int64_t a, int64_t b )
        {
            int64_t r;
            while ( b != 0 )
            {
                r = a % b;
                a = b;
                b = r;
            }
            return a;
        }
        static int64_t lcm( int64_t a, int64_t b )
        {
            return a / gcd( a, b ) * b;
        }

    public:
        fraction( ) : m_numerator( 0 ), m_denominator( 1 )
        {
        }

        fraction( int64_t n ) : m_numerator( n ), m_denominator( 1 )
        {
        }

        fraction( int64_t n, int64_t d, bool do_normalize = true )
        {
            if ( n == 0 )
            {
                m_numerator   = 0;
                m_denominator = 1;
            }
            else
            {
                m_numerator   = n;
                m_denominator = d;

                if ( do_normalize )
                {
                    normalize( );
                }
            }
        }

        int64_t numerator( ) const
        {
            return m_numerator;
        }

        int64_t denominator( ) const
        {
            return m_denominator;
        }

        void normalize( )
        {
            int sign = 1;
            if ( m_numerator < 0 )
            {
                sign        = -sign;
                m_numerator = -m_numerator;
            }
            if ( m_denominator < 0 )
            {
                sign          = -sign;
                m_denominator = -m_denominator;
            }

            int64_t tmp   = gcd( m_numerator, m_denominator );
            m_numerator   = m_numerator / tmp;
            m_denominator = m_denominator / tmp;
            if ( sign < 0 )
                m_numerator = -m_numerator;
        }

        fraction normalized( ) const
        {
            fraction f( *this );
            f.normalize( );
            return f;
        }

        int64_t as_int64_t( ) const
        {
            return m_numerator / m_denominator;
        }

        double as_double( ) const
        {
            return ( double )m_numerator / m_denominator;
        }

        explicit operator int64_t( ) const
        {
            return as_int64_t( );
        }

        explicit operator double( ) const
        {
            return as_double( );
        }

    private:
        int64_t m_numerator, m_denominator;
    };

    inline fraction operator-( const fraction& lhs )
    {
        return fraction( -lhs.numerator( ), lhs.denominator( ) );
    }

    inline fraction operator+( const fraction& lhs, const fraction& rhs )
    {
        int64_t e = fraction::lcm( lhs.denominator( ), rhs.denominator( ) );
        return fraction( lhs.numerator( ) * ( e / lhs.denominator( ) ) +
                             rhs.numerator( ) * ( e / rhs.denominator( ) ),
                         e );
    }

    inline fraction& operator+=( fraction& lhs, const fraction& rhs )
    {
        lhs = lhs + rhs;
        return lhs;
    }

    inline fraction operator-( const fraction& lhs, const fraction& rhs )
    {
        int64_t e = fraction::lcm( lhs.denominator( ), rhs.denominator( ) );
        return fraction(
            lhs.numerator( ) * e / lhs.denominator( ) - rhs.numerator( ) * e / rhs.denominator( ), e );
    }

    inline fraction& operator-=( fraction& lhs, const fraction& rhs )
    {
        lhs = lhs - rhs;
        return lhs;
    }

    inline fraction operator*( const fraction& lhs, const fraction& rhs )
    {
        fraction tmp( lhs.numerator( ) * rhs.numerator( ), lhs.denominator( ) * rhs.denominator( ) );
        return tmp;
    }

    inline fraction& operator*=( fraction& lhs, const fraction& rhs )
    {
        fraction tmp( lhs.numerator( ) * rhs.numerator( ), lhs.denominator( ) * rhs.denominator( ) );
        lhs = tmp;
        return lhs;
    }

    inline fraction operator*( int64_t lhs, const fraction& rhs )
    {
        fraction tmp( lhs * rhs.numerator( ), rhs.denominator( ) );
        return tmp;
    }

    inline fraction operator*( const fraction& rhs, int64_t lhs )
    {
        fraction tmp( lhs * rhs.numerator( ), rhs.denominator( ) );
        return tmp;
    }

    inline fraction operator/( const fraction& lhs, const fraction& rhs )
    {
        fraction tmp( lhs.numerator( ) * rhs.denominator( ), lhs.denominator( ) * rhs.numerator( ) );
        return tmp;
    }
    inline fraction& operator/=( fraction& lhs, const fraction& rhs )
    {
        fraction tmp( lhs.numerator( ) * rhs.denominator( ), lhs.denominator( ) * rhs.numerator( ) );
        lhs = tmp;
        return lhs;
    }

    inline bool operator==( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) == 0;
    }

    inline bool operator!=( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) != 0;
    }

    inline bool operator<( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) < 0;
    }

    inline bool operator>( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) > 0;
    }

    inline bool operator<=( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) <= 0;
    }

    inline bool operator>=( const fraction& lhs, const fraction& rhs )
    {
        fraction difference = lhs - rhs;
        return difference.numerator( ) >= 0;
    }

    inline std::ostream& operator<<( std::ostream& os, const fraction& a )
    {
        os << a.numerator( );
        if ( a.denominator( ) > 1 )
        {
            os << "/" << a.denominator( );
        }
        return os;
    }
} // namespace dmk
