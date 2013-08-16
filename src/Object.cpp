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
#include "colors.h"
#include <iostream>



Object::Object() {
	try {
		x = 0;
		y = 0;
		xInterp = 0;
		yInterp = 0;
		moving = false;
		distanceFromExit = 0;
		texture = NULL;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::Object(): " << e.what() << std::endl;
	}
}

Object::~Object() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::~Object(): " << e.what() << std::endl;
	}
}

void Object::setPos( uint_least8_t newX, uint_least8_t newY ) {
	try {
		setX( newX );
		setY( newY );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::setPos(): " << e.what() << std::endl;
	}
}

uint_least8_t Object::getY() {
	try {
		return y;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::getY(): " << e.what() << std::endl;
		return UINT_LEAST8_MAX;
	}
}

uint_least8_t Object::getX() {
	try {
		return x;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::getX(): " << e.what() << std::endl;
		return UINT_LEAST8_MAX;
	}
}

void Object::moveY( int_fast8_t val ) {
	try {
		if( !moving ) {
			y += val;
			moving = true;
		} else {
			yInterp = y;
			xInterp = x;
			y += val;
			moving = true;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::moveY(): " << e.what() << std::endl;
	}
}

void Object::moveX( int_fast8_t val ) {
	try {
		if( !moving ) {
			x += val;
			moving = true;
		} else {
			xInterp = x;
			yInterp = y;
			x += val;
			moving = true;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::moveX(): " << e.what() << std::endl;
	}
}

void Object::setX( uint_least8_t val ) {
	try {
		x = val;
		xInterp = x;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::setX(): " << e.what() << std::endl;
	}
}

void Object::setY( uint_least8_t val ) {
	try {
		y = val;
		yInterp = y;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::setY(): " << e.what() << std::endl;
	}
}

void Object::draw( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
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

		uint_least16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture != NULL ) {
			int_least16_t cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
			int_least16_t cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );
			irr::video::SColor colorArray[] = {colorTwo, colorTwo, colorTwo, colorTwo};
			driver->draw2DImage( texture,
								 irr::core::rect< irr::s32 >( cornerX, cornerY, cornerX + size, cornerY + size ),
								 irr::core::rect< irr::s32 >( irr::core::position2d< irr::s32 >( 0, 0 ), texture->getSize() ),
								 0, //The clipping rectangle, so we can draw only part of the texture if we want. Zero means draw the whole thing.
								 0, //"Array of 4 colors denoting the color values of the corners of the destRect". Zero means use the texture's own colors.
								 true ); //Whether to use the texture's alpha channel
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::draw(): " << e.what() << std::endl;
	}
}

void Object::setColor( irr::video::SColor newColor ) {
	try {
		if( newColor == BLACK ) {
			colorOne = BLACK;
			colorTwo = GRAY;
		} else if( newColor == GRAY ) {
			colorOne = GRAY;
			colorTwo = BLACK;
		} else if( newColor == BLUE) {
			colorOne = BLUE;
			colorTwo = LIGHTBLUE;
		} else if( newColor == LIGHTBLUE ) {
			colorOne = LIGHTBLUE;
			colorTwo = BLUE;
		} else if( newColor == GREEN ) {
			colorOne = GREEN;
			colorTwo = LIGHTGREEN;
		} else if( newColor == LIGHTGREEN ) {
			colorOne = LIGHTGREEN;
			colorTwo = GREEN;
		} else if( newColor == CYAN ) {
			colorOne = CYAN;
			colorTwo = LIGHTCYAN;
		} else if( newColor == LIGHTCYAN ) {
			colorOne = LIGHTCYAN;
			colorTwo = CYAN;
		} else if( newColor == RED ) {
			colorOne = RED;
			colorTwo = LIGHTRED;
		} else if( newColor == LIGHTRED ) {
			colorOne = LIGHTRED;
			colorTwo = RED;
		} else if( newColor == MAGENTA ) {
			colorOne = MAGENTA;
			colorTwo = LIGHTMAGENTA;
		} else if( newColor == LIGHTMAGENTA ) {
			colorOne = LIGHTMAGENTA;
			colorTwo = MAGENTA;
		} else if( newColor == BROWN ) {
			colorOne = YELLOW;
			colorTwo = BROWN;
		} else if( newColor == YELLOW ) {
			colorOne = BROWN;
			colorTwo = YELLOW;
		} else if( newColor == LIGHTGRAY ) {
			colorOne = LIGHTGRAY;
			colorTwo = WHITE;
		} else if( newColor == WHITE ) {
			colorOne = WHITE;
			colorTwo = LIGHTGRAY;
		} else {
			colorOne = GREEN; //Magenta on a green background: Never used in DOS games because, on composite monitors, it left too much color 'smearing'. Likewise, it should never be used in this game.
			colorTwo = MAGENTA;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::setColor(): " << e.what() << std::endl;
	}
}

irr::video::SColor Object::getColorOne() {
	try {
		return colorOne;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::getColorOne(): " << e.what() << std::endl;
		irr::video::SColor c;
		return c;
	}
}

irr::video::SColor Object::getColorTwo() {
	try {
		return colorTwo;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::getColorTwo(): " << e.what() << std::endl;
		irr::video::SColor c;
		return c;
	}
}

irr::video::SColor Object::getColor() {
	try {
		return getColorOne();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::getColor(): " << e.what() << std::endl;
		irr::video::SColor c;
		return c;
	}
}

void Object::setColorBasedOnNum( uint_least8_t num ) {
	try {
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
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Object::setColorBasedOnNum(): " << e.what() << std::endl;
	}
}
