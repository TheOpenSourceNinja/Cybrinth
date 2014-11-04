/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The NetworkManager class is responsible for networking stuff. It will probably be completely rewritten, as it does not work now.
 */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "Integers.h"
#include "MazeManager.h"
#include "PreprocessorCommands.h"
#include "RakNet/RakPeerInterface.h"
#ifdef HAVE_SSTREAM
	#include <sstream>
#endif //HAVE_SSTREAM
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR

//#include "MainGame.h"
class MainGame; //Avoids circular dependency

class NetworkManager {
	public:
		NetworkManager();
		virtual ~NetworkManager();
		
		bool getConnectionStatus();
		void processPackets();
		void sendMaze( std::minstd_rand::result_type randomSeed );
		void setup( MainGame* newGM, bool newIsServer );
	protected:
	private:
		
		RakNet::SystemAddress clientID; //This is in both the client and server examples
		std::string clientPort; //Clients need to know this
		
		uint16_t deSerializeU16( std::string input );
		int16_t deSerializeS16( std::string input );
		uint32_t deSerializeU32( std::string input );
		int32_t deSerializeS32( std::string input );
		
		MainGame* mg;
		bool isConnected;
		bool isServer;
		RakNet::SystemAddress latestClientAddress;
		RakNet::RakPeerInterface* me; //A representation of the local peer
		std::string password; //No real security provided by passwords, especially since we're using an unencrypted connection. It just helps prevent other programs from accidentally connecting.
		
		std::string serializeU16( uint16_t input );
		std::string serializeS16( int16_t input );
		std::string serializeU32( uint32_t input );
		std::string serializeS32( int32_t input );
		
		std::string serverIP; //Clients need to know this
		std::string serverPort;
};

#endif // NETWORKMANAGER_H
