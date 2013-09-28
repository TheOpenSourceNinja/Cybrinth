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

#include "Collectable.h"
#include "colors.h"
#include <iostream>



Collectable::Collectable() {
	try {
		x = 0;
		y = 0;
		setType( KEY );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::Collectable(): " << e.what() << std::endl;
	}
}

Collectable::~Collectable() {
	try {
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::~Collectable(): " << e.what() << std::endl;
	}
}

void Collectable::draw( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
		uint_least16_t smaller = height;
		if( smaller > width ) {
			smaller = width;
		}

		//wcout << L"desired size: " << smaller << std::endl;

		if( texture->getSize() != irr::core::dimension2d< irr::u32 >( smaller, smaller ) ) {
			resizeImage( driver, smaller, smaller );
		}

		Object::draw( driver, width, height );
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::draw(): " << e.what() << std::endl;
	}
}

Collectable::type_t Collectable::getType() {
	try {
		return type;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::getType(): " << e.what() << std::endl;
	}
}

irr::video::ITexture* Collectable::imageToTexture( irr::video::IVideoDriver* driver, irr::video::IImage* oldImage, irr::core::stringw name ) {
	try {
		irr::video::ITexture* texture = driver->addTexture( name.c_str(), oldImage );
		texture->grab();
		return texture;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::imageToTexture(): " << e.what() << std::endl;
		return nullptr;
	}
}

