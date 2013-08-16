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
#include "MenuOption.h"
#include "colors.h"
#include <iostream>

#include <irrlicht.h>

MenuOption::MenuOption() {
	try {
		x = 0;
		y = 0;
		setText( "" );
		font = NULL;
		setDimension();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

MenuOption::~MenuOption() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::~MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setText( irr::core::stringw newText ) {
	try {
		text = newText;
		setDimension();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::setText(): " << e.what() << std::endl;
	}
}

void MenuOption::setFont( irr::gui::IGUIFont* newFont ) {
	try {
		font = newFont;
		setDimension();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::setFont(): " << e.what() << std::endl;
	}
}

void MenuOption::setDimension() {
	try {
		if( font != NULL ) {
			dimension = font->getDimension( text.c_str() );
		} else {
			dimension = irr::core::dimension2d<uint_least16_t>( 0, 0 );
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::setDimension(): " << e.what() << std::endl;
	}
}

void MenuOption::draw( irr::video::IVideoDriver* driver ) {
	try {
		//driver->draw2DImage( texture, core::position2d<int_least16_t>( x, y ) );
		if( font != NULL ) {
			font->draw( text, irr::core::rect< irr::s32 >( x, y, dimension.Width, dimension.Height ), LIGHTCYAN );
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::draw(): " << e.what() << std::endl;
	}
}

void MenuOption::setX( uint_least16_t val ) {
	try {
		x = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

void MenuOption::setY( uint_least16_t val ) {
	try {
		y = val;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::MenuOption(): " << e.what() << std::endl;
	}
}

uint_least16_t MenuOption::getX() {
	try {
		return x;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::getX(): " << e.what() << std::endl;
		return UINT_LEAST16_MAX;
	}
}

uint_least16_t MenuOption::getY() {
	try {
		return y;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::getY(): " << e.what() << std::endl;
		return UINT_LEAST16_MAX;
	}
}

bool MenuOption::contains( irr::core::position2d<uint_least16_t> test ) {
	try {
		return ( test.X >= x && test.Y >= y && test.X <= x + dimension.Width && test.Y <= y + dimension.Height );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}

bool MenuOption::contains( uint_least16_t testX, uint_least16_t testY ) {
	try {
		return contains( irr::core::position2d<uint_least16_t>( testX, testY ) );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in MenuOption::contains(): " << e.what() << std::endl;
		return false;
	}
}
