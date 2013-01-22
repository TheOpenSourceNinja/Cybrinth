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

using namespace irr;

class Object {
	public:
		Object();
		virtual ~Object();
		u8 getY();
		u8 getX();
		void setPos( u8 newX, u8 newY );
		void setX( u8 val );
		void setY( u8 val );
		void moveY( s8 val );
		void moveX( s8 val );
		void draw( video::IVideoDriver* driver, u32 width, u32 height );
		virtual void loadImage( video::IVideoDriver* driver ) = 0;
		void setColorBasedOnNum( u8 num );
		void setColor( video::SColor newColor );
		video::SColor getColorOne();
		video::SColor getColorTwo();
		video::SColor getColor();
		u16 distanceFromExit;
	protected:
		u8 x;
		f64 xInterp;
		u8 y;
		f64 yInterp;
		bool moving;
		video::ITexture* image;
		video::SColor colorOne;
		video::SColor colorTwo;
	private:
};

#endif // OBJECT_H
