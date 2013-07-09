#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <irrlicht.h>
#include <stdint.h>

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
		unsigned char getAction();
		/** Set action
		 * \param val New value to set
		 */
		void setAction( unsigned char val );
		/** Access player
		 * \return The current value of player
		 */
		uint8_t getPlayer();
		/** Set player
		 * \param val New value to set
		 */
		void setPlayer( uint8_t val );
	protected:
	private:
		irr::EKEY_CODE key; //!< Member variable "key"
		unsigned char action; //!< Member variable "action"
		uint8_t player;
};

#endif // KEYMAPPING_H
