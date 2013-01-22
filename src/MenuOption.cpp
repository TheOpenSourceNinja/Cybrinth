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

using namespace irr;

MenuOption::MenuOption() {
	x = 0;
	y = 0;
	dimension = core::dimension2d<u32>( 0, 0 );
	text = "";
	font = NULL;
}

MenuOption::~MenuOption() {
	//dtor
}

void MenuOption::setText( core::stringw newText ) {
	text = newText;
	setDimension();
}

void MenuOption::setFont( gui::IGUIFont* newFont ) {
	font = newFont;
	setDimension();
}

void MenuOption::setDimension() {
	if( font != NULL ) {
		dimension = font->getDimension( text.c_str() );
	} else {
		dimension = core::dimension2d<u32>( 0, 0 );
	}
}

void MenuOption::draw( video::IVideoDriver* driver ) {
	//driver->draw2DImage( image, core::position2d<s32>( x, y ) );
	if( font != NULL ) {
		font->draw( text, core::rect<s32>( x, y, dimension.Width, dimension.Height ), LIGHTCYAN );
	}
}

void MenuOption::setX( u32 val ) {
	x = val;
}

void MenuOption::setY( u32 val ) {
	y = val;
}

u32 MenuOption::getX() {
	return x;
}

u32 MenuOption::getY() {
	return y;
}

bool MenuOption::isWithin( core::position2d<u32> test ) {
	return ( test.X >= x && test.Y >= y && test.X <= x + dimension.Width && test.Y <= y + dimension.Height );
	//return true;
}

bool MenuOption::isWithin( u32 testX, u32 testY ) {
	return isWithin( core::position2d<u32>( testX, testY ) );
}
