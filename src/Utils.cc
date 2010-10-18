/* Copyright (C) 2004 Bart 'plors' Hakvoort
 * Copyright (C) 2008, 2009, 2010 Curtis Gedak
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

#include "../include/Utils.h"

#include <sstream>
#include <iomanip>
#include <regex.h>
#include <locale.h>


namespace GParted
{

Sector Utils::round( double double_value )
{
	 return static_cast<Sector>( double_value + 0.5 ) ;
}

Gtk::Label * Utils::mk_label( const Glib::ustring & text,
			      bool use_markup,
			      Gtk::AlignmentEnum x_align,
			      Gtk::AlignmentEnum y_align,
			      bool wrap,
			      bool selectable,
			      const Glib::ustring & text_color )
{

	Gtk::Label * label = manage( new Gtk::Label( text, x_align, y_align ) ) ;

	label ->set_use_markup( use_markup ) ;
	label ->set_line_wrap( wrap ) ;
	label ->set_selectable( selectable ) ;

	if ( text_color != "black" )
	{
		Gdk::Color color( text_color ) ;
		label ->modify_fg( label ->get_state(), color ) ;
	}

	return label ;
}

Glib::ustring Utils::num_to_str( Sector number )
{
	std::stringstream ss ;
	ss << number ;
	return ss .str() ;
}

//use http://developer.gnome.org/projects/gup/hig/2.0/design.html#Palette as a starting point..
Glib::ustring Utils::get_color( FILESYSTEM filesystem )
{
	switch( filesystem )
	{
		case FS_UNALLOCATED	: return "#A9A9A9" ;	// ~ medium grey
		case FS_UNKNOWN		: return "#000000" ;	//black
		case FS_UNFORMATTED	: return "#000000" ;	//black
		case FS_EXTENDED	: return "#7DFCFE" ;	// ~ light blue
		case FS_BTRFS		: return "#FF9955" ;	//orange
		case FS_EXT2		: return "#9DB8D2" ;	//blue hilight
		case FS_EXT3		: return "#7590AE" ;	//blue medium
		case FS_EXT4		: return "#4B6983" ;	//blue dark
		case FS_LINUX_SWAP	: return "#C1665A" ;	//red medium
		case FS_FAT16		: return "#00FF00" ;	//green
		case FS_FAT32		: return "#18D918" ;	// ~ medium green
		case FS_NTFS		: return "#42E5AC" ;	// ~ light turquoise
		case FS_REISERFS	: return "#ADA7C8" ;	//purple hilight
		case FS_REISER4		: return "#887FA3" ;	//purple medium
		case FS_XFS			: return "#EED680" ;	//accent yellow
		case FS_JFS			: return "#E0C39E" ;	//face skin medium
		case FS_HFS			: return "#E0B6AF" ;	//red hilight
		case FS_HFSPLUS		: return "#C0A39E" ;	// ~ serene red
		case FS_UFS			: return "#D1940C" ;	//accent yellow dark
		case FS_USED		: return "#F8F8BA" ;	// ~ light tan yellow
		case FS_UNUSED		: return "#FFFFFF" ;	//white
		case FS_LVM2		: return "#CC9966" ;	// ~ medium brown
		case FS_LUKS		: return "#625B81" ;	//purple dark

		default				: return "#000000" ;
	}
}

Glib::RefPtr<Gdk::Pixbuf> Utils::get_color_as_pixbuf( FILESYSTEM filesystem, int width, int height )
{
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, false, 8, width, height ) ;

	if ( pixbuf )
	{
		std::stringstream hex( get_color( filesystem ) .substr( 1 ) + "00" ) ;
		unsigned long dec ;
		hex >> std::hex >> dec ;

		pixbuf ->fill( dec ) ;
	}

	return pixbuf ;
}

Glib::ustring Utils::get_filesystem_string( FILESYSTEM filesystem )
{
	switch( filesystem )
	{
		case FS_UNALLOCATED	: return
				/* TO TRANSLATORS:  unallocated
				 * means that this space on the disk device does
				 * not contain a recognized file system, and is in
				 * other words unallocated.
				 */
				_("unallocated") ;
		case FS_UNKNOWN		: return
				/* TO TRANSLATORS:  unknown
				 * means that this space within this partition does
				 * not contain a file system known to GParted, and
				 * is in other words unknown.
				 */
				_("unknown") ;
		case FS_UNFORMATTED	: return
				/* TO TRANSLATORS:  unformatted
				 * means that the space within this partition will not
				 * be formatted with a known file system by GParted.
				 */
				_("unformatted") ;
		case FS_EXTENDED	: return "extended" ;
		case FS_BTRFS		: return "btrfs" ;
		case FS_EXT2		: return "ext2" ;
		case FS_EXT3		: return "ext3" ;
		case FS_EXT4		: return "ext4" ;
		case FS_LINUX_SWAP	: return "linux-swap" ;
		case FS_FAT16		: return "fat16" ;
		case FS_FAT32		: return "fat32" ;
		case FS_NTFS		: return "ntfs" ;
		case FS_REISERFS	: return "reiserfs" ;
		case FS_REISER4		: return "reiser4" ;
		case FS_XFS		: return "xfs" ;
		case FS_JFS		: return "jfs" ;
		case FS_HFS		: return "hfs" ;
		case FS_HFSPLUS		: return "hfs+" ;
		case FS_UFS		: return "ufs" ;
		case FS_USED		: return _("used") ;
		case FS_UNUSED		: return _("unused") ;
		case FS_LVM2		: return "lvm2" ;
		case FS_LUKS		: return "crypt-luks" ;

		default			: return "" ;
	}
}

