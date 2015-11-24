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

#include "cmgen.h"

namespace dmk
{

#define DMK_BUILDER_DEBUG
    //#define DMK_BUILDER_NOP

    // Configure & Build
    class builder
    {
    public:
        // Create builder and set architectures/configurations from the masks
        static ptr<builder> create( const project* proj,
                                    const std::string& arch_mask,
                                    const std::string& config_mask );

        // Create builder and set architectures/configurations from the lists
        static ptr<builder> create( const project* proj,
                                    const architectures& archs,
                                    const configurations& configs );

        // Configure (all choosen architechtures)
        void configure( )
        {
            for ( const architecture& a : m_archs )
            {
                configure_arch( a, false );
            }
        }
        // Configure (all choosen architechtures)
        // Skip if it's already configured
        void configure_once( )
        {
            for ( const architecture& a : m_archs )
            {
                configure_arch( a, true );
            }
        }
        // Clean (all choosen architechtures)
        void configure_clean( )
        {
            for ( const architecture& a : m_archs )
            {
                configure_clean_arch( a );
            }
        }
        // Build (all choosen architechtures)
        void build( )
        {
            for ( const architecture& a : m_archs )
            {
                build_arch( a, false );
            }
        }
        // Build (all choosen architechtures)
        // Skip if it's already built
        void build_once( )
        {
            for ( const architecture& a : m_archs )
            {
                build_arch( a, true );
            }
        }
        // Clean (all choosen architechtures)
        void build_clean( )
        {
            for ( const architecture& a : m_archs )
            {
                build_clean_arch( a );
            }
        }

        // Clean both (all choosen architechtures)
        void clean_all( )
        {
            build_clean( );
            configure_clean( );
        }
        // Clean & Configure (all choosen architechtures)
        void reconfigure( )
        {
            configure_clean( );
            configure( );
        }
        // Clean & Build (all choosen architechtures)
        void rebuild( )
        {
            build_clean( );
            build( );
        }

    protected:
        enum kind
        { //                   configure   build
            Unspecified,
            SingleConfig, //   A, C        A, C
            MultiConfig, //    A           A, C
            MultiBuild //      A           A
        };
        static kind str_to_kind( const std::string& str, kind default_kind )
        {
            if ( str == "singleconfig" )
                return SingleConfig;
            if ( str == "multiconfig" )
                return MultiConfig;
            if ( str == "multibuild" )
                return MultiBuild;
            return default_kind;
        }
        builder( const project* proj, const architectures& archs, const configurations& configs )
            : m_project( proj ),
              m_archs( archs ),
              m_configs( configs ),
              m_original_source_dir( proj->source_dir( ) ),
              m_kind( Unspecified )
        {
            m_data         = m_project->data( );
            m_project_name = m_project->name( );
            m_insource     = m_data["insource"] || 0;
            m_source_dir   = m_original_source_dir;
            m_multi_config = configurations( { configuration::all( ) } );
        }
        // Get configs for configure stage
        const configurations& c_configs( ) const
        {
            switch ( get_kind( ) )
            {
            case SingleConfig:
                return m_configs;
            case MultiConfig:
            case MultiBuild:
                return m_multi_config;
            }
        }

        // Get configs for build stage
        const configurations& b_configs( ) const
        {
            switch ( get_kind( ) )
            {
            case SingleConfig:
            case MultiConfig:
                return m_configs;
            case MultiBuild:
                return m_multi_config;
            }
        }

        kind get_kind( ) const
        {
            if ( m_kind == Unspecified )
            {
                return m_kind = str_to_kind( m_data["kind"] || "", get_default_kind( ) );
            }
            else
            {
                return m_kind;
            }
        }

        virtual kind get_default_kind( ) const
        {
            return SingleConfig;
        }
        // Configure project for given architecture
        void configure_arch( const architecture& a, bool once )
        {
            if ( once && project::is_configured( m_project_name, a ) )
                return;
            for ( const configuration& c : c_configs( ) )
            {
                console_title ct( true, "Configuring {} {} {}...", m_project_name, a.name, c.name );
                prepare( a, c );
#if !defined DMK_BUILDER_NOP
                if ( m_insource )
                {
                    if ( !build_process::quiet )
                        println( "In-source build: {}", m_configure_dir );
                    copy_content( m_original_source_dir, m_configure_dir );
                    m_source_dir = m_configure_dir;
                }

                do_configure( );
#else
#endif
            }
            project::set_configured( m_project_name, a );
        }

