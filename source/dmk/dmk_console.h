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
#include "dmk_path.h"
#include "dmk_result.h"
#if defined( DMK_OS_WIN )
#include <windows.h>
#elif defined( DMK_OS_MAC )
#include <mach-o/dyld.h>
#endif
#include <iostream>
#include <cstdlib>
#include <map>
#include <functional>
#include <exception>

namespace dmk
{
    enum text_color : uint32_t
    {
        Black         = 0x00,
        DarkBlue      = 0x01,
        DarkGreen     = 0x02,
        DarkCyan      = 0x03,
        DarkRed       = 0x04,
        DarkMagenta   = 0x05,
        DarkYellow    = 0x06,
        LightGrey     = 0x07,
        Gray          = 0x08,
        Blue          = 0x09,
        Green         = 0x0A,
        Cyan          = 0x0B,
        Red           = 0x0C,
        Magenta       = 0x0D,
        Yellow        = 0x0E,
        White         = 0x0F,
        BgBlack       = 0x00,
        BgDarkBlue    = 0x10,
        BgDarkGreen   = 0x20,
        BgDarkCyan    = 0x30,
        BgDarkRed     = 0x40,
        BgDarkMagenta = 0x50,
        BgDarkYellow  = 0x60,
        BgLightGrey   = 0x70,
        BgGray        = 0x80,
        BgBlue        = 0x90,
        BgGreen       = 0xA0,
        BgCyan        = 0xB0,
        BgRed         = 0xC0,
        BgMagenta     = 0xD0,
        BgYellow      = 0xE0,
        BgWhite       = 0xF0,

        Normal = BgBlack | LightGrey,
    };

    enum console_buffer
    {
        ConsoleStdOutput,
        ConsoleStdError
    };

#if defined( DMK_OS_WIN )
    typedef HANDLE console_handle_t;

    inline console_handle_t console_handle( console_buffer console = ConsoleStdOutput )
    {
        static HANDLE con_out = ::GetStdHandle( STD_OUTPUT_HANDLE );
        static HANDLE con_err = ::GetStdHandle( STD_ERROR_HANDLE );
        return console == ConsoleStdOutput ? con_out : con_err;
    }
#endif

    struct console_color
    {
    public:
        console_color( text_color c, console_buffer console = ConsoleStdOutput )
            : m_old( get( console ) ), m_console( console )
        {
            set( c, m_console );
        }
        ~console_color( )
        {
            set( m_old, m_console );
        }

    private:
        text_color get( console_buffer console = ConsoleStdOutput )
        {
#ifdef DMK_OS_WIN
            CONSOLE_SCREEN_BUFFER_INFO info;
            ::GetConsoleScreenBufferInfo( console_handle( console ), &info );
            return static_cast<text_color>( info.wAttributes & 0xFF );
#else
            return Normal;
#endif
        }

        void set( text_color new_color, console_buffer console = ConsoleStdOutput )
        {
#ifdef DMK_OS_WIN
            ::SetConsoleTextAttribute( console_handle( console ), static_cast<WORD>( new_color ) );
#endif
        }
        text_color m_old;
        console_buffer m_console;
    };

    template <text_color color, console_buffer console = ConsoleStdOutput>
    struct colored_text_tpl : public console_color
    {
    public:
        colored_text_tpl( ) : console_color( color, console )
        {
        }

    private:
    };

    typedef colored_text_tpl<DarkBlue> darkblue_text;
    typedef colored_text_tpl<DarkGreen> darkgreen_text;
    typedef colored_text_tpl<DarkCyan> darkcyan_text;
    typedef colored_text_tpl<DarkRed> darkred_text;
    typedef colored_text_tpl<DarkMagenta> darkmagenta_text;
    typedef colored_text_tpl<DarkYellow> darkyellow_text;
    typedef colored_text_tpl<LightGrey> lightgrey_text;
    typedef colored_text_tpl<Gray> gray_text;
    typedef colored_text_tpl<Blue> blue_text;
    typedef colored_text_tpl<Green> green_text;
    typedef colored_text_tpl<Cyan> cyan_text;
    typedef colored_text_tpl<Red> red_text;
    typedef colored_text_tpl<Magenta> magenta_text;
    typedef colored_text_tpl<Yellow> yellow_text;
    typedef colored_text_tpl<White> white_text;

