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
#include <dmk_time.h>
#include <set>

#include "expressions.h"
#include "cmgen.h"
#include "project.h"
#include "fetchers.h"
#include "configurers.h"

namespace dmk
{
    ptr<const environment> env;
    bool build_process::quiet = false;

    namespace usage
    {
        static const std::string select      = "project";
        static const std::string create      = "project";
        static const std::string reset       = "project";
        static const std::string remove      = "project";
        static const std::string fetch       = "";
        static const std::string up          = "";
        static const std::string cls         = "";
        static const std::string license     = "";
        static const std::string batch       = "projects\t[ arch\t[ config ] ]";
        static const std::string get         = "[ prop_mask ]";
        static const std::string help        = "";
        static const std::string deps        = "module";
        static const std::string data        = "[ arch\t[ config ] ]";
        static const std::string vars        = "[ arch\t[ config ] ]";
        static const std::string envvars     = "[ arch\t[ config ] ]";
        static const std::string exit        = "";
        static const std::string import      = "project";
        static const std::string configure   = "[ arch\t[ config ] ]";
        static const std::string reconfigure = "[ arch\t[ config ] ]";
        static const std::string build       = "[ arch\t[ config ] ]";
        static const std::string rebuild     = "[ arch\t[ config ] ]";
        static const std::string clean       = "[ arch\t[ config ] ]";
    }

    class cmds : public command_processor
    {
    public:
        std::string project_name;

    private:
        std::vector<std::string> m_project_stack;

        void push_project( )
        {
            m_project_stack.push_back( project_name );
        }

        void pop_project( )
        {
            if ( m_project_stack.empty( ) )
            {
                throw fatal_error( "pop_project: m_project_stack is empty" );
            }
            project_name = m_project_stack.back( );
            m_project_stack.pop_back( );
        }

    protected:
        virtual std::string get_prompt( ) const override
        {
            return project_name.empty( ) ? env->root_dir.string( )
                                         : ( env->source_root_dir / project_name ).string( );
        }

        void do_select( const std::string& name )
        {
            get_project( name );
            project_name = name;
            println( "Selected project: {}", project_name );
        }

        void do_import( bool once, const std::string& name )
        {
            if ( once && project::is_imported( name ) )
            {
                return;
            }
            create_directories( env->source_root_dir / name );
            project::set_imported( name );
            project::clear_configured( name );
            do_select( name );
            fetch( );
        }

        void do_import_tree( redo_mode mode, const std::string& name )
        {
            auto deps = project::get_dependencies( name );
            if ( !deps.empty( ) )
            {
                if ( !build_process::quiet )
                    println( "List of dependencies: {}", join( deps, ", " ) );
                push_project( );
                for ( const std::string& dep : reversed( deps ) )
                {
                    if ( !build_process::quiet )
                        println( "Importing dependencies... {}", dep );
                    do_import( mode != DoForce, dep );
                }
                pop_project( );
            }

            do_import( mode == DoOnce, name );
        }

        void do_auto_patch( const project* proj )
        {
            if ( is_nonempty_directory( env->modules_dir / proj->name( ) ) )
            {
                copy_content( env->modules_dir / proj->name( ), proj->source_dir( ), false );
            }
        }

        void do_patch( const project* proj )
        {
            path module_dir = env->modules_dir / proj->name( );
            if ( is_nonempty_directory( module_dir ) )
            {
                copy_content( module_dir, proj->source_dir( ) );
            }
            for ( auto p : directory_iterator( proj->source_dir( ) ) )
            {
                if ( matches( "apply*.patch", p.path( ).filename( ).string( ) ) )
                {
                    if ( !is_file( p.path( ).string( ) + ".applied" ) )
                    {
                        println( "Applying patch..." );
                        exec<build_process>(
                            proj->source_dir( ), env->patch_path, "-l -u -p0 -i {}", qo( p ) );
                        touch_file( p.path( ).string( ) + ".applied" );
                    }
                }
            }
            if ( proj->data( ).has_key( "afterimport" ) )
            {
                std::string script = proj->data( )["afterimport"] || "";
                exec<build_process>( proj->source_dir( ), proj->source_dir( ) / script, "" );
            }
        }

