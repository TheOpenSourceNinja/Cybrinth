//This file originally downloaded from http://www.michaelzeilfelder.de/irrlicht.htm#TrueType. The copyright statement and license below do not apply to that original version, only to the modified version contained here.
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

#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <string>
#include <map>
#include <irrlicht.h>

namespace irr {
namespace gui {
class IGUIFont;
}
}
class CGUITTFace;

class FontManager {
	public:
		FontManager();
		~FontManager();
		irr::gui::IGUIFont* GetTtFont( irr::video::IVideoDriver* driver, const char* filename_, unsigned int size_, bool antiAlias_ = true, bool transparency_ = true );

	protected:
		std::string MakeFontIdentifier( const char* filename_, unsigned int size_, bool antiAlias_, bool transparency_ );

	private:
		typedef std::map<std::string, CGUITTFace*> FaceMap;
		FaceMap mFaceMap;

		typedef std::map<std::string, irr::gui::IGUIFont*> FontMap;
		FontMap mFontMap;
};

#endif // FONT_MANAGER_H