        // Clean project for given architecture
        void configure_clean_arch( const architecture& a )
        {
            for ( const configuration& c : c_configs( ) )
            {
                console_title ct( true, "Cleaning {} {} {}...", m_project_name, a.name, c.name );
                prepare( a, c );
#if !defined DMK_BUILDER_NOP
                do_configure_clean( );
#else
#endif
            }
            project::clear_configured( m_project_name, a, false );
        }

        // Build project for given architecture
        void build_arch( const architecture& a, bool once )
        {
            if ( once && project::is_built( m_project_name, a ) )
                return;
            configure_arch( a, true );
            for ( const configuration& c : b_configs( ) )
            {
                console_title ct( true, "Building {} {} {}...", m_project_name, a.name, c.name );
                prepare( a, c );
#if !defined DMK_BUILDER_NOP
                do_build( );
#else
#endif
            }
            project::set_built( m_project_name, a );
        }

        // Clean project for given architecture
        void build_clean_arch( const architecture& a )
        {
            for ( const configuration& c : b_configs( ) )
            {
                console_title ct( true, "Cleaning {} {} {}...", m_project_name, a.name, c.name );
                prepare( a, c );
#if !defined DMK_BUILDER_NOP
                do_build_clean( );
#else
#endif
            }
            project::clear_built( m_project_name, a, false );
        }

        // Perform configuring (overridded in the derived classes)
        virtual void do_configure( )
        {
        }

        // Perform building (overridded in the derived classes)
        virtual void do_build( )
        {
        }

        // Perform cleaning (overridded in the derived classes)
        // Default action is to clean configure directory
        virtual void do_configure_clean( )
        {
            remove_content( m_configure_dir );
        }

        // Perform cleaning (overridded in the derived classes)
        // Default action is to clean build directory
        virtual void do_build_clean( )
        {
            project::remove_built( m_project_name, m_arch, m_config );
        }

        // Set internal variables
        void prepare( const architecture& arch, const configuration& config )
        {
            m_arch   = arch;
            m_config = config;
            m_data   = m_project->data( arch, config );

            if ( get_kind( ) == MultiConfig )
            {
                m_configure_dir = m_project->output_dir(
                    project::dir::configure, arch, configuration::all( ), m_project_name );
            }
            else
            {
                m_configure_dir =
                    m_project->output_dir( project::dir::configure, arch, config, m_project_name );
            }
            create_directories( m_configure_dir );
            path lib_dir = m_project->output_dir( project::dir::libraries, m_arch, m_config );
            path bin_dir = m_project->output_dir( project::dir::binaries, m_arch, m_config );
            path inc_dir = m_project->output_dir( project::dir::includes, m_arch, m_config );
            path out_dir = m_project->output_dir( project::dir::install, m_arch, m_config );
            create_directories( lib_dir );
            create_directories( bin_dir );
            create_directories( inc_dir );
            create_directories( out_dir );

#if defined DMK_BUILDER_DEBUG
            if ( !build_process::quiet )
            {
                debug_text c;
                println( "prepare( {}, {} )", m_arch.name, m_config.name );
                println( "    configure_dir = {}", m_configure_dir.string( ) );
                println( "    lib_dir = {}", bin_dir.string( ) );
                println( "    bin_dir = {}", lib_dir.string( ) );
                println( "    inc_dir = {}", inc_dir.string( ) );
                println( "    out_dir = {}", out_dir.string( ) );
                println( "data:\n{}", m_data.stringify( ) );
            }
#endif
        }
        const project* m_project;
        std::string m_project_name;
        json m_data;
        architectures m_archs;
        configurations m_multi_config;
        configurations m_configs;
        path m_original_source_dir;
        path m_source_dir;
        path m_configure_dir;
        architecture m_arch;
        configuration m_config;
        mutable kind m_kind;
        bool m_insource;
    };

    // Builder for prebuilt binaries.
    // No action is performed only message
    class prebuilt_builder : public builder
    {
    public:
    protected:
        using builder::builder;
        friend class builder;
        virtual kind get_default_kind( ) const override
        {
            return MultiBuild;
        }
        virtual void do_configure( ) override
        {
            if ( !build_process::quiet )
                println( "Project {} has prebuilt binaries. No need to configure it", m_project_name );
        }
        virtual void do_build( ) override
        {
            if ( !build_process::quiet )
                println( "Project {} has prebuilt binaries. No need to build it", m_project_name );
        }
    };

