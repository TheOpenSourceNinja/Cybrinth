/**
 * Copyright © 2012-2014 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#include "SystemSpecificsManager.h"

std::wstring SystemSpecificsManager::getEnvironmentVariable( std::string name ) {
	char * result = getenv( name.c_str() );
	if( result != 0 && result != NULL && result != nullptr ) {
		return sc.toStdWString( result );
	} else {
		std::wstring error = L"Environment variable ";
		error.append( sc.toStdWString( name ) );
		error.append( L" not found." );
		throw( error );
	}
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getFontFolders() {
	std::vector< boost::filesystem::path > fontFolders;
	fontFolders.push_back( boost::filesystem::current_path() );
	#if defined WINDOWS
		std::wcout << L"Running Windows" << std::endl;
		fontFolders.push_back( L"C:\Windows\Fonts");
		fontFolders.push_back( L"C:\WINNT\Fonts");
	#elif defined LINUX
		std::wcout << L"Running Linux" << std::endl;
		fontFolders.push_back( L"/usr/share/X11/fonts/" );
		fontFolders.push_back( L"/usr/share/fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/.fonts/");
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined MACOSX
		std::wcout << L"Running Mac" << std::endl;
		fontFolders.push_back( L"/Library/Fonts/" );
		fontFolders.push_back( L"/Network/Library/Fonts/" );
		fontFolders.push_back( L"/System/Library/Fonts/" );
		fontFolders.push_back( L"/System Folder/Fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/Library/Fonts/");
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		std::wcout << L"Running something else" << std::endl;
		fontFolders.push_back( L"/usr/X11R6/lib/X11/fonts/" ); //OpenBSD documentation suggests fonts may be stored in these two locations
		fontFolders.push_back( L"/usr/local/lib/X11/fonts/" );
	#endif //What about other operating systems? I don't know where BSD etc. put their font files.
	
	//for( std::vector< boost::filesystem::path >::iterator i = fontFolders.begin(); i != fontFolders.end(); i++ ) {
	for( std::vector< boost::filesystem::path >::size_type i = 0; i < fontFolders.size(); i++ ) {
		//if( !exists( &i ) ) {
		if( !exists( fontFolders.at( i ) ) ) {
			//fontFolders.erase( i );
			fontFolders.erase( fontFolders.begin() + i );
		}
	}
	
	return fontFolders;
}

SystemSpecificsManager::SystemSpecificsManager()
{
	//ctor
}

SystemSpecificsManager::~SystemSpecificsManager()
{
	//dtor
}
