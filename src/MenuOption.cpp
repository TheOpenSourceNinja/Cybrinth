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
	x = 0;
	y = 0;
	setText( "" );
	font = NULL;
	setDimension();
}

MenuOption::~MenuOption() {
	//dtor
}

void MenuOption::setText( irr::core::stringw newText ) {
	text = newText;
	setDimension();
}

void MenuOption::setFont( irr::gui::IGUIFont* newFont ) {
	font = newFont;
	setDimension();
}

void MenuOption::setDimension() {
	if( font != NULL ) {
		dimension = font->getDimension( text.c_str() );
	} else {
		dimension = irr::core::dimension2d<uint_least16_t>( 0, 0 );
	}
}

void MenuOption::draw( irr::video::IVideoDriver* driver ) {
	//driver->draw2DImage( texture, core::position2d<int_least16_t>( x, y ) );
	if( font != NULL ) {
		font->draw( text, irr::core::rect< irr::s32 >( x, y, dimension.Width, dimension.Height ), LIGHTCYAN );
	}
}

void MenuOption::setX( uint_least16_t val ) {
	x = val;
}

void MenuOption::setY( uint_least16_t val ) {
	y = val;
}

uint_least16_t MenuOption::getX() {
	return x;
}

uint_least16_t MenuOption::getY() {
	return y;
}

bool MenuOption::contains( irr::core::position2d<uint_least16_t> test ) {
	return ( test.X >= x && test.Y >= y && test.X <= x + dimension.Width && test.Y <= y + dimension.Height );
	//return true;
}

bool MenuOption::contains( uint_least16_t testX, uint_least16_t testY ) {
	return contains( irr::core::position2d<uint_least16_t>( testX, testY ) );
}
