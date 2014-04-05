#ifndef IMAGEMODIFIER_H
#define IMAGEMODIFIER_H

#include "Integers.h"
#include "PreprocessorCommands.h"

#include <irrlicht/irrlicht.h>
using namespace irr;

class ImageModifier {
	public:
		ImageModifier();
		virtual ~ImageModifier();
		video::ITexture* resize( video::ITexture* image, uint_fast32_t width, uint_fast32_t height, video::IVideoDriver* driver );
		video::IImage* resize( video::IImage* image, uint_fast32_t width, uint_fast32_t height, video::IVideoDriver* driver );
		video::IImage* textureToImage( video::IVideoDriver* driver, video::ITexture* texture );
		video::ITexture* imageToTexture( video::IVideoDriver* driver, video::IImage* texture, core::stringw name );
	protected:
	private:
};

#endif // IMAGEMODIFIER_H