    typedef colored_text_tpl<Gray> debug_text;

    typedef colored_text_tpl<DarkBlue, ConsoleStdError> darkblue_err_text;
    typedef colored_text_tpl<DarkGreen, ConsoleStdError> darkgreen_err_text;
    typedef colored_text_tpl<DarkCyan, ConsoleStdError> darkcyan_err_text;
    typedef colored_text_tpl<DarkRed, ConsoleStdError> darkred_err_text;
    typedef colored_text_tpl<DarkMagenta, ConsoleStdError> darkmagenta_err_text;
    typedef colored_text_tpl<DarkYellow, ConsoleStdError> darkyellow_err_text;
    typedef colored_text_tpl<LightGrey, ConsoleStdError> lightgrey_err_text;
    typedef colored_text_tpl<Gray, ConsoleStdError> gray_err_text;
    typedef colored_text_tpl<Blue, ConsoleStdError> blue_err_text;
    typedef colored_text_tpl<Green, ConsoleStdError> green_err_text;
    typedef colored_text_tpl<Cyan, ConsoleStdError> cyan_err_text;
    typedef colored_text_tpl<Red, ConsoleStdError> red_err_text;
    typedef colored_text_tpl<Magenta, ConsoleStdError> magenta_err_text;
    typedef colored_text_tpl<Yellow, ConsoleStdError> yellow_err_text;
    typedef colored_text_tpl<White, ConsoleStdError> white_err_text;

    typedef colored_text_tpl<Gray, ConsoleStdError> debug_err_text;

    int fail_exit( )
    {
#ifndef NDEBUG
        getchar( );
#endif
        return 1;
    }

    class console_error : public error
    {
    public:
        using error::error;
    };

    void println( const std::string& message )
    {
        fmt::print( "{}", message );
        fmt::print( "\n" );
    }

    void println( const char* message )
    {
        fmt::print( "{}", message );
        fmt::print( "\n" );
    }

    template <typename... Args>
    void println( const std::string& message, const Args&... args )
    {
        fmt::print( message, args... );
        fmt::print( "\n" );
    }
    template <typename... Args>
    void println( const char* message, const Args&... args )
    {
        fmt::print( message, args... );
        fmt::print( "\n" );
    }

    void errorln( const std::string& message )
    {
        red_text c;
        fmt::print( "{}", message );
        fmt::print( "\n" );
    }

    void errorln( const char* message )
    {
        red_text c;
        fmt::print( "{}", message );
        fmt::print( "\n" );
    }

    template <typename... Args>
    void errorln( const std::string& message, const Args&... args )
    {
        red_text c;
        fmt::print( message, args... );
        fmt::print( "\n" );
    }
    template <typename... Args>
    void errorln( const char* message, const Args&... args )
    {
        red_text c;
        fmt::print( message, args... );
        fmt::print( "\n" );
    }

#ifdef DMK_OS_WIN

    struct handle_finalizer
    {
    public:
        handle_finalizer( HANDLE handle ) : m_handle( handle )
        {
        }
        handle_finalizer( handle_finalizer&& h ) : m_handle( h.m_handle )
        {
            h.m_handle = 0;
        }
        handle_finalizer& operator=( handle_finalizer&& h )
        {
            m_handle   = h.m_handle;
            h.m_handle = 0;
        }
        handle_finalizer( const handle_finalizer& ) = delete;
        handle_finalizer& operator=( const handle_finalizer& ) = delete;
        ~handle_finalizer( )
        {
            if ( m_handle != 0 && m_handle != INVALID_HANDLE_VALUE )
                CloseHandle( m_handle );
        }