void Collectable::loadTexture( irr::video::IVideoDriver* driver ) {
	try {
		switch( type ) {
			case KEY: {
					texture = driver->getTexture( L"key.png" );

					if( texture == nullptr ) {
						//Uncomment the following if using key.h instead of key.c:
						/*#include "compiled-images/key.h"
						irr::video::IImage* temp = driver->createImage( irr::video::ECF_A8R8G8B8, irr::core::dimension2d< irr::u32 >( width, height ) );
						temp->fill( INVISIBLE );

						auto* data = header_data;
						auto topLeftPixel[ 3 ]; //Assume that the top left pixel is supposed to be invisible
						for( auto y = 0; y < height; ++y ) {
							for( auto x = 0; x < width; ++x ) {
								auto pixel[ 3 ];
								HEADER_PIXEL( data, pixel );
								if( y == 0 && x == 0 ) {
									topLeftPixel[ 0 ] = pixel[ 0 ];
									topLeftPixel[ 1 ] = pixel[ 1 ];
									topLeftPixel[ 2 ] = pixel[ 2 ];
								}

								//std::wcout << "Pixel: " << static_cast< uint8_t >( pixel[ 0 ] ) << L'\t' << static_cast< uint8_t >( pixel[ 1 ] ) << L'\t' <<  static_cast< uint8_t >( pixel[ 2 ] ) << std::endl;
								if( pixel[ 0 ] == topLeftPixel[ 0 ] && pixel[ 1 ] == topLeftPixel[ 1 ] && pixel[ 2 ] == topLeftPixel[ 2 ] ) {
									temp->setPixel( x, y, irr::video::SColor( 0, 0, 0, 0 ) );
								} else {
									temp->setPixel( x, y, irr::video::SColor( 255, pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] ) );
								}
							}
						}

						texture = imageToTexture( driver, temp, "key" );*/
						
						//Key.c:
						#include "compiled-images/key.c"
						
						irr::video::ECOLOR_FORMAT format = irr::video::ECF_UNKNOWN;
						switch( gimp_image.bytes_per_pixel ) {
							case 2: {
								format = irr::video::ECF_R5G6B5;
								break;
							}
							case 3: {
								format = irr::video::ECF_R8G8B8;
								break;
							}
							case 4: {
								format = irr::video::ECF_A8R8G8B8;
							}
						}
						//The following line works but produces the wrong colors, since GIMP outputs in RGBA and Irrlicht apparently expects ARGB
						//irr::video::IImage* temp = driver->createImageFromData( format, irr::core::dimension2d< irr::u32 >( gimp_image.width, gimp_image.height ), const_cast< unsigned char* >( gimp_image.pixel_data ), false, false );
						
						irr::video::IImage* temp = driver->createImage( format, irr::core::dimension2d< irr::u32 >( gimp_image.width, gimp_image.height ) );
						
						for( auto y = 0; y < gimp_image.height; ++y ) {
							for( auto x = 0; x < gimp_image.width; ++x ) {
								char pixel[ 4 ];
								pixel[ 0 ] = gimp_image.pixel_data[ ( y * gimp_image.width * gimp_image.bytes_per_pixel ) + ( x * gimp_image.bytes_per_pixel ) + 0 ]; //Red
								pixel[ 1 ] = gimp_image.pixel_data[ ( y * gimp_image.width * gimp_image.bytes_per_pixel ) + ( x * gimp_image.bytes_per_pixel ) + 1 ]; //Green
								pixel[ 2 ] = gimp_image.pixel_data[ ( y * gimp_image.width * gimp_image.bytes_per_pixel ) + ( x * gimp_image.bytes_per_pixel ) + 2 ]; //Blue
								
								//Alpha
								if( gimp_image.bytes_per_pixel == 4 ) {
									pixel[ 3 ] = gimp_image.pixel_data[ ( y * gimp_image.width * gimp_image.bytes_per_pixel ) + ( x * gimp_image.bytes_per_pixel ) + 3 ];
								} else {
									pixel[ 3 ] = 255;
								}
								
								temp->setPixel( x, y, irr::video::SColor( pixel[ 3 ], pixel[ 0 ], pixel[ 1 ], pixel[ 2 ] ) );
							}
						}
						
						texture = imageToTexture( driver, temp, "key" );
					}

					break;
				}
			default: break;
		}

		if( texture == nullptr ) {
			irr::video::IImage* temp = driver->createImage( irr::video::ECF_A1R5G5B5, irr::core::dimension2d< irr::u32 >( 2, 2 ) );
			temp->fill( WHITE );
			texture = imageToTexture( driver, temp, "generic collectable" );
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::loadTexture(): " << e.what() << std::endl;
	}
}

void Collectable::reset() {
}

void Collectable::resizeImage( irr::video::IVideoDriver* driver, uint_least16_t width, uint_least16_t height ) {
	try {
		if( texture == nullptr || texture->getOriginalSize().Width < width || texture->getOriginalSize().Height < height ) {
			switch( type ) {
				case KEY: { //COLLECTABLE_KEY loads the texture from an image, so load the image before resizing
						loadTexture( driver );
						break;
					}
				default: {
					if( texture == nullptr ) {
						loadTexture( driver );
					}
					break;
				}
			}
		}
		irr::video::IImage* tempImage = textureToImage( driver, texture );
		//driver->removeTexture( texture );
		//texture->drop();
		irr::video::IImage* tempImage2 = driver->createImage( tempImage->getColorFormat(), irr::core::dimension2d< irr::u32 >( width, height ) );
		tempImage->copyToScaling( tempImage2 );
		//tempImage->drop();
		texture = imageToTexture( driver, tempImage2, L"resized" );
		//tempImage2->drop();
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::resizeImage(): " << e.what() << std::endl;
	}
}

void Collectable::setType( type_t newType ) {
	try {
		type = newType;

		switch( type ) {
			case KEY: {
					setColor( YELLOW );
					break;
				}
			default: break;
		}
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::setType(): " << e.what() << std::endl;
	}
}

irr::video::IImage* Collectable::textureToImage( irr::video::IVideoDriver* driver, irr::video::ITexture* texture ) {
	try {
		irr::video::IImage* newImage = driver->createImageFromData( texture->getColorFormat(), texture->getSize(), texture->lock(), false );
		texture->unlock();
		return newImage;
	} catch ( std::exception e ) {
		std::wcerr << L"Error in Collectable::textureToImage(): " << e.what() << std::endl;
		return nullptr;
	}
}
