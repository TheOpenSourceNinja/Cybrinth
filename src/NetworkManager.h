/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2015.
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
#include <random>
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
		std::string getPort();
		
		void processPackets();
		
		void sendMaze( std::minstd_rand::result_type randomSeed );
		void sendPlayerPos( uint_fast8_t playerNum );
		void sendPlayerPosXMove( uint_fast8_t playerNum, int_fast8_t direction );
		void sendPlayerPosYMove( uint_fast8_t playerNum, int_fast8_t direction );
		void setPort( std::string newPort );
		void setup( MainGame* newGM, bool newIsServer );
		
		void tellNewClientItsPlayer( uint_fast8_t playerNum );
	protected:
	private:
		
		class ClientInfo {
			public:
				ClientInfo() : isReadyToPlay( false ) {  };
				RakNet::RakNetGUID guid;
				bool isReadyToPlay;
		};
		
		std::vector< ClientInfo > clients;
		
		RakNet::SystemAddress clientID; //This is in both the client and server examples
		
		uint8_t deSerializeU8( std::string input );
		int8_t deSerializeS8( std::string input );
		uint16_t deSerializeU16( std::string input );
		int16_t deSerializeS16( std::string input );
		uint32_t deSerializeU32( std::string input );
		int32_t deSerializeS32( std::string input );
		
		MainGame* mg;
		bool isConnected;
		bool isServer;
		RakNet::SystemAddress latestClientAddress;
		RakNet::RakPeerInterface* me; //A representation of the local peer
		std::string password; //No real security provided by passwords, especially since we're using an unencrypted connection. It just helps prevent other programs, or other versions of the same program, from accidentally connecting.
		
		std::string serializeU8( uint8_t input );
		std::string serializeS8( int8_t input );
		std::string serializeU16( uint16_t input );
		std::string serializeS16( int16_t input );
		std::string serializeU32( uint32_t input );
		std::string serializeS32( int32_t input );
		
		std::string serverIP; //Clients need to know this
		std::string serverPort;
		
		enum command_t : uint8_t { NEWMAZE, TELEPORTPLAYER, MOVEPLAYERONX, MOVEPLAYERONY, TELLPLAYERNUMBER, READYTOPLAY };
};

#endif // NETWORKMANAGER_H