    private:
        HANDLE m_handle;
    };

    typedef std::vector<handle_finalizer> handle_finalizers;

#endif

    struct process
    {
    public:
        explicit process( const path& program, const path& working_dir = current_path( ) )
            : m_program( program ), m_working_dir( working_dir ), m_exit_code( 0 ), m_log_output( false )
        {
        }
        process& operator( )( const std::string& value )
        {
            if ( !m_args.empty( ) )
                m_args += " ";
            m_args += value;
            return *this;
        }
        process& operator( )( const char* value )
        {
            if ( !m_args.empty( ) )
                m_args += " ";
            m_args += value;
            return *this;
        }
        process& operator( )( const path& value )
        {
            if ( !m_args.empty( ) )
                m_args += " ";
            m_args += qo( value );
            return *this;
        }
        void set_env( const std::map<std::string, std::string>& map )
        {
            m_environ.insert( map.begin( ), map.end( ) );
        }
        void set_env( const std::string& name, const std::string& value )
        {
            m_environ[name] = value;
        }
        template <typename... Args>
        process& operator( )( const std::string& key, const Args&... args )
        {
            return ( *this )( fmt::format( key, args... ) );
        }
        void log_output( bool log )
        {
            m_log_output = log;
        }
        void operator( )( bool may_throw = true )
        {
#if !defined DMK_OS_POSIX
            m_args = replace_all( m_args, '\\', '/' );
#endif
            before( );
#if defined DMK_OS_POSIX
            std::string cmd = m_program.string( ) + " " + m_args;
            m_exit_code     = 0;
// m_exit_code     = spawnve( P_WAIT, cmd, argv, envp );
#elif defined DMK_OS_WIN
            handle_finalizers handles;

            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            zeroize( si );
            zeroize( pi );
            si.cb = sizeof( si );
            if ( m_log_output )
            {
                SECURITY_ATTRIBUTES sa;
                sa.nLength              = sizeof( SECURITY_ATTRIBUTES );
                sa.lpSecurityDescriptor = NULL;
                sa.bInheritHandle       = TRUE;

                path stdout_log, stderr_log;
                log_path( stdout_log, stderr_log );

                si.hStdOutput = CreateFileW( stdout_log.wstring( ).c_str( ),
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                                             &sa,
                                             CREATE_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL,
                                             0 );

                if ( si.hStdOutput == INVALID_HANDLE_VALUE )
                {
                    throw error( system_error, "Can't create log file for {} ({})", m_program, stdout_log );
                }
                handles.push_back( si.hStdOutput );
                si.hStdError = CreateFileW( stderr_log.wstring( ).c_str( ),
                                            GENERIC_WRITE,
                                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                                            &sa,
                                            CREATE_ALWAYS,
                                            FILE_ATTRIBUTE_NORMAL,
                                            0 );
                if ( si.hStdError == INVALID_HANDLE_VALUE )
                {
                    throw error( system_error, "Can't create log file for {} ({})", m_program, stderr_log );
                }
                handles.push_back( si.hStdError );
                si.dwFlags = STARTF_USESTDHANDLES;
            }

            std::vector<wchar_t> args;
            args.push_back( '"' );
            std::wstring progr( m_program.wstring( ) );
            args.insert( args.end( ), progr.begin( ), progr.end( ) );
            args.push_back( '"' );
            args.push_back( ' ' );
            args.insert( args.end( ), m_args.begin( ), m_args.end( ) );
            args.push_back( 0 );
            std::string ext = asci_lowercase( m_program.extension( ).string( ) );
            if ( ext == "bat" || ext == "cmd" )
            {
                std::wstring cmd = L"cmd.exe /S /C ";
                args.insert( args.begin( ), cmd.begin( ), cmd.end( ) );
            }
            args.reserve( args.size( ) + 100 );
            std::vector<wchar_t> env;
            LPWCH current_env = GetEnvironmentStringsW( );

            for ( LPWCH c_env = current_env; *c_env; )
            {
                size_t len = wcslen( c_env );
                env.insert( env.end( ), c_env, c_env + len );
                env.push_back( 0 );
                c_env += len + 1;
            }
            FreeEnvironmentStringsW( current_env );
            for ( const auto& e : m_environ )
            {
                env.insert( env.end( ), e.first.begin( ), e.first.end( ) );
                env.push_back( '=' );
                env.insert( env.end( ), e.second.begin( ), e.second.end( ) );
                env.push_back( 0 );
            }
            env.push_back( 0 );
            if ( !CreateProcessW( NULL,
                                  args.data( ),
                                  NULL,
                                  NULL,
                                  m_log_output ? TRUE : FALSE,
                                  NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
                                  env.data( ),
                                  m_working_dir.wstring( ).c_str( ),
                                  &si,
                                  &pi ) )
            {
                throw error( system_error, "Can't start process {}", m_program.string( ) );
            }
            handles.push_back( pi.hThread );
            handles.push_back( pi.hProcess );
            DWORD wait = WaitForSingleObject( pi.hProcess, INFINITE );
            if ( wait != WAIT_OBJECT_0 )
            {
                throw error( system_error, "Can't wait for process {}", m_program.string( ) );
            }
            DWORD ec = 0;
            if ( !GetExitCodeProcess( pi.hProcess, &ec ) )
            {
                throw error( system_error, "Can't get exit code for process {}", m_program.string( ) );
            }
            m_exit_code = ec;
#else
#error "not implemented"
#endif
            after( );

            if ( m_exit_code != 0 && may_throw )
            {
                throw console_error( "{} returns {}", m_program.filename( ).string( ), m_exit_code );
            }
        }
        int exit_code( ) const
        {
            return m_exit_code;
        }