    struct cmake_console : public build_process
    {
    public:
        cmake_console( const path& working_dir ) : build_process( env->cmake_path, working_dir )
        {
        }
        cmake_console& D( const std::string& name, bool value )
        {
            ( *this )( "-D" + name + ":BOOL={}", value ? "ON" : "OFF" );
            return *this;
        }
        cmake_console& D( const std::string& name, const char* value )
        {
            ( *this )( "-D" + name + ":STRING={}", qo( value ) );
            return *this;
        }
        cmake_console& D( const std::string& name, const std::string& value )
        {
            ( *this )( "-D" + name + ":STRING={}", qo( value ) );
            return *this;
        }
        cmake_console& D( const std::string& name, const path& value )
        {
            ( *this )( "-D" + name + ":PATH={}", qo( value ) );
            return *this;
        }
    };

    class cmake_builder : public builder
    {
    public:
    protected:
        using builder::builder;
        friend class builder;
        virtual kind get_default_kind( ) const override
        {
            return MultiConfig;
        }
        virtual void do_configure( ) override
        {
            cmake_console cmake( m_configure_dir );
            cmake( "-G{}", qo( m_arch.generator ) );
            cmake( "--no-warn-unused-cli" );

            std::string flags   = join_list( m_data["flags"], " " );
            std::string defines = join_list( m_data["defines"], ";" );

            variable_list vars =
                env->variables + m_project->public_variables( m_arch, m_config ) + dynamic_variables( );

            for ( const auto& var : vars )
            {
                if ( var.first.find( '(' ) != std::string::npos )
                    continue;
                if ( ends_with( var.first, "_dir" ) || ends_with( var.first, "_path" ) ||
                     contains( var.first, "_dir_" ) || contains( var.first, "_path_" ) )
                    cmake.D( "CMGEN_" + asci_uppercase( var.first ), path( var.second ) );
                else
                    cmake.D( "CMGEN_" + asci_uppercase( var.first ), var.second );
            }
            for ( const auto& opt : m_data["options"].as_object( ) )
            {
                if ( opt.second.is_bool( ) )
                {
                    cmake.D( opt.first, opt.second.as_bool( ) );
                }
                else
                {
                    cmake.D( opt.first, opt.second.as_string( "" ) );
                }
            }

            cmake.D( "CMGEN_FLAGS", flags );
            cmake.D( "CMGEN_DEFINES", defines );
            cmake.D( "CMAKE_PREFIX_PATH", env->modules_dir );
            cmake.D( "CMAKE_INSTALL_PREFIX",
                     m_project->output_dir( project::dir::install, m_arch, m_config ) );
            if ( get_kind( ) == SingleConfig )
            {
                cmake.D( "CMAKE_BUILD_TYPE", m_config.name );
            }

            cmake( "-C {}", qo( env->root_dir / "config.cmake" ) );
            if ( m_data.has_key( "cmake_dir" ) )
            {
                cmake( m_source_dir / ( m_data["cmake_dir"] || "" ) );
            }
            else
            {
                cmake( m_source_dir );
            }
            cmake( );
        }
        virtual void do_build( ) override
        {
            exec<build_process>( m_configure_dir, env->cmake_path, "--build . --config {} ", m_config.name );
            bool install = m_data["cmakeinstall"] || 0;
            if ( install )
            {
                exec<build_process>( m_configure_dir,
                                     env->cmake_path,
                                     "-DCMAKE_INSTALL_CONFIG_NAME={} -P cmake_install.cmake",
                                     m_config.name );
            }
        }
        virtual void do_build_clean( ) override
        {
            exec<build_process>(
                m_configure_dir, env->cmake_path, "--build . --config {} --target clean", m_config.name );
        }
    };

    class scons_builder : public builder
    {
    public:
    protected:
        using builder::builder;
        friend class builder;
        virtual void do_build( ) override
        {
            exec<build_process>( m_configure_dir,
                                 env->scons_path,
                                 "--{} {}",
                                 m_arch.bitness,
                                 join( m_data["options"].flatten( ), " " ) );

            exec<build_process>( m_configure_dir,
                                 env->scons_path,
                                 "--{} {} install",
                                 m_arch.bitness,
                                 join( m_data["options"].flatten( ), " " ) );
        }
    };

