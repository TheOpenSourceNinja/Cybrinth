#include "XPMImageLoader.h"
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#ifdef HAVE_IOSTREAM
	#include <iostream>
#endif //HAVE_IOSTREAM
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <map>

XPMImageLoader::XPMImageLoader() {
	//ctor
}

void XPMImageLoader::loadImageCommon( irr::video::IVideoDriver* driver, irr::video::IImage* storage, char ** xpm ) {
	if( xpm not_eq nullptr and driver not_eq nullptr and storage not_eq nullptr ) {
		std::string temp = std::string( xpm[ 0 ] );
		std::string widthString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string heightString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string numberOfColorsString = temp.substr( 0, temp.find( ' ' ) );
		temp = temp.substr( temp.find( ' ' ) + 1 );
		std::string charPerPixelString = temp;
		
		std::cout << widthString << std::endl;
		std::cout << heightString << std::endl;
		std::cout << numberOfColorsString << std::endl;
		std::cout << charPerPixelString << std::endl;
		
		irr::u32 width = std::stoul( widthString );
		irr::u32 height = std::stoul( heightString );
		uint32_t numberOfColors = std::stoul( numberOfColorsString );
		uint8_t charPerPixel = std::stoul( charPerPixelString );
		
		irr::video::ECOLOR_FORMAT format = irr::video::ECF_UNKNOWN;
		if( numberOfColors <= UINT16_MAX ) {
			format = irr::video::ECF_A1R5G5B5;
		} else {
			format = irr::video::ECF_A8R8G8B8;
		}
		
		std::map< std::string, irr::video::SColor > colorMap;
		
		irr::video::IImage* newImage = driver->createImage( format, irr::core::dimension2d< decltype( width ) >( width, height ) );
		for( decltype( numberOfColors ) i = 1; i <= numberOfColors; ++i ) {
			std::string kxpm = xpm[ i ];
			std::string id = kxpm.substr( 0, charPerPixel );
			
			colorType_t colorType = INVALID_DO_NOT_USE;
			auto pos = kxpm.find( 'c' ); //c means the image is in color
			if( pos == std::string::npos ) {
				pos = kxpm.find( 'g' ); //g means grayscale with more than 4 levels
				if( pos == std::string::npos or pos == kxpm.find( "g4" ) ) {
					pos = kxpm.find( "g4" ); //g4 means 4-level grayscale
					if( pos == std::string::npos ) {
						pos = kxpm.find( 'm' ); //m means monochrome
						if( pos == std::string::npos ) {
							pos = kxpm.find( 's' ); //s means "symbolic", which I guess means the names of colors.
							assert( pos != std::string::npos ); //Since I don't know for sure how to deal with symbolic colors, just produce an error for now. TODO: Deal with this properly
						} else {
							colorType = MONOCHROME; //The monochrome color type will probably never be used in this program: GIMP, even when an image is monochrome, will save it as "color".
						}
					} else {
						colorType = FOURLEVELGRAYSCALE;
					}
				} else {
					colorType = GRAYSCALE;
				}
			} else {
				colorType = COLOR;
			}
			std::string colorString = kxpm.substr( pos + 2 );
			colorString.erase( std::remove_if( colorString.begin(), colorString.end(), ::isspace ), colorString.end() );
			
			irr::video::SColor color;
			
			if( colorString == "None" ) {
				color.set( 0, 0, 0, 0 ); //Invisible
			} else if( colorString.find( '#' ) != std::string::npos ) {
				colorString = colorString.substr( colorString.find( '#' ) + 1 );
				std::string redString = colorString.substr( 0, 2 );
				std::string greenString = colorString.substr( 2, 2 );
				std::string blueString = colorString.substr( 4, 2 );
				
				irr::u32 red = stoul( redString, nullptr, 16 );
				irr::u32 green = stoul( greenString, nullptr, 16 );
				irr::u32 blue = stoul( blueString, nullptr, 16 );
				
				color.set( 255, red, green, blue );
			} else if( colorString.find( '%' ) != std::string::npos ) {
				//ALERT -- UNTESTED CODE -- I am assuming that HSV values are stored similarly to how RGB values are: as a symbol followed by two hex digits per channel. I have no way to test this since every XPM creator program I've tried stores RGB.
				colorString = colorString.substr( colorString.find( '%' ) + 1 );
				std::string hueString = colorString.substr( 0, 2 );
				std::string saturationString = colorString.substr( 2, 2 );
				std::string valueString = colorString.substr( 4, 2 );
				
				irr::f32 hue = std::atof( hueString.c_str() );
				irr::f32 saturation = std::atof( saturationString.c_str() );
				irr::f32 value = std::atof( valueString.c_str() );
				
				irr::video::SColorHSL colorHSL( hue, saturation, value ); //NOTE: I know there's a difference between the HSV and HSL colorspaces, but I just can't be bothered to program the conversion right now.
				irr::video::SColorf colorF;
				colorHSL.toRGB( colorF );
				color = colorF.toSColor();
			} else if( colorType == SYMBOLIC ) {
				//TODO: Figure out how to deal with symbolic color names
			} else {
				//If we get here, then we should be dealing with color names (which somehow are not the same as symbolic color names)
				boost::algorithm::to_lower( colorString );
				
				uint_fast8_t alpha = 255;
				//The following is based on the official rgb.txt from https://cgit.freedesktop.org/xorg/app/rgb/tree/rgb.txt
				//TODO: Finish copying this list.
				if( colorString == "snow" ) {
					color.set( alpha, 255, 250, 250 );
				} else if( colorString == "ghostwhite" ) {
					color.set( alpha, 248, 248, 255 );
				} else if( colorString == "whitesmoke" ) {
					color.set( alpha, 245, 245, 245 );
				} else if( colorString == "gainsboro" ) {
					color.set( alpha, 220, 220, 220 );
				} else if( colorString == "floralwhite" ) {
					color.set( alpha, 255, 250, 240 );
				} else if( colorString == "oldlace" ) {
					color.set( alpha, 248, 248, 255 );
				} else if( colorString == "linen" ) {
					color.set( alpha, 250, 240, 230 );
				} else if( colorString == "antiquewhite" ) {
					color.set( alpha, 250, 235, 215 );
				} else if( colorString == "papayawhip" ) {
					color.set( alpha, 255, 239, 213 );
				} else if( colorString == "blanchedalmond" ) {
					color.set( alpha, 255, 235, 205 );
				} else if( colorString == "bisque" ) {
					color.set( alpha, 255, 228, 196 );
				} else if( colorString == "peachpuff" ) {
					color.set( alpha, 255, 218, 185 );
				} else if( colorString == "navajowhite" ) {
					color.set( alpha, 255, 222, 173 );
				} else if( colorString == "moccasin" ) {
					color.set( alpha, 255, 235, 205 );
				} else if( colorString == "cornsilk" ) {
					color.set( alpha, 255, 248, 220 );
				} else if( colorString == "ivory" ) {
					color.set( alpha, 255, 255, 240 );
				} else if( colorString == "lemonchiffon" ) {
					color.set( alpha, 255, 250, 205 );
				} else if( colorString == "seashell" ) {
					color.set( alpha, 255, 245, 238 );
				} else if( colorString == "honeydew" ) {
					color.set( alpha, 240, 255, 240 );
				} else if( colorString == "mintcream" ) {
					color.set( alpha, 245, 255, 250 );
				} else if( colorString == "azure" ) {
					color.set( alpha, 240, 255, 255 );
				} else if( colorString == "aliceblue" ) {
					color.set( alpha, 240, 248, 255 );
				} else if( colorString == "lavender" ) {
					color.set( alpha, 230, 230, 250 );
				} else if( colorString == "lavenderblush" ) {
					color.set( alpha, 255, 240, 245 );
				} else if( colorString == "mistyrose" ) {
					color.set( alpha, 255, 228, 225 );
				} else if( colorString == "white" ) {
					color.set( alpha, 255, 255, 255 );
				} else if( colorString == "black" ) {
					color.set( alpha, 0, 0, 0 );
				} else if( colorString == "darkslategray" or colorString == "darkslategrey" ) {
					color.set( alpha, 47, 79, 79 );
				} else if( colorString == "dimgray" or colorString == "dimgrey" ) {
					color.set( alpha, 105, 105, 105 );
				} else if( colorString == "slategray" or colorString == "slategrey" ) {
					color.set( alpha, 112, 128, 144 );
				} else if( colorString == "lightslategray" or colorString == "lightslategrey" ) {
					color.set( alpha, 119, 136, 153 );
				} else if( colorString == "gray" or colorString == "grey" or colorString == "x11gray" or colorString == "x11grey" ) {
					color.set( alpha, 190, 190, 190 );
				} else if( colorString == "webgray" or colorString == "webgrey" ) {
					color.set( alpha, 128, 128, 128 );
				} else if( colorString == "lightgray" or colorString == "lightgrey" ) {
					color.set( alpha, 211, 211, 211 );
				} else if( colorString == "midnightblue" ) {
					color.set( alpha, 25, 25, 112 );
				} else if( colorString == "navy" or colorString == "navyblue" ) {
					color.set( alpha, 0, 0, 128 );
				} else if( colorString == "cornflowerblue" ) {
					color.set( alpha, 100, 149, 237 );
				} else if( colorString == "darkslateblue" ) {
					color.set( alpha, 72, 61, 139 );
				} else if( colorString == "slateblue" ) {
					color.set( alpha, 106, 90, 205 );
				} else if( colorString == "mediumslateblue" ) {
					color.set( alpha, 123, 104, 238 );
				} else if( colorString == "lightslateblue" ) {
					color.set( alpha, 132, 112, 255 );
				} else if( colorString == "mediumblue" ) {
					color.set( alpha, 0, 0, 205 );
				} else if( colorString == "royalblue" ) {
					color.set( alpha, 65, 105, 225 );
				} else if( colorString == "blue" ) {
					color.set( alpha, 0, 0, 255 );
				} else if( colorString == "dodgerblue" ) {
					color.set( alpha, 30, 144, 255 );
				} else if( colorString == "deepskyblue" ) {
					color.set( alpha, 0, 191, 255 );
				} else if( colorString == "skyblue" ) {
					color.set( alpha, 135, 206, 235 );
				} else if( colorString == "lightskyblue" ) {
					color.set( alpha, 135, 206, 250 );
				} else if( colorString == "steelblue" ) {
					color.set( alpha, 70, 130, 180 );
				} else if( colorString == "lightsteelblue" ) {
					color.set( alpha, 176, 196, 222 );
				} else if( colorString == "lightblue" ) {
					color.set( alpha, 173, 216, 230 );
				} else if( colorString == "powderblue" ) {
					color.set( alpha, 176, 224, 230 );
				} else if( colorString == "paleturquoise" ) {
					color.set( alpha, 175, 238, 238 );
				} else if( colorString == "darkturquoise" ) {
					color.set( alpha, 0, 206, 209 );
				} else if( colorString == "mediumturquoise" ) {
					color.set( alpha, 72, 209, 204 );
				} else if( colorString == "turquoise" ) {
					color.set( alpha, 64, 224, 208 );
				} else if( colorString == "cyan" or colorString == "aqua" ) {
					color.set( alpha, 0, 255, 255 );
				} else if( colorString == "lightcyan" ) {
					color.set( alpha, 224, 255, 255 );
				} else if( colorString == "cadetblue" ) {
					color.set( alpha, 95, 158, 160 );
				} else if( colorString == "mediumaquamarine" ) {
					color.set( alpha, 102, 205, 170 );
				} else if( colorString == "aquamarine" ) {
					color.set( alpha, 127, 255, 212 );
				} else if( colorString == "darkgreen" ) {
					color.set( alpha, 0, 100, 0 );
				} else if( colorString == "darkolivegreen" ) {
					color.set( alpha, 85, 107, 47 );
				} else if( colorString == "darkseagreen" ) {
					color.set( alpha, 143, 188, 143 );
				} else if( colorString == "seagreen" ) {
					color.set( alpha, 46, 139, 87 );
				} else if( colorString == "mediumseagreen" ) {
					color.set( alpha, 60, 179, 113 );
				} else if( colorString == "lightseagreen" ) {
					color.set( alpha, 32, 178, 170 );
				} else if( colorString == "palegreen" ) {
					color.set( alpha, 152, 251, 152 );
				} else if( colorString == "springgreen" ) {
					color.set( alpha, 0, 255, 127 );
				} else if( colorString == "lawngreen" ) {
					color.set( alpha, 124, 252, 0 );
				} else if( colorString == "green" or colorString == "lime" or colorString == "x11green" ) {
					color.set( alpha, 0, 255, 0 );
				} else if( colorString == "webgreen" ) {
					color.set( alpha, 0, 128, 0 );
				} else if( colorString == "chartreuse" ) {
					color.set( alpha, 127, 255, 0 );
				} else if( colorString == "mediumspringgreen" ) {
					color.set( alpha, 0, 250, 154 );
				} else if( colorString == "greenyellow" ) {
					color.set( alpha, 173, 255, 47 );
				} else if( colorString == "limegreen" ) {
					color.set( alpha, 50, 205, 50 );
				} else if( colorString == "yellowgreen" ) {
					color.set( alpha, 154, 205, 50 );
				} else if( colorString == "forestgreen" ) {
					color.set( alpha, 34, 139, 34 );
				} else if( colorString == "olivedrab" ) {
					color.set( alpha, 107, 142, 35 );
				} else if( colorString == "darkkhaki" ) {
					color.set( alpha, 189, 183, 107 );
				} else if( colorString == "khaki" ) {
					color.set( alpha, 240, 230, 140 );
				} else if( colorString == "palegoldenrod" ) {
					color.set( alpha, 238, 232, 170 );
				} else if( colorString == "lightgoldenrodyellow" ) {
					color.set( alpha, 250, 250, 210 );
				} else if( colorString == "lightyellow" ) {
					color.set( alpha, 255, 255, 224 );
				} else if( colorString == "yellow" ) {
					color.set( alpha, 255, 255, 0 );
				} else if( colorString == "gold" ) {
					color.set( alpha, 255, 215, 0 );
				} else if( colorString == "lightgoldenrod" ) {
					color.set( alpha, 238, 221, 130 );
				} else if( colorString == "goldenrod" ) {
					color.set( alpha, 218, 165, 32 );
				} else if( colorString == "darkgoldenrod" ) {
					color.set( alpha, 184, 134, 11 );
				} else if( colorString == "rosybrown" ) {
					color.set( alpha, 188, 143, 143 );
				} else if( colorString == "indianred" ) {
					color.set( alpha, 205, 92, 92 );
				} else if( colorString == "saddlebrown" ) {
					color.set( alpha, 139, 69, 19 );
				} else if( colorString == "sienna" ) {
					color.set( alpha, 160, 82, 45 );
				} else if( colorString == "peru" ) {
					color.set( alpha, 205, 133, 63 );
				}
				
				/* Remaining colors:
222 184 135		burlywood
245 245 220		beige
245 222 179		wheat
244 164  96		sandy brown
244 164  96		SandyBrown
210 180 140		tan
210 105  30		chocolate
178  34  34		firebrick
165  42  42		brown
233 150 122		dark salmon
233 150 122		DarkSalmon
250 128 114		salmon
255 160 122		light salmon
255 160 122		LightSalmon
255 165   0		orange
255 140   0		dark orange
255 140   0		DarkOrange
255 127  80		coral
240 128 128		light coral
240 128 128		LightCoral
255  99  71		tomato
255  69   0		orange red
255  69   0		OrangeRed
255   0   0		red
255 105 180		hot pink
255 105 180		HotPink
255  20 147		deep pink
255  20 147		DeepPink
255 192 203		pink
255 182 193		light pink
255 182 193		LightPink
219 112 147		pale violet red
219 112 147		PaleVioletRed
176  48  96		maroon
176  48  96		x11 maroon
176  48  96		X11Maroon
128   0   0		web maroon
128   0   0		WebMaroon
199  21 133		medium violet red
199  21 133		MediumVioletRed
208  32 144		violet red
208  32 144		VioletRed
255   0 255		magenta
255   0 255		fuchsia
238 130 238		violet
221 160 221		plum
218 112 214		orchid
186  85 211		medium orchid
186  85 211		MediumOrchid
153  50 204		dark orchid
153  50 204		DarkOrchid
148   0 211		dark violet
148   0 211		DarkViolet
138  43 226		blue violet
138  43 226		BlueViolet
160  32 240		purple
160  32 240		x11 purple
160  32 240		X11Purple
128   0 128		web purple
128   0 128		WebPurple
147 112 219		medium purple
147 112 219		MediumPurple
216 191 216		thistle
255 250 250		snow1
238 233 233		snow2
205 201 201		snow3
139 137 137		snow4
255 245 238		seashell1
238 229 222		seashell2
205 197 191		seashell3
139 134 130		seashell4
255 239 219		AntiqueWhite1
238 223 204		AntiqueWhite2
205 192 176		AntiqueWhite3
139 131 120		AntiqueWhite4
255 228 196		bisque1
238 213 183		bisque2
205 183 158		bisque3
139 125 107		bisque4
255 218 185		PeachPuff1
238 203 173		PeachPuff2
205 175 149		PeachPuff3
139 119 101		PeachPuff4
255 222 173		NavajoWhite1
238 207 161		NavajoWhite2
205 179 139		NavajoWhite3
139 121	 94		NavajoWhite4
255 250 205		LemonChiffon1
238 233 191		LemonChiffon2
205 201 165		LemonChiffon3
139 137 112		LemonChiffon4
255 248 220		cornsilk1
238 232 205		cornsilk2
205 200 177		cornsilk3
139 136 120		cornsilk4
255 255 240		ivory1
238 238 224		ivory2
205 205 193		ivory3
139 139 131		ivory4
240 255 240		honeydew1
224 238 224		honeydew2
193 205 193		honeydew3
131 139 131		honeydew4
255 240 245		LavenderBlush1
238 224 229		LavenderBlush2
205 193 197		LavenderBlush3
139 131 134		LavenderBlush4
255 228 225		MistyRose1
238 213 210		MistyRose2
205 183 181		MistyRose3
139 125 123		MistyRose4
240 255 255		azure1
224 238 238		azure2
193 205 205		azure3
131 139 139		azure4
131 111 255		SlateBlue1
122 103 238		SlateBlue2
105  89 205		SlateBlue3
 71  60 139		SlateBlue4
 72 118 255		RoyalBlue1
 67 110 238		RoyalBlue2
 58  95 205		RoyalBlue3
 39  64 139		RoyalBlue4
  0   0 255		blue1
  0   0 238		blue2
  0   0 205		blue3
  0   0 139		blue4
 30 144 255		DodgerBlue1
 28 134 238		DodgerBlue2
 24 116 205		DodgerBlue3
 16  78 139		DodgerBlue4
 99 184 255		SteelBlue1
 92 172 238		SteelBlue2
 79 148 205		SteelBlue3
 54 100 139		SteelBlue4
  0 191 255		DeepSkyBlue1
  0 178 238		DeepSkyBlue2
  0 154 205		DeepSkyBlue3
  0 104 139		DeepSkyBlue4
135 206 255		SkyBlue1
126 192 238		SkyBlue2
108 166 205		SkyBlue3
 74 112 139		SkyBlue4
176 226 255		LightSkyBlue1
164 211 238		LightSkyBlue2
141 182 205		LightSkyBlue3
 96 123 139		LightSkyBlue4
198 226 255		SlateGray1
185 211 238		SlateGray2
159 182 205		SlateGray3
108 123 139		SlateGray4
202 225 255		LightSteelBlue1
188 210 238		LightSteelBlue2
162 181 205		LightSteelBlue3
110 123 139		LightSteelBlue4
191 239 255		LightBlue1
178 223 238		LightBlue2
154 192 205		LightBlue3
104 131 139		LightBlue4
224 255 255		LightCyan1
209 238 238		LightCyan2
180 205 205		LightCyan3
122 139 139		LightCyan4
187 255 255		PaleTurquoise1
174 238 238		PaleTurquoise2
150 205 205		PaleTurquoise3
102 139 139		PaleTurquoise4
152 245 255		CadetBlue1
142 229 238		CadetBlue2
122 197 205		CadetBlue3
 83 134 139		CadetBlue4
  0 245 255		turquoise1
  0 229 238		turquoise2
  0 197 205		turquoise3
  0 134 139		turquoise4
  0 255 255		cyan1
  0 238 238		cyan2
  0 205 205		cyan3
  0 139 139		cyan4
151 255 255		DarkSlateGray1
141 238 238		DarkSlateGray2
121 205 205		DarkSlateGray3
 82 139 139		DarkSlateGray4
127 255 212		aquamarine1
118 238 198		aquamarine2
102 205 170		aquamarine3
 69 139 116		aquamarine4
193 255 193		DarkSeaGreen1
180 238 180		DarkSeaGreen2
155 205 155		DarkSeaGreen3
105 139 105		DarkSeaGreen4
 84 255 159		SeaGreen1
 78 238 148		SeaGreen2
 67 205 128		SeaGreen3
 46 139	 87		SeaGreen4
154 255 154		PaleGreen1
144 238 144		PaleGreen2
124 205 124		PaleGreen3
 84 139	 84		PaleGreen4
  0 255 127		SpringGreen1
  0 238 118		SpringGreen2
  0 205 102		SpringGreen3
  0 139	 69		SpringGreen4
  0 255	  0		green1
  0 238	  0		green2
  0 205	  0		green3
  0 139	  0		green4
127 255	  0		chartreuse1
118 238	  0		chartreuse2
102 205	  0		chartreuse3
 69 139	  0		chartreuse4
192 255	 62		OliveDrab1
179 238	 58		OliveDrab2
154 205	 50		OliveDrab3
105 139	 34		OliveDrab4
202 255 112		DarkOliveGreen1
188 238 104		DarkOliveGreen2
162 205	 90		DarkOliveGreen3
110 139	 61		DarkOliveGreen4
255 246 143		khaki1
238 230 133		khaki2
205 198 115		khaki3
139 134	 78		khaki4
255 236 139		LightGoldenrod1
238 220 130		LightGoldenrod2
205 190 112		LightGoldenrod3
139 129	 76		LightGoldenrod4
255 255 224		LightYellow1
238 238 209		LightYellow2
205 205 180		LightYellow3
139 139 122		LightYellow4
255 255	  0		yellow1
238 238	  0		yellow2
205 205	  0		yellow3
139 139	  0		yellow4
255 215	  0		gold1
238 201	  0		gold2
205 173	  0		gold3
139 117	  0		gold4
255 193	 37		goldenrod1
238 180	 34		goldenrod2
205 155	 29		goldenrod3
139 105	 20		goldenrod4
255 185	 15		DarkGoldenrod1
238 173	 14		DarkGoldenrod2
205 149	 12		DarkGoldenrod3
139 101	  8		DarkGoldenrod4
255 193 193		RosyBrown1
238 180 180		RosyBrown2
205 155 155		RosyBrown3
139 105 105		RosyBrown4
255 106 106		IndianRed1
238  99	 99		IndianRed2
205  85	 85		IndianRed3
139  58	 58		IndianRed4
255 130	 71		sienna1
238 121	 66		sienna2
205 104	 57		sienna3
139  71	 38		sienna4
255 211 155		burlywood1
238 197 145		burlywood2
205 170 125		burlywood3
139 115	 85		burlywood4
255 231 186		wheat1
238 216 174		wheat2
205 186 150		wheat3
139 126 102		wheat4
255 165	 79		tan1
238 154	 73		tan2
205 133	 63		tan3
139  90	 43		tan4
255 127	 36		chocolate1
238 118	 33		chocolate2
205 102	 29		chocolate3
139  69	 19		chocolate4
255  48	 48		firebrick1
238  44	 44		firebrick2
205  38	 38		firebrick3
139  26	 26		firebrick4
255  64	 64		brown1
238  59	 59		brown2
205  51	 51		brown3
139  35	 35		brown4
255 140 105		salmon1
238 130	 98		salmon2
205 112	 84		salmon3
139  76	 57		salmon4
255 160 122		LightSalmon1
238 149 114		LightSalmon2
205 129	 98		LightSalmon3
139  87	 66		LightSalmon4
255 165	  0		orange1
238 154	  0		orange2
205 133	  0		orange3
139  90	  0		orange4
255 127	  0		DarkOrange1
238 118	  0		DarkOrange2
205 102	  0		DarkOrange3
139  69	  0		DarkOrange4
255 114	 86		coral1
238 106	 80		coral2
205  91	 69		coral3
139  62	 47		coral4
255  99	 71		tomato1
238  92	 66		tomato2
205  79	 57		tomato3
139  54	 38		tomato4
255  69	  0		OrangeRed1
238  64	  0		OrangeRed2
205  55	  0		OrangeRed3
139  37	  0		OrangeRed4
255   0	  0		red1
238   0	  0		red2
205   0	  0		red3
139   0	  0		red4
255  20 147		DeepPink1
238  18 137		DeepPink2
205  16 118		DeepPink3
139  10	 80		DeepPink4
255 110 180		HotPink1
238 106 167		HotPink2
205  96 144		HotPink3
139  58  98		HotPink4
255 181 197		pink1
238 169 184		pink2
205 145 158		pink3
139  99 108		pink4
255 174 185		LightPink1
238 162 173		LightPink2
205 140 149		LightPink3
139  95 101		LightPink4
255 130 171		PaleVioletRed1
238 121 159		PaleVioletRed2
205 104 137		PaleVioletRed3
139  71	 93		PaleVioletRed4
255  52 179		maroon1
238  48 167		maroon2
205  41 144		maroon3
139  28	 98		maroon4
255  62 150		VioletRed1
238  58 140		VioletRed2
205  50 120		VioletRed3
139  34	 82		VioletRed4
255   0 255		magenta1
238   0 238		magenta2
205   0 205		magenta3
139   0 139		magenta4
255 131 250		orchid1
238 122 233		orchid2
205 105 201		orchid3
139  71 137		orchid4
255 187 255		plum1
238 174 238		plum2
205 150 205		plum3
139 102 139		plum4
224 102 255		MediumOrchid1
209  95 238		MediumOrchid2
180  82 205		MediumOrchid3
122  55 139		MediumOrchid4
191  62 255		DarkOrchid1
178  58 238		DarkOrchid2
154  50 205		DarkOrchid3
104  34 139		DarkOrchid4
155  48 255		purple1
145  44 238		purple2
125  38 205		purple3
 85  26 139		purple4
171 130 255		MediumPurple1
159 121 238		MediumPurple2
137 104 205		MediumPurple3
 93  71 139		MediumPurple4
255 225 255		thistle1
238 210 238		thistle2
205 181 205		thistle3
139 123 139		thistle4
  0   0   0		gray0
  0   0   0		grey0
  3   3   3		gray1
  3   3   3		grey1
  5   5   5		gray2
  5   5   5		grey2
  8   8   8		gray3
  8   8   8		grey3
 10  10  10 		gray4
 10  10  10 		grey4
 13  13  13 		gray5
 13  13  13 		grey5
 15  15  15 		gray6
 15  15  15 		grey6
 18  18  18 		gray7
 18  18  18 		grey7
 20  20  20 		gray8
 20  20  20 		grey8
 23  23  23 		gray9
 23  23  23 		grey9
 26  26  26 		gray10
 26  26  26 		grey10
 28  28  28 		gray11
 28  28  28 		grey11
 31  31  31 		gray12
 31  31  31 		grey12
 33  33  33 		gray13
 33  33  33 		grey13
 36  36  36 		gray14
 36  36  36 		grey14
 38  38  38 		gray15
 38  38  38 		grey15
 41  41  41 		gray16
 41  41  41 		grey16
 43  43  43 		gray17
 43  43  43 		grey17
 46  46  46 		gray18
 46  46  46 		grey18
 48  48  48 		gray19
 48  48  48 		grey19
 51  51  51 		gray20
 51  51  51 		grey20
 54  54  54 		gray21
 54  54  54 		grey21
 56  56  56 		gray22
 56  56  56 		grey22
 59  59  59 		gray23
 59  59  59 		grey23
 61  61  61 		gray24
 61  61  61 		grey24
 64  64  64 		gray25
 64  64  64 		grey25
 66  66  66 		gray26
 66  66  66 		grey26
 69  69  69 		gray27
 69  69  69 		grey27
 71  71  71 		gray28
 71  71  71 		grey28
 74  74  74 		gray29
 74  74  74 		grey29
 77  77  77 		gray30
 77  77  77 		grey30
 79  79  79 		gray31
 79  79  79 		grey31
 82  82  82 		gray32
 82  82  82 		grey32
 84  84  84 		gray33
 84  84  84 		grey33
 87  87  87 		gray34
 87  87  87 		grey34
 89  89  89 		gray35
 89  89  89 		grey35
 92  92  92 		gray36
 92  92  92 		grey36
 94  94  94 		gray37
 94  94  94 		grey37
 97  97  97 		gray38
 97  97  97 		grey38
 99  99  99 		gray39
 99  99  99 		grey39
102 102 102 		gray40
102 102 102 		grey40
105 105 105 		gray41
105 105 105 		grey41
107 107 107 		gray42
107 107 107 		grey42
110 110 110 		gray43
110 110 110 		grey43
112 112 112 		gray44
112 112 112 		grey44
115 115 115 		gray45
115 115 115 		grey45
117 117 117 		gray46
117 117 117 		grey46
120 120 120 		gray47
120 120 120 		grey47
122 122 122 		gray48
122 122 122 		grey48
125 125 125 		gray49
125 125 125 		grey49
127 127 127 		gray50
127 127 127 		grey50
130 130 130 		gray51
130 130 130 		grey51
133 133 133 		gray52
133 133 133 		grey52
135 135 135 		gray53
135 135 135 		grey53
138 138 138 		gray54
138 138 138 		grey54
140 140 140 		gray55
140 140 140 		grey55
143 143 143 		gray56
143 143 143 		grey56
145 145 145 		gray57
145 145 145 		grey57
148 148 148 		gray58
148 148 148 		grey58
150 150 150 		gray59
150 150 150 		grey59
153 153 153 		gray60
153 153 153 		grey60
156 156 156 		gray61
156 156 156 		grey61
158 158 158 		gray62
158 158 158 		grey62
161 161 161 		gray63
161 161 161 		grey63
163 163 163 		gray64
163 163 163 		grey64
166 166 166 		gray65
166 166 166 		grey65
168 168 168 		gray66
168 168 168 		grey66
171 171 171 		gray67
171 171 171 		grey67
173 173 173 		gray68
173 173 173 		grey68
176 176 176 		gray69
176 176 176 		grey69
179 179 179 		gray70
179 179 179 		grey70
181 181 181 		gray71
181 181 181 		grey71
184 184 184 		gray72
184 184 184 		grey72
186 186 186 		gray73
186 186 186 		grey73
189 189 189 		gray74
189 189 189 		grey74
191 191 191 		gray75
191 191 191 		grey75
194 194 194 		gray76
194 194 194 		grey76
196 196 196 		gray77
196 196 196 		grey77
199 199 199 		gray78
199 199 199 		grey78
201 201 201 		gray79
201 201 201 		grey79
204 204 204 		gray80
204 204 204 		grey80
207 207 207 		gray81
207 207 207 		grey81
209 209 209 		gray82
209 209 209 		grey82
212 212 212 		gray83
212 212 212 		grey83
214 214 214 		gray84
214 214 214 		grey84
217 217 217 		gray85
217 217 217 		grey85
219 219 219 		gray86
219 219 219 		grey86
222 222 222 		gray87
222 222 222 		grey87
224 224 224 		gray88
224 224 224 		grey88
227 227 227 		gray89
227 227 227 		grey89
229 229 229 		gray90
229 229 229 		grey90
232 232 232 		gray91
232 232 232 		grey91
235 235 235 		gray92
235 235 235 		grey92
237 237 237 		gray93
237 237 237 		grey93
240 240 240 		gray94
240 240 240 		grey94
242 242 242 		gray95
242 242 242 		grey95
245 245 245 		gray96
245 245 245 		grey96
247 247 247 		gray97
247 247 247 		grey97
250 250 250 		gray98
250 250 250 		grey98
252 252 252 		gray99
252 252 252 		grey99
255 255 255 		gray100
255 255 255 		grey100
169 169 169		dark grey
169 169 169		DarkGrey
169 169 169		dark gray
169 169 169		DarkGray
0     0 139		dark blue
0     0 139		DarkBlue
0   139 139		dark cyan
0   139 139		DarkCyan
139   0 139		dark magenta
139   0 139		DarkMagenta
139   0   0		dark red
139   0   0		DarkRed
144 238 144		light green
144 238 144		LightGreen
220  20  60		crimson
 75   0 130		indigo
128 128   0		olive
102  51 153		rebecca purple
102  51 153		RebeccaPurple
192 192 192		silver
  0 128 128		teal
   * */
			}
			colorMap[ id ] = color;
		}
		
		decltype( height ) lineNumberStart = numberOfColors + 1;
		for( decltype( height ) lineNumber = lineNumberStart; lineNumber < height + lineNumberStart; ++lineNumber ) {
			std::string line = xpm[ lineNumber ];
			for( decltype( width ) x = 0; x < width; ++x ) {
				std::string id = line.substr( x * charPerPixel, charPerPixel );
				newImage->setPixel( x, lineNumber - lineNumberStart, colorMap[ id ] );
			}
		}
		
		newImage->copyToScaling( storage );
	}
}

