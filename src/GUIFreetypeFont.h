//This file originally downloaded from http://www.michaelzeilfelder.de/irrlicht.htm#TrueType. The copyright statement and license below do not apply to that original version, only to the modified version contained here.
/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 *
 * @section LICENSE
 * Copyright © 2012-2017.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * The GUIFreetypeFont class is used to represent fonts.
 */

//TODO: See http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=49898

#ifndef _GUI_FREETYPE_FONT_H
#define _GUI_FREETYPE_FONT_H

#include "PreprocessorCommands.h"

#if COMPILE_WITH_FREETYPE

#include "Integers.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING

class CGUITTFace : public irr::IReferenceCounted {
	public:
		CGUITTFace();
		virtual ~CGUITTFace();

		bool load( const irr::io::path& filename );

		FT_Face		face;			   // handle to face

	private:
		static int			countClassObjects;
		static FT_Library	library;	// handle to library
};

class CGUIFreetypeFont;

class CGUITTGlyph : public irr::IReferenceCounted {
	public:
		CGUITTGlyph();
		virtual ~CGUITTGlyph();

		bool cached;
		void cache( irr::u32 idx_, const CGUIFreetypeFont * freetypeFont );

		irr::u32 size; //Character size in pixels
		irr::u32 top;
		irr::u32 left;
		irr::u32 texw;
		irr::u32 texh;
		irr::u32 imgw;
		irr::u32 imgh;
		irr::video::ITexture *tex;
		irr::u32 top16;
		irr::u32 left16;
		irr::u32 texw16;
		irr::u32 texh16;
		irr::u32 imgw16;
		irr::u32 imgh16;
		irr::video::ITexture *tex16;
		irr::u8 *image;

	private:
		void setGlyphTextureFlags( irr::video::IVideoDriver* driver_ );
		void restoreTextureFlags( irr::video::IVideoDriver* driver_ );

		static bool mTexFlag16;
		static bool mTexFlag32;
		static bool mTexFlagMip;
};

class CGUIFreetypeFont : public irr::gui::IGUIFont {
		friend class CGUITTGlyph;

	public:

		//! constructor
		explicit CGUIFreetypeFont( irr::video::IVideoDriver* Driver );

		//! destructor
		virtual ~CGUIFreetypeFont();

		//! loads a truetype font file
		bool attach( CGUITTFace *Face, irr::u32 size );

		//! draws an text and clips it to the specified rectangle if wanted
		virtual void draw( const irr::core::stringw& text, const irr::core::rect<irr::s32>& position, irr::video::SColor color, bool hcenter = false, bool vcenter = false, const irr::core::rect<irr::s32>* clip = 0 );

		//! returns the dimension of a text
		virtual irr::core::dimension2d< irr::u32 > getDimension( irr::core::stringw text ) const;

		//! returns the dimension of a text
		virtual irr::core::dimension2d< irr::u32 > getDimension( std::wstring text ) const;

		//! returns the dimension of a text
		virtual irr::core::dimension2d< irr::u32 > getDimension( const wchar_t* text ) const;

		//! Calculates the index of the character in the text which is on a specific position.
		virtual irr::s32 getCharacterFromPos( const wchar_t* text, irr::s32 pixel_x ) const;

		//! Not yet supported
		virtual void setKerningWidth( irr::s32 kerning __attribute__ ((unused)) ) {}

		//! Not yet supported
		virtual void setKerningHeight( irr::s32 kerning __attribute__ ((unused)) ) {}

		//! Not yet supported
		virtual irr::s32 getKerningWidth( const wchar_t* thisLetter __attribute__ ((unused)) = 0, const wchar_t* previousLetter __attribute__ ((unused)) = 0)	const {
			return 0;
		}

		//! Not yet supported
		virtual irr::s32 getKerningHeight()	const {
			return 0;
		}

		//! Not yet supported
		virtual void setInvisibleCharacters( const wchar_t *s __attribute__ ((unused)) ) {}


		bool AntiAlias;
		bool Transparency;

	protected:
		void clearGlyphs();

	private:
		irr::u32 getWidthFromCharacter( wchar_t c ) const;
		irr::u32 getGlyphByChar( wchar_t c ) const;
		irr::u32 getGlyphByIndex( irr::u32 idx ) const;
		irr::video::IVideoDriver* Driver;
		irr::core::array< CGUITTGlyph* > Glyphs;
		CGUITTFace * TrueTypeFace;
		mutable irr::core::dimension2d<irr::u32> LargestGlyph;
};

#endif // #if COMPILE_WITH_FREETYPE

#endif	// _GUI_FREETYPE_FONT_H