    class command_builder : public builder
    {
    public:
    protected:
        using builder::builder;
        friend class builder;
        virtual void do_configure( ) override
        {
            path command = "configure_" + env->platform + DMK_COMM_EXT;
            command = m_project->source_dir( ) / command;
            if ( !is_file( command ) )
                return;
            build_process cmd( command, m_configure_dir );

            variable_list list =
                env->variables + m_project->public_variables( m_arch, m_config ) + dynamic_variables( );
            cmd.set_env( list.transform( "CMGEN_", "", text_case::upper ) );
            cmd.set_env( "CMGEN_OPTIONS", join( m_data["options"].flatten( ), " " ) );
            cmd( );
        }
        virtual void do_build( ) override
        {
            path command = "build_" + env->platform + DMK_COMM_EXT;
            command = m_project->source_dir( ) / command;
            if ( !is_file( command ) )
                return;
            build_process cmd( command, m_configure_dir );

            variable_list vars =
                env->variables + m_project->public_variables( m_arch, m_config ) + dynamic_variables( );
            cmd.set_env( vars.transform( "CMGEN_", "", text_case::upper ) );
            cmd.set_env( "CMGEN_OPTIONS", join( m_data["options"].flatten( ), " " ) );
            cmd( );
        }
    };

    class qmake_builder : public builder
    {
    public:
    protected:
        using builder::builder;
        friend class builder;
        virtual kind get_default_kind( ) const override
        {
            return MultiBuild;
        }
        path get_qmakefile( ) const
        {
            path qmakefile = m_data["qmake"] || "";
            if ( qmakefile.empty( ) || !is_file( m_source_dir / qmakefile ) )
                throw error( "Can't find qmake file for {}", m_project_name );
            return qmakefile;
        }
        virtual void do_configure( ) override
        {
            path qmakefile = get_qmakefile( );
            build_process cmd( env->configure_qmake_path, m_configure_dir );

            variable_list vars =
                env->variables + m_project->public_variables( m_arch, m_config ) + dynamic_variables( );
            cmd.set_env( vars.transform( "CMGEN_", "", text_case::upper ) );
            cmd.set_env( "CMGEN_QMAKEFILE", ( m_source_dir / qmakefile ).string( ) );
            cmd( );
        }
        virtual void do_build( ) override
        {
            path qmakefile = get_qmakefile( );
            build_process cmd( env->build_qmake_path, m_configure_dir );

            variable_list vars =
                env->variables + m_project->public_variables( m_arch, m_config ) + dynamic_variables( );
            cmd.set_env( vars.transform( "CMGEN_", "", text_case::upper ) );
            cmd.set_env( "CMGEN_QMAKEFILE", ( m_source_dir / qmakefile ).string( ) );
            cmd( );
        }
    };

    std::string guess_type( const project* proj )
    {
        path source = proj->source_dir( );
#if defined DMK_OS_POSIX
        if ( is_file( source / "configure" ) )
        {
            return "configure";
        }
#endif
        if ( is_file( source / fmt::format( "prebuilt_{}.txt", env->platform ) ) )
        {
            return "prebuilt";
        }
        if ( is_file( source / fmt::format( "build_{}" DMK_COMM_EXT, env->platform ) ) ||
             is_file( source / fmt::format( "configure_{}" DMK_COMM_EXT, env->platform ) ) )
        {
            return "command";
        }
        if ( is_file( source / "CMakeLists.txt" ) )
        {
            return "cmake";
        }
        if ( is_file( source / "SConstruct" ) )
        {
            return "scons";
        }
        if ( is_file( source / fmt::format( "{}.pro", proj->name( ) ) ) )
        {
            return "qmake";
        }
        return std::string( );
    }

    ptr<builder> builder::create( const project* proj,
                                  const architectures& archs,
                                  const configurations& configs )
    {
        const std::string type = proj->data( )["type"] || guess_type( proj );

        if ( type == "cmake" )
        {
            return ptr<builder>( new cmake_builder( proj, archs, configs ) );
        }
        if ( type == "scons" )
        {
            return ptr<builder>( new scons_builder( proj, archs, configs ) );
        }
        if ( type == "command" )
        {
            return ptr<builder>( new command_builder( proj, archs, configs ) );
        }
        if ( type == "prebuilt" )
        {
            return ptr<builder>( new prebuilt_builder( proj, archs, configs ) );
        }
        if ( type == "qmake" )
        {
            return ptr<builder>( new qmake_builder( proj, archs, configs ) );
        }
        throw error( "Unknown builder type: {}", type );
    }

    ptr<builder> builder::create( const project* proj,
                                  const std::string& arch_mask,
                                  const std::string& config_mask )
    {
        auto archs   = env->find_archs( arch_mask );
        auto configs = env->find_configs( config_mask );
        return create( proj, archs, configs );
    }
}
