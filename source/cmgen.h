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

#include <vector>
#include <memory>
#include <dmk.h>
#include <dmk_string.h>
#include <dmk_json.h>
#include <dmk_path.h>
#include <dmk_console.h>
#include <thread>

#include "expressions.h"

namespace dmk
{

    struct build_process : public process
    {
    public:
        explicit build_process( const path& program, const path& working_dir = current_path( ) )
            : process( program, working_dir )
        {
            log_output( quiet );
        }
        static bool quiet;

    protected:
        virtual void before( ) override
        {
            create_directories( m_working_dir );
            if ( !quiet )
            {
                {
                    debug_text c;
                    println( m_working_dir.string( ) );
                }
                {
                    cyan_text c;
                    println( m_program.filename( ).string( ) + ' ' + m_args );
                }
            }
        }
        virtual void log_path( path& stdout_log, path& stderr_log ) override
        {
            char buff[64];
            std::string title = console_title::get( );
            title             = replace_all( title, "...", "" );
            title             = replace_all_not_of( title, " \\|/:;<>~?*\"", '_' );

            std::time_t time = std::time( NULL );
            std::tm* tm = std::localtime( &time );
            std::strftime( buff, countof( buff ), "%Y%m%d-%H%M%S", tm );

            stdout_log = unique_path( temp_directory_path( ),
                                      ".stdout.log",
                                      title + "_" + m_program.filename( ).string( ) + buff + "-%04d" );
            stderr_log = unique_path( temp_directory_path( ),
                                      ".stderr.log",
                                      title + "_" + m_program.filename( ).string( ) + buff + "-%04d" );
        }
    };

    template <typename _Type>
    using ptr = std::shared_ptr<_Type>;

    enum redo_mode
    {
        /* ? */ DoOnce,
        /*   */ DoAlways,
        /* ! */ DoForce,
    };

    inline void safe_copy_all( const path& source, const path& target, int depth = 0 )
    {
        if ( !build_process::quiet )
            fmt::print( "{: <79}\r", source.string( ).substr( 0, 79 ) );
        try
        {
            if ( is_directory( source ) )
            {
                create_directories( target );
                for ( auto f : directory_iterator( source ) )
                {
                    safe_copy_all( f.path( ), target / f.path( ).filename( ), depth + 1 );
                }
            }
            else
            {
                fix_write_rights( target );
                copy_file( source, target, DMK_COPY_OVERWRITE );
            }
        }
        catch ( const error& e )
        {
            throw;
        }
        catch ( const std::exception& e )
        {
            throw error( e, "Can't copy file or directory \"{}\" -> \"{}\"", source, target );
        }
        if ( depth == 0 )
        {
            if ( !build_process::quiet )
                fmt::print( "{: <79}\r", "" );
        }
    }

    inline void safe_remove_all( const path& p, int depth = 0 )
    {
        if ( !build_process::quiet )
            fmt::print( "{: <79}\r", p.string( ).substr( 0, 79 ) );
        try
        {
            if ( is_directory( p ) )
            {
                for ( auto f : directory_iterator( p ) )
                {
                    safe_remove_all( f.path( ), depth + 1 );
                }
            }
            else
            {
                fix_write_rights( p );
            }
            remove( p );
        }
        catch ( const error& e )
        {
            throw;
        }
        catch ( const std::exception& e )
        {
            throw error( e, "Can't remove file or directory \"{}\"", p.string( ) );
        }
        if ( depth == 0 )
        {
            if ( !build_process::quiet )
                fmt::print( "{: <79}\r", "" );
        }
    }

    inline void move_content( const path& directory, const path& target, bool print = !build_process::quiet )
    {
        yellow_text c;
        create_directories( target );
        path p;
        try
        {
            for ( const directory_entry& entry : directory_iterator( directory ) )
            {
                p = entry;
                if ( print )
                {
                    println( "Moving directory contents {} -> {}", directory.string( ), target.string( ) );
                    print = false;
                }
                safe_remove_all( target / entry.path( ).filename( ) );
                rename( entry, target / entry.path( ).filename( ) );
            }
        }
        catch ( const std::exception& e )
        {
            throw error( e, "Can't rename file or directory \"{}\"", p.string( ) );
        }
    }

