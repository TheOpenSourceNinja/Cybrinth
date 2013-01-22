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

#ifndef PLAYERSTART_H
#define PLAYERSTART_H

#include "Object.h"


class PlayerStart : public Object {
	public:
		PlayerStart();
		virtual ~PlayerStart();
		void reset();
		void loadImage( video::IVideoDriver* driver );
		void loadImage( video::IVideoDriver* driver, u32 size );
		void draw( video::IVideoDriver* driver, u32 width, u32 height );
	protected:
	private:
};

#endif // PLAYERSTART_H
