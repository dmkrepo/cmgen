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
#include "dmk_fraction.h"
#if defined( DMK_OS_WIN )
#include <windows.h>
#elif defined( DMK_OS_MAC )
#include <mach/mach_time.h>
#endif
#include <iostream>
#include <string>

namespace dmk
{

#if defined( DMK_OS_WIN )

    inline uint64_t _os_timer_counter( )
    {
        LARGE_INTEGER freq;
        QueryPerformanceCounter( &freq );
        return freq.QuadPart;
    }

    inline fraction _os_timer_scale( )
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency( &freq );
        return fraction( 1, freq.QuadPart );
    }
#elif defined( DMK_OS_MAC )
    inline uint64_t _os_timer_counter( )
    {
        return mach_absolute_time( );
    }

    inline fraction _os_timer_scale( )
    {
        mach_timebase_info_data_t sTimebaseInfo;
        ( void )mach_timebase_info( &sTimebaseInfo );
        return fraction( sTimebaseInfo.numer, sTimebaseInfo.denom );
    }

#endif

    inline fraction cpu_time( )
    {
        static fraction scale = _os_timer_scale( );
        return scale * _os_timer_counter( );
    }

    struct elapsed_timer
    {
    public:
        elapsed_timer( ) : m_time( cpu_time( ) )
        {
        }
        fraction elapsed( ) const
        {
            return cpu_time( ) - m_time;
        }
        void restart( )
        {
            m_time = cpu_time( );
        }

    private:
        fraction m_time;
    };

    struct bench_timer;

    inline void bench_result( const fraction& time, const std::string& task, uint64_t iterations = 0 )
    {
        std::cout << "task: " << task << " elapsed time: " << time.as_double( ) << std::endl;
    }

    struct bench_task
    {
    public:
        bench_task( const std::string& name ) : m_name( name ), m_count( 0 )
        {
        }
        bench_task( std::string&& name ) DMK_NOEXCEPT : m_name( std::move( name ) )
        {
        }
        ~bench_task( )
        {
            bench_result( m_time, m_name, m_count );
        }

    private:
        friend struct bench_timer;
        std::string m_name;
        fraction m_time;
        uint64_t m_count;
    };

    struct bench_timer
    {
    public:
        bench_timer( bench_task& task ) : m_task( task ), m_start_time( cpu_time( ) )
        {
        }
        ~bench_timer( )
        {
            fraction elapsed = cpu_time( ) - m_start_time;
            m_task.m_time += elapsed;
            m_task.m_count++;
        }

    private:
        bench_task& m_task;
        fraction m_start_time;
    };

    struct bench_simple_timer
    {
    public:
        bench_simple_timer( const std::string& name ) : m_name( name ), m_start_time( cpu_time( ) )
        {
        }
        ~bench_simple_timer( )
        {
            fraction elapsed = cpu_time( ) - m_start_time;
            bench_result( elapsed, m_name );
        }

    private:
        std::string m_name;
        fraction m_start_time;
    };
} // namespace dmk
