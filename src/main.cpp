/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/
#include "GameManager.h"
#include <iostream>

int main() {
	std::wcout << L"Now starting " << PACKAGE_STRING << L". Please report bugs to " << PACKAGE_BUGREPORT << L". Enjoy!" << std::endl;
	try {
		GameManager* gm = new GameManager();
		int exitValue = gm->run();
		delete gm;
		exit( exitValue );
	} catch ( std::exception e ) {
		std::wcerr << L"Error caught by main(): " << e.what() << std::endl;
		exit( EXIT_FAILURE );
	}
	return EXIT_FAILURE;
}
