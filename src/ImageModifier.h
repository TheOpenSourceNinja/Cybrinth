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
 * The ImageModifier class does various things related to image manipulation and conversion.
 */

#ifndef IMAGEMODIFIER_H
#define IMAGEMODIFIER_H

#include "Integers.h"
#include "PreprocessorCommands.h"

#include <irrlicht/irrlicht.h>

class ImageModifier {
	public:
		ImageModifier();
		virtual ~ImageModifier();
		irr::video::ITexture* resize( irr::video::ITexture* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver );
		irr::video::IImage* resize( irr::video::IImage* image, uint_fast32_t width, uint_fast32_t height, irr::video::IVideoDriver* driver );
		irr::video::IImage* textureToImage( irr::video::IVideoDriver* driver, irr::video::ITexture* texture );
		irr::video::ITexture* imageToTexture( irr::video::IVideoDriver* driver, irr::video::IImage* texture, irr::core::stringw name );
	protected:
	private:
};

#endif // IMAGEMODIFIER_H
