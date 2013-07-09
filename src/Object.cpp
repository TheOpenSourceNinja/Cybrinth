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

void Object::setPos( uint8_t newX, uint8_t newY ) {
	setX( newX );
	setY( newY );
}

uint8_t Object::getY() {
	return y;
}

uint8_t Object::getX() {
	return x;
}

void Object::moveY( int8_t val ) {
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

void Object::moveX( int8_t val ) {
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

void Object::setX( uint8_t val ) {
	x = val;
	xInterp = x;
}

void Object::setY( uint8_t val ) {
	y = val;
	yInterp = y;
}

void Object::draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {
	if( moving ) {
		float speed = .2;

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

	uint32_t size;

	if( width < height ) {
		size = width;
	} else {
		size = height;
	}

	if( image != NULL ) {
		int32_t cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		int32_t cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );
		//driver->draw2DImage(image, core::position2d<int32_t>(cornerX, cornerY));
		irr::video::SColor colorArray[] = {colorOne, colorOne, colorOne, colorOne};
		driver->draw2DImage( image,
							 irr::core::rect<int32_t>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 irr::core::rect<int32_t>( irr::core::position2d<int32_t>( 0, 0 ), image->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the image if we want. Zero means draw the whole thing.
							 colorArray,
							 true );
	}
}

/*void Object::loadImage(video::IVideoDriver* driver) {
	image = driver->addTexture(core::dimension2d<uint32_t>(1,1), "empty");
}*/

void Object::setColor( irr::video::SColor newColor ) {
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

irr::video::SColor Object::getColorOne() {
	return colorOne;
}

irr::video::SColor Object::getColorTwo() {
	return colorTwo;
}

irr::video::SColor Object::getColor() {
	return getColorOne();
}

void Object::setColorBasedOnNum( uint8_t num ) {
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