Glib::ustring Utils::get_filesystem_software( FILESYSTEM filesystem )
{
	switch( filesystem )
	{
		case FS_BTRFS       : return "btrfs-tools" ;
		case FS_EXT2        : return "e2fsprogs" ;
		case FS_EXT3        : return "e2fsprogs" ;
		case FS_EXT4        : return "e2fsprogs v1.41+" ;
		case FS_FAT16       : return "dosfstools, mtools" ;
		case FS_FAT32       : return "dosfstools, mtools" ;
		case FS_HFS         : return "hfsutils" ;
		case FS_HFSPLUS     : return "hfsprogs" ;
		case FS_JFS         : return "jfsutils" ;
		case FS_LINUX_SWAP  : return "util-linux" ;
		case FS_NTFS        : return "ntfsprogs" ;
		case FS_REISER4     : return "reiser4progs" ;
		case FS_REISERFS    : return "reiserfsprogs" ;
		case FS_UFS         : return "" ;
		case FS_XFS         : return "xfsprogs" ;

		default             : return "" ;
	}
}

Glib::ustring Utils::format_size( Sector sectors, Byte_Value sector_size )
{
	std::stringstream ss ;
	ss << std::setiosflags( std::ios::fixed ) << std::setprecision( 2 ) ;

	if ( (sectors * sector_size) < KIBIBYTE )
	{
		ss << sector_to_unit( sectors, sector_size, UNIT_BYTE ) ;
		return String::ucompose( _("%1 B"), ss .str() ) ;
	}
	else if ( (sectors * sector_size) < MEBIBYTE )
	{
		ss << sector_to_unit( sectors, sector_size, UNIT_KIB ) ;
		return String::ucompose( _("%1 KiB"), ss .str() ) ;
	}
	else if ( (sectors * sector_size) < GIBIBYTE )
	{
		ss << sector_to_unit( sectors, sector_size, UNIT_MIB ) ;
		return String::ucompose( _("%1 MiB"), ss .str() ) ;
	}
	else if ( (sectors * sector_size) < TEBIBYTE )
	{
		ss << sector_to_unit( sectors, sector_size, UNIT_GIB ) ;
		return String::ucompose( _("%1 GiB"), ss .str() ) ;
	}
	else
	{
		ss << sector_to_unit( sectors, sector_size, UNIT_TIB ) ;
		return String::ucompose( _("%1 TiB"), ss .str() ) ;
	}
}

Glib::ustring Utils::format_time( std::time_t seconds )
{
	Glib::ustring time ;

	int unit = static_cast<int>( seconds / 3600 ) ;
	if ( unit < 10 )
		time += "0" ;
	time += num_to_str( unit ) + ":" ;
	seconds %= 3600 ;

	unit = static_cast<int>( seconds / 60 ) ;
	if ( unit < 10 )
		time += "0" ;
	time += num_to_str( unit ) + ":" ;
	seconds %= 60 ;

	if ( seconds < 10 )
		time += "0" ;
	time += num_to_str( seconds ) ;

	return time ;
}

