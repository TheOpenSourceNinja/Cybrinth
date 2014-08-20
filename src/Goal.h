/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The Goal class is an object that the players have to reach.
 */

#ifndef GOAL_H
#define GOAL_H

#include "Object.h"
#include "PreprocessorCommands.h"

#include <irrlicht/irrlicht.h>

class Goal : public Object {
	public:
		Goal();
		virtual ~Goal();
		
		void createTexture( irr::IrrlichtDevice* device, uint_fast16_t size );
		
		void draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height );
		
		void loadTexture( irr::IrrlichtDevice* device );
		
		void reset();
	protected:
	private:
};

#endif // GOAL_H
