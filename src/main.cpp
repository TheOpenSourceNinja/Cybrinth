/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * This file exists so that people new to the codebase will see that the MainGame is where all the real code is.
 */
 
#include "MainGame.h"
#ifdef HAVE_IOSTREAM
	#include <iostream>
#else
	//What can we do if someone doesn't have iostream? I don't know.
#endif //HAVE_IOSTREAM

#include <locale>
#include <codecvt>

#include "StringConverter.h"

int main( int argc, char *argv[] ) {
	const std::locale utf8Locale( std::locale(), new std::codecvt_utf8< wchar_t > ); //This program and its libraries use wide character strings extensively, hence the wchar_t
	std::locale::global( utf8Locale ); //Sets the C++ global locale but not the C global locale. In theory, this should make the whole program use UTF-8 for reading and writing files.
	//std::wcout.imbue( utf8Locale ); //In theory, this should make the whole program use UTF-8 for reading and writing to standard input/output.
	//std::wcin.imbue( utf8Locale );
	
	//PACKAGE_NAME, PACKAGE_VERSION, and PACKAGE_BUGREPORT are defined by Autoconf and passed to the compiler by command line arguments - you won't find them in any header. The same goes for HAVE_IOSTREAM above.
	std::wcout << L"Now starting " << PACKAGE_NAME << L" version " << PACKAGE_VERSION << L". Please report bugs to " << PACKAGE_BUGREPORT << L". Enjoy!" << std::endl;
	
	try {
		std::wstring fileToLoad = L"";
		
		bool runAsScreenSaver = false;
		
		std::wcout << L"Number of arguments: " << argc << std::endl;
		
		if( argc > 1 ) {
			StringConverter sc;
			
			for( decltype( argc ) argNum = 1; argNum < argc; ++argNum ) {
				
				auto argument = sc.toStdWString( argv[ argNum ] );
				
				std::wcout << argument << std::endl;
				
				if( argc > argNum + 1 and argument.compare( L"-window-id" ) == 0 ) {
					//The XScreenSaver daemon passes a window ID as both a command-line argument and as an environment variable.
					runAsScreenSaver = true;
					argNum += 1; //We ignore the window ID argument; the environment variable will be used.
				} else if( argument.compare( L"-run-as-screensaver" ) == 0 ) {
					//The reason we ignore the window ID argument above is because the documentation for the GNOME-Screensaver daemon says it uses the environment variable. So we define this other command-line argument which any screensaver daemon should use.
					runAsScreenSaver = true;
				}
			}
		}
		
		MainGame mg( fileToLoad, runAsScreenSaver ); //Lots of stuff gets set up in the MainGame constructor
		return mg.run(); //Now that everything's set up, transfer control to MainGame.run()
	} catch ( std::exception &e ) {
		std::wcerr << L"Error caught by main(): " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_FAILURE;
}