        void do_fetch( )
        {
            ptr<project> project = get_project( project_name );
            json package = project->data( )["source"];
            for ( const json& a : package.flatten_array( ) )
            {
                ptr<fetcher> f = fetcher::create( project.get( ), a, project->source_dir( ) );
                f->fetch( );
            }
            do_patch( project.get( ) );
        }

        void copy_license( const ptr<project>& project,
                           const path& item,
                           const path& target_name,
                           const path& target_dir )
        {
            path file = item;
            if ( !file.is_absolute( ) )
            {
                file = project->source_dir( ) / file;
            }
            if ( !is_regular_file( file ) )
            {
                throw error( "Can't find license file: {}", file );
            }
            create_directories( target_dir );
            path target_file = target_dir / target_name;
            if ( !build_process::quiet )
                println( "Copying file {} -> {}", file, target_file );
            fix_write_rights( target_file );
            copy_file( file, target_file, DMK_COPY_OVERWRITE );
        }

        void do_license( )
        {
            ptr<project> project = get_project( project_name );
            json licenses        = project->data( )["license"];
            path target_dir = env->licenses_dir / project_name;
            if ( licenses.is_object( ) )
            {
                for ( const json::objectpair& l : licenses.as_object( ) )
                {
                    copy_license( project, l.second || "", l.first, target_dir );
                }
                return;
            }
            else if ( licenses.is_array( ) || licenses.is_string( ) )
            {
                for ( const json& l : licenses.flatten( ) )
                {
                    copy_license( project, l || "", path( l.as_string( ) ).filename( ), target_dir );
                }
                return;
            }

            std::vector<path> paths;

            paths += entries( project->source_dir( ), "license*" );
            paths += entries( project->source_dir( ), "copying*" );
            paths += entries( project->source_dir( ), "lgpl*" );

            if ( paths.empty( ) && project->data( )["source"] != "local" )
            {
                throw error( "Can't find license for project {}", project_name );
            }

            for ( auto file : paths )
            {
                copy_license( project, file, file.filename( ), target_dir );
                return;
            }
        }

        ptr<project> get_project( const std::string& name )
        {
            if ( name == "" )
            {
                throw command_error( "Project isn't selected" );
            }
            if ( !project::is_module( name ) )
            {
                throw command_error( "Module {} doesn't exist", name );
            }
            if ( !project::is_imported( name ) )
            {
                throw command_error( "Module {} isn't imported", name );
            }

            try
            {
                return ptr<project>( new project( name ) );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't load project {}", name );
            }
        }

        void do_build( void ( builder::*action )( ), const std::string& arch, const std::string& config )
        {
            ptr<project> proj = get_project( project_name );
            do_auto_patch( proj.get( ) );
            ptr<builder> b = builder::create( proj.get( ), arch, config );
            ( b.get( )->*action )( );
        }

        void do_build_tree( void ( builder::*action_current )( ),
                            void ( builder::*action_dependencies )( ),
                            const std::string& arch,
                            const std::string& config )
        {
            get_project( project_name );
            if ( action_dependencies )
            {
                auto deps = project::get_dependencies( project_name );
                if ( !deps.empty( ) )
                {
                    if ( !build_process::quiet )
                        println( "List of dependencies: {}", join( deps, ", " ) );
                    push_project( );
                    for ( const std::string& dep : reversed( deps ) )
                    {
                        project_name = dep;
                        do_build( action_dependencies, arch, config );
                    }
                    pop_project( );
                }
            }

            do_build( action_current, arch, config );
        }

        void do_remove( const std::string& name )
        {
            if ( !project::is_module( name ) )
                throw error( "Module {} doesn't exist", name );
            if ( !project::is_imported( name ) )
                throw error( "Module {} isn't imported", name );
            if ( project::is_local( name ) )
                throw error( "Project {} is local", name );
            fmt::print( "Removing {}... type y to confirm : ", name );
            std::string y;
            std::getline( std::cin, y );
            if ( y == "y" )
            {
                project::clear_imported( name );
                if ( project_name == name )
                {
                    project_name = "";
                }
            }
            else
            {
                println( "Cancelled" );
            }
        }

