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
 * The Collectable class is a generic class for in-game items, such as keys and acid.
 */

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "Object.h"
#include "ImageModifier.h"
#include "PreprocessorCommands.h"

#include <irrlicht/irrlicht.h>

class Collectable : public Object {
	public:
		enum type_t : uint_fast8_t { KEY, ACID };
		
		void createTexture( irr::IrrlichtDevice* device, uint_fast16_t size = 1 );
		
		Collectable();
		virtual ~Collectable();

		void draw( irr::IrrlichtDevice* device, uint_fast16_t width, uint_fast16_t height );
		
		type_t getType();
		
		void loadTexture( irr::IrrlichtDevice* device );
		void loadTexture( irr::IrrlichtDevice* device, uint_fast8_t size );
		
		bool owned;
		
		void reset();
		
		void setType( type_t newType );
	protected:
	private:
		type_t type;
};

#endif // COLLECTABLE_H
