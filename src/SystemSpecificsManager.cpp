/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2017.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The SystemSpecificsManager class is responsible for anything that varies from one operating system to another, such as determining which folders are likely to contain fonts.
 */

#ifdef HAVE_STDLIB_H
	#include <stdlib.h>
#endif //HAVE_STDLIB_H. I don't know what we'll do if we don't have this header.

#include <wchar.h>
#include <iostream>
#include <iterator>
#include <sstream>
#include <boost/algorithm/string/split.hpp>

#include "CustomException.h"
#include "SystemSpecificsManager.h"
#include "PreprocessorCommands.h"

std::wstring SystemSpecificsManager::getEnvironmentVariable( std::string name ) {
	char * result = getenv( name.c_str() );
	//cppcheck-suppress duplicateExpression
	if( result not_eq 0 and result not_eq NULL and result not_eq nullptr ) {
		return sc.toStdWString( result );
	} else {
		std::wstring error = L"Environment variable ";
		error.append( sc.toStdWString( name ) );
		error.append( L" not found." );
		throw( CustomException( error ) );
	}
}

std::wstring SystemSpecificsManager::getEnvironmentVariable( std::wstring name ) {
	wchar_t * result = nullptr;
	#ifdef HAVE__WGETENV
		result = _wgetenv( name.c_str() );
	#endif // HAVE__WGETENV
	//cppcheck-suppress duplicateExpression
	if( result not_eq 0 and result not_eq NULL and result not_eq nullptr ) {
		return sc.toStdWString( result );
	} else {
		return getEnvironmentVariable( sc.toStdString( name ) );
	}
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getDataFolders() {
	std::vector< boost::filesystem::path > dataFolders;
	
	std::wstring common = L"Cybrinth";
	
	#if defined WINDOWS
	{
		dataFolders.push_back( getEnvironmentVariable( "%ProgramFiles%" ) + common );
		dataFolders.push_back( getEnvironmentVariable( "%AppData%" ) + common );
		dataFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + common );
		dataFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + common );
	}
	#elif defined LINUX
		dataFolders.push_back( L"/usr/local/share/Cybrinth/" );
		try {
			dataFolders.push_back( getEnvironmentVariable( L"HOME" ) + common );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		std::wstring dataDirsString = getEnvironmentVariable( L"XDG_DATA_DIRS" );
		std::vector< decltype( dataDirsString ) > dataDirsVector;
		boost::algorithm::split( dataDirsVector, dataDirsString, [](wchar_t c) { return c == L':'; } );
		
		for( decltype( dataDirsVector.size() ) i = 0; i < dataDirsVector.size(); ++i ) {
			dataFolders.push_back( dataDirsVector.at( i ) + common );
		}
		
	#elif defined MACOSX
		dataFolders.push_back( L"/Library/" + common );
		dataFolders.push_back( L"/Network/Library/" + common );
		dataFolders.push_back( L"/System/Library/" + common );
		dataFolders.push_back( L"/System Folder/" + common );
		dataFolders.push_back( L"/Library/" );
		dataFolders.push_back( L"/Network/Library/" );
		dataFolders.push_back( L"/System/Library/" );
		dataFolders.push_back( L"/System Folder/" );
		try {
			dataFolders.push_back( getEnvironmentVariable( L"HOME" ) + common );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		try {
			dataFolders.push_back( getEnvironmentVariable( L"HOME" ) + common );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#endif //What about other operating systems? I don't know where BSD etc. put their picture files.
	
	dataFolders.push_back( boost::filesystem::current_path() );
	
	for( decltype( dataFolders.size() ) i = 0; i < dataFolders.size(); i++ ) {
		if( not canBeUsedAsFolder( dataFolders.at( i ) ) ) {
			dataFolders.erase( dataFolders.begin() + i );
			i -= 1;
		}
	}
	
	return dataFolders;
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getFontFolders() {
	std::vector< boost::filesystem::path > fontFolders;
	
	{
		auto dataFolders = getDataFolders();
		fontFolders.insert( fontFolders.end(), dataFolders.begin(), dataFolders.end() );
	}
	
	#if defined WINDOWS
		try {
			fontFolders.push_back( getEnvironmentVariable( "%SYSTEMROOT%" ) + L"\Fonts" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		try {
			fontFolders.push_back( getEnvironmentVariable( "%WINDIR%" ) + L"\Fonts" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined LINUX
		fontFolders.push_back( L"/usr/share/X11/fonts/" );
		fontFolders.push_back( L"/usr/share/fonts/opentype" ); //This and the next line are a workaround: the first font my system finds in /usr/share/fonts is invisible
		fontFolders.push_back( L"/usr/share/fonts/truetype" );
		fontFolders.push_back( L"/usr/share/fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/.fonts/" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined MACOSX
		fontFolders.push_back( L"/Library/Fonts/" );
		fontFolders.push_back( L"/Network/Library/Fonts/" );
		fontFolders.push_back( L"/System/Library/Fonts/" );
		fontFolders.push_back( L"/System Folder/Fonts/" );
		try {
			fontFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Library/Fonts/");
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		fontFolders.push_back( L"/usr/X11R6/lib/X11/fonts/" ); //FreeBSD documentation says that X11 fonts are located here. OpenBSD documentation suggests fonts may be stored in this and the following location.
		fontFolders.push_back( L"/usr/local/lib/X11/fonts/" );
	#endif //What about other operating systems? I don't know where BSD etc. put their font files.
	
	fontFolders.push_back( boost::filesystem::current_path() );
	
	{
		auto iter = std::unique( fontFolders.begin(), fontFolders.end() );
		fontFolders.resize( std::distance( fontFolders.begin(), iter ) );
	}
	
	for( decltype( fontFolders.size() ) i = 0; i < fontFolders.size(); i++ ) {
		if( not canBeUsedAsFolder( fontFolders.at( i ) ) ) {
			fontFolders.erase( fontFolders.begin() + i );
			i -= 1;
		}
	}
	
	return fontFolders;
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getImageFolders() {
	std::vector< boost::filesystem::path > imageFolders;
	imageFolders.push_back( boost::filesystem::current_path() / boost::filesystem::path( L"Images" ) );
	
	std::wstring common = L"Cybrinth\\Images";
	
	#if defined WINDOWS
	{
		std::wstring pics = L"Pictures";
		std::wstring mypics = L"My Pictures";
		try {
			imageFolders.push_back( getEnvironmentVariable( "%ProgramFiles%" ) + common );
			imageFolders.push_back( getEnvironmentVariable( "%AppData%" ) + common );
			imageFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + common );
			imageFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + pics );
			imageFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + mypics );
			imageFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + common );
			imageFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + pics );
			imageFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + mypics );
		} catch( std::exception &error ) {
			//ignore exception
		}
	}
	#elif defined LINUX
		imageFolders.push_back( L"/usr/local/share/Cybrinth/Images" );
		try {
			imageFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Pictures/" + common );
			imageFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Pictures/" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		std::wstring dataDirsString = getEnvironmentVariable( L"XDG_DATA_DIRS" );
		std::vector< decltype( dataDirsString ) > dataDirsVector;
		boost::algorithm::split( dataDirsVector, dataDirsString, [](wchar_t c) { return c == L':'; } );
		
		for( decltype( dataDirsVector.size() ) i = 0; i < dataDirsVector.size(); ++i ) {
			imageFolders.push_back( dataDirsVector.at( i ) + common );
		}
		
	#elif defined MACOSX
		imageFolders.push_back( L"/Library/Pictures/" + common );
		imageFolders.push_back( L"/Network/Library/Pictures/" + common );
		imageFolders.push_back( L"/System/Library/Pictures/" + common );
		imageFolders.push_back( L"/System Folder/Pictures/" + common );
		try {
			imageFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Pictures/" + common );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		try {
			imageFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Pictures/" + common );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#endif //What about other operating systems? I don't know where BSD etc. put their picture files.
	
	{
		auto dataFolders = getDataFolders();
		imageFolders.insert( imageFolders.end(), dataFolders.begin(), dataFolders.end() );
	}
	
	{
		auto iter = std::unique( imageFolders.begin(), imageFolders.end() );
		imageFolders.resize( std::distance( imageFolders.begin(), iter ) );
	}
	
	for( decltype( imageFolders.size() ) i = 0; i < imageFolders.size(); i++ ) {
		if( not canBeUsedAsFolder( imageFolders.at( i ) ) ) {
			imageFolders.erase( imageFolders.begin() + i );
			i -= 1;
		}
	}
	
	return imageFolders;
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getMusicFolders() {
	std::vector< boost::filesystem::path > musicFolders;
	musicFolders.push_back( boost::filesystem::current_path() );
	
	std::wstring common = L"Cybrinth\\Music";
	
	#if defined WINDOWS
		std::wstring music = L"Music";
		std::wstring myMusic = L"My Music";
		try {
			musicFolders.push_back( getEnvironmentVariable( "%ProgramFiles%" ) + common );
			musicFolders.push_back( getEnvironmentVariable( "%AppData%" ) + common );
			musicFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + common );
			musicFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + music );
			musicFolders.push_back( getEnvironmentVariable( "%UserProfile%" ) + myMusic );
			musicFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + common );
			musicFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + music );
			musicFolders.push_back( getEnvironmentVariable( "%AllUsersProfile%" ) + myMusic );
		} catch( std::exception &error ) {
			//ignore exception
		}
	#elif defined LINUX
		musicFolders.push_back( L"/usr/local/share/Cybrinth/Music" );
		try {
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" + common );
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		std::wstring dataDirsString = getEnvironmentVariable( L"XDG_DATA_DIRS" );
		std::vector< decltype( dataDirsString ) > dataDirsVector;
		boost::algorithm::split( dataDirsVector, dataDirsString, [](wchar_t c) { return c == L':'; } );
		
		for( decltype( dataDirsVector.size() ) i = 0; i < dataDirsVector.size(); ++i ) {
			musicFolders.push_back( dataDirsVector.at( i ) + common );
			musicFolders.push_back( dataDirsVector.at( i ) );
		}
	#elif defined MACOSX
		musicFolders.push_back( L"/Library/Music/" + common );
		musicFolders.push_back( L"/Network/Library/Music/" + common );
		musicFolders.push_back( L"/System/Library/Music/" + common );
		musicFolders.push_back( L"/System Folder/Music/" + common );
		musicFolders.push_back( L"/Library/Music/" );
		musicFolders.push_back( L"/Network/Library/Music/" );
		musicFolders.push_back( L"/System/Library/Music/" );
		musicFolders.push_back( L"/System Folder/Music/" );
		try {
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" + common );
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		try {
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" + common );
			musicFolders.push_back( getEnvironmentVariable( L"HOME" ) + L"/Music/" );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#endif //What about other operating systems? I don't know where BSD etc. put their font files.
	
	{
		auto dataFolders = getDataFolders();
		musicFolders.insert( musicFolders.end(), dataFolders.begin(), dataFolders.end() );
	}
	
	{
		auto iter = std::unique( musicFolders.begin(), musicFolders.end() );
		musicFolders.resize( std::distance( musicFolders.begin(), iter ) );
	}
	
	for( decltype( musicFolders.size() ) i = 0; i < musicFolders.size(); i++ ) {
		if( not canBeUsedAsFolder( musicFolders.at( i ) ) ) {
			musicFolders.erase( musicFolders.begin() + i );
			i -= 1;
		}
	}
	
	return musicFolders;
}

std::vector< boost::filesystem::path > SystemSpecificsManager::getConfigFolders() {
	std::vector< boost::filesystem::path > configFolders;
	std::wstring packageName = sc.toStdWString( PACKAGE_NAME );
	packageName += L"/";
	#if defined WINDOWS
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%APPDATA%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%LOCALAPPDATA%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%HOMEPATH%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%ALLUSERSPROFILE%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%ProgramW6432%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%ProgramFiles%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%ProgramFiles(X86)%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "%PROGRAMDATA%" ) + L"/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#elif defined LINUX
		//NOTE: Any changes made in the Linux section should be reflected in the 'other' section below.
		
		{
			//FreeDesktop.org says "There is a single base directory relative to which user-specific data files should be written.".
			//configFolders.push_back( getEnvironmentVariable( "XDG_CONFIG_HOME" ) );
			std::wstring folderString = L"";
			try {
				folderString = getEnvironmentVariable( "XDG_CONFIG_HOME" );
			} catch( std::exception &error ) {
				try {
					
					//"If $XDG_CONFIG_HOME is either not set or empty, a default equal to $HOME/.config should be used."
					auto homeString = getEnvironmentVariable( "HOME" );
					folderString = homeString + L"/.config";
					
				} catch ( std::exception &errorTwo ) {
					//do nothing
				}
			}
			configFolders.push_back( folderString + L"/" + packageName );
			configFolders.push_back( folderString );
			configFolders.push_back( L"/usr/local/share/" + packageName );
			
		}
		
		try {
			//FreeDesktop.org says "There is a set of preference ordered base directories relative to which configuration files should be searched.".
			auto xdgFoldersString = getEnvironmentVariable( "XDG_CONFIG_DIRS" );
			
			std::vector< decltype( xdgFoldersString ) > xdgFoldersVector;
			boost::algorithm::split( xdgFoldersVector, xdgFoldersString, [](wchar_t c) { return c == L':'; } );
			
			for( decltype( xdgFoldersVector.size() ) i = 0; i < xdgFoldersVector.size(); ++i ) {
				configFolders.push_back( xdgFoldersVector.at( i ) + L"/" + packageName );
			}
			
			for( decltype( xdgFoldersVector.size() ) i = 0; i < xdgFoldersVector.size(); ++i ) {
				configFolders.push_back( xdgFoldersVector.at( i ) );
			}
			
		} catch( std::exception &error ) {
			//"If $XDG_CONFIG_DIRS is either not set or empty, a value equal to /etc/xdg should be used."
			configFolders.push_back( L"/etc/xdg" + packageName );
			configFolders.push_back( L"/etc/xdg" );
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/." + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		configFolders.push_back( L"/etc/" + packageName );
		
	#elif defined MACOSX
		configFolders.push_back( L"/Network/Library/Preferences/" + packageName );
		
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/Library/Preferences/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/Library/Application Support/" + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
	#else
		//What about other operating systems? I don't know where BSD etc. put their config files.
		//This section is just a copy of the Linux section above.
		
		{
			//FreeDesktop.org says "There is a single base directory relative to which user-specific data files should be written.".
			//configFolders.push_back( getEnvironmentVariable( "XDG_CONFIG_HOME" ) );
			auto folderString = L"";
			try {
				folderString = getEnvironmentVariable( "XDG_CONFIG_HOME" );
			} catch( std::exception &error ) {
				try {
					
					//"If $XDG_CONFIG_HOME is either not set or empty, a default equal to $HOME/.config should be used."
					auto homeString = getEnvironmentVariable( "HOME" );
					folderString = homeString + L"/.config";
					
				} catch ( std::exception &errorTwo ) {
					//do nothing
				}
			}
			configFolders.push_back( folderString + L"/" + packageName );
			configFolders.push_back( folderString );
			configFolders.push_back( L"/usr/local/share/" + packageName );
			
		}
		
		try {
			//FreeDesktop.org says "There is a set of preference ordered base directories relative to which configuration files should be searched.".
			auto xdgFoldersString = getEnvironmentVariable( "XDG_CONFIG_DIRS" );
			
			std::vector< decltype( xdgFoldersString ) > xdgFoldersVector;
			boost::algorithm::split( xdgFoldersVector, xdgFoldersString, [](wchar_t c) { return c == L':'; } );
			
			for( decltype( xdgFoldersVector.size() ) i = 0; i < xdgFoldersVector.size(); ++i ) {
				configFolders.push_back( xdgFoldersVector.at( i ) + L"/" + packageName );
			}
			
			for( decltype( xdgFoldersVector.size() ) i = 0; i < xdgFoldersVector.size(); ++i ) {
				configFolders.push_back( xdgFoldersVector.at( i ) );
			}
			
		} catch( std::exception &error ) {
			//"If $XDG_CONFIG_DIRS is either not set or empty, a value equal to /etc/xdg should be used."
			configFolders.push_back( L"/etc/xdg" + packageName );
			configFolders.push_back( L"/etc/xdg" );
		}
		
		try {
			configFolders.push_back( getEnvironmentVariable( "HOME" ) + L"/." + packageName );
		} catch( std::exception &error ) {
			//Environment variable not found, so ignore it. Do nothing.
		}
		
		configFolders.push_back( L"/etc/" + packageName );
		
	#endif
	
	configFolders.push_back( boost::filesystem::current_path() );
	
	{
		auto iter = std::unique( configFolders.begin(), configFolders.end() );
		configFolders.resize( std::distance( configFolders.begin(), iter ) );
	}
	
	for( decltype( configFolders.size() ) i = 0; i < configFolders.size(); i++ ) {
		if( not canBeUsedAsFolder( configFolders.at( i ) ) ) {
			configFolders.erase( configFolders.begin() + i );
			i -= 1;
		}
	}
	
	return configFolders;
}

bool SystemSpecificsManager::canBeUsedAsFolder( boost::filesystem::path folder ) {
	bool goodSoFar = exists( folder );
	
	if( goodSoFar ) {
		if( not is_directory( folder ) ) {
			if( is_symlink( folder ) ) {
				if( not is_directory( read_symlink( folder ) ) ) {
					goodSoFar = false;
				}
			} else {
				goodSoFar = false;
			}
		}
	}
	
	return goodSoFar;
}

SystemSpecificsManager::SystemSpecificsManager() {
	//ctor
}

SystemSpecificsManager::~SystemSpecificsManager() {
	//dtor
}