        bool batch_task( const std::string& task, const std::function<void( )>& func )
        {
            elapsed_timer t;
            build_process::quiet = true;
            try
            {
                func( );
                green_err_text c;
                fmt::print( stderr, "--- {}: ok ({:4.2f}s)\n", task, t.elapsed( ).as_double( ) );
            }
            catch ( const std::exception& e )
            {
                red_err_text c;
                fmt::print( stderr, "--- {}: failed ({:4.2f}s)\n", task, t.elapsed( ).as_double( ) );
                fmt::print( stderr, "{}\n", e.what( ) );
                build_process::quiet = false;
                return false;
            }
            build_process::quiet = false;
            return true;
        }

        void do_batch( redo_mode mode,
                       const std::string& name,
                       const std::string& arch,
                       const std::string& config )
        {
            if ( !batch_task( "import",
                              [&]( )
                              {
                                  import( DoOnce, name );
                              } ) )
                return;
            if ( !batch_task( "configure",
                              [&]( )
                              {
                                  do_select( name );
                                  configure( mode, arch, config );
                              } ) )
                return;
            if ( !batch_task( "build",
                              [&]( )
                              {
                                  do_select( name );
                                  build( mode, arch, config );
                              } ) )
                return;
        }

    public:
        void select( const std::string& name )
        {
            try
            {
                do_select( name );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't select project {}", name );
            }
        }

        void import( redo_mode mode, const std::string& name )
        {
            path module_path = project::module_path( name );
            try
            {
                if ( !project::is_module( name ) )
                {
                    throw error( "Module {} doesn't exist", name );
                }
                do_import_tree( mode, name );
            }
            catch ( const std::exception& e )
            {
                project::clear_imported( name );
                throw command_error( e, "Couldn't import project {}", name );
            }
        }

        void up( )
        {
            project_name = "";
        }

        void remove( const std::string& name )
        {
            try
            {
                do_remove( name );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't remove project {}", name );
            }
        }

        void cls( )
        {
            system( "cls" );
        }

        void fetch( )
        {
            try
            {
                do_fetch( );
                do_license( );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't update project {}", project_name );
            }
        }

        void license( )
        {
            try
            {
                do_license( );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't get license for project {}", project_name );
            }
        }

        void deps( const std::string& name )
        {
            try
            {
                auto deps = project::get_dependencies( name );
                println( "{}: {}", name, join( deps, ", " ) );
                for ( auto dep : deps )
                {
                    auto subdeps = project::get_dependencies( dep );
                    println( "    {}: {}", dep, join( subdeps, ", " ) );
                }
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't get dependencies for {}", project_name );
            }
        }

        void configure( redo_mode mode, const std::string& arch, const std::string& config )
        {
            try
            {
                switch ( mode )
                {
                case DoOnce:
                    do_build_tree( &builder::configure_once, &builder::build_once, arch, config );
                    break;
                case DoAlways:
                    do_build_tree( &builder::configure, &builder::build_once, arch, config );
                    break;
                case DoForce:
                    do_build_tree( &builder::configure, &builder::build, arch, config );
                    break;
                }
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't configure project {}", project_name );
            }
        }

        void reconfigure( redo_mode mode, const std::string& arch, const std::string& config )
        {
            try
            {
                switch ( mode )
                {
                case DoAlways:
                    do_build_tree( &builder::reconfigure, &builder::build_once, arch, config );
                    break;
                case DoForce:
                    do_build_tree( &builder::reconfigure, &builder::rebuild, arch, config );
                    break;
                }
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't reconfigure project {}", project_name );
            }
        }

        void reset( const std::string& arch, const std::string& config )
        {
            try
            {
                do_build_tree( &builder::configure_clean, nullptr, arch, config );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't reset project {}", project_name );
            }
        }

