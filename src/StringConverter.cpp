/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * This class exists just to convert between various types of strings. I have no idea whether I'm doing it right. I just want to make sure this program can work with non-English characters. If anyone can suggest a way to improve my code, fix any bugs, or whatever, PLEASE send in a patch!
 */
 
#include "StringConverter.h"
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM
#include <cwchar>

//#include <codecvt>
#include <locale>


StringConverter::StringConverter() {
	try {
		wCharArray = nullptr;
		charArray = nullptr;
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

irr::core::stringw StringConverter::toIrrlichtStringW( std::string input ) {
	return toIrrlichtStringW( toStdWString( input ) );
}

irr::core::stringw StringConverter::toIrrlichtStringW( std::wstring input ) {
	try {
		irr::core::stringw result;
		for( auto it = input.begin(); it < input.end(); ++it ) {
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

std::wstring StringConverter::toStdWString( irr::io::path input ) {
	return toStdWString( input.c_str() );
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

std::string StringConverter::toStdString( std::wstring input ) { //TODO: Make this function work safely.
	try {
		std::string result;
		result.assign( input.begin(), input.end() ); //Definitely not the right way do to this, but the code below (which is correct according to what I've read online) does not compile for me.
		/*std::wstring_convert< std::codecvt_utf8< std::wstring::value_type >, std::wstring::value_type > cv;
		result = cv.to_bytes( input );*/
		
		return result;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdWString(): " << e.what() << std::endl;
		std::string w;
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
		
		auto currentLocale = setlocale( LC_ALL, "" );
		const char* source = input.c_str();
		size_t size = mbstowcs( nullptr, source, 0 ) + 1;
		wchar_t* dest = new wchar_t[ size ];
		wmemset( dest, 0, size );
		mbstowcs( dest, source, size );
		result = dest;
		delete [] dest;
		setlocale( LC_ALL, currentLocale );
		
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

std::wstring StringConverter::toStdWString( unsigned char* input ) {
	try {
		return toStdWString( ( char * ) input );
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

std::wstring StringConverter::toStdWString( float input, const wchar_t* format, size_t maxLength ) {
	std::vector< wchar_t > buffer;
	buffer.resize( maxLength );
	//wchar_t buffer[ maxLength ];
	swprintf( buffer.data(), maxLength, format, input );
	std::wstring result = buffer.data();
	return result;
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
	if( wCharArray not_eq nullptr ) {
		delete wCharArray;
	}
	
	wCharArray = ( decltype( wCharArray ) ) input.c_str();
	
	return wCharArray;
}

const wchar_t* StringConverter::toWCharArray( irr::core::stringw input ) {
	if( wCharArray not_eq nullptr ) {
		delete wCharArray;
	}
	
	wCharArray = ( decltype( wCharArray ) ) input.c_str();
	
	return wCharArray;
}

const wchar_t* StringConverter::toWCharArray( char* input ) {
	if( wCharArray not_eq nullptr ) {
		delete wCharArray;
	}
	
	wCharArray = ( decltype( wCharArray ) ) toStdWString( input ).c_str();
	
	return wCharArray;
}

const wchar_t* StringConverter::toWCharArray( const char* input ) {
	if( wCharArray not_eq nullptr ) {
		delete wCharArray;
	}
	
	wCharArray = ( decltype( wCharArray ) ) toStdWString( input ).c_str();
	
	return wCharArray;
}

std::string StringConverter::toStdString( uint8_t input ) {
	try {
		return std::to_string( input );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::toStdString" << e.what() << std::endl;
		std::string w;
		return w;
	}
}

StringConverter::~StringConverter() {
	try {
		if( wCharArray not_eq nullptr ) {
			delete wCharArray;
		}
		
		if( charArray not_eq nullptr ) {
			delete charArray;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in StringConverter::~StringConverter(): " << e.what() << std::endl;
	}
}
