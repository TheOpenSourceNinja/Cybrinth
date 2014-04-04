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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif //HAVE_STDLIB_H. I don't know what we'll do if we don't have this header.

#include <wchar.h>

#include "SystemSpecificsManager.h"

std::wstring SystemSpecificsManager::getEnvironmentVariable( std::string name ) {
	char * result = getenv( name.c_str() );
	//cppcheck-suppress duplicateExpression
	if( result != 0 && result != NULL && result != nullptr ) {
		return sc.toStdWString( result );
	} else {
		std::wstring error = L"Environment variable ";
		error.append( sc.toStdWString( name ) );
		error.append( L" not found." );
		throw( error );
	}
}

std::wstring SystemSpecificsManager::getEnvironmentVariable( std::wstring name ) {
	wchar_t * result = nullptr;
	#ifdef HAVE__WGETENV
		result = _wgetenv( name.c_str() );
	#endif // HAVE__WGETENV
	//cppcheck-suppress duplicateExpression
	if( result != 0 && result != NULL && result != nullptr ) {
		return sc.toStdWString( result );
	} else {
		return getEnvironmentVariable( sc.toStdString( name ) );
	}
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getFontFolders() {
	std::vector< boost::filesystem::path > fontFolders;
	fontFolders.push_back( boost::filesystem::current_path() );
	#if defined WINDOWS
		fontFolders.push_back( L"C:\Windows\Fonts");
		fontFolders.push_back( L"C:\WINNT\Fonts");
	#elif defined LINUX
		fontFolders.push_back( L"/usr/share/X11/fonts/" );
		fontFolders.push_back( L"/usr/share/fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/.fonts/");
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined MACOSX
		fontFolders.push_back( L"/Library/Fonts/" );
		fontFolders.push_back( L"/Network/Library/Fonts/" );
		fontFolders.push_back( L"/System/Library/Fonts/" );
		fontFolders.push_back( L"/System Folder/Fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Library/Fonts/");
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		fontFolders.push_back( L"/usr/X11R6/lib/X11/fonts/" ); //OpenBSD documentation suggests fonts may be stored in these two locations
		fontFolders.push_back( L"/usr/local/lib/X11/fonts/" );
	#endif //What about other operating systems? I don't know where BSD etc. put their font files.
	
	//for( std::vector< boost::filesystem::path >::iterator i = fontFolders.begin(); i != fontFolders.end(); i++ ) {
	for( decltype( fontFolders.size() ) i = 0; i < fontFolders.size(); i++ ) {
		//if( !exists( &i ) ) {
		if( !exists( fontFolders.at( i ) ) || ( !is_directory( fontFolders.at( i ) ) && ( is_symlink( fontFolders.at( i ) ) && !is_directory( read_symlink( fontFolders.at( i ) ) ) ) ) ) {
			//fontFolders.erase( i );
			fontFolders.erase( fontFolders.begin() + i );
		}
	}
	
	return fontFolders;
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getConfigFolders() {
	std::vector< boost::filesystem::path > configFolders;
	configFolders.push_back( boost::filesystem::current_path() );
	std::wstring packageName = sc.toStdWString( PACKAGE_NAME );
	packageName += L"/";
	#if defined WINDOWS
		try {
			configFolders.push_back( getEnvironmentVariable( "%APPDATA%" ) + L"/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		try {
			configFolders.push_back( getEnvironmentVariable( "%LOCALAPPDATA%" ) + L"/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		try {
			configFolders.push_back( getEnvironmentVariable( "%PROGRAMDATA%" ) + L"/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined LINUX
		configFolders.push_back( L"/etc/" + sc.toStdWString( PACKAGE_NAME ) );
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/." + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		try {
			configFolders.push_back( getEnvironmentVariable( "XDG_CONFIG_HOME" ) + L"/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined MACOSX
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/Library/Application Support/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		//This section is just a copy of the Linux section above.
		configFolders.push_back( L"/etc/" + packageName );
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/." + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		try {
			configFolders.push_back( getEnvironmentVariable( "XDG_CONFIG_HOME" ) + L"/" + packageName );
		} catch( std::wstring error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#endif //What about other operating systems? I don't know where BSD etc. put their config files.
	
	for( decltype( configFolders.size() ) i = 0; i < configFolders.size(); i++ ) {
		if( !exists( configFolders.at( i ) ) ) {
			configFolders.erase( configFolders.begin() + i );
		}
	}
	
	return configFolders;
}

SystemSpecificsManager::SystemSpecificsManager()
{
	//ctor
}

SystemSpecificsManager::~SystemSpecificsManager()
{
	//dtor
}
