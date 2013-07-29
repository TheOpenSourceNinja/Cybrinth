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
#include "colors.h"
#include "Goal.h"
#include <iostream>

using namespace std;

Goal::Goal() {
	setColor( WHITE );
}

Goal::~Goal() {
	//dtor
}

void Goal::loadTexture( irr::video::IVideoDriver* driver ) {
	//texture = driver->getTexture( "diamond.png" );
	loadTexture( driver, 1 );
}

void Goal::loadTexture( irr::video::IVideoDriver* driver, uint32_t size ) {
	irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d<uint32_t>( size, size ) );
	tempImage->fill( irr::video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

	for( uint32_t x = 0; x <= ( size / 2 ); x++ ) {
		for( uint32_t y = 0; y <= x; y++ ) {
			tempImage->setPixel( x, y + ( size / 2 ), WHITE );
			tempImage->setPixel( size - x, y + ( size / 2 ), WHITE );
			tempImage->setPixel( x, size - ( y + ( size / 2 ) ), WHITE );
			tempImage->setPixel( size - x, size - ( y + ( size / 2 ) ), WHITE );
		}
	}

	driver->removeTexture( texture );
	texture = driver->addTexture( L"goalDiamond", tempImage );
}

void Goal::draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {
	//Object::draw( driver, width, height );

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
