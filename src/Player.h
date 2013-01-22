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

#ifndef PLAYER_H
#define PLAYER_H

#include <irrlicht.h>
#include "Object.h"
#include "colors.h"

using namespace irr;

class Player : public Object {
	public:
		Player();
		virtual ~Player();
		void draw( video::IVideoDriver* driver, u32 width, u32 height );
		void loadImage( video::IVideoDriver* driver );
		void loadImage( video::IVideoDriver* driver, u32 size );
		void moveX( s8 val );
		void moveY( s8 val );
		u32 stepsTaken;
	protected:
	private:
};

#endif // PLAYER_H
