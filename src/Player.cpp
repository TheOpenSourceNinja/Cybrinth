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

using namespace irr;
using namespace std;

Player::Player() {
	x = 0;
	y = 0;
	xInterp = 0;
	yInterp = 0;
	moving = false;
	setColor( RED );
	image = NULL;
	stepsTaken = 0;
}

Player::~Player() {
	//dtor
}

void Player::loadImage( video::IVideoDriver* driver ) {
	//image = driver->getTexture( "circle.png" );
	loadImage( driver, 1 );
}

void Player::loadImage( video::IVideoDriver* driver, u32 size ) {
	video::IImage *tempImage = driver->createImage( video::ECF_A8R8G8B8, core::dimension2d<u32>( size, size ) );
	tempImage->fill( video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

	s32 radius = size / 2;
	core::position2d< s32 > origin( radius, radius );
	f32 rSquared = pow( radius, 2 );
	for( s32 x = -radius; x < radius; x++ ) {
		s32 height = static_cast< s32 >( sqrt( rSquared - pow( x, 2 ) ) );
		for( s32 y = -height; y < height; y++ ) {
			tempImage->setPixel( x + origin.X, y + origin.Y, WHITE );
		}
	}

	driver->removeTexture( image );
	image = driver->addTexture( L"playerCircle", tempImage );
}

void Player::draw( video::IVideoDriver* driver, u32 width, u32 height ) {
	//Object::draw(driver, width, height);

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

	if( image->getSize().Width != size ) {
		loadImage( driver, size );
	}

	if( image != NULL ) {
		s32 cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		s32 cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );
		video::SColor colorArray[] = {colorTwo, colorTwo, colorTwo, colorTwo};
		driver->draw2DImage( image,
							 core::rect<s32>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 core::rect<s32>( core::position2d<s32>( 0, 0 ), image->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the image if we want. Zero means draw the whole thing.
							 colorArray,
							 true );

		size /= 2;
		cornerX = ( xInterp * width ) + (( width / 2 ) - ( size / 2 ) );
		cornerY = ( yInterp * height ) + (( height / 2 ) - ( size / 2 ) );

		for( u8 a = 0; a < 4; a++ ) {
			colorArray[a] = colorOne;
		}

		driver->draw2DImage( image,
							 core::rect<s32>( cornerX, cornerY, cornerX + size, cornerY + size ),
							 core::rect<s32>( core::position2d<s32>( 0, 0 ), image->getSize() ),
							 0, //The clipping rectangle, so we can draw only part of the image if we want. Zero means draw the whole thing.
							 colorArray,
							 true );
	}
}

void Player::moveX( s8 val ) {
	Object::moveX( val );
	stepsTaken += 1;
}

void Player::moveY( s8 val ) {
	Object::moveY( val );
	stepsTaken += 1;
}