        void build( redo_mode mode, const std::string& arch, const std::string& config )
        {
            try
            {
                switch ( mode )
                {
                case DoOnce:
                    do_build_tree( &builder::build_once, &builder::build_once, arch, config );
                    break;
                case DoAlways:
                    do_build_tree( &builder::build, &builder::build_once, arch, config );
                    break;
                case DoForce:
                    do_build_tree( &builder::build, &builder::build, arch, config );
                    break;
                }
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't build project {}", project_name );
            }
        }

        void rebuild( redo_mode mode, const std::string& arch, const std::string& config )
        {
            try
            {
                switch ( mode )
                {
                case DoAlways:
                    do_build_tree( &builder::rebuild, &builder::build_once, arch, config );
                    break;
                case DoForce:
                    do_build_tree( &builder::rebuild, &builder::rebuild, arch, config );
                    break;
                }
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't rebuild project {}", project_name );
            }
        }

        void clean( const std::string& arch, const std::string& config )
        {
            try
            {
                do_build_tree( &builder::build_clean, nullptr, arch, config );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't clean project {}", project_name );
            }
        }

        std::string flag_list( const std::string& flag, const std::string& name )
        {
            std::vector<std::string> result;
            for ( auto a : env->archs )
            {
                result.push_back( is_file( project::flag_path( name, flag, a ) ) ? "Y" : "." );
            }
            return join( result, " " );
        }

        void batch( redo_mode mode,
                    const std::string& projects,
                    const std::string& arch,
                    const std::string& config )
        {
            std::vector<std::string> masks = split( projects, ',' );
            std::vector<std::string> list;
            for ( const std::string& mask : masks )
            {
                for ( const directory_entry& entry : directory_iterator( env->modules_dir ) )
                {
                    if ( !is_file( entry ) )
                        continue;
                    std::string name = entry.path( ).stem( ).string( );
                    if ( matches( mask, name ) )
                    {
                        list.push_back( name );
                    }
                }
            }
            std::set<std::string> original_list( list.begin( ), list.end( ) );
            for ( size_t i = 0; i < list.size( ); )
            {
                std::string name              = list[i];
                std::vector<std::string> deps = project::get_dependencies( name );
                deps = reversed( deps );
                list.insert( list.begin( ) + i, deps.begin( ), deps.end( ) );
                i += 1 + deps.size( );
            }
            for ( size_t i = 1; i < list.size( ); )
            {
                std::string name = list[i];
                auto it = list.begin( ) + i;
                if ( std::find( list.begin( ), it, name ) != it )
                {
                    list.erase( it );
                }
                else
                {
                    i++;
                }
            }
            println( "projects included in the batch: {}", join( list, ", " ) );
            for ( const std::string& name : list )
            {
                push_project( );
                if ( original_list.find( name ) != original_list.end( ) )
                {
                    {
                        cyan_err_text c;
                        fmt::print( stderr, "- project: {}\n", name );
                    }
                    do_batch( mode, name, arch, config );
                }
                else
                {
                    {
                        cyan_err_text c;
                        fmt::print( stderr, "- project: {} (dep)\n", name );
                    }
                    do_batch( DoOnce, name, arch, config );
                }
                pop_project( );
            }
        }

        void modules( const std::string& pattern )
        {
            static const char* yes_no[2] = { ".", "Y" };
            size_t c                     = 0;
            green_text col;
            println(
                "{:20} {:20} {:10} {:10} {:10}", "module", "version", "imported", "configured", "built" );
            println( "{0:-<20} {0:-<20} {0:-<10} {0:-<10} {0:-<10}", "" );
            for ( const directory_entry& entry : directory_iterator( env->modules_dir ) )
            {
                if ( !is_file( entry ) )
                    continue;
                std::string name = entry.path( ).stem( ).string( );
                if ( pattern.empty( ) || matches( pattern, name ) )
                {
                    try
                    {
                        json data = file_get_json( entry );
                        println( "{:20} {:20} {:10} {:10} {:10}",
                                 name,
                                 data["version"].as_string( "unknown version" ),
                                 yes_no[project::is_imported( name )],
                                 flag_list( "configured", name ),
                                 flag_list( "built", name ) );
                        c++;
                    }
                    catch ( const std::exception& e )
                    {
                        errorln( "Can't load module {}\n{}", name, e.what( ) );
                    }
                }
            }
            if ( c == 0 )
            {
                red_text col;
                println( "No modules matched the pattern {}", pattern );
            }
        }