double Utils::sector_to_unit( Sector sectors, Byte_Value sector_size, SIZE_UNIT size_unit )
{
	switch ( size_unit )
	{
		case UNIT_BYTE	:
			return sectors * sector_size ;

		case UNIT_KIB	:
			return sectors / ( static_cast<double>( KIBIBYTE ) / sector_size );
		case UNIT_MIB	:
			return sectors / ( static_cast<double>( MEBIBYTE ) / sector_size );
		case UNIT_GIB	:
			return sectors / ( static_cast<double>( GIBIBYTE ) / sector_size );
		case UNIT_TIB	:
			return sectors / ( static_cast<double>( TEBIBYTE ) / sector_size );

		default:
			return sectors ;
	}
}

int Utils::execute_command( const Glib::ustring & command )
{
	Glib::ustring dummy ;
	return execute_command( command, dummy, dummy ) ;
}

int Utils::execute_command( const Glib::ustring & command,
		     	    Glib::ustring & output,
			    Glib::ustring & error,
		     	    bool use_C_locale )
{
	int exit_status = -1 ;
	std::string std_out, std_error ;

	try
	{
		if ( use_C_locale )
		{
			std::vector<std::string> envp, argv;
			envp .push_back( "LC_ALL=C" ) ;
			envp .push_back( "PATH=" + Glib::getenv( "PATH" ) ) ;

			argv .push_back( "sh" ) ;
			argv .push_back( "-c" ) ;
			argv .push_back( command ) ;

			Glib::spawn_sync( ".",
					  argv,
					  envp,
					  Glib::SPAWN_SEARCH_PATH,
					  sigc::slot<void>(),
					  &std_out,
					  &std_error,
					  &exit_status ) ;
		}
		else
			Glib::spawn_command_line_sync( "sh -c '" + command + "'", &std_out, &std_error, &exit_status ) ;
	}
	catch ( Glib::Exception & e )
	{
		 error = e .what() ;

		 return -1 ;
	}

	output = Utils::cleanup_cursor( std_out ) ;
	error = std_error ;

	return exit_status ;
}

Glib::ustring Utils::regexp_label( const Glib::ustring & text,
				const Glib::ustring & regular_sub_expression )
{
	//Extract text from a regular sub-expression.  E.g., "text we don't want (text we want)"
	Glib::ustring label = "";
	regex_t    preg ;
	int        nmatch = 2 ;
	regmatch_t pmatch[  2 ] ;
	int rc = regcomp( &preg, regular_sub_expression .c_str(), REG_EXTENDED | REG_ICASE | REG_NEWLINE ) ;
    if(rc == 0)
	{
		if (regexec(&preg, text.c_str(), nmatch, pmatch, 0) == 0)
			label = text .substr( pmatch[1].rm_so, pmatch[1].rm_eo - pmatch[1].rm_so ) ;

		regfree(&preg);
	}
	return label ;
}

Glib::ustring Utils::fat_compliant_label( const Glib::ustring & label )
{
	//Limit volume label to 11 characters
	Glib::ustring text = label ;
	if( text .length() > 11 )
		text .resize( 11 ) ;
	return text ;
}

Glib::ustring Utils::create_mtoolsrc_file( char file_name[], const char drive_letter,
		const Glib::ustring & device_path )
{
	//Create mtools config file
	//NOTE:  file_name will be changed by the mkstemp() function call.
	Glib::ustring err_msg = "" ;
	int fd ;
	fd = mkstemp( file_name ) ;
	if( fd != -1 ) {
		Glib::ustring fcontents =
			/* TO TRANSLATORS:  # Temporary file created by gparted.  It may be deleted.
			 * means that this file is only used while gparted is applying operations.
			 * If for some reason this file exists at any other time, then the message is
			 * meant to inform a user that the file can be deleted with no harmful effects.
			 * This file is typically created, exists for less than a few seconds, and is
			 * then deleted by gparted.  Under normal circumstances a user should never
			 * see this file.
			 */
			_("# Temporary file created by gparted.  It may be deleted.\n") ;

		//The following file contents are mtools keywords (see man mtools.conf)
		fcontents = String::ucompose(
						"drive %1: file=\"%2\"\nmtools_skip_check=1\n", drive_letter, device_path
					);

		if( write( fd, fcontents .c_str(), fcontents .size() ) == -1 ) {
			err_msg = String::ucompose(
							/* TO TRANSLATORS: looks like
							 * Label operation failed:  Unable to write to temporary file /tmp/Y56ZZ3M13LM.
							 */
							_("Label operation failed:  Unable to write to temporary file %1.\n")
							, file_name
						) ;
		}
		close( fd ) ;
	}
	else
	{
		err_msg = String::ucompose(
						/* TO TRANSLATORS: looks like
						 * Label operation failed:  Unable to create temporary file /tmp/Y56ZZ3M13LM.
						 */
						_("Label operation failed:  Unable to create temporary file %1.\n")
						, file_name
					) ;
	}
	return err_msg ;
}