    protected:
        virtual void before( )
        {
        }
        virtual void after( )
        {
        }
        virtual void log_path( path& stdout_log, path& stderr_log )
        {
            char buff[64];

            std::time_t time = std::time( NULL );
            std::tm* tm = std::localtime( &time );
            std::strftime( buff, countof( buff ), "%Y%m%d-%H%M%S", tm );

            stdout_log = unique_path(
                temp_directory_path( ), ".stdout.log", m_program.filename( ).string( ) + buff + "-%04d" );
            stderr_log = unique_path(
                temp_directory_path( ), ".stderr.log", m_program.filename( ).string( ) + buff + "-%04d" );
        }
        const path m_program;
        const path m_working_dir;
        std::map<std::string, std::string> m_environ;
        std::string m_args;
        int m_exit_code;
        bool m_log_output;
    };

    template <typename _Process = process>
    void exec( const path& curdir, const path& program, const std::string& args )
    {
        _Process p( program, curdir );
        p( args );
        p( );
    }

    template <typename _Process = process, typename... Args>
    void exec( const path& curdir, const path& program, const std::string& args, const Args&... params )
    {
        std::string args_str = fmt::format( args, params... );
        exec<_Process>( curdir, program, args_str );
    }

    class fatal_error : public error
    {
    public:
        using error::error;
    };

    class command_error : public error
    {
    public:
        using error::error;
    };

    class subcommand_error : public error
    {
    public:
        using error::error;
    };

