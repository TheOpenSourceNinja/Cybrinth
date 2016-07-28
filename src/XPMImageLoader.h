#ifndef XPMIMAGELOADER_H
#define XPMIMAGELOADER_H

#include "Collectable.h"
#include "MenuOption.h"
#include <irrlicht/irrlicht.h>

class XPMImageLoader {
	public:
		/** Default constructor */
		XPMImageLoader();
		
		enum other_t { PLAYER, GOAL, START };
		
		void loadImageCommon( irr::video::IVideoDriver* driver, irr::video::IImage* storage, char ** xpm );
		void loadMenuOptionImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, MenuOption::option_t type );
		void loadCollectableImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, Collectable::type_t type );
		void loadOtherImage( irr::video::IVideoDriver* driver, irr::video::IImage* storage, other_t type );
	protected:
	private:
		enum colorType_t { COLOR, GRAYSCALE, FOURLEVELGRAYSCALE, MONOCHROME, SYMBOLIC, INVALID_DO_NOT_USE };
};

#endif // XPMIMAGELOADER_H
