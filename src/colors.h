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

#define NUMCOLORS 16 //Does not count INVISIBLE or the different color pallettes

#define BLACK irr::video::SColor( 255, 0, 0, 0 )
#define BLUE irr::video::SColor( 255, 0, 0, 170 )
#define GREEN irr::video::SColor( 255, 0, 170, 0 )
#define CYAN irr::video::SColor( 255, 0, 170, 170 )
#define RED irr::video::SColor( 255, 170, 0, 0 )
#define MAGENTA irr::video::SColor( 255, 170, 0, 170 )
#define BROWN irr::video::SColor( 255, 170, 85, 0 )
#define GRAY irr::video::SColor( 255, 85, 85, 85 )
#define LIGHTGRAY irr::video::SColor( 255, 170, 170, 170 )
#define LIGHTBLUE irr::video::SColor( 255, 85, 85, 255 )
#define LIGHTGREEN irr::video::SColor( 255, 85, 255, 85 )
#define LIGHTCYAN irr::video::SColor( 255, 85, 255, 255 )
#define LIGHTRED irr::video::SColor( 255, 255, 85, 85 )
#define LIGHTMAGENTA irr::video::SColor( 255, 255, 85, 255 )
#define YELLOW irr::video::SColor( 255, 255, 255, 85 )
#define WHITE irr::video::SColor( 255, 255, 255, 255 )
#define INVISIBLE irr::video::SColor( 0, 0, 0, 0 )

//Grayscale conversions:
//Converted using the luminosity method described here: http://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
//Luminosity = round( 0.21*Red + 0.72*Green + 0.07*Blue )
#define BLACK_GRAYSCALE irr::video::SColor( 255, 0, 0, 0 )
#define BLUE_GRAYSCALE irr::video::SColor( 255, 12, 12, 12 )
#define GREEN_GRAYSCALE irr::video::SColor( 255, 122, 122, 122 )
#define CYAN_GRAYSCALE irr::video::SColor( 255, 134, 134, 134 )
#define RED_GRAYSCALE irr::video::SColor( 255, 36, 36, 36 )
#define MAGENTA_GRAYSCALE irr::video::SColor( 255, 48, 48, 48 )
#define BROWN_GRAYSCALE irr::video::SColor( 255, 97, 97, 97 )
#define GRAY_GRAYSCALE irr::video::SColor( 255, 85, 85, 85 )
#define LIGHTGRAY_GRAYSCALE irr::video::SColor( 255, 170, 170, 170 )
#define LIGHTBLUE_GRAYSCALE irr::video::SColor( 255, 97, 97, 97 )
#define LIGHTGREEN_GRAYSCALE irr::video::SColor( 255, 207, 207, 207 )
#define LIGHTCYAN_GRAYSCALE irr::video::SColor( 255, 219, 219, 219 )
#define LIGHTRED_GRAYSCALE irr::video::SColor( 255, 121, 121, 121 )
#define LIGHTMAGENTA_GRAYSCALE irr::video::SColor( 255, 133, 133, 133 )
#define YELLOW_GRAYSCALE irr::video::SColor( 255, 243, 243, 243 )
#define WHITE_GRAYSCALE irr::video::SColor( 255, 255, 255, 255 )
#define INVISIBLE_GRAYSCALE irr::video::SColor( 0, 0, 0, 0 )

//Greenscale conversions:
//Just the grayscale versions, but with red and blue channels set to zero
#define BLACK_GREENSCALE irr::video::SColor( 255, 0, 0, 0 )
#define BLUE_GREENSCALE irr::video::SColor( 255, 0, 12, 0 )
#define GREEN_GREENSCALE irr::video::SColor( 255, 0, 122, 0 )
#define CYAN_GREENSCALE irr::video::SColor( 255, 0, 134, 0 )
#define RED_GREENSCALE irr::video::SColor( 255, 0, 36, 0 )
#define MAGENTA_GREENSCALE irr::video::SColor( 255, 0, 48, 0 )
#define BROWN_GREENSCALE irr::video::SColor( 255, 0, 97, 0 )
#define GRAY_GREENSCALE irr::video::SColor( 255, 0, 85, 0 )
#define LIGHTGRAY_GREENSCALE irr::video::SColor( 255, 0, 170, 0 )
#define LIGHTBLUE_GREENSCALE irr::video::SColor( 255, 0, 97, 0 )
#define LIGHTGREEN_GREENSCALE irr::video::SColor( 255, 0, 207, 0 )
#define LIGHTCYAN_GREENSCALE irr::video::SColor( 255, 0, 219, 0 )
#define LIGHTRED_GREENSCALE irr::video::SColor( 255, 0, 121, 0 )
#define LIGHTMAGENTA_GREENSCALE irr::video::SColor( 255, 0, 133, 0 )
#define YELLOW_GREENSCALE irr::video::SColor( 255, 0, 243, 0 )
#define WHITE_GREENSCALE irr::video::SColor( 255, 0, 255, 0 )
#define INVISIBLE_GREENSCALE irr::video::SColor( 0, 0, 0, 0 )

//Amberscale conversions:
//Just the grayscale versions, with the blue channel set to zero, true amber being outside the sRGB colorspace.
#define BLACK_AMBERSCALE irr::video::SColor( 255, 0, 0, 0 )
#define BLUE_AMBERSCALE irr::video::SColor( 255, 12, 12, 0 )
#define GREEN_AMBERSCALE irr::video::SColor( 255, 122, 122, 0 )
#define CYAN_AMBERSCALE irr::video::SColor( 255, 134, 134, 0 )
#define RED_AMBERSCALE irr::video::SColor( 255, 36, 36, 0 )
#define MAGENTA_AMBERSCALE irr::video::SColor( 255, 48, 48, 0 )
#define BROWN_AMBERSCALE irr::video::SColor( 255, 97, 97, 0 )
#define GRAY_AMBERSCALE irr::video::SColor( 255, 85, 85, 0 )
#define LIGHTGRAY_AMBERSCALE irr::video::SColor( 255, 170, 170, 0 )
#define LIGHTBLUE_AMBERSCALE irr::video::SColor( 255, 97, 97, 0 )
#define LIGHTGREEN_AMBERSCALE irr::video::SColor( 255, 207, 207, 0 )
#define LIGHTCYAN_AMBERSCALE irr::video::SColor( 255, 219, 219, 0 )
#define LIGHTRED_AMBERSCALE irr::video::SColor( 255, 121, 121, 0 )
#define LIGHTMAGENTA_AMBERSCALE irr::video::SColor( 255, 133, 133, 0 )
#define YELLOW_AMBERSCALE irr::video::SColor( 255, 243, 243, 0 )
#define WHITE_AMBERSCALE irr::video::SColor( 255, 255, 255, 0 )
#define INVISIBLE_AMBERSCALE irr::video::SColor( 0, 0, 0, 0 )

#endif // COLORS_H_INCLUDED