    struct arguments
    {
    public:
        arguments( int argc, char** argv, char** envp )
        {
#if defined DMK_OS_WIN
            wchar_t CurrentExe[MAX_PATH] = { 0 };
            GetModuleFileNameW( NULL, CurrentExe, countof( CurrentExe ) );
            m_executable = CurrentExe;
#elif defined DMK_OS_MAC
            char CurrentExe[PATH_MAX];
            uint32_t len = sizeof( CurrentExe );
            if ( _NSGetExecutablePath( CurrentExe, &len ) != 0 )
            {
                CurrentExe[0] = '\0';
            }
            else
            {
                char* canonicalPath = realpath( CurrentExe, NULL );
                if ( canonicalPath != NULL )
                {
                    strncpy( CurrentExe, canonicalPath, len );
                    free( canonicalPath );
                }
            }
            m_executable = CurrentExe;
#else
            m_executable = argv[0];
#endif
            m_args.reserve( argc );
            for ( size_t i = 1; i < argc; i++ )
            {
                m_args.push_back( argv[i] );
            }
            for ( char** env = envp; *env; env++ )
            {
                m_env.push_back( *env );
            }
        }
        const path& executable( ) const
        {
            return m_executable;
        }
        size_t count( ) const
        {
            return m_args.size( );
        }
        const std::string& operator[]( size_t index ) const
        {
            static const std::string empty;
            return index < m_args.size( ) ? m_args[index] : empty;
        }
        std::string operator( )( const std::string& name ) const
        {
            std::vector<std::string>::const_iterator iter = find( m_args, name );
            if ( iter != m_args.end( ) )
            {
                std::string value = iter->substr( name.size( ) + 1 );
                return value;
            }
            return std::string( );
        }
        std::string extract( const std::string& name,
                             const std::string& env_name      = std::string( ),
                             const std::string& default_value = std::string( ) )
        {
            std::vector<std::string>::const_iterator iter = find( m_args, name );
            if ( iter != m_args.end( ) )
            {
                std::string value = iter->substr( name.size( ) + 1 );
                m_args.erase( iter );
                return value;
            }
            if ( !env_name.empty( ) )
            {
                std::vector<std::string>::const_iterator iter = find( m_env, env_name );
                if ( iter != m_env.end( ) )
                {
                    return iter->substr( env_name.size( ) + 1 );
                }
            }
            return default_value;
        }
        const std::vector<std::string>& args( ) const
        {
            return m_args;
        }
        const std::vector<std::string>& env( ) const
        {
            return m_env;
        }

    private:
        static std::vector<std::string>::const_iterator find( const std::vector<std::string>& values,
                                                              const std::string& name )
        {
            std::string search = name + '=';
            for ( size_t i = 0; i < values.size( ); i++ )
            {
                if ( values[i].size( ) >= search.size( ) &&
                     values[i].substr( 0, name.size( ) + 1 ) == search )
                {
                    return values.begin( ) + i;
                }
            }
            return values.end( );
        }
        path m_executable;
        std::vector<std::string> m_args;
        std::vector<std::string> m_env;
    };

