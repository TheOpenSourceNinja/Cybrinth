#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <irrlicht.h>
#ifdef HAVE_STRING
#include <string>
#endif //HAVE_STRING
#include "Integers.h"
#include "PreprocessorCommands.h"

class KeyMapping
{
	public:
		enum joystickDirection_t { JOYSTICK_UP, JOYSTICK_DOWN, JOYSTICK_LEFT, JOYSTICK_RIGHT, JOYSTICK_DO_NOT_USE };
		enum action_t { ACTION_MENU, ACTION_SCREENSHOT, ACTION_VOLUME_UP, ACTION_VOLUME_DOWN, ACTION_UP, ACTION_DOWN, ACTION_RIGHT, ACTION_LEFT, ACTION_DO_NOT_USE };
		/** Default constructor */
		KeyMapping();
		/** Default destructor */
		virtual ~KeyMapping();
		/** Access key
		 * \return The current value of key
		 */
		irr::EKEY_CODE getKey();
		/** Set key
		 * \param val New value to set
		 */
		void setKey( irr::EKEY_CODE val );
		/** Access gamepadButton
		 * \return The current value of gamepadButton
		 */
		uint_fast8_t getGamepadButton();
		/** Set gamepadButton
		 * \param val New value to set
		 */
		void setGamepadButton( uint_fast8_t val );
		/** Access gamepadNumber
		 * \return The current value of gamepadNumber
		 */
		uint_fast8_t getGamepadNumber();
		/** Set gamepadNumber
		 * \param val New value to set
		 */
		void setGamepadNumber( uint_fast8_t val );
		/** Access action
		 * \return The current value of action
		 */
		action_t getAction();
		/** Convert action to string
		 * \return A string representing the action.
		 */
		std::wstring getActionAsString();
		/** Set action
		 * \param val New value to set
		 */
		void setAction( action_t val );
		/** Set action based on string input
		 * \param val A string representing the action to use
		 */
		void setActionFromString( std::wstring val );

		/** Access mouseEvent
		 * \return The current value of mouseEvent
		 */
		irr::EMOUSE_INPUT_EVENT getMouseEvent();
		/** Access mouseWheelUp
		 * \return The current value of mouseWheelUp
		 */
		bool getMouseWheelUp();
		/** Access player
		 * \return The current value of player
		 */
		uint_fast8_t getPlayer();
		/** Set player
		 * \param val New value to set
		 */
		void setPlayer( uint_fast8_t val );
		/** Set mouse event
		 * \param val New value to set
		 */
		void setMouseEvent( irr::EMOUSE_INPUT_EVENT val );
		/** Set mouseWheelUp
		 * \param val New value to set
		 */
		void setMouseWheelUp( bool val );
		
		/** Access joystickDirection
		 * \return The current value of joystickDirection
		 */
		joystickDirection_t getJoystickDirection();
		/** Set joystickDirection
		 * \param val New value to set
		 */
		void setJoystickDirection( joystickDirection_t val );
		
		bool activated; //To be set, unset, and accessed by GameManager. Indicates whether the relevant button has been pressed, or joystick moved, etc. Ensures smooth player movements.
	protected:
	private:
		irr::EKEY_CODE key;
		irr::EMOUSE_INPUT_EVENT mouseEvent;
		uint_fast8_t gamepadButton;
		uint_fast8_t gamepadNumber;
		bool mouseWheelUp;
		action_t action;
		uint_fast8_t player;
		joystickDirection_t joystickDirection;
};

#endif // KEYMAPPING_H
