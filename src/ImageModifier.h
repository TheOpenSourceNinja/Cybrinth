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
