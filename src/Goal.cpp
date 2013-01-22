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
#include "Goal.h"
#include <iostream>

using namespace std;
using namespace irr;

Goal::Goal() {
	setColor( WHITE );
}

Goal::~Goal() {
	//dtor
}

void Goal::loadImage( video::IVideoDriver* driver ) {
	//image = driver->getTexture( "diamond.png" );
	loadImage( driver, 1 );
}

void Goal::loadImage( video::IVideoDriver* driver, u32 size ) {
	video::IImage *tempImage = driver->createImage( video::ECF_A8R8G8B8, core::dimension2d<u32>( size, size ) );
	tempImage->fill( video::SColor( 0, 0, 0, 0) ); //Fills the image with invisibility!

	for( u32 x = 0; x <= ( size / 2 ); x++ ) {
		for( u32 y = 0; y <= x; y++ ) {
			tempImage->setPixel( x, y + ( size / 2 ), WHITE );
			tempImage->setPixel( size - x, y + ( size / 2 ), WHITE );
			tempImage->setPixel( x, size - ( y + ( size / 2 ) ), WHITE );
			tempImage->setPixel( size - x, size - ( y + ( size / 2 ) ), WHITE );
		}
	}

	driver->removeTexture( image );
	image = driver->addTexture( L"goalDiamond", tempImage );
}

void Goal::draw( video::IVideoDriver* driver, u32 width, u32 height ) {
	//Object::draw( driver, width, height );

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
