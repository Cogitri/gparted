/* Copyright (C) 2004 Bart
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Curtis Gedak
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
 
#include "../include/fat16.h"

/*****
//For some reason unknown, this works without these include statements.
#include <stdlib.h>    // 'C' library for mkstemp()
#include <unistd.h>    // 'C' library for write(), close()
#include <stdio.h>     // 'C' library for remove()
*****/

namespace GParted
{

const Glib::ustring fat16::Change_UUID_Warning [] =
	{ _( "Changing the UUID might invalidate the Windows Product Activation (WPA) key"
	   )
	, _( "On FAT and NTFS file systems, the"
	     " Volume Serial Number is used as the UUID."
	     " Changing the Volume Serial Number on the Windows system"
	     " partition, normally C:, might invalidate the WPA key."
	     " An invalid WPA key will prevent login until you reactivate Windows."
	   )
	, _( "Changing the UUID on external storage media and non-system partitions"
	     " is usually safe, but guarantees cannot be given."
	   )
	,    ""
	} ;

const Glib::ustring fat16::get_custom_text( CUSTOM_TEXT ttype, int index )
{
	int i ;
	switch ( ttype ) {
		case CTEXT_CHANGE_UUID_WARNING :
			for ( i = 0 ; i < index && Change_UUID_Warning[ i ] != "" ; i++ ) {
				// Just iterate...
			}
			return Change_UUID_Warning [ i ] ;
		default :
			return FileSystem::get_custom_text( ttype, index ) ;
	}
}

FS fat16::get_filesystem_support()
{
	FS fs ;
	fs .filesystem = GParted::FS_FAT16 ;
		
	//find out if we can create fat16 file systems
	if ( ! Glib::find_program_in_path( "mkdosfs" ) .empty() )
		fs .create = GParted::FS::EXTERNAL ;
	
	if ( ! Glib::find_program_in_path( "dosfsck" ) .empty() )
	{
		fs .check = GParted::FS::EXTERNAL ;
		fs .read = GParted::FS::EXTERNAL ;
	}

	if ( ! Glib::find_program_in_path( "mdir" ) .empty() )
		fs .read_uuid = FS::EXTERNAL ;

	if ( ! Glib::find_program_in_path( "mlabel" ) .empty() ) {
		fs .read_label = FS::EXTERNAL ;
		fs .write_label = FS::EXTERNAL ;
		fs .write_uuid = FS::EXTERNAL ;
	}

#ifdef HAVE_LIBPARTED_FS_RESIZE
	//resizing of start and endpoint are provided by libparted
	fs .grow = GParted::FS::LIBPARTED ;
	fs .shrink = GParted::FS::LIBPARTED ;
#endif
#ifdef HAVE_LIBPARTED_3_0_0_PLUS
	fs .move = FS::GPARTED ;
#else
	fs .move = GParted::FS::LIBPARTED ;
#endif

	fs .copy = GParted::FS::GPARTED ;
	
	fs .MIN = 16 * MEBIBYTE ;
	fs .MAX = (4096 - 1) * MEBIBYTE ;  //Maximum seems to be just less than 4096 MiB
	
	return fs ;
}

void fat16::set_used_sectors( Partition & partition ) 
{
	exit_status = Utils::execute_command( "dosfsck -n -v " + partition .get_path(), output, error, true ) ;
	if ( exit_status == 0 || exit_status == 1 || exit_status == 256 )
	{
		//total file system size in logical sectors
		index = output .rfind( "\n", output .find( "sectors total" ) ) +1 ;
		if ( index >= output .length() || sscanf( output .substr( index ) .c_str(), "%Ld", &T ) != 1 )
			T = -1 ;

		//bytes per logical sector
		index = output .rfind( "\n", output .find( "bytes per logical sector" ) ) +1 ;
		if ( index >= output .length() || sscanf( output .substr( index ) .c_str(), "%Ld", &S ) != 1 )
			S = -1 ;

		if ( T > -1 && S > -1 )
			T = Utils::round( T * ( S / double(partition .sector_size) ) ) ;

		//free clusters
		index = output .find( ",", output .find( partition .get_path() ) + partition .get_path() .length() ) +1 ;
		if ( index < output .length() && sscanf( output .substr( index ) .c_str(), "%Ld/%Ld", &S, &N ) == 2 ) 
			N -= S ;
		else
			N = -1 ;

		//bytes per cluster
		index = output .rfind( "\n", output .find( "bytes per cluster" ) ) +1 ;
		if ( index >= output .length() || sscanf( output .substr( index ) .c_str(), "%Ld", &S ) != 1 )
			S = -1 ;
	
		if ( N > -1 && S > -1 )
			N = Utils::round( N * ( S / double(partition .sector_size) ) ) ;

		if ( T > -1 && N > -1 )
			partition .set_sector_usage( T, N ) ;
	}
	else
	{
		if ( ! output .empty() )
			partition .messages .push_back( output ) ;
		
		if ( ! error .empty() )
			partition .messages .push_back( error ) ;
	}
}

void fat16::read_label( Partition & partition )
{
	//Create mtools config file
	char fname[] = "/tmp/gparted-XXXXXXXX" ;
	char dletter = 'H' ;
	Glib::ustring err_msg = "" ;
	err_msg = Utils::create_mtoolsrc_file( fname, dletter, partition.get_path() ) ;
	if( err_msg.length() != 0 )
		partition .messages .push_back( err_msg );

	Glib::ustring cmd = String::ucompose( "export MTOOLSRC=%1 && mlabel -s %2:", fname, dletter ) ;

	if ( ! Utils::execute_command( cmd, output, error, true ) )
	{
		partition .label = Utils::trim( Utils::regexp_label( output, "Volume label is ([^(]*)" ) ) ;
	}
	else
	{
		if ( ! output .empty() )
			partition .messages .push_back( output ) ;
		
		if ( ! error .empty() )
			partition .messages .push_back( error ) ;
	}
	
	//Delete mtools config file
	err_msg = Utils::delete_mtoolsrc_file( fname );
}

bool fat16::write_label( const Partition & partition, OperationDetail & operationdetail )
{
	//Create mtools config file
	char fname[] = "/tmp/gparted-XXXXXXXX" ;
	char dletter = 'H' ;
	Glib::ustring err_msg = "" ;
	err_msg = Utils::create_mtoolsrc_file( fname, dletter, partition.get_path() ) ;

	Glib::ustring cmd = "" ;
	if( partition .label .empty() )
		cmd = String::ucompose( "export MTOOLSRC=%1 && mlabel -c %2:", fname, dletter ) ;
	else
		cmd = String::ucompose( "export MTOOLSRC=%1 && mlabel %2:\"%3\"", fname, dletter, Utils::fat_compliant_label( partition .label ) ) ;
	
	operationdetail .add_child( OperationDetail( cmd, STATUS_NONE, FONT_BOLD_ITALIC ) ) ;
	
	int exit_status = Utils::execute_command( cmd, output, error ) ;

	if ( ! output .empty() )
		operationdetail .get_last_child() .add_child( OperationDetail( output, STATUS_NONE, FONT_ITALIC ) ) ;

	if ( ! error .empty() )
		operationdetail .get_last_child() .add_child( OperationDetail( error, STATUS_NONE, FONT_ITALIC ) ) ;

	//Delete mtools config file
	err_msg = Utils::delete_mtoolsrc_file( fname );

	return ( exit_status == 0 );
}

void fat16::read_uuid( Partition & partition )
{
	//Create mtools config file
	char fname[] = "/tmp/gparted-XXXXXXXX" ;
	char dletter = 'H' ;
	Glib::ustring err_msg = "" ;
	err_msg = Utils::create_mtoolsrc_file( fname, dletter, partition.get_path() ) ;
	if( err_msg.length() != 0 )
		partition .messages .push_back( err_msg );

	Glib::ustring cmd = String::ucompose( "export MTOOLSRC=%1 && mdir -f %2:", fname, dletter ) ;

	if ( ! Utils::execute_command( cmd, output, error, true ) )
	{
		partition .uuid = Utils::regexp_label( output, "Volume Serial Number is[[:blank:]]([^[:space:]]+)" ) ;
		if ( partition .uuid == "0000-0000" )
			partition .uuid .clear() ;
	}
	else
	{
		if ( ! output .empty() )
			partition .messages .push_back( output ) ;

		if ( ! error .empty() )
			partition .messages .push_back( error ) ;
	}

	err_msg = Utils::delete_mtoolsrc_file( fname );
}

bool fat16::write_uuid( const Partition & partition, OperationDetail & operationdetail )
{
	//Create mtools config file
	char fname[] = "/tmp/gparted-XXXXXXXX" ;
	char dletter = 'H' ;
	Glib::ustring err_msg = "" ;
	err_msg = Utils::create_mtoolsrc_file( fname, dletter, partition.get_path() ) ;

	// Wait some time - 'random' UUIDs turn out identical if generated in quick succession...
	sleep(1);
	Glib::ustring cmd = String::ucompose( "export MTOOLSRC=%1 && mlabel -s -n %2:", fname, dletter ) ;

	operationdetail .add_child( OperationDetail( cmd, STATUS_NONE, FONT_BOLD_ITALIC ) ) ;

	int exit_status = Utils::execute_command( cmd, output, error ) ;

	if ( ! output .empty() )
		operationdetail .get_last_child() .add_child( OperationDetail( output, STATUS_NONE, FONT_ITALIC ) ) ;

	if ( ! error .empty() )
		operationdetail .get_last_child() .add_child( OperationDetail( error, STATUS_NONE, FONT_ITALIC ) ) ;

	//Delete mtools config file
	err_msg = Utils::delete_mtoolsrc_file( fname );

	return ( exit_status == 0 );
}

bool fat16::create( const Partition & new_partition, OperationDetail & operationdetail )
{
	return ! execute_command( "mkdosfs -F16 -v -n \"" + Utils::fat_compliant_label( new_partition .label ) + "\" " + new_partition .get_path(), operationdetail ) ;
}

bool fat16::resize( const Partition & partition_new, OperationDetail & operationdetail, bool fill_partition )
{
	return true ;
}

bool fat16::move( const Partition & partition_new
                , const Partition & partition_old
                , OperationDetail & operationdetail
                )
{
	return true ;
}

bool fat16::copy( const Glib::ustring & src_part_path,
		  const Glib::ustring & dest_part_path,
		  OperationDetail & operationdetail )
{
	return true ;
}

bool fat16::check_repair( const Partition & partition, OperationDetail & operationdetail )
{
	exit_status = execute_command( "dosfsck -a -w -v " + partition .get_path(), operationdetail ) ;

	return ( exit_status == 0 || exit_status == 1 || exit_status == 256 ) ;
}

bool fat16::remove( const Partition & partition, OperationDetail & operationdetail )
{
	return true ;
}

} //GParted


