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

#include "Collectable.h"
#include <iostream>

using namespace std;

Collectable::Collectable() {
	x = 0;
	y = 0;
	type = COLLECTABLE_KEY;
}

Collectable::~Collectable() {
	//dtor
}

void Collectable::setType( u8 newType ) {
	type = newType;

	switch( type ) {
		case COLLECTABLE_KEY: {
				setColor( YELLOW );
				break;
			}
		default: break;
	}
}

u8 Collectable::getType() {
	return type;
}

void Collectable::loadImage( video::IVideoDriver* driver ) {
	switch( type ) {
		case COLLECTABLE_KEY: {
				image = driver->getTexture( L"key.png" );

				if( image == NULL ) {
					wcerr << L"Key image not loaded" << endl;
				}

				break;
			}
		default: break;
	}
}

/*void Collectable::draw(video::IVideoDriver* driver, u32 width, u32 height) {
	if (image != NULL) {
		f64 cornerX = xInterp*width;
		f64 cornerY = yInterp*height;
		driver->draw2DImage(image, core::position2d<s32>(cornerX, cornerY));
	} else {
		wcerr << L"Image is null" << endl;
	}
}
*/
