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
#include <cmath>

using namespace std;

Player::Player() {
	x = 0;
	y = 0;
	xInterp = 0;
	yInterp = 0;
	moving = false;
	setColor( RED );
	texture = NULL;
	stepsTaken = 0;
	isHuman = true;
}

Player::~Player() {
	//dtor
}

void Player::loadTexture( irr::video::IVideoDriver* driver ) {
	loadTexture( driver, 1 );
}

//Draws a filled circle. Somebody please implement a faster algorithm.
void Player::loadTexture( irr::video::IVideoDriver* driver, uint32_t size ) {
	irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< uint32_t >( size, size ) );
	tempImage->fill( irr::video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

	int32_t radius = size / 2;
	irr::core::position2d< int32_t > origin( radius, radius );
	float rSquared = pow( radius, 2 );
	for( int32_t x = -radius; x <= 0; x++ ) {
		int32_t height = static_cast< int32_t >( sqrt( rSquared - pow( x, 2 ) ) );
		for( int32_t y = -height; y <= 0; y++ ) {
			tempImage->setPixel( x + origin.X, y + origin.Y, WHITE );
			tempImage->setPixel( x + origin.X, -y + origin.Y, WHITE );
			tempImage->setPixel( -x + origin.X, y + origin.Y, WHITE );
			tempImage->setPixel( -x + origin.X, -y + origin.Y, WHITE );
		}
	}

	driver->removeTexture( texture );
	texture = driver->addTexture( L"playerCircle", tempImage );
}

void Player::draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {

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

	if( texture->getSize().Width != size ) {
		loadTexture( driver, size );
	}

	if( texture != NULL ) {
		int32_t cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		int32_t cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );
		irr::video::SColor colorArray[] = {colorTwo, colorTwo, colorTwo, colorTwo};
		driver->draw2DImage( texture,
							 irr::core::rect<int32_t>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 irr::core::rect<int32_t>( irr::core::position2d<int32_t>( 0, 0 ), texture->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the texture if we want. Zero means draw the whole thing.
							 colorArray,
							 true );

		size /= 2;
		cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );

		for( uint8_t a = 0; a < 4; a++ ) {
			colorArray[a] = colorOne;
		}

		driver->draw2DImage( texture,
							 irr::core::rect<int32_t>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 irr::core::rect<int32_t>( irr::core::position2d<int32_t>( 0, 0 ), texture->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the texture if we want. Zero means draw the whole thing.
							 colorArray,
							 true );
	}
}

void Player::moveX( int8_t val ) {
	Object::moveX( val );
	stepsTaken += 1;
}

void Player::moveY( int8_t val ) {
	Object::moveY( val );
	stepsTaken += 1;
}
