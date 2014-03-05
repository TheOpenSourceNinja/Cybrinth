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

#ifndef COLLECTABLE_H
#define COLLECTABLE_H

#include "Object.h"
#include "ImageModifier.h"
#include "PreprocessorCommands.h"

#include <irrlicht.h>

class Collectable : public Object {
	public:
		enum type_t { KEY };

		Collectable();
		virtual ~Collectable();

		void draw( irr::video::IVideoDriver* driver, uint_fast16_t width, uint_fast16_t height );
		type_t getType();
		void loadTexture( irr::video::IVideoDriver* driver );
		void reset();
		void setType( type_t newType );
	protected:
	private:
		type_t type;
		ImageModifier resizer;
};

#endif // COLLECTABLE_H
