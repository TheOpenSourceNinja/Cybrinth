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

#include "gui_freetype_font.h"

#if COMPILE_WITH_FREETYPE //TODO: Uncomment this before compiling!

#include <cassert>
#include "Integers.h"
#include <iostream>

using namespace irr;
using namespace gui;

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif


// --------------------------------------------------------
CGUITTGlyph::CGUITTGlyph()
	: IReferenceCounted()
	, cached( false )
	, size( 0 )
	, top( 0 )
	, left( 0 )
	, texw( 0 )
	, texh( 0 )
	, imgw( 0 )
	, imgh( 0 )
	, tex( NULL )
	, top16( 0 )
	, left16( 0 )
	, texw16( 0 )
	, texh16( 0 )
	, imgw16( 0 )
	, imgh16( 0 )
	, tex16( NULL )
	, image( NULL ) {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTGlyph::CGUITTGlyph(): " << e.what() << std::endl;
	}
}

CGUITTGlyph::~CGUITTGlyph() {
	try {
		delete[] image;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTGlyph::~CGUITTGlyph(): " << e.what() << std::endl;
	}
}

void CGUITTGlyph::cache( u32 idx_, const CGUIFreetypeFont * freetypeFont ) {
	try {
		assert( freetypeFont );

		FT_Face face = freetypeFont->TrueTypeFace->face;

		FT_Set_Pixel_Sizes( face, 0, size );

		if( size > freetypeFont->LargestGlyph.Height )
			freetypeFont->LargestGlyph.Height = size;

		if( !FT_Load_Glyph( face, idx_, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP ) ) {
			FT_GlyphSlot glyph = face->glyph;
			FT_Bitmap  bits;

			if( glyph->format == ft_glyph_format_outline ) {
				if( !FT_Render_Glyph( glyph, FT_RENDER_MODE_NORMAL ) ) {
					bits = glyph->bitmap;
					u8 *pt = bits.buffer;
					delete[] image;
					image = new u8[bits.width * bits.rows];
					memcpy( image, pt, bits.width * bits.rows );
					top = glyph->bitmap_top;
					left = glyph->bitmap_left;
					imgw = 1;
					imgh = 1;
					texw = bits.width;
					texh = bits.rows;

					for( ;; ) {
						if( imgw > texw ) {
							break;
						} else {
							imgw <<= 1;
						}
					}

					for( ;; ) {
						if( imgh > texh ) {
							break;
						} else {
							imgh <<= 1;
						}
					}

					if( imgw > imgh ) {
						imgh = imgw;
					} else {
						imgw = imgh;
					}

					s32 offx = left;
					s32 offy = size - top;

					if( offx + texw > freetypeFont->LargestGlyph.Width )
						freetypeFont->LargestGlyph.Width = offx + texw;

					if( offy + texh > freetypeFont->LargestGlyph.Height )
						freetypeFont->LargestGlyph.Height = offy + texh;

					u32 *texd = new u32[imgw*imgh];
					memset( texd, 0, imgw * imgh * sizeof( u32 ) );
					u32 *texp = texd;
					bool cflag = ( freetypeFont->Driver->getDriverType() == video::EDT_DIRECT3D8 );

					for( int i = 0; i < bits.rows; ++i ) {
						u32 *rowp = texp;

						for( int j = 0; j < bits.width; j++ ) {
							if( *pt ) {
								if( cflag ) {
									*rowp = *pt;
									*rowp *= 0x01010101;
								} else {
									*rowp = *pt << 24;
									*rowp |= 0xffffff;
								}
							} else {
								*rowp = 0;
							}

							pt++;
							++rowp;
						}

						texp += imgw;
					}

					c8 name[128];
					sprintf( name, "ttf%d_%d_%p", idx_, size, ( void * ) freetypeFont );
					video::IImage *img = freetypeFont->Driver->createImageFromData( video::ECF_A8R8G8B8, core::dimension2d< u32 >( imgw, imgh ), texd );
					setGlyphTextureFlags( freetypeFont->Driver );
					tex = freetypeFont->Driver->addTexture( name, img );
					img->drop();
					restoreTextureFlags( freetypeFont->Driver );
					delete[] texd;
					cached = true;
				}
			}
		}

		if( !FT_Load_Glyph( face, idx_, FT_LOAD_NO_HINTING | FT_LOAD_RENDER | FT_LOAD_MONOCHROME ) ) {
			FT_GlyphSlot glyph = face->glyph;
			FT_Bitmap bits = glyph->bitmap;
			u8 *pt = bits.buffer;
			top16 = glyph->bitmap_top;
			left16 = glyph->bitmap_left;
			imgw16 = 1;
			imgh16 = 1;
			texw16 = bits.width;
			texh16 = bits.rows;

			for( ;; ) {
				if( imgw16 >= texw16 ) {
					break;
				} else {
					imgw16 <<= 1;
				}
			}

			for( ;; ) {
				if( imgh16 >= texh16 ) {
					break;
				} else {
					imgh16 <<= 1;
				}
			}

			if( imgw16 > imgh16 ) {
				imgh16 = imgw16;
			} else {
				imgw16 = imgh16;
			}

			s32 offx = left;
			s32 offy = size - top;

			if( offx + texw > freetypeFont->LargestGlyph.Width )
				freetypeFont->LargestGlyph.Width = offx + texw;

			if( offy + texh > freetypeFont->LargestGlyph.Height )
				freetypeFont->LargestGlyph.Height = offy + texh;


			u16 *texd16 = new u16[imgw16*imgh16];
			memset( texd16, 0, imgw16 * imgh16 * sizeof( u16 ) );
			u16 *texp16 = texd16;

			for( int y = 0; y < bits.rows; ++y ) {
				u16 *rowp = texp16;

				for( int x = 0; x < bits.width; x++ ) {
					if( pt[y * bits.pitch + ( x / 8 )] & ( 0x80 >> ( x % 8 ) ) ) {
						*rowp = 0xffff;
					}

					++rowp;
				}

				texp16 += imgw16;
			}

			c8 name[128];
			sprintf( name, "ttf%d_%d_%p_16", idx_, size, ( void * ) freetypeFont );
			video::IImage *img = freetypeFont->Driver->createImageFromData( video::ECF_A1R5G5B5, core::dimension2d< u32 >( imgw16, imgh16 ), texd16 );
			setGlyphTextureFlags( freetypeFont->Driver );
			tex16 = freetypeFont->Driver->addTexture( name, img );
			img->drop();
			restoreTextureFlags( freetypeFont->Driver );
	//		freetypeFont->Driver->makeColorKeyTexture(tex16,video::SColor(0,0,0,0));
			delete[] texd16;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTGlyph::cache(): " << e.what() << std::endl;
	}
}

bool CGUITTGlyph::mTexFlag16 = false;
bool CGUITTGlyph::mTexFlag32 = true;
bool CGUITTGlyph::mTexFlagMip = false;

void CGUITTGlyph::setGlyphTextureFlags( video::IVideoDriver* driver_ ) {
	try {
		mTexFlag16 = driver_->getTextureCreationFlag( video::ETCF_ALWAYS_16_BIT );
		mTexFlag16 = driver_->getTextureCreationFlag( video::ETCF_ALWAYS_32_BIT );
		mTexFlagMip = driver_->getTextureCreationFlag( video::ETCF_CREATE_MIP_MAPS );
		driver_->setTextureCreationFlag( video::ETCF_ALWAYS_16_BIT, false );
		driver_->setTextureCreationFlag( video::ETCF_ALWAYS_16_BIT, true );
		driver_->setTextureCreationFlag( video::ETCF_CREATE_MIP_MAPS, false );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTGlyph::setGlyphTextureFlags(): " << e.what() << std::endl;
	}
}

void CGUITTGlyph::restoreTextureFlags( video::IVideoDriver* driver_ ) {
	try {
		driver_->setTextureCreationFlag( video::ETCF_ALWAYS_16_BIT, mTexFlag16 );
		driver_->setTextureCreationFlag( video::ETCF_ALWAYS_16_BIT, mTexFlag16 );
		driver_->setTextureCreationFlag( video::ETCF_CREATE_MIP_MAPS, mTexFlagMip );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTGlyph::restoreTextureFlags(): " << e.what() << std::endl;
	}
}

// --------------------------------------------------------
FT_Library	CGUITTFace::library  = 0;
int CGUITTFace::countClassObjects = 0;

CGUITTFace::CGUITTFace()
	: face( 0 ) {
	try {
		++countClassObjects;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTFace::CGUITTFace(): " << e.what() << std::endl;
	}
}

CGUITTFace::~CGUITTFace() {
	try {
		if( face )
			FT_Done_Face( face );

		--countClassObjects;
		assert( countClassObjects >= 0 );

		if( !countClassObjects && library ) {
			FT_Done_FreeType( library );
			library = 0;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTFace::~CGUITTFace(): " << e.what() << std::endl;
	}
}

//! loads a font file
bool CGUITTFace::load( const irr::io::path& filename ) {
	try {
		if( !library ) {
			if( FT_Init_FreeType( &library ) ) {
				return	false;
			}
		}

		core::stringc ansiFilename( filename ); // path can be anything but freetype can only work with ansi-filenames

		if( FT_New_Face( library, ansiFilename.c_str(), 0, &face ) ) {
			return	false;
		}

		return true;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUITTFace::load(): " << e.what() << std::endl;
		return false;
	}
}

// --------------------------------------------------------
//! constructor
CGUIFreetypeFont::CGUIFreetypeFont( video::IVideoDriver* driver )
	: Driver( driver )
	, TrueTypeFace( 0 ) {
	try {
		#ifdef DEBUG
		setDebugName( "CGUIFreetypeFont" );
		#endif

		if( Driver )
			Driver->grab();

		AntiAlias = false;
		Transparency = false;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::CGUIFreetypeFont(): " << e.what() << std::endl;
	}
}



//! destructor
CGUIFreetypeFont::~CGUIFreetypeFont() {
	try {
		if( TrueTypeFace )
			TrueTypeFace->drop();

		if( Driver )
			Driver->drop();

		clearGlyphs();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::~CGUIFreetypeFont(): " << e.what() << std::endl;
	}
}

bool CGUIFreetypeFont::attach( CGUITTFace *Face, u32 size ) {
	try {
		if( !Driver || !Face )
			return false;

		Face->grab();

		if( TrueTypeFace )
			TrueTypeFace->drop();

		TrueTypeFace = Face;

		if( !TrueTypeFace )
			return false;

		clearGlyphs();
		Glyphs.reallocate( TrueTypeFace->face->num_glyphs );
		Glyphs.set_used( TrueTypeFace->face->num_glyphs );

		for( int i = 0; i < TrueTypeFace->face->num_glyphs; ++i ) {
			CGUITTGlyph * glyph = new CGUITTGlyph();

			glyph->size = size;
	//		glyph->cache((wchar_t)i + 1);

			Glyphs[i] = glyph;
		}

		// TODO: this is a workaround to get a probably ok height for getDimensions. So we check a few extreme characters which usually make trouble.
		getGlyphByChar( L'A' );
		getGlyphByChar( L'g' );
		getGlyphByChar( L'.' );
		getGlyphByChar( L'(' );

		return	true;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::attach(): " << e.what() << std::endl;
		return false;
	}
}

void CGUIFreetypeFont::clearGlyphs() {
	try {
		for( unsigned int i = 0; i < Glyphs.size(); ++i ) {
			if( Glyphs[i] ) {
				Glyphs[i]->drop();
			}

			Glyphs[i] = 0;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::clearGlyphs(): " << e.what() << std::endl;
	}
}

u32 CGUIFreetypeFont::getGlyphByChar( wchar_t c ) const {
	try {
		u32 idx = FT_Get_Char_Index( TrueTypeFace->face, c );

		if( idx && !Glyphs[idx - 1]->cached )
			Glyphs[idx - 1]->cache( idx, this );

		return	idx;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::getGlyphByChar(): " << e.what() << std::endl;
		return 0;
	}
}

//! returns the dimension of a text
core::dimension2d< u32 > CGUIFreetypeFont::getDimension( const wchar_t* text ) const {
	try {
		core::dimension2d< u32 > dim( 0, Glyphs[0]->size );

		for( const wchar_t* p = text; *p; ++p ) {
			dim.Width += getWidthFromCharacter( *p );
		}

		// TODO: The correct solution might be working with TrueTypeFace->height but I can't figure out how to use units_per_EM
		// even if I know which FT_Render_Mode I used. I'm sure there is some way to figure that out, but I have to give up for now.
		if( TrueTypeFace && LargestGlyph.Height > dim.Height )
			dim.Height = LargestGlyph.Height;

		return dim;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::getDimension(): " << e.what() << std::endl;
		return core::dimension2d< u32 >();
	}
}


inline u32 CGUIFreetypeFont::getWidthFromCharacter( wchar_t c ) const {
	try {
		u16 n = getGlyphByChar( c );

		if( n > 0 ) {
			int w = Glyphs[n - 1]->texw;
			int_least16_t left = Glyphs[n - 1]->left;

			if( w + left > 0 )
				return w + left;
		}

		if( c >= 0x2000 ) {
			return	Glyphs[0]->size;
		} else {
			return	Glyphs[0]->size / 2;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::getWidthFromCharacter(): " << e.what() << std::endl;
		return 0;
	}
}


//! draws an text and clips it to the specified rectangle if wanted
void CGUIFreetypeFont::draw( const irr::core::stringw& textstring, const irr::core::rect< irr::s32 >& position, video::SColor color, bool hcenter, bool vcenter, const core::rect< irr::s32 >* clip ) {
	try {
		if( !Driver )
			return;

		core::dimension2d< irr::s32 > textDimension;
		core::position2d< irr::s32 > offset = position.UpperLeftCorner;
		video::SColor colors[4];

		for( int i = 0; i < 4; ++i ) {
			colors[i] = color;
		}

		const wchar_t * text = textstring.c_str();

		if( hcenter || vcenter ) {
			textDimension = getDimension( text );

			if( hcenter )
				offset.X = (( position.getWidth() - textDimension.Width ) >> 1 ) + offset.X;

			if( vcenter )
				offset.Y = (( position.getHeight() - textDimension.Height ) >> 1 ) + offset.Y;
		}

		u32 n;

		while( *text ) {
			n = getGlyphByChar( *text );

			if( n > 0 ) {
				if( AntiAlias ) {
	//				s32 imgw = Glyphs[n-1]->imgw;
	//				s32 imgh = Glyphs[n-1]->imgh;
					s32 texw = Glyphs[n-1]->texw;
					s32 texh = Glyphs[n-1]->texh;
					s32 offx = Glyphs[n-1]->left;
					s32 offy = Glyphs[n-1]->size - Glyphs[n-1]->top;

					if( Driver->getDriverType() != video::EDT_SOFTWARE ) {
						if( !Transparency )
							color.color |= 0xff000000;

						Driver->draw2DImage( Glyphs[n-1]->tex, core::position2d< irr::s32 >( offset.X + offx, offset.Y + offy ), core::rect< irr::s32 >( 0, 0, texw, texh ), clip, color, true );
					} else {
						s32 a = color.getAlpha();
						s32 r = color.getRed();
						s32 g = color.getGreen();
						s32 b = color.getBlue();
						u8 *pt = Glyphs[n-1]->image;

						if( !Transparency )	a = 255;

						for( int y = 0; y < texh; ++y ) {
							for( int x = 0; x < texw; x++ ) {
								if( !clip || clip->isPointInside( core::position2d< irr::s32 >( offset.X + x + offx, offset.Y + y + offy ) ) ) {
									if( *pt ) {
										Driver->draw2DRectangle( video::SColor(( a * *pt ) / 255, r, g, b ), core::rect< irr::s32 >( offset.X + x + offx, offset.Y + y + offy, offset.X + x + offx + 1, offset.Y + y + offy + 1 ) );
									}

									pt++;
								}
							}
						}
					}
				} else {
	//				s32 imgw = Glyphs[n-1]->imgw16;
	//				s32 imgh = Glyphs[n-1]->imgh16;
					s32 texw = Glyphs[n-1]->texw16;
					s32 texh = Glyphs[n-1]->texh16;
					s32 offx = Glyphs[n-1]->left16;
					s32 offy = Glyphs[n-1]->size - Glyphs[n-1]->top16;

					if( !Transparency ) {
						color.color |= 0xff000000;
					}

					Driver->draw2DImage( Glyphs[n-1]->tex16,
										 core::position2d< irr::s32 >( offset.X + offx, offset.Y + offy ),
										 core::rect< irr::s32 >( 0, 0, texw, texh ),
										 clip, color, true );
				}

				offset.X += getWidthFromCharacter( *text );
			} else {
				offset.X += getWidthFromCharacter( *text );
			}

			++text;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::draw(): " << e.what() << std::endl;
	}
}

//! Calculates the index of the character in the text which is on a specific position.
irr::s32  CGUIFreetypeFont::getCharacterFromPos( const wchar_t* text, irr::s32 pixel_x ) const {
	try {
		irr::s32 x = 0;
		irr::s32 idx = 0;

		while( text[idx] ) {
			x += getWidthFromCharacter( text[idx] );

			if( x >= pixel_x )
				return idx;

			++idx;
		}

		return -1;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in CGUIFreetypeFont::getCharacterFromPos(): " << e.what() << std::endl;
		return -1;
	}
}

#endif // #if COMPILE_WITH_FREETYPE
