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
#include "Player.h"
#include <irrlicht.h>
#include <iostream>
#include "colors.h"



Player::Player() {
	try {
		x = 0;
		y = 0;
		xInterp = 0;
		yInterp = 0;
		moving = false;
		setColor( RED );
		texture = nullptr;
		isHuman = true;
		reset();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::Player(): " << e.what() << std::endl;
	}
}

Player::~Player() {
	try {
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::~Player(): " << e.what() << std::endl;
	}
}

void Player::loadTexture( irr::video::IVideoDriver* driver ) {
	try {
		loadTexture( driver, 1 );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::loadTexture(): " << e.what() << std::endl;
	}
}

//Draws a filled circle. Somebody please implement a faster algorithm.
void Player::loadTexture( irr::video::IVideoDriver* driver, uint_fast16_t size ) {
	try {
		irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) ); //Colorspace should be irr::video::A1R5G5B5 but that causes a bug on my current laptop.
		tempImage->fill( irr::video::SColor( 0, 0, 0, 0 ) ); //Fills the image with invisibility!
		tempImage->setPixel( size - 1, size - 1, irr::video::SColor( 0, 0, 0, 0 ) ); //Workaround for a bug in Irrlicht's software renderer

		int_fast16_t radius = size / 2;
		irr::core::position2d< int_fast16_t > origin( radius, radius );
		float rSquared = pow( radius, 2 );
		for( int_fast16_t x = -radius; x <= 0; ++x ) {
			int_fast16_t height = static_cast< int_fast16_t >( sqrt( rSquared - pow( x, 2 ) ) );
			for( int_fast16_t y = -height; y <= 0; ++y ) {
				tempImage->setPixel( x + origin.X, y + origin.Y, colorOne );
				tempImage->setPixel( x + origin.X, -y + origin.Y, colorOne );
				tempImage->setPixel( -x + origin.X, y + origin.Y, colorOne );
				tempImage->setPixel( -x + origin.X, -y + origin.Y, colorOne );
			}
		}

		size /= 2;

		radius = size / 2;
		rSquared = pow( radius, 2 );
		for( int_fast16_t x = -radius; x <= 0; ++x ) {
			int_fast16_t height = static_cast< int_fast16_t >( sqrt( rSquared - pow( x, 2 ) ) );
			for( int_fast16_t y = -height; y <= 0; ++y ) {
				tempImage->setPixel( x + origin.X, y + origin.Y, colorTwo );
				tempImage->setPixel( x + origin.X, -y + origin.Y, colorTwo );
				tempImage->setPixel( -x + origin.X, y + origin.Y, colorTwo );
				tempImage->setPixel( -x + origin.X, -y + origin.Y, colorTwo );
			}
		}


		driver->removeTexture( texture );
		texture = driver->addTexture( L"playerCircle", tempImage );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::loadTexture(): " << e.what() << std::endl;
	}
}

void Player::draw( irr::video::IVideoDriver* driver, uint_fast16_t width, uint_fast16_t height ) {
	try {
		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture->getSize().Width != size ) {
			loadTexture( driver, size );
		}

		Object::draw( driver, width, height );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::draw(): " << e.what() << std::endl;
	}
}

void Player::moveX( int_fast8_t val ) {
	try {
		Object::moveX( val );
		stepsTaken += 1;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveX(): " << e.what() << std::endl;
	}
}

void Player::moveY( int_fast8_t val ) {
	try {
		Object::moveY( val );
		stepsTaken += 1;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::moveY(): " << e.what() << std::endl;
	}
}

void Player::reset() {
	try {
		stepsTaken = 0;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in Player::reset(): " << e.what() << std::endl;
	}
}
