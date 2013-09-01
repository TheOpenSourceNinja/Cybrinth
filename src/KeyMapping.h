#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <irrlicht.h>
#include "Integers.h"

class KeyMapping
{
	public:
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
		/** Access action
		 * \return The current value of action
		 */
		wchar_t getAction(); //TODO: Is there any way to use an enumerated type here rather than a character?
		/** Set action
		 * \param val New value to set
		 */
		void setAction( wchar_t val );
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
		uint_least8_t getPlayer();
		/** Set player
		 * \param val New value to set
		 */
		void setPlayer( uint_least8_t val );
		/** Set mouse event
		 * \param val New value to set
		 */
		void setMouseEvent( irr::EMOUSE_INPUT_EVENT val );
		/** Set mouseWheelUp
		 * \param val New value to set
		 */
		void setMouseWheelUp( bool val );
	protected:
	private:
		irr::EKEY_CODE key;
		irr::EMOUSE_INPUT_EVENT mouseEvent;
		bool mouseWheelUp;
		wchar_t action;
		uint_least8_t player;
};

#endif // KEYMAPPING_H