void XPMImageLoader::loadCollectableImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, Collectable::type_t type ) {
	
	char ** xpm = nullptr;
	switch( type ) {
		case Collectable::KEY: {
			#include "compiled-images/items/key.xpm"
			xpm = key_xpm;
			break;
		}
		case Collectable::ACID: {
			#include "compiled-images/items/acid.xpm"
			xpm = acid_xpm;
			break;
		}
		default: {
			break;
		}
	}
	
	loadImageCommon( driver, storage, xpm );
}

void XPMImageLoader::loadMenuOptionImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, MenuOption::option_t type ) {
	
	char ** xpm = nullptr;
	switch( type ) {
		case MenuOption::NEW_MAZE: {
			#include "compiled-images/menu icons/new_maze.xpm"
			xpm = new_maze_xpm;
			break;
		}
		case MenuOption::RESTART_MAZE: {
			#include "compiled-images/menu icons/restart_maze.xpm"
			xpm = restart_maze_xpm;
			break;
		}
		case MenuOption::LOAD_MAZE: {
			#include "compiled-images/menu icons/load_maze.xpm"
			xpm = load_maze_xpm;
			break;
		}
		case MenuOption::SAVE_MAZE: {
			#include "compiled-images/menu icons/save_maze.xpm"
			xpm = save_maze_xpm;
			break;
		}
		case MenuOption::SETTINGS: {
			#include "compiled-images/menu icons/settings.xpm"
			xpm = settings_xpm;
			break;
		}
		case MenuOption::EXIT_GAME: {
			#include "compiled-images/menu icons/exit_game.xpm"
			xpm = exit_game_xpm;
			break;
		}
		case MenuOption::BACK_TO_GAME: {
			#include "compiled-images/menu icons/back_to_game.xpm"
			xpm = back_to_game_xpm;
			break;
		}
		case MenuOption::FREEDOM: {
			#include "compiled-images/menu icons/freedom.xpm"
			xpm = freedom_xpm;
			break;
		}
		case MenuOption::CANCEL: {
			#include "compiled-images/menu icons/cancel.xpm"
			xpm = cancel_xpm;
			break;
		}
		case MenuOption::OK: {
			#include "compiled-images/menu icons/ok.xpm"
			xpm = ok_xpm;
			break;
		}
		case MenuOption::UNDO_CHANGES: {
			#include "compiled-images/menu icons/undo_changes.xpm"
			xpm = undo_changes_xpm;
			break;
		}
		case MenuOption::RESET_TO_DEFAULTS: {
			#include "compiled-images/menu icons/reset_to_defaults.xpm"
			xpm = reset_to_defaults_xpm;
			break;
		}
		default: {
			std::wcerr << L"Error in XPMImageLoader::loadMenuOptionImage(): Type " << ( unsigned int ) type << " not handled in switch statement." << std::endl;
			break;
		}
	}
	
	loadImageCommon( driver, storage, xpm );
}

void XPMImageLoader::loadOtherImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, other_t type ) {
	char ** xpm = nullptr;
	switch( type ) {
		case PLAYER: {
			#include "compiled-images/players/poker_chip.xpm"
			xpm = poker_chip_xpm;
			break;
		}
		case GOAL: {
			#include "compiled-images/goal.xpm"
			xpm = goal_xpm;
			break;
		}
		case START: {
			#include "compiled-images/start.xpm"
			xpm = start_xpm;
			break;
		}
	}
	assert( xpm != nullptr );
	loadImageCommon( driver, storage, xpm );
}
