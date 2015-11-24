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

    class project
    {
    public:
        project( const std::string& name ) : m_name( name ), m_source_dir( env->source_root_dir / name )
        {
            if ( !is_directory( m_source_dir ) )
            {
                throw command_error( "Project doesn't exist in directory {}", m_source_dir );
            }
            path data_path  = module_path( name );
            m_original_data = file_get_json( data_path );
            m_version = m_original_data["version"].as_string( );
            create_directories( m_source_dir );

            m_external_variables = env->env + env->variables + dynamic_variables( );

            m_variables["project"]    = name;
            m_variables["module"]     = name;
            m_variables["source_dir"] = m_source_dir.string( );
            for ( const json::objectpair& v : m_original_data.as_object( ) )
            {
                if ( v.first.find( '|' ) != std::string::npos ) // skip conditions
                    continue;
                switch ( v.second.get_type( ) )
                {
                case json::String:
                case json::Int:
                case json::Double:
                    m_variables["this." + v.first] = v.second.to_string( );
                    break;
                }
            }

            if ( !m_version.empty( ) )
            {
                m_variables["version"]            = m_version;
                m_variables["version(.)"]         = m_version;
                m_variables["version()"]          = replace_all( m_version, ".", "" );
                m_variables["version(-)"]         = replace_all( m_version, ".", "-" );
                m_variables["version(_)"]         = replace_all( m_version, ".", "_" );
                m_variables["version(/)"]         = replace_all( m_version, ".", "/" );
                m_variables["version_no"]         = replace_all( m_version, ".", "" );
                m_variables["version_minus"]      = replace_all( m_version, ".", "-" );
                m_variables["version_underscore"] = replace_all( m_version, ".", "_" );
                m_variables["version_slash"]      = replace_all( m_version, ".", "/" );

                std::vector<std::string> parts = split( m_version, "." );
                m_variables["version1"]        = parts.size( ) >= 1 ? parts[0] : "";
                m_variables["version2"]        = parts.size( ) >= 2 ? parts[1] : "";
                m_variables["version3"]        = parts.size( ) >= 3 ? parts[2] : "";
                m_variables["version4"]        = parts.size( ) >= 4 ? parts[3] : "";
            }

            for ( const architecture& arch : env->archs )
            {
                for ( const configuration& cfg : env->configs_all )
                {
                    m_cross_variables +=
                        dir_variables( arch, cfg, m_name )
                            .transform(
                                "", "_" + asci_lowercase( arch.name ) + "_" + asci_lowercase( cfg.name ) );
                }
                m_cross_variables +=
                    root_dir_variables( arch, m_name ).transform( "", "_" + asci_lowercase( arch.name ) );
            }
            m_data = ( m_external_variables + variables( ) + cross_variables( ) )( m_original_data );
        }
        const std::string& name( ) const
        {
            return m_name;
        }
        const std::string& version( ) const
        {
            return m_version;
        }
        const json& data( ) const
        {
            return m_data;
        }
        json data( const architecture& arch, const configuration& config ) const
        {
            return ( m_external_variables + variables( arch, config ) )( m_original_data );
        }
        const path& source_dir( ) const
        {
            return m_source_dir;
        }
        const variable_list& variables( ) const
        {
            return m_variables;
        }
        const variable_list& cross_variables( ) const
        {
            return m_cross_variables;
        }

        static variable_list root_dir_variables( const architecture& arch, const std::string& name = "" )
        {
            variable_list vars;

            vars["configure_root_dir"] = output_root_dir( dir::configure, arch ).string( );
            vars["lib_root_dir"]       = output_root_dir( dir::libraries, arch ).string( );
            vars["bin_root_dir"]       = output_root_dir( dir::binaries, arch ).string( );
            vars["inc_root_dir"]       = output_root_dir( dir::includes, arch ).string( );
            vars["install_root_dir"]   = output_root_dir( dir::install, arch ).string( );
            return vars;
        }

        static variable_list all_dir_variables( const architecture& arch, const std::string& name = "" )
        {
            variable_list vars;

            vars["configure_all_dir"] = output_all_dir( dir::configure, arch ).string( );
            vars["lib_all_dir"]       = output_all_dir( dir::libraries, arch ).string( );
            vars["bin_all_dir"]       = output_all_dir( dir::binaries, arch ).string( );
            vars["inc_all_dir"]       = output_all_dir( dir::includes, arch ).string( );
            vars["install_all_dir"]   = output_all_dir( dir::install, arch ).string( );
            return vars;
        }

        static variable_list cfg_dir_variables( const architecture& arch, const configuration& config )
        {
            variable_list vars;

            vars["configure_cfg_dir"] = output_cfg_dir( dir::configure, arch, config ).string( );
            vars["lib_cfg_dir"]       = output_cfg_dir( dir::libraries, arch, config ).string( );
            vars["bin_cfg_dir"]       = output_cfg_dir( dir::binaries, arch, config ).string( );
            vars["inc_cfg_dir"]       = output_cfg_dir( dir::includes, arch, config ).string( );
            vars["install_cfg_dir"]   = output_cfg_dir( dir::install, arch, config ).string( );
            return vars;
        }

        static variable_list dir_variables( const architecture& arch,
                                            const configuration& config,
                                            const std::string& name )
        {
            variable_list vars;

            vars["configure_dir"] = output_dir( dir::configure, arch, config, name ).string( );
            vars["lib_dir"]       = output_dir( dir::libraries, arch, config, name ).string( );
            vars["bin_dir"]       = output_dir( dir::binaries, arch, config, name ).string( );
            vars["inc_dir"]       = output_dir( dir::includes, arch, config, name ).string( );
            vars["install_dir"]   = output_dir( dir::install, arch, config, name ).string( );
            return vars;
        }

        static variable_list work_variables( const architecture& arch,
                                             const configuration& config,
                                             const std::string& name = "" )
        {
            variable_list vars;

            vars[arch.lower_name]   = var_true;
            vars[config.lower_name] = var_true;
            vars["arch"]            = arch.name;
            vars["arch_suffix"]     = arch.suffix;
            vars["arch_bitness"]    = arch.bitness;
            vars["arch_generator"]  = arch.generator;
            vars["config"]          = config.name;

            vars += root_dir_variables( arch, name );
            vars += all_dir_variables( arch, name );
            vars += cfg_dir_variables( arch, config );
            if ( !name.empty( ) )
                vars += dir_variables( arch, config, name );
            return vars;
        }

        variable_list variables( const architecture& arch, const configuration& config ) const
        {
            return m_variables + m_cross_variables + work_variables( arch, config, m_name );
        }

        variable_list public_variables( const architecture& arch, const configuration& config ) const
        {
            return m_variables + work_variables( arch, config, m_name );
        }

        enum class dir
        {
            libraries,
            binaries,
            includes,
            configure,
            install
        };

        static const std::string& dirname( dir dir )
        {
            static const std::string libraries = "lib";
            static const std::string binaries  = "bin";
            static const std::string includes  = "inc";
            static const std::string configure = "conf";
            static const std::string install   = "out";

            switch ( dir )
            {
            case project::dir::libraries:
                return libraries;
            case project::dir::binaries:
                return binaries;
            case project::dir::includes:
                return includes;
            case project::dir::configure:
                return configure;
            case project::dir::install:
                return install;
            }
            return install;
        }

        path output_dir( dir dir, const architecture& arch, const configuration& config ) const
        {
            return output_dir( dir, arch, config, m_name );
        }

        static path output_root_dir( dir dir, const architecture& arch )
        {
            return env->platform_dir / ( dirname( dir ) + arch.suffix );
        }

        static path output_cfg_dir( dir dir, const architecture& arch, const configuration& config )
        {
            return output_root_dir( dir, arch ) / config.name;
        }

        static path output_all_dir( dir dir, const architecture& arch )
        {
            return output_cfg_dir( dir, arch, configuration::all( ) );
        }

        static path output_dir( dir dir,
                                const architecture& arch,
                                const configuration& config,
                                const std::string& project )
        {
            return output_cfg_dir( dir, arch, config ) / project;
        }

        static void remove_imported( const std::string& name )
        {
            remove_directory( env->source_root_dir / name );
        }
        static void remove_configured( const std::string& name, const architecture& arch )
        {
            for ( auto c : env->configs_all )
            {
                remove_directory( output_dir( dir::configure, arch, c, name ) );
            }
        }
        static void remove_built( const std::string& name, const architecture& arch )
        {
            for ( auto c : env->configs_all )
            {
                remove_built( name, arch, c );
            }
        }
        static void remove_built( const std::string& name,
                                  const architecture& arch,
                                  const configuration& config )
        {
            remove_directory( output_dir( dir::libraries, arch, config, name ) );
            remove_directory( output_dir( dir::binaries, arch, config, name ) );
            remove_directory( output_dir( dir::includes, arch, config, name ) );
            remove_directory( output_dir( dir::install, arch, config, name ) );
        }

        static bool is_imported( const std::string& name )
        {
            return is_file( flag_path( name, "imported" ) );
        }
        static void set_imported( const std::string& name )
        {
            touch_file( flag_path( name, "imported" ) );
        }
        static void clear_imported( const std::string& name )
        {
            remove_if_exists( flag_path( name, "imported" ) );
            remove_imported( name );
            for ( auto a : env->archs )
                clear_configured( name, a );
        }

        static bool is_configured( const std::string& name, const architecture& arch )
        {
            return is_file( flag_path( name, "configured", arch ) );
        }
        static void set_configured( const std::string& name, const architecture& arch )
        {
            touch_file( flag_path( name, "configured", arch ) );
        }
        static void clear_configured( const std::string& name, bool clean = true )
        {
            for ( auto a : env->archs )
            {
                clear_configured( name, a, clean );
            }
        }
        static void clear_configured( const std::string& name, const architecture& arch, bool clean = true )
        {
            remove_if_exists( flag_path( name, "configured", arch ) );
            if ( clean )
            {
                remove_configured( name, arch );
            }
            clear_built( name, arch, clean );
        }

        static bool is_built( const std::string& name, const architecture& arch )
        {
            return is_file( flag_path( name, "built", arch ) );
        }
        static void set_built( const std::string& name, const architecture& arch )
        {
            touch_file( flag_path( name, "built", arch ) );
        }
        static void clear_built( const std::string& name, const architecture& arch, bool clean = true )
        {
            remove_if_exists( flag_path( name, "built", arch ) );
            if ( clean )
            {
                remove_built( name, arch );
            }
        }

        static path flag_path( const std::string& name, const std::string& flag, const architecture& arch )
        {
            return env->flags_dir / ( name + '-' + flag + '-' + env->platform + '-' + arch.name );
        }

        static path flag_path( const std::string& name, const std::string& flag )
        {
            return env->flags_dir / ( name + '-' + flag );
        }

        static path module_path( const std::string& name )
        {
            return ( env->modules_dir / name ).string( ) + ".txt";
        }

        static bool is_module( const std::string& name )
        {
            return is_file( module_path( name ) );
        }

        static void build_dependencies_list( std::vector<std::string>& list, const std::string& name )
        {
            json module = file_get_json( project::module_path( name ) );
            for ( const json& d : module["dependencies"].flatten( ) )
            {
                std::string dep = d || "";
                if ( dep.empty( ) )
                    continue;
                // if this dependency isn't in the list...
                auto it = std::find( list.begin( ), list.end( ), dep );
                if ( it == list.end( ) )
                {
                    list.push_back( dep );
                    build_dependencies_list( list, dep );
                }
                else
                {
                    list.erase( it );
                    list.push_back( dep );
                }
            }
        }

        static std::vector<std::string> get_dependencies( const std::string& name )
        {
            std::vector<std::string> list;
            build_dependencies_list( list, name );
            return list;
        }

    private:
        const std::string m_name;
        const path m_source_dir;
        std::string m_version;
        json m_original_data;
        json m_data;
        variable_list m_variables;
        variable_list m_cross_variables;
        variable_list m_external_variables;
    };
}