    inline void move_content_strip1( const path& directory,
                                     const path& target,
                                     bool print = !build_process::quiet )
    {
        yellow_text c;
        create_directories( target );
        path p;
        try
        {
            for ( const directory_entry& entry : directory_iterator( directory ) )
            {
                for ( const directory_entry& entry2 : directory_iterator( entry ) )
                {
                    p = entry2;
                    if ( print )
                    {
                        println(
                            "Moving directory contents {} -> {}", directory.string( ), target.string( ) );
                        print = false;
                    }
                    safe_remove_all( target / entry2.path( ).filename( ) );
                    rename( entry2, target / entry2.path( ).filename( ) );
                }
            }
        }
        catch ( const std::exception& e )
        {
            throw error( e, "Can't rename file or directory \"{}\"", p.string( ) );
        }
    }

    inline void remove_content( const path& directory, bool print = !build_process::quiet )
    {
        yellow_text c;
        if ( !is_directory( directory ) )
        {
            return;
        }
        for ( const directory_entry& entry : directory_iterator( directory ) )
        {
            if ( print )
            {
                println( "Removing directory contents {}", directory.string( ) );
                print = false;
            }
            safe_remove_all( entry );
        }
    }

    inline void remove_directory( const path& directory, bool print = !build_process::quiet )
    {
        remove_content( directory, print );
        if ( is_directory( directory ) && print )
            println( "Removing directory {}", directory.string( ) );
        remove( directory );
    }

    inline void copy_content( const path& directory, const path& target, bool print = !build_process::quiet )
    {
        yellow_text c;
        create_directories( target );
        for ( const directory_entry& entry : directory_iterator( directory ) )
        {
            if ( print )
            {
                println( "Copying directory contents {} -> {}", directory.string( ), target.string( ) );
                print = false;
            }
            safe_copy_all( entry, target / entry.path( ).filename( ) );
        }
    }

    inline void copy_content_hard_link( const path& directory,
                                        const path& target,
                                        bool print = !build_process::quiet )
    {
        yellow_text c;
        create_directories( target );
        for ( const directory_entry& entry : directory_iterator( directory ) )
        {
            if ( print )
            {
                println( "Copying directory contents as symlinks {} -> {}",
                         directory.string( ),
                         target.string( ) );
                print = false;
            }
            safe_remove_all( target / entry.path( ).filename( ) );
            create_hard_link( entry, target / entry.path( ).filename( ) );
        }
    }

    inline std::string join_list( const json& value,
                                  const std::string& delimeter,
                                  const std::string& prefix  = "",
                                  const std::string& infix   = "=",
                                  const std::string& postfix = "" )
    {
        if ( value.is_string( ) )
        {
            return value.as_string( );
        }
        else if ( value.is_array( ) )
        {
            std::string str;
            for ( const auto& a : value.as_array( ) )
            {
                if ( !str.empty( ) )
                    str += delimeter;
                str += prefix + a.to_string( ) + postfix;
            }
            return str;
        }
        else if ( value.is_object( ) )
        {
            std::string str;
            for ( const auto& o : value.as_object( ) )
            {
                if ( !str.empty( ) )
                    str += delimeter;
                str += prefix + o.first + infix + o.second.to_string( ) + postfix;
            }
            return str;
        }
        else
        {
            return std::string( );
        }
    }

    struct platform_param
    {
    public:
        platform_param( )
        {
        }
        explicit platform_param( const json::objectpair& value )
            : name( value.first ), lower_name( asci_lowercase( name ) ), data( value.second )
        {
        }
        bool empty( ) const
        {
            return name.empty( );
        }
        bool matches( const std::string& pattern ) const
        {
            return pattern.empty( ) || dmk::matches( pattern, name );
        }
        std::string name;
        std::string lower_name;
        json data;
    };

    struct architecture : public platform_param
    {
    public:
        explicit architecture( const json::objectpair& value )
            : platform_param( value ),
              suffix( data["suffix"].as_string( name ) ),
              bitness( data["bitness"].as_string( ) ),
              generator( data["generator"].as_string( ) )
        {
        }
        architecture( ) = default;
        architecture( const architecture& ) = default;
        architecture( architecture&& ) = default;
        architecture& operator=( const architecture& ) = default;
        architecture& operator=( architecture&& ) = default;
        std::string suffix;
        std::string bitness;
        std::string generator;
    };

    struct configuration : public platform_param
    {
    public:
        static const configuration& all( )
        {
            static const configuration& c( "All" );
            return c;
        }
        using platform_param::platform_param;
        configuration( ) = default;
        configuration( const configuration& ) = default;
        configuration( configuration&& ) = default;
        configuration& operator=( const configuration& ) = default;
        configuration& operator=( configuration&& ) = default;

    private:
        configuration( const char* name )
        {
            this->name = name;
        }
    };

