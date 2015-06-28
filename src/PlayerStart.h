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
 * The PlayerStart class represents a given player's beginning point in the maze. Like a Goal but it's at the start rather than the end.
 */

#ifndef PLAYERSTART_H
#define PLAYERSTART_H

#include "Object.h"
#include "PreprocessorCommands.h"

#include <irrlicht/irrlicht.h>

class PlayerStart : public Object {
	public:
		PlayerStart();
		virtual ~PlayerStart();
		
		void createTexture( irr::IrrlichtDevice* device, uint_fast16_t size );
		
		void draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height );
		
		void loadTexture( irr::IrrlichtDevice* device );
		
		void reset();
	protected:
	private:
};

#endif // PLAYERSTART_H
