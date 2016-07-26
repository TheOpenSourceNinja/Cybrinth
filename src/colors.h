/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2016.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed 'as is' in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of TITLE, MERCHANTABILITY, COMPLETE DESTRUCTION OF EVIL MONSTERS, or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * It's a list of colors. These are CGA colors. I'm kind of going for that look, but without the low screen resolution limit. See https://en.wikipedia.org/w/index.php?title=Color_Graphics_Adapter&oldid=621715104#Color_palette
 */

#ifndef COLORS_H_INCLUDED
#define COLORS_H_INCLUDED

#include <irrlicht/irrlicht.h>

#include "PreprocessorCommands.h"

#define BLACK irr::video::SColor( 255, 0, 0, 0 )
#define BLUE irr::video::SColor( 255, 0, 0, 170 )
#define GREEN irr::video::SColor( 255, 0, 170, 0 )
#define CYAN irr::video::SColor( 255, 0, 170, 170 )
#define RED irr::video::SColor( 255, 170, 0, 0 )
#define MAGENTA irr::video::SColor( 255, 170, 0, 170 )
#define BROWN irr::video::SColor( 255, 170, 85, 0 )
#define GRAY irr::video::SColor( 255, 85, 85, 85 )
#define GREY irr::video::SColor( 255, 85, 85, 85 )
#define LIGHTGRAY irr::video::SColor( 255, 170, 170, 170 )
#define LIGHTGREY irr::video::SColor( 255, 170, 170, 170 )
#define LIGHTBLUE irr::video::SColor( 255, 85, 85, 255 )
#define LIGHTGREEN irr::video::SColor( 255, 85, 255, 85 )
#define LIGHTCYAN irr::video::SColor( 255, 85, 255, 255 )
#define LIGHTRED irr::video::SColor( 255, 255, 85, 85 )
#define LIGHTMAGENTA irr::video::SColor( 255, 255, 85, 255 )
#define YELLOW irr::video::SColor( 255, 255, 255, 85 )
#define WHITE irr::video::SColor( 255, 255, 255, 255 )
#define INVISIBLE irr::video::SColor( 0, 0, 0, 0 )

#define NUMCOLORS 16 //Does not count duplicates (like grey/gray) or INVISIBLE

#endif // COLORS_H_INCLUDED