    typedef std::vector<architecture> architectures;
    typedef std::vector<configuration> configurations;

#if defined DMK_OS_WIN
#define DMK_EXEC_EXT ".exe"
#define DMK_COMM_EXT ".cmd"
#else
#define DMK_EXEC_EXT ""
#define DMK_COMM_EXT ".sh"
#endif

    namespace dir
    {
        constexpr const char* source   = "source";
        constexpr const char* projects = "projects";
        constexpr const char* licenses = "licenses";
        constexpr const char* modules  = "modules";
        constexpr const char* flags    = "flags";
    }

    constexpr const char* var_true = "1";

    typedef json_format<> options_format;

    class environment
    {
    public:
        std::string platform; // msvc
        std::string platform_name; // Visual Studio 2015
        std::string platform_version; // msvc2015
        std::string platform_alt_version; // msvc14
        path dev_dir;
        path tools_dir;
        path root_dir;
        path source_root_dir;
        path modules_dir;
        path platform_dir;
        path licenses_dir;
        path flags_dir;
        path temp_dir;
        path git_path;
        path hg_path;
        path tar_path;
        path curl_path;
        path wget_path;
        path cmake_path;
        path make_path;
        path python_path;
        path scons_path;
        path unzip_path;
        path msys_dir;
        path msys_bin_dir;
        path ext_dir;
        path nasm_path;
        path patch_path;
        path perl_path;
        path jom_path;
        path ruby_path;
        path configure_qmake_path;
        path build_qmake_path;
        path sevenzip_path;
        json options;
        configurations configs;
        configurations configs_all;
        architectures archs;
        variable_list variables;
        variable_list env;