Glib::ustring Utils::delete_mtoolsrc_file( const char file_name[] )
{
	//Delete mtools config file
	Glib::ustring err_msg = "" ;
	remove( file_name ) ;
	return err_msg ;
}

Glib::ustring Utils::trim( const Glib::ustring & src, const Glib::ustring & c /* = " \t\r\n" */ )
{
	//Trim leading and trailing whitespace from string
	Glib::ustring::size_type p2 = src.find_last_not_of(c);
	if (p2 == Glib::ustring::npos) return Glib::ustring();
	Glib::ustring::size_type p1 = src.find_first_not_of(c);
	if (p1 == Glib::ustring::npos) p1 = 0;
	return src.substr(p1, (p2-p1)+1);
}

Glib::ustring Utils::cleanup_cursor( const Glib::ustring & text )
{
	//Clean up text for commands that use cursoring to display progress.
	Glib::ustring str = text;
	//remove backspace '\b' and delete previous character.  Used in mke2fs output.
	for ( unsigned int index = str .find( "\b" ) ; index < str .length() ; index = str .find( "\b" ) ) {
		if ( index > 0 )
			str .erase( index - 1, 2 ) ;
		else
			str .erase( index, 1 ) ;
	}

	//Remove carriage return and line up to previous line feed.  Used in ntfsclone output.
	//NOTE:  Normal linux line end is line feed.  DOS uses CR + LF.
	for ( unsigned int index1 = str .find( "\r") ; index1 < str .length() ; index1 = str .find( "\r" ) )
	{
		//Only process if next character is not a LF.
		if ( str .at(index1 + 1) != '\n')
		{
			//find point to start erase from.
			unsigned int index2 = str .rfind( "\n", index1 ) ;
			//find end point to erase up to.
			unsigned int index3 = str .find( "\n", index1 ) ;
			unsigned int index4 = str .rfind( "\r", index3 ) ;
			//perform erase if indices are valid
			if ( ( index2 <= index1 ) && ( index4 > index2 ) && ( index4 < str .length() ) )
				str .erase( index2 + 1, index4 - index2 ) ;
		}
	}
	return str;
}

Glib::ustring Utils::get_lang()
{
	//Extract base language from string that may look like "en_CA.UTF-8"
	//  and return in the form "en-CA"
	Glib::ustring lang = setlocale( LC_CTYPE, NULL ) ;

	//Strip off anything after the period "." or at sign "@"
	lang = Utils::regexp_label( lang .c_str(), "^([^.@]*)") ;

	//Convert the underscore "_" to a hyphen "-"
	Glib::ustring sought = "_" ;
	Glib::ustring replacement = "-" ;
	//NOTE:  Application crashes if string replace is called and sought is not found,
	//       so we need to only perform replace if the sought is found.
	if ( lang .find(sought) != Glib::ustring::npos )
		lang .replace( lang .find(sought), sought .size(), replacement ) ;

	return lang ;
}

//The tokenize method copied and adapted from:
//  http://www.linuxselfhelp.com/HOWTO/C++Programming-HOWTO-7.html
void Utils::tokenize( const Glib::ustring& str,
                      std::vector<Glib::ustring>& tokens,
                      const Glib::ustring& delimiters = " " )
{
	// Skip delimiters at beginning.
	Glib::ustring::size_type lastPos = str.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	Glib::ustring::size_type pos     = str.find_first_of(delimiters, lastPos);

	while (Glib::ustring::npos != pos || Glib::ustring::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of(delimiters, lastPos);
	}
}


} //GParted..
