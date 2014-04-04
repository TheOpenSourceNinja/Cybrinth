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
#include "MenuOption.h"
#include "colors.h"
#include "StringConverter.h"
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM

#include <irrlicht.h>

MenuOption::MenuOption() {
	try {
		x = 0;
		y = 0;
		font = nullptr;
		setText( L"" );
		setDimension();
		highlighted = false;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

MenuOption::~MenuOption() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::~MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setText( irr::core::stringw newText ) {
	try {
		text = newText;
		setDimension();
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setText(): " << e.what() << std::endl;
	}
}

void MenuOption::setFont( irr::gui::IGUIFont* newFont ) {
	try {
		font = newFont;
		setDimension();
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setFont(): " << e.what() << std::endl;
	}
}

void MenuOption::setDimension() {
	try {
		if( font != nullptr ) {
			StringConverter sc;
			dimension = font->getDimension( sc.toStdWString( text ).c_str() ); //sc.toWCharArray( text ) );
		} else {
			dimension = irr::core::dimension2d<uint_fast16_t>( 0, 0 );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::setDimension(): " << e.what() << std::endl;
	}
}

void MenuOption::draw( irr::video::IVideoDriver* driver ) {
	try {
		if( font != nullptr ) {
			irr::core::rect< irr::s32 > background( x, y, x + dimension.Width, y + dimension.Height );
			driver->draw2DRectangle( BLACK, background );
			
			irr::video::SColor textColor;
			if( highlighted ) {
				textColor = LIGHTCYAN;
				driver->draw2DRectangleOutline( background, textColor );
			} else {
				textColor = CYAN;
			}
			font->draw( text, irr::core::rect< irr::s32 >( x, y, dimension.Width, dimension.Height ), textColor );
		} else {
			throw( L"Font is null" );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::draw(): " << e.what() << std::endl;
	}
	 catch ( std::wstring &e ) {
		std::wcerr << L"Error in MenuOption::draw(): " << e << std::endl;
	}
}

void MenuOption::setX( uint_fast16_t val ) {
	try {
		x = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setY( uint_fast16_t val ) {
	try {
		y = val;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

uint_fast16_t MenuOption::getX() {
	try {
		return x;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::getX(): " << e.what() << std::endl;
		return UINT_FAST16_MAX;
	}
}

uint_fast16_t MenuOption::getY() {
	try {
		return y;
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::getY(): " << e.what() << std::endl;
		return UINT_FAST16_MAX;
	}
}

bool MenuOption::contains( irr::core::position2d< int_fast32_t > test ) {
	try {
		return ( ( test.X >= x && test.Y >= y ) && ( test.X <= x + dimension.Width && test.Y <= y + dimension.Height ) );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}

bool MenuOption::contains( int_fast32_t testX, int_fast32_t testY ) {
	try {
		return contains( irr::core::position2d< int_fast32_t >( testX, testY ) );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}
