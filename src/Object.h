/**
 * Copyright © 2012-2014 James Dearing.
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

#include <irrlicht.h>
#include "Integers.h"
#include "PreprocessorCommands.h"

class Object {
	public:
		Object();
		virtual ~Object();
		uint_fast8_t getY();
		uint_fast8_t getX();
		void setPos( uint_fast8_t newX, uint_fast8_t newY );
		void setX( uint_fast8_t val );
		void setY( uint_fast8_t val );
		void moveY( int_fast8_t val );
		void moveX( int_fast8_t val );
		void draw( irr::video::IVideoDriver* driver, uint_fast16_t width, uint_fast16_t height );
		virtual void loadTexture( irr::video::IVideoDriver* driver ) = 0;
		virtual void reset() = 0;
		void setColorBasedOnNum( uint_fast8_t num );
		void setColor( irr::video::SColor newColor );
		irr::video::SColor getColorOne();
		irr::video::SColor getColorTwo();
		irr::video::SColor getColor();
		uint_fast16_t distanceFromExit;
	protected:
		uint_fast8_t x;
		float xInterp;
		uint_fast8_t y;
		float yInterp;
		bool moving;
		irr::video::ITexture* texture;
		irr::video::SColor colorOne;
		irr::video::SColor colorTwo;
	private:
};

#endif // OBJECT_H
