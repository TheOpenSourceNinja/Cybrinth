/**
 * Copyright © 2012-2015 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

//This file is just here to gather together a bunch of preprocessor commands that used to be repeated over and over in different files.

#ifndef PREPROCESSOR_COMMANDS_H_INCLUDED
#define PREPROCESSOR_COMMANDS_H_INCLUDED

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions

//! freetype support enabled with 1 and disabled with 0
#define COMPILE_WITH_FREETYPE 1

#if defined __unix || defined __unix__ || defined unix || defined BSD
	#define UNIX
#endif

#if defined linux || defined __linux__ || defined __linux || defined __gnu_linux
	#define LINUX
#endif

#if defined __MACH__ && defined __APPLE__
	#define MACOSX
#endif

#if defined _WIN32 || defined _WIN64
	#define WINDOWS
#endif

#endif // PREPROCESSOR_COMMANDS_H_INCLUDED