        void data( const std::string& arch, const std::string& config )
        {
            try
            {
                ptr<project> project = get_project( project_name );

                yellow_text c;
                json data;
                if ( !config.empty( ) )
                {
                    auto archs   = env->find_archs( arch );
                    auto configs = env->find_configs( config );
                    data = project->data( archs[0], configs[0] );
                    println(
                        "Showing configuration for {} {} {}", project_name, archs[0].name, configs[0].name );
                }
                else if ( !arch.empty( ) )
                {
                    auto archs = env->find_archs( arch );
                    data = project->data( archs[0], configuration::all( ) );
                    println( "Showing configuration for {} {}", project_name, archs[0].name );
                }
                else
                {
                    data = project->data( );
                    println( "Showing configuration for {}", project_name );
                }
                println( replace_all( data.stringify( ), "\t", "  " ) );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't get project's configuration" );
            }
        }
        void vars( const std::string& arch, const std::string& config )
        {
            try
            {
                yellow_text c;
                variable_list list = env->variables;
                println( "{:30}   {}", "Variable", "Value" );
                println( "{0:-<30}   {0:-<44}", "" );
                if ( !project_name.empty( ) )
                {
                    ptr<project> project = get_project( project_name );
                    list += project->variables( ) + project->cross_variables( );
                }
                if ( !config.empty( ) )
                {
                    list += project::work_variables(
                        env->find_archs( arch )[0], env->find_configs( config )[0], project_name );
                }
                else if ( !arch.empty( ) )
                {
                    list += project::work_variables(
                        env->find_archs( arch )[0], configuration::all( ), project_name );
                }
                list += dynamic_variables( );
                list.print( );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't get variables" );
            }
        }
        void envvars( const std::string& arch, const std::string& config )
        {
            try
            {
                yellow_text c;
                variable_list list = env->variables;
                println( "{:30}   {}", "Variable", "Value" );
                println( "{0:-<30}   {0:-<44}", "" );
                if ( !project_name.empty( ) )
                {
                    ptr<project> project = get_project( project_name );
                    list += project->variables( );
                }
                if ( !config.empty( ) )
                {
                    list += project::work_variables(
                        env->find_archs( arch )[0], env->find_configs( config )[0], project_name );
                }
                else if ( !arch.empty( ) )
                {
                    list += project::work_variables(
                        env->find_archs( arch )[0], configuration::all( ), project_name );
                }
                list += dynamic_variables( );
                list = list.transform( "CMGEN_", "", text_case::upper );
                list.print( );
            }
            catch ( const std::exception& e )
            {
                throw command_error( e, "Couldn't get variables" );
            }
        }
    };