    class command_processor
    {
    public:
        typedef std::function<void( const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string&,
                                    const std::string& )> func_t;
        command_processor( ) : m_terminate( false )
        {
        }
        void execute( const std::vector<std::string>& tokens )
        {
            auto it = m_commands.find( tokens[0] );
            if ( it != m_commands.end( ) )
            {
                const command_t& cmd = it->second;
                std::vector<std::string> args( tokens.begin( ) + 1, tokens.end( ) );

                if ( args.size( ) < cmd.min_args )
                {
                    errorln( "Too few arguments to command" );
                    println( "usage:" );
                    println( "    {: <20}{}", tokens[0], cmd.usage );
                }
                else if ( args.size( ) > cmd.max_args )
                {
                    errorln( "Too many arguments to command" );
                    println( "usage:" );
                    println( "    {: <20}{}", tokens[0], cmd.usage );
                }
                else
                {
                    cmd.func( args.size( ) >= 1 ? args[0] : "",
                              args.size( ) >= 2 ? args[1] : "",
                              args.size( ) >= 3 ? args[2] : "",
                              args.size( ) >= 4 ? args[3] : "",
                              args.size( ) >= 5 ? args[4] : "",
                              args.size( ) >= 6 ? args[5] : "",
                              args.size( ) >= 7 ? args[6] : "",
                              args.size( ) >= 8 ? args[7] : "",
                              args.size( ) >= 9 ? args[8] : "",
                              args.size( ) >= 10 ? args[9] : "" );
                }
            }
            else
            {
                throw command_error( "Unrecognized command: {}. Type help for a list of supported commands",
                                     tokens[0] );
            }
        }
        void alias( const std::string& alias, const std::string& command )
        {
            command_t cmd = m_commands[command];
            cmd.alias_for = command;
            m_commands.insert_or_assign( alias, cmd );
        }
        void bind( const std::string& command,
                   const std::string& usage,
                   func_t&& func,
                   unsigned min_args = 0,
                   unsigned max_args = 0 )
        {
            m_commands.insert_or_assign( command, { std::move( func ), usage, min_args, max_args, "" } );
        }
        virtual std::string get_prompt( ) const
        {
            return current_path( ).string( );
        }
        void cd( const std::string& dir )
        {
            if ( dir.empty( ) )
            {
                println( "{}", current_path( ) );
                return;
            }
            path working_dir;
            if ( path( dir ).is_absolute( ) )
            {
                working_dir = path( dir );
            }
            else
            {
                working_dir = current_path( ) / path( dir );
            }
            println( "{}", working_dir );
            if ( !is_directory( working_dir ) )
            {
                throw command_error( "Directory doesn't exist: {}", working_dir );
            }
            current_path( working_dir );
        }
        void help( )
        {
            green_text c;
            for ( const auto& command : m_commands )
            {
                if ( command.second.alias_for.empty( ) )
                {
                    println( "{: <20}{}", command.first, command.second.usage );
                }
            }
            println( "" );
            for ( const auto& command : m_commands )
            {
                if ( !command.second.alias_for.empty( ) )
                {
                    println( "{: <20}is an alias for {}", command.first, command.second.alias_for );
                }
            }
        }
        void exit( )
        {
            m_terminate = true;
        }
        int execute_command( const std::vector<std::string>& tokens )
        {
            if ( tokens.size( ) > 0 )
            {
                try
                {
                    execute( tokens );
                }
                catch ( const fatal_error& error )
                {
                    errorln( "Fatal exception while executing command {}", tokens[0] );
                    errorln( error.what( ) );
                    return fail_exit( );
                }
                catch ( const std::exception& error )
                {
                    errorln( "Exception while executing command {}", tokens[0] );
                    errorln( error.what( ) );
                }
            }
            return 0;
        }
        int interactive( )
        {
            while ( !m_terminate )
            {
                std::string line;
                {
                    console_color c( text_color::Gray );
                    fmt::print( get_prompt( ) );
                    fmt::print( ">" );
                }
                std::getline( std::cin, line );

                std::vector<std::string> tokens = tokenize_params( line );
                int exit_code = execute_command( tokens );
                if ( exit_code != 0 )
                {
                    return exit_code;
                }
            }
            return 0;
        }

        struct command_t
        {
            func_t func;
            std::string usage;
            unsigned min_args;
            unsigned max_args;
            std::string alias_for;
        };

    private:
        ordered_map<command_t> m_commands;
        bool m_terminate;
    };

    struct console_title
    {
    public:
        static std::string get( )
        {
#if defined DMK_OS_WIN
            char buffer[1024] = { 0 };
            GetConsoleTitleA( buffer, countof( buffer ) );
            return buffer;
#else
            return "";
#endif
        }
        static void set( const std::string& title )
        {
#if defined DMK_OS_WIN
            SetConsoleTitleA( title.c_str( ) );
#else
#endif
        }
        explicit console_title( const std::string& message ) : m_saved( get( ) )
        {
            set( message );
        }
        template <typename... Args>
        explicit console_title( const std::string& message, const Args&... args )
            : m_saved( get( ) )
        {
            std::string value = fmt::format( message, args... );
            set( value );
        }
        template <typename... Args>
        console_title( bool print, const std::string& message, const Args&... args )
            : m_saved( get( ) )
        {
            std::string value = fmt::format( message, args... );
            set( value );
            if ( print )
                println( value );
        }
        ~console_title( )
        {
            set( m_saved );
        }

    private:
        std::string m_saved;
    };

} // namespace dmk
