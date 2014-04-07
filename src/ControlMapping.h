#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <irrlicht/irrlicht.h>
#ifdef HAVE_STRING
	#include <string>
#endif //HAVE_STRING
#include "Integers.h"
#include "PreprocessorCommands.h"

class ControlMapping {
	public:
		enum controllerDirection_t { CONTROLLER_INCREASE, CONTROLLER_DECREASE, CONTROLLER_DO_NOT_USE };
		enum mouseDirection_t { MOUSE_UP, MOUSE_DOWN, MOUSE_RIGHT, MOUSE_LEFT, MOUSE_DO_NOT_USE };
		enum action_t { ACTION_MENU_ACTIVATE, ACTION_MENU_UP, ACTION_MENU_DOWN, ACTION_SCREENSHOT, ACTION_VOLUME_UP, ACTION_VOLUME_DOWN, ACTION_PLAYER_UP, ACTION_PLAYER_DOWN, ACTION_PLAYER_RIGHT, ACTION_PLAYER_LEFT, ACTION_DO_NOT_USE };
		ControlMapping();
		virtual ~ControlMapping();
		irr::EKEY_CODE getKey();
		void setKey( irr::EKEY_CODE val );
		uint_fast8_t getControllerButton();
		void setControllerButton( uint_fast8_t val );
		uint_fast8_t getControllerNumber();
		void setControllerNumber( uint_fast8_t val );
		action_t getAction();
		void setAction( action_t val );
		irr::EMOUSE_INPUT_EVENT getMouseEvent();
		bool getMouseWheelUp();
		uint_fast8_t getPlayer();
		void setPlayer( uint_fast8_t val );
		void setMouseEvent( irr::EMOUSE_INPUT_EVENT val );
		void setMouseWheelUp( bool val );
		controllerDirection_t getControllerDirection();
		void setControllerDirection( controllerDirection_t val );
		uint_fast8_t getControllerAxis();
		void setControllerAxis( uint_fast8_t val );
		void setMouseDirection( mouseDirection_t val );
		mouseDirection_t getMouseDirection();
		
		bool activated; //To be set, unset, and accessed by GameManager. Indicates whether the relevant button has been pressed, or controller moved, etc. Ensures smooth player movements.
	protected:
	private:
		irr::EKEY_CODE key;
		irr::EMOUSE_INPUT_EVENT mouseEvent;
		uint_fast8_t controllerButton;
		uint_fast8_t controllerNumber;
		bool mouseWheelUp;
		action_t action;
		uint_fast8_t player;
		controllerDirection_t controllerDirection;
		uint_fast8_t controllerAxis;
		mouseDirection_t mouseDirection;
};

#endif // KEYMAPPING_H