    int cmgen_run( arguments& args )
    {
        using namespace std::placeholders;
        using std::bind;
        namespace u = usage;
        cmds cp;

        std::string proj = args.extract( "--project" );
        if ( !proj.empty( ) )
        {
            cp.select( proj );
        }

        cp.bind( "select", u::select, bind( &cmds::select, &cp, _1 ), 1, 1 );
        cp.bind( "fetch", u::fetch, bind( &cmds::fetch, &cp ) );
        cp.bind( "up", u::up, bind( &cmds::up, &cp ) );
        cp.bind( "cls", u::cls, bind( &cmds::cls, &cp ) );
        cp.bind( "remove", u::remove, bind( &cmds::remove, &cp, _1 ), 1, 1 );
        cp.bind( "license", u::license, bind( &cmds::license, &cp ), 0, 0 );

        cp.bind( "modules", u::data, bind( &cmds::modules, &cp, _1 ), 0, 1 );
        cp.bind( "deps", u::deps, bind( &cmds::deps, &cp, _1 ), 1, 1 );
        cp.bind( "data", u::data, bind( &cmds::data, &cp, _1, _2 ), 0, 2 );
        cp.bind( "vars", u::vars, bind( &cmds::vars, &cp, _1, _2 ), 0, 2 );
        cp.bind( "env", u::envvars, bind( &cmds::envvars, &cp, _1, _2 ), 0, 2 );
        cp.bind( "help", u::help, bind( &cmds::help, &cp ) );
        cp.bind( "exit", u::exit, bind( &cmds::exit, &cp ) );

        cp.bind( "reset", u::reset, bind( &cmds::reset, &cp, _1, _2 ), 0, 2 );
        cp.bind( "clean", u::clean, bind( &cmds::clean, &cp, _1, _2 ), 0, 2 );

        cp.bind( "import", u::import, bind( &cmds::import, &cp, DoAlways, _1 ), 1, 1 );
        cp.bind( "configure", u::configure, bind( &cmds::configure, &cp, DoAlways, _1, _2 ), 0, 2 );
        cp.bind( "build", u::build, bind( &cmds::build, &cp, DoAlways, _1, _2 ), 0, 2 );
        cp.bind( "reconfigure", u::reconfigure, bind( &cmds::reconfigure, &cp, DoAlways, _1, _2 ), 0, 2 );
        cp.bind( "rebuild", u::rebuild, bind( &cmds::rebuild, &cp, DoAlways, _1, _2 ), 0, 2 );
        cp.bind( "batch", u::batch, bind( &cmds::batch, &cp, DoAlways, _1, _2, _3 ), 0, 3 );

        cp.bind( "import!", u::import, bind( &cmds::import, &cp, DoForce, _1 ), 1, 1 );
        cp.bind( "configure!", u::configure, bind( &cmds::configure, &cp, DoForce, _1, _2 ), 0, 2 );
        cp.bind( "build!", u::build, bind( &cmds::build, &cp, DoForce, _1, _2 ), 0, 2 );
        cp.bind( "reconfigure!", u::reconfigure, bind( &cmds::reconfigure, &cp, DoForce, _1, _2 ), 0, 2 );
        cp.bind( "rebuild!", u::rebuild, bind( &cmds::rebuild, &cp, DoForce, _1, _2 ), 0, 2 );
        cp.bind( "batch!", u::batch, bind( &cmds::batch, &cp, DoForce, _1, _2, _3 ), 0, 3 );

        cp.bind( "import?", u::import, bind( &cmds::import, &cp, DoOnce, _1 ), 1, 1 );
        cp.bind( "configure?", u::configure, bind( &cmds::configure, &cp, DoOnce, _1, _2 ), 0, 2 );
        cp.bind( "build?", u::build, bind( &cmds::build, &cp, DoOnce, _1, _2 ), 0, 2 );
        cp.bind( "batch?", u::batch, bind( &cmds::batch, &cp, DoOnce, _1, _2, _3 ), 0, 3 );

        cp.alias( "project", "select" );
        cp.alias( "update", "fetch" );

        cp.alias( "make", "configure" );
        cp.alias( "remake", "reconfigure" );
        cp.alias( "make!", "configure!" );
        cp.alias( "remake!", "reconfigure!" );
        cp.alias( "make?", "configure?" );
        cp.alias( "test", "batch" );

        if ( args.count( ) > 0 )
        {
            return cp.execute_command( args.args( ) );
        }
        else
        {
            return cp.interactive( );
        }
    }
}

int main( int argc, char** argv, char** envp )
{
    using namespace dmk;
    console_title::set( "CMGen" );

    arguments args( argc, argv, envp );
    try
    {
        println( "CMGen v0.3" );
        env.reset( new environment( args ) );
        println( "Type help for a list of supported commands" );
    }
    catch ( const std::exception& e )
    {
        errorln( "Exception during initialization" );
        errorln( e.what( ) );
        return fail_exit( );
    }

    return cmgen_run( args );
}
