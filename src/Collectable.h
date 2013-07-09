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

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "Object.h"

#include <irrlicht.h>

#define COLLECTABLE_KEY 1

class Collectable : public Object {
	public:
		Collectable();
		virtual ~Collectable();
		void setType( uint8_t );
		uint8_t getType();
		void loadImage( irr::video::IVideoDriver* driver );
		//void draw(video::IVideoDriver* driver, uint32_t width, uint32_t height);
	protected:
	private:
		uint8_t type;
};

#endif // COLLECTABLE_H
