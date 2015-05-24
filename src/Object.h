/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The Object class contains those properties common to players, goals, and collectables.
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <irrlicht/irrlicht.h>
#include "Integers.h"
#include "PreprocessorCommands.h"
#include "ImageModifier.h"

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
		void draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height );
		virtual void loadTexture( irr::IrrlichtDevice* device ) = 0;
		void loadTexture( irr::IrrlichtDevice* device, uint_fast16_t size, irr::core::stringw fileName );
		virtual void createTexture( irr::IrrlichtDevice* device, uint_fast16_t size = 1 ) = 0; //To be called by subclasses when they can't get their texture loaded
		virtual void reset() = 0;
		void setColorBasedOnNum( uint_fast8_t num );
		void setColors( irr::video::SColor newColorOne, irr::video::SColor newColorTwo );
		irr::video::SColor getColorOne();
		irr::video::SColor getColorTwo();
		uint_fast16_t distanceFromExit;
		void adjustImageColors( irr::video::IImage* image );
	protected:
		uint_fast8_t x;
		float xInterp;
		uint_fast8_t y;
		float yInterp;
		bool moving;
		irr::video::ITexture* texture;
		irr::video::SColor colorOne;
		irr::video::SColor colorTwo;
		ImageModifier resizer;
		irr::video::IVideoDriver* driver;
	private:
};

#endif // OBJECT_H
