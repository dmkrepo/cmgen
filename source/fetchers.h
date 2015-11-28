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

    // Download/Clone/Copy external sources into local directory
    class fetcher
    {
    public:
        static ptr<fetcher> create( const project* proj, const json& package, const path& destination );
        void fetch( )
        {
            console_title ct( true, "Fetching {}...", m_project->name( ) );
            do_fetch( );
            for ( auto t : m_temporaries )
            {
                yellow_text c;
                if ( !build_process::quiet )
                {
                    println( "Removing temporary {}", t.string( ) );
                }
                safe_remove_all( t );
            }
            m_temporaries.clear( );
        }

    protected:
        virtual void do_fetch( )
        {
        }
        fetcher( const project* proj, const json& package, const path& destination )
            : m_project( proj ), m_package( package ), m_destination( destination )
        {
        }
        const project* m_project;

        std::vector<path> m_temporaries;
        const json m_package;
        const path m_destination;
    };

    // Clone git repository
    class git_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            if ( is_directory( m_destination / ".git" ) )
            {
                exec<build_process>( m_destination, env->git_path, "pull" );
            }
            else
            {
                if ( is_file( m_destination / ".DS_Store" ) )
                {
                    remove( m_destination / ".DS_Store" );
                }
                std::string url    = m_package["url"] || "";
                std::string branch = m_package["branch"] || "master";
                exec<build_process>( m_destination.parent_path( ),
                                     env->git_path,
                                     "clone -b {} --single-branch --depth 1 {} {}",
                                     qo( branch ),
                                     url,
                                     m_destination.filename( ) );
            }
        }
    };

    // Clone hg repository
    class hg_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            if ( is_directory( m_destination / ".hg" ) )
            {
                exec<build_process>( m_destination, env->hg_path, "pull" );
            }
            else
            {
                std::string url    = m_package["url"] || "";
                std::string branch = m_package["branch"] || "default";
                exec<build_process>( m_destination.parent_path( ),
                                     env->hg_path,
                                     "clone -b {} {} {}",
                                     qo( branch ),
                                     url,
                                     m_destination.filename( ) );
            }
        }
    };

    // Download archive (base class)
    class download_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        // Fix filename for sf.net links
        static path extract_filename( const path& filepath )
        {
            if ( filepath.filename( ).string( ) == "download" )
            {
                return filepath.parent_path( ).filename( );
            }
            return filepath.filename( );
        }
        // Download file to temporary folder and return file path
        path download( )
        {
            std::string url = m_package["url"] || "";
            path filename   = path( m_package["file"] || extract_filename( url ).string( ) );
            path tmpfolder = unique_path( env->temp_dir, "", "tmpfolder%04d" );
            create_directory( tmpfolder );
            path tmpfile = tmpfolder / filename;
            exec<build_process>(
                tmpfolder, env->curl_path, "-o {} -L {} --stderr -", qo( tmpfile ), qo( url ) );
            m_temporaries.push_back( tmpfolder );
            return tmpfile;
        }
    };

    // Download archive and uncompress using tar
    class archive_fetcher : public download_fetcher
    {
    protected:
        using download_fetcher::download_fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            path target_dir  = path( m_package["target_dir"] || m_destination.string( ) );
            path tmpfile     = download( );
            int strip_levels = m_package["strip"] || 1;

            exec<build_process>( m_destination,
                                 env->tar_path,
                                 "-xf {} --strip={} -C {}",
                                 qo( tmpfile ),
                                 strip_levels,
                                 qo( target_dir ) );
        }
    };

    // Download archive and uncompress using unzip
    class ziparchive_fetcher : public download_fetcher
    {
    protected:
        using download_fetcher::download_fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            std::string url  = m_package["url"] || "";
            path target_dir  = path( m_package["target_dir"] || m_destination.string( ) );
            path tmpfile     = download( );
            int strip_levels = m_package["strip"] || 1;

            path target_tmp_dir = target_dir / "tmp-zip";
            create_directories( target_tmp_dir );
            exec<build_process>(
                m_destination, env->unzip_path, "-q {} -d {}", qo( tmpfile ), qo( target_tmp_dir ) );

            if ( strip_levels == 0 )
                move_content( target_tmp_dir, target_dir );
            else if ( strip_levels == 1 )
                move_content_strip1( target_tmp_dir, target_dir );
            else
                throw error( "ziparchive_fetcher: Invalid strip value ({})", strip_levels );
        }
    };

    // Download archive and uncompress using 7zip
    class sevenziparchive_fetcher : public download_fetcher
    {
    protected:
        using download_fetcher::download_fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            std::string url  = m_package["url"] || "";
            path target_dir  = path( m_package["target_dir"] || m_destination.string( ) );
            path tmpfile     = download( );
            int strip_levels = m_package["strip"] || 1;

            path target_tmp_dir = target_dir / "tmp-7zip";
            create_directories( target_tmp_dir );
            std::string fn = tmpfile.filename( ).string( );
            if ( ends_with( fn, ".tar.gz" ) || ends_with( fn, ".tar.bz" ) || ends_with( fn, ".tar.xz" ) )
            {
                path tar_file = tmpfile;
                tar_file.replace_extension( );
                exec<build_process>( tmpfile.parent_path( ), env->sevenzip_path, "x {}", qo( tmpfile ) );
                m_temporaries.push_back( tar_file );
                // stage 1:
                // stage 2: unpack tar
                exec<build_process>(
                    m_destination, env->sevenzip_path, "x -o{} {}", qo( target_tmp_dir ), qo( tar_file ) );
            }
            else
            {
                exec<build_process>(
                    m_destination, env->sevenzip_path, "x -o{} {}", qo( target_tmp_dir ), qo( tmpfile ) );
            }

            if ( strip_levels == 0 )
                move_content( target_tmp_dir, target_dir );
            else if ( strip_levels == 1 )
                move_content_strip1( target_tmp_dir, target_dir );
            else
                throw error( "sevenziparchive_fetcher: Invalid strip value ({})", strip_levels );
        }
    };

    // Download installer and run it
    class installer_fetcher : public download_fetcher
    {
    protected:
        using download_fetcher::download_fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            std::string command = m_package["command"] || "";
            path tmpfile = download( );
            exec<build_process>( m_destination, tmpfile, command );
        }
    };

    // Download from local directory
    class copy_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            path copy_dir   = path( m_package["copy_dir"] || "" );
            path target_dir = path( m_package["target_dir"] || m_destination.string( ) );
            if ( !is_directory( copy_dir ) )
            {
                throw command_error( "Can't find directory: {}", qo( copy_dir ) );
            }
            create_directories( target_dir );
            copy_content( copy_dir, target_dir );
        }
    };

    // Download from ftp using wget
    class ftp_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            std::string url = m_package["url"] || "";
            exec<build_process>( m_destination,
                                 env->wget_path,
                                 "-m -np -nH --cut-dirs={} {}",
                                 count_substr( url, '/' ) - 3,
                                 qo( url ) );
        }
    };

    // List of fetchers (json array or json object)
    class list_fetcher : public fetcher
    {
    protected:
        using fetcher::fetcher;
        friend class fetcher;
        virtual void do_fetch( ) override
        {
            for ( const json& item : m_package.flatten( ) )
            {
                if ( item.is_null( ) )
                    continue;
                ptr<fetcher> f = fetcher::create( m_project, item, m_destination );
                f->fetch( );
            }
        }
    };

    ptr<fetcher> fetcher::create( const project* proj, const json& package, const path& destination )
    {
        if ( package.is_array( ) )
        {
            return ptr<fetcher>( new list_fetcher( proj, package, destination ) );
        }
        else if ( package.is_object( ) )
        {
            const std::string type = package["type"].as_string( ) || "";

            if ( type == "" || type == "list" )
            {
                return ptr<fetcher>( new list_fetcher( proj, package, destination ) );
            }
            else if ( type == "git" )
            {
                return ptr<fetcher>( new git_fetcher( proj, package, destination ) );
            }
            else if ( type == "hg" )
            {
                return ptr<fetcher>( new hg_fetcher( proj, package, destination ) );
            }
            else if ( type == "archive" )
            {
                return ptr<fetcher>( new archive_fetcher( proj, package, destination ) );
            }
            else if ( type == "ziparchive" )
            {
                return ptr<fetcher>( new ziparchive_fetcher( proj, package, destination ) );
            }
            else if ( type == "7zarchive" || type == "7ziparchive" )
            {
                return ptr<fetcher>( new sevenziparchive_fetcher( proj, package, destination ) );
            }
            else if ( type == "installer" )
            {
                return ptr<fetcher>( new installer_fetcher( proj, package, destination ) );
            }
            else if ( type == "copy" )
            {
                return ptr<fetcher>( new copy_fetcher( proj, package, destination ) );
            }
            else if ( type == "ftp" )
            {
                return ptr<fetcher>( new ftp_fetcher( proj, package, destination ) );
            }
        }
        throw error( "Unknown fetcher type" );
    }
}
