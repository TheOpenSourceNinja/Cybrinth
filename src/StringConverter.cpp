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
#include "StringConverter.h"
#include <iostream>

/**
This class exists just to convert between Irrlicht's wide character strings and TagLib's wide character strings. I have no idea whether I'm doing it right. I just want to make sure this program can work with non-English characters. If anyone can suggest a way to improve my code, fix any bugs, or whatever, PLEASE send in a patch!
**/


StringConverter::StringConverter() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in StringConverter::StringConverter(): " << e.what() << std::endl;
	}
}

irr::core::stringw StringConverter::convert( TagLib::wstring input ) {
	try {
		TagLib::wstring::iterator it;
		irr::core::stringw result;
		for( it = input.begin(); it < input.end(); it++ ) {
			result.append( *it );
		}
		result.validate(); //Not sure what this does; Irrlicht documentation just says "verify the existing string". Figured it probably wouldn't hurt to call the function here.
		return result;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in StringConverter::convert(): " << e.what() << std::endl;
		irr::core::stringw s;
		return s;
	}
}

TagLib::wstring StringConverter::convert( irr::core::stringw input ) {
	try {
		TagLib::wstring result;
		result.append( input.c_str() );
		return result;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in StringConverter::convert(): " << e.what() << std::endl;
		TagLib::wstring w;
		return w;
	}
}

StringConverter::~StringConverter() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in StringConverter::~StringConverter(): " << e.what() << std::endl;
	}
}
