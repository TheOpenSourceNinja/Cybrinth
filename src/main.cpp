/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * This file exists so that people new to the codebase will see that the GameManager is where all the real code is.
 */
 
#include "GameManager.h"
#ifdef HAVE_IOSTREAM
	#include <iostream>
#else
	//What can we do if someone doesn't have iostream? I don't know.
#endif //HAVE_IOSTREAM

int main() {
	//PACKAGE_NAME, PACKAGE_VERSION, and PACKAGE_BUGREPORT are defined by Autoconf and passed to the compiler by command line arguments - you won't find them in any header. The same goes for HAVE_IOSTREAM above.
	std::wcout << L"Now starting " << PACKAGE_NAME << L" version " << PACKAGE_VERSION << L". Please report bugs to " << PACKAGE_BUGREPORT << L". Enjoy!" << std::endl;
	try {
		GameManager gm; //Lots of stuff gets set up in the GameManager constructor
		return gm.run(); //Now that everything's set up, transfer control to GameManager.run()
	} catch ( std::exception &e ) {
		std::wcerr << L"Error caught by main(): " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_FAILURE;
}
