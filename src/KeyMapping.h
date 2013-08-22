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
		wchar_t getAction();
		/** Set action
		 * \param val New value to set
		 */
		void setAction( wchar_t val );
		/** Access player
		 * \return The current value of player
		 */
		uint_least8_t getPlayer();
		/** Set player
		 * \param val New value to set
		 */
		void setPlayer( uint_least8_t val );
	protected:
	private:
		irr::EKEY_CODE key; //!< Member variable "key"
		wchar_t action; //!< Member variable "action"
		uint_least8_t player;
};

#endif // KEYMAPPING_H
