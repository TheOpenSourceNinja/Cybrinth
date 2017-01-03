/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2017.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The Goal class is an object that the players have to reach.
 */
 
#include "colors.h"
#include "Goal.h"
#ifdef HAVE_IOSTREAM
#include <iostream>
#endif //HAVE_IOSTREAM
#include <boost/filesystem.hpp>
#include "StringConverter.h"
#include "XPMImageLoader.h"


Goal::Goal() {
	try {
		setColors( GRAY, LIGHTGREEN );
		name = L"Goal";
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::Goal(): " << e.what() << std::endl;
	}
}

Goal::~Goal() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::~Goal(): " << e.what() << std::endl;
	}
}

void Goal::createTexture( irr::IrrlichtDevice* device, uint_fast16_t size ) {
	try {
		XPMImageLoader loader;
		driver = device->getVideoDriver();
		
		irr::video::IImage* tempImage = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( size, size ) );
		loader.loadOtherImage( driver, tempImage, XPMImageLoader::GOAL );
		
		irr::core::stringw textureName = "goal-xpm";
		
		adjustImageColors( tempImage );
		
		texture = resizer.imageToTexture( driver, tempImage, textureName );

		if( texture == nullptr ) {
			irr::video::IImage* temp = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( size, size ) );
			temp->fill( WHITE );
			texture = resizer.imageToTexture( driver, temp, "generic goal" );
		}
		
		if( texture not_eq nullptr and texture->getSize() not_eq irr::core::dimension2d< irr::u32 >( size, size ) ) {
			auto newTexture = resizer.resize( texture, size, size, driver );
			driver->removeTexture( texture );
			texture = newTexture;
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::createTexture(): " << e.what() << std::endl;
	}
}

void Goal::draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height ) {
	try {
		uint_fast16_t size;

		if( width < height ) {
			size = width;
		} else {
			size = height;
		}

		if( texture->getSize().Width not_eq size ) {
			Object::loadTexture( device, size, name );
			if( texture == nullptr or texture == NULL ) {
				createTexture( device, size );
			}
		}

		Object::draw( device, width, height );
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in Goal::draw(): " << e.what() << std::endl;
	}
}

void Goal::loadTexture( irr::IrrlichtDevice* device ) {
	uint_fast16_t size = 1;
	Object::loadTexture( device, size, name );
	if( texture == nullptr or texture == NULL ) {
		createTexture( device, size );
	}
}

void Goal::reset() {
}
