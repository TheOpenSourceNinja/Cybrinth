#ifndef XPMIMAGELOADER_H
#define XPMIMAGELOADER_H

#include "Collectable.h"
#include <irrlicht/irrlicht.h>

class XPMImageLoader {
	public:
		/** Default constructor */
		XPMImageLoader();
		
		void loadImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, Collectable::type_t type );
	protected:
	private:
		enum colorType_t { COLOR, GRAYSCALE, MONOCHROME, SYMBOLIC };
};

#endif // XPMIMAGELOADER_H
