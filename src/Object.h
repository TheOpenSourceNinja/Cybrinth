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

#ifndef OBJECT_H
#define OBJECT_H

#include "colors.h"
#include <irrlicht.h>
#include <stdint.h>

class Object {
	public:
		Object();
		virtual ~Object();
		uint8_t getY();
		uint8_t getX();
		void setPos( uint8_t newX, uint8_t newY );
		void setX( uint8_t val );
		void setY( uint8_t val );
		void moveY( int8_t val );
		void moveX( int8_t val );
		void draw( irr::video::IVideoDriver* driver, uint32_t width, uint32_t height );
		virtual void loadImage( irr::video::IVideoDriver* driver ) = 0;
		void setColorBasedOnNum( uint8_t num );
		void setColor( irr::video::SColor newColor );
		irr::video::SColor getColorOne();
		irr::video::SColor getColorTwo();
		irr::video::SColor getColor();
		uint16_t distanceFromExit;
	protected:
		uint8_t x;
		float xInterp;
		uint8_t y;
		float yInterp;
		bool moving;
		irr::video::ITexture* image;
		irr::video::SColor colorOne;
		irr::video::SColor colorTwo;
	private:
};

#endif // OBJECT_H
