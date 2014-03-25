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
#include "StringConverter.h"
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM

/**
This class exists just to convert between Irrlicht's wide character strings and TagLib's wide character strings. I have no idea whether I'm doing it right. I just want to make sure this program can work with non-English characters. If anyone can suggest a way to improve my code, fix any bugs, or whatever, PLEASE send in a patch!
**/


StringConverter::StringConverter() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::StringConverter(): " << e.what() << std::endl;
	}
}

/*irr::core::stringw StringConverter::toIrrlichtStringW( TagLib::wstring input ) {
	try {
		TagLib::wstring::iterator it;
		irr::core::stringw result;
		for( it = input.begin(); it < input.end(); ++it ) {
			result.append( *it );
		}
		result.validate(); //Not sure what this does; Irrlicht documentation just says "verify the existing string". Figured it probably wouldn't hurt to call the function here.
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toIrrlichtStringW(): " << e.what() << std::endl;
		irr::core::stringw s;
		return s;
	}
}*/

irr::core::stringw StringConverter::toIrrlichtStringW( std::wstring input ) {
	try {
		std::wstring::iterator it;
		irr::core::stringw result;
		for( it = input.begin(); it < input.end(); ++it ) {
			result.append( *it );
		}
		result.validate(); //Not sure what this does; Irrlicht documentation just says "verify the existing string". Figured it probably wouldn't hurt to call the function here.
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toIrrlichtStringW(): " << e.what() << std::endl;
		irr::core::stringw s;
		return s;
	}
}

irr::core::stringw StringConverter::toIrrlichtStringW( long int input ) {
	return toIrrlichtStringW( toStdWString( input ) );
}

TagLib::wstring StringConverter::toTaglibWString ( irr::core::stringw input ) {
	try {
		TagLib::wstring result;
		result.append( input.c_str() );
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toTaglibWString(): " << e.what() << std::endl;
		TagLib::wstring w;
		return w;
	}
}

TagLib::wstring StringConverter::toTaglibWString( std::wstring input ) {
	try {
		TagLib::wstring result;
		result.append( input );
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toTaglibWString(): " << e.what() << std::endl;
		TagLib::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString ( irr::core::stringw input ) {
	try {
		std::wstring result;
		result.append( input.c_str() );
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toTaglibWString(): " << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( TagLib::wstring input ) {
	try {
		std::wstring result;
		result.append( input );
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString(): " << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( std::string input ) {
	try {
		std::wstring result;
		result.assign( input.begin(), input.end() );
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString(): " << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( char* input ) {
	try {
		return toStdWString( std::string( input ) );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( wchar_t* input ) {
	try {
		return std::wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( const char* input ) {
	try {
		return toStdWString( std::string( input ) );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( const wchar_t* input ) {
	try {
		return std::wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( int input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( long input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( long long input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( unsigned input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( unsigned long input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( unsigned long long input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( float input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( double input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

std::wstring StringConverter::toStdWString( long double input ) {
	try {
		return std::to_wstring( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString" << e.what() << std::endl;
		std::wstring w;
		return w;
	}
}

const wchar_t* StringConverter::toWCharArray( std::wstring input ) {
	return input.c_str();
}

const wchar_t* StringConverter::toWCharArray( irr::core::stringw input ) {
	return input.c_str();
}

const wchar_t* StringConverter::toWCharArray( char* input ) {
	return toStdWString( input ).c_str();
}

const wchar_t* StringConverter::toWCharArray( const char* input ) {
	return toStdWString( input ).c_str();
}

StringConverter::~StringConverter() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::~StringConverter(): " << e.what() << std::endl;
	}
}