    public:
        environment( arguments& args )
        {
            dev_dir   = args.executable( ).parent_path( ).parent_path( );
            tools_dir = dev_dir / "tools";
#if defined( DMK_OS_WIN )
            variables["win"] = var_true;
#endif
#if defined( DMK_OS_OSX )
            variables["osx"] = var_true;
            variables["mac"] = var_true;
#endif
            variables["cpus"] = std::to_string( std::max( 1u, std::thread::hardware_concurrency( ) ) );
            variables["build_cpus"] =
                std::to_string( std::max( 1u, std::thread::hardware_concurrency( ) - 1 ) );

            for ( const std::string str : args.env( ) )
            {
                size_t p = str.find( "=" );
                if ( p != std::string::npos )
                {
                    env['%' + str.substr( 0, p ) + '%'] = str.substr( p + 1 );
                }
            }

            path root = args.extract( "--root", "CMGEN_ROOT", current_path( ).string( ) );
            while ( !is_root_dir( root ) && root.parent_path( ) != root )
            {
                root = root.parent_path( );
            }
            if ( !is_root_dir( root ) )
            {
                println( "Can't find root directory" );
                println( "You can specify the root directory by the following ways:" );
                println( "    --root=C:\\root\\dir on the command line" );
                println( "    Set CMGEN_ROOT environment variable" );
                println( "        (globally or just before starting cmgen)" );
                println( "    Start cmgen inside root directory" );
                throw fatal_error( "Invalid root directory" );
            }

#if defined DMK_OS_WIN
            static const std::string default_platform = "msvc2015";
#elif defined DMK_OS_MAC
            static const std::string default_platform = "xcode";
#endif

            std::string platform = args.extract( "--platform", "CMGEN_PLATFORM", default_platform );

            initialize_dirs( root );
            initialize_platform( platform );
        }
        void initialize_dirs( const path& root )
        {
            root_dir        = root;
            source_root_dir = root_dir / dir::source;
            modules_dir     = root_dir / dir::modules;
            licenses_dir    = root_dir / dir::licenses;
            flags_dir       = root_dir / dir::flags;

            create_directories( source_root_dir );
            create_directories( modules_dir );
            create_directories( licenses_dir );
            create_directories( flags_dir );
            variables["root_dir"]        = root_dir.string( );
            variables["dev_dir"]         = dev_dir.string( );
            variables["tools_dir"]       = tools_dir.string( );
            variables["modules_dir"]     = modules_dir.string( );
            variables["licenses_dir"]    = licenses_dir.string( );
            variables["source_root_dir"] = source_root_dir.string( );
            variables["flags_dir"]       = flags_dir.string( );

            println( "Dev dir: {}", dev_dir );
            println( "Root dir: {}", root_dir );
            println( "Tools dir: {}", tools_dir );
            println( "Source dir: {}", source_root_dir );
            println( "Modules dir: {}", modules_dir );
            println( "Licenses dir: {}", licenses_dir );
            load_paths( );
        }
        path find_msvc_dir( const char* env ) const
        {
            return path( exclude_trailing( std::getenv( env ), '\\' ) ).parent_path( ).parent_path( ) / "VC";
        }
        void initialize_platform( const std::string& id )
        {
            platform_version = id;
            if ( id.substr( 0, 4 ) == "msvc" )
            {
                platform = "msvc";
                std::string ver;
                std::string year;
                path bin_dir;
                if ( id == "msvc2015" )
                {
                    ver     = "14";
                    year    = "2015";
                    bin_dir = find_msvc_dir( "VS140COMNTOOLS" );
                }
                else if ( id == "msvc2013" )
                {
                    ver     = "12";
                    year    = "2013";
                    bin_dir = find_msvc_dir( "VS120COMNTOOLS" );
                }
                else if ( id == "msvc2012" )
                {
                    ver     = "11";
                    year    = "2012";
                    bin_dir = find_msvc_dir( "VS110COMNTOOLS" );
                }
                else if ( id == "msvc2010" )
                {
                    ver     = "10";
                    year    = "2010";
                    bin_dir = find_msvc_dir( "VS100COMNTOOLS" );
                }
                else
                {
                    throw fatal_error( "Unknown platform id: {}", id );
                }
                platform_name         = "Visual Studio " + year;
                platform_version      = platform + ver;
                platform_alt_version  = platform + year;
                variables["msvcver"]  = ver;
                variables["msvcyear"] = year;
                variables["msvc_dir"] = bin_dir.string( );
            }
            else if ( id == "xcode" )
            {
                platform = "xcode";
                std::string ver;
                std::string year;
                year                   = "2015";
                ver                    = "7";
                platform_name          = "XCode " + ver;
                platform_version       = platform + ver;
                platform_alt_version   = platform + year;
                variables["xcodever"]  = ver;
                variables["xcodeyear"] = year;
            }
            else
            {
                throw fatal_error( "Unknown platform id: {}", id );
            }
            platform_dir                      = root_dir / platform;
            variables[platform]               = var_true;
            variables[platform_version]       = var_true;
            variables[platform_alt_version]   = var_true;
            variables["platform"]             = platform;
            variables["platform_version"]     = platform_version;
            variables["platform_alt_version"] = platform_alt_version;
            variables["platform_name"]        = platform_name;
            variables["platform_dir"]         = qo( platform_dir );

            println( "Platform: {} ({})", platform_name, platform_version );

            return load_config( );
        }
        static bool is_root_dir( const path& root )
        {
            return is_file( root / "cmgen.txt" );
        }
        architectures find_archs( const std::string& pattern, bool at_least_one = true ) const
        {
            architectures result;
            std::vector<std::string> keys;
            for ( const architecture& a : archs )
            {
                if ( pattern.empty( ) || matches( pattern, a.name ) )
                {
                    result.push_back( a );
                }
                keys.push_back( a.name );
            }
            if ( at_least_one && result.empty( ) )
                throw error( "No architectures matched the pattern {} (available architectures: {})",
                             pattern,
                             join( keys, ", " ) );
            return result;
        }
        configurations find_configs( const std::string& pattern, bool at_least_one = true ) const
        {
            configurations result;
            std::vector<std::string> keys;
            for ( const configuration& c : configs )
            {
                if ( pattern.empty( ) || matches( pattern, c.name ) )
                {
                    result.push_back( c );
                }
                keys.push_back( c.name );
            }
            if ( at_least_one && result.empty( ) )
                throw error( "No configurations matched the pattern {} (available configurations: {})",
                             pattern,
                             join( keys, ", " ) );
            return result;
        }
        void print( ) const
        {
            println( "{}", options.stringify<true, options_format>( ) );
        }
        path add_dir( const path& dir, const std::string& name )
        {
            if ( !is_directory( dir ) )
            {
                throw fatal_error( "Can't find {}. Run {}", qo( dir ), dev_dir / "install" DMK_COMM_EXT );
            }
            variables[name + "_dir"] = dir.string( );
            return dir;
        }
        path add_tool( const path& tool, const std::string& name )
        {
            path tool_file = tool;
            if ( !tool_file.is_absolute( ) )
                tool_file = find_in_path( tool_file );
            if ( !is_file( tool_file ) )
            {
                throw fatal_error(
                    "Can't find {}. Run {}", qo( tool_file ), dev_dir / "install" DMK_COMM_EXT );
            }
            variables[name + "_path"] = tool_file.string( );
            variables[name + "_dir"]  = tool_file.parent_path( ).string( );
            return tool_file;
        }
        void load_paths( )
        {
            ext_dir = add_dir( dev_dir / "ext", "ext" );

#if defined DMK_OS_WIN
            temp_dir      = add_dir( env["%TMP%"], "temp" );
            msys_dir      = add_dir( tools_dir / "msys", "msys" );
            msys_bin_dir  = add_dir( msys_dir / "usr" / "bin", "msys_bin" );
            git_path      = add_tool( tools_dir / "PortableGit" / "bin" / "git" DMK_EXEC_EXT, "git" );
            hg_path       = add_tool( tools_dir / "hg" / "hg" DMK_EXEC_EXT, "hg" );
            cmake_path    = add_tool( tools_dir / "cmake" / "bin" / "cmake" DMK_EXEC_EXT, "cmake" );
            ruby_path     = add_tool( tools_dir / "ruby" / "bin" / "ruby" DMK_EXEC_EXT, "ruby" );
            python_path   = add_tool( tools_dir / "python" / "python" DMK_EXEC_EXT, "python" );
            scons_path    = add_tool( tools_dir / "python" / "scons.bat", "scons" );
            perl_path     = add_tool( tools_dir / "perl" / "perl" / "bin" / "perl" DMK_EXEC_EXT, "perl" );
            sevenzip_path = add_tool( tools_dir / "7zip" / "7za" DMK_EXEC_EXT, "sevenzip" );
            jom_path      = add_tool( tools_dir / "jom" / "jom" DMK_EXEC_EXT, "jom" );
            tar_path      = add_tool( msys_bin_dir / "bsdtar" DMK_EXEC_EXT, "tar" );
            unzip_path    = add_tool( msys_bin_dir / "unzip" DMK_EXEC_EXT, "unzip" );
            curl_path     = add_tool( msys_bin_dir / "curl" DMK_EXEC_EXT, "curl" );
            wget_path     = add_tool( msys_bin_dir / "wget" DMK_EXEC_EXT, "wget" );
            patch_path    = add_tool( msys_bin_dir / "patch" DMK_EXEC_EXT, "patch" );
            make_path     = add_tool( msys_bin_dir / "make" DMK_EXEC_EXT, "make" );
            nasm_path     = add_tool( msys_bin_dir / "nasm" DMK_EXEC_EXT, "nasm" );
#else
            temp_dir                                  = add_dir( "/tmp", "temp" );
            git_path                                  = add_tool( "git", "git" );
            hg_path                                   = add_tool( "hg", "hg" );
#if defined DMK_OS_MAC
            cmake_path                                = add_tool( tools_dir / "cmake/CMake.app/Contents/bin/cmake", "cmake" );
#else
            cmake_path = add_tool( "cmake", "cmake" );
#endif
            ruby_path                                 = add_tool( "ruby", "ruby" );
            python_path                               = add_tool( "python", "python" );
            scons_path                                = add_tool( "scons", "scons" );
            perl_path                                 = add_tool( "perl", "perl" );
            sevenzip_path                             = add_tool( "7za", "sevenzip" );
            tar_path                                  = add_tool( "bsdtar", "tar" );
            unzip_path                                = add_tool( "unzip", "unzip" );
            curl_path                                 = add_tool( "curl", "curl" );
            wget_path                                 = add_tool( "wget", "wget" );
            patch_path                                = add_tool( "patch", "patch" );
            make_path                                 = add_tool( "make", "make" );
            nasm_path                                 = add_tool( "nasm", "nasm" );
            jom_path                                  = add_tool( "make", "jom" );
#endif
            configure_qmake_path = add_tool( ext_dir / "configure_qmake" DMK_COMM_EXT, "configure_qmake" );
            build_qmake_path     = add_tool( ext_dir / "build_qmake" DMK_COMM_EXT, "build_qmake" );
        }

        void load_config( )
        {
            json config = file_get_json<options_format>( root_dir / "cmgen.txt" );

            config = ( variables + env )( config );

            json::object arch_obj = config["archs"].as_object( );
            for ( const json::objectpair& o : arch_obj )
            {
                archs.push_back( architecture( o ) );
            }
            json::object config_obj = config["configs"].as_object( );
            for ( const json::objectpair& o : config_obj )
            {
                configs.push_back( configuration( o ) );
            }
            configs_all = configs;
            configs_all.push_back( configuration::all( ) );
        }
    };

    extern ptr<const environment> env;
}
