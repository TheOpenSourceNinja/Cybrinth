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
#include "PlayerStart.h"

PlayerStart::PlayerStart() {
	reset();
}

PlayerStart::~PlayerStart() {
	//dtor
}

void PlayerStart::reset() {
	image = NULL;
	x = 0;
	y = 0;
	distanceFromExit = 0;
}

void PlayerStart::loadImage( irr::video::IVideoDriver* driver ) {
	loadImage( driver, 1 );
}

void PlayerStart::loadImage( irr::video::IVideoDriver* driver, uint32_t size ) {
	irr::video::IImage *tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d<uint32_t>( size, size ) );
	tempImage->fill( WHITE ); //Fills the image with invisibility!
	setColor( BLACK );

	driver->removeTexture( image );
	image = driver->addTexture( L"playerStart", tempImage );
}

void PlayerStart::draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height ) {
	uint32_t size;

	if( width < height ) {
		size = width;
	} else {
		size = height;
	}

	if( image == NULL || ( image != NULL && image->getSize().Width != size ) ) {
		loadImage( driver, size );
	}

	Object::draw( driver, width, height );
}
