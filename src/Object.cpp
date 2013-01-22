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
#include "Object.h"
#include <iostream>

using namespace std;

Object::Object() {
	x = 0;
	y = 0;
	xInterp = 0;
	yInterp = 0;
	moving = false;
	distanceFromExit = 0;
	image = NULL;
}

Object::~Object() {
}

void Object::setPos( u8 newX, u8 newY ) {
	setX( newX );
	setY( newY );
}

u8 Object::getY() {
	return y;
}

u8 Object::getX() {
	return x;
}

void Object::moveY( s8 val ) {
	if( !moving ) {
		y += val;
		moving = true;
	} else {
		yInterp = y;
		xInterp = x;
		y += val;
		moving = true;
	}
}

void Object::moveX( s8 val ) {
	if( !moving ) {
		x += val;
		moving = true;
	} else {
		xInterp = x;
		yInterp = y;
		x += val;
		moving = true;
	}
}

void Object::setX( u8 val ) {
	x = val;
	xInterp = x;
}

void Object::setY( u8 val ) {
	y = val;
	yInterp = y;
}

void Object::draw( video::IVideoDriver* driver, u32 width, u32 height ) {
	if( moving ) {
		f64 speed = .2;

		if( x > xInterp ) {
			xInterp += speed;
		} else if( x < xInterp ) {
			xInterp -= speed;
		}

		if( y > yInterp ) {
			yInterp += speed;
		} else if( y < yInterp ) {
			yInterp -= speed;
		}

		if(( x >= ( xInterp - speed ) ) && ( x <= ( xInterp + speed ) ) && ( y >= ( yInterp - speed ) ) && ( y <= ( yInterp + speed ) ) ) {
			moving = false;
			xInterp = x;
			yInterp = y;
		}
	} else {
		xInterp = x;
		yInterp = y;
	}

	u32 size;

	if( width < height ) {
		size = width;
	} else {
		size = height;
	}

	if( image != NULL ) {
		s32 cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		s32 cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );
		//driver->draw2DImage(image, core::position2d<s32>(cornerX, cornerY));
		video::SColor colorArray[] = {colorOne, colorOne, colorOne, colorOne};
		driver->draw2DImage( image,
							 core::rect<s32>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 core::rect<s32>( core::position2d<s32>( 0, 0 ), image->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the image if we want. Zero means draw the whole thing.
							 colorArray,
							 true );
	}
}

/*void Object::loadImage(video::IVideoDriver* driver) {
	image = driver->addTexture(core::dimension2d<u32>(1,1), "empty");
}*/

void Object::setColor( video::SColor newColor ) {
	if( newColor == BLACK ) {
		colorTwo = BLACK;
		colorOne = GRAY;
	} else if( newColor == GRAY ) {
		colorTwo = GRAY;
		colorOne = BLACK;
	} else if( newColor == BLUE) {
		colorTwo = BLUE;
		colorOne = LIGHTBLUE;
	} else if( newColor == LIGHTBLUE ) {
		colorTwo = LIGHTBLUE;
		colorOne = BLUE;
	} else if( newColor == GREEN ) {
		colorTwo = GREEN;
		colorOne = LIGHTGREEN;
	} else if( newColor == LIGHTGREEN ) {
		colorTwo = LIGHTGREEN;
		colorOne = GREEN;
	} else if( newColor == CYAN ) {
		colorTwo = CYAN;
		colorOne = LIGHTCYAN;
	} else if( newColor == LIGHTCYAN ) {
		colorTwo = LIGHTCYAN;
		colorOne = CYAN;
	} else if( newColor == RED ) {
		colorTwo = RED;
		colorOne = LIGHTRED;
	} else if( newColor == LIGHTRED ) {
		colorTwo = LIGHTRED;
		colorOne = RED;
	} else if( newColor == MAGENTA ) {
		colorTwo = MAGENTA;
		colorOne = LIGHTMAGENTA;
	} else if( newColor == LIGHTMAGENTA ) {
		colorTwo = LIGHTMAGENTA;
		colorOne = MAGENTA;
	} else if( newColor == BROWN ) {
		colorTwo = YELLOW;
		colorOne = BROWN;
	} else if( newColor == YELLOW ) {
		colorTwo = BROWN;
		colorOne = YELLOW;
	} else if( newColor == LIGHTGRAY ) {
		colorTwo = LIGHTGRAY;
		colorOne = WHITE;
	} else if( newColor == WHITE ) {
		colorTwo = WHITE;
		colorOne = LIGHTGRAY;
	} else {
		colorTwo = GREEN; //Magenta on a green background: Never used in DOS games because, on composite monitors, it left too much color 'smearing'. Likewise, it should never be used in this game.
		colorOne = MAGENTA;
	}
}

video::SColor Object::getColorOne() {
	return colorOne;
}

video::SColor Object::getColorTwo() {
	return colorTwo;
}

video::SColor Object::getColor() {
	return getColorOne();
}

void Object::setColorBasedOnNum( u8 num ) {
	switch( num % 13 ) {
		case 0: {
			setColor( RED ); //Special case: We don't want the player to be black (color 0) against a black background;
			//setColor( BLACK );
			break;
		}
		case 1: {
			setColor( BLUE );
			break;
		}
		case 2: {
			setColor( GREEN );
			break;
		}
		case 3: {
			setColor( CYAN );
			break;
		}
		case 4: {
			setColor( LIGHTRED ); //Normally red would be used here
			break;
		}
		case 5: {
			setColor( MAGENTA );
			break;
		}
		case 6: {
			setColor( BROWN );
			break;
		}
		case 7: {
			setColor( LIGHTGRAY );
			break;
		}
		case 8: {
			setColor( LIGHTBLUE ); //Skipping over gray because its alternate color is black
			break;
		}
		case 9: {
			setColor( LIGHTGREEN );
			break;
		}
		case 10: {
			setColor( LIGHTCYAN );
			break;
		}
		case 11: {
			setColor( LIGHTMAGENTA ); //Skipping over light red here since we've already used it earlier
			break;
		}
		case 12: {
			setColor( YELLOW );
			break;
		}
	}
}
