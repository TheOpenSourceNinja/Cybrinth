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

#include "Goal.h"
#include "MazeCell.h"
#include "PlayerStart.h"
#include "Collectable.h"
#include "PreprocessorCommands.h"
#include <irrlicht/irrlicht.h>
#ifdef HAVE_SSTREAM
	#include <sstream>
#endif //HAVE_SSTREAM
#ifdef HAVE_VECTOR
	#include <vector>
#endif //HAVE_VECTOR

#if defined WINDOWS
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif
#include <string>

//#include "GameManager.h"
class GameManager; //Avoids circular dependency

class NetworkManager {
	public:
		NetworkManager();
		virtual ~NetworkManager();
		bool setup( bool isServer );
		void setGameManager( GameManager * newGM );
		bool hasNewPlayerConnected();
		uint_fast8_t getNewPlayer();
		int checkForConnections();
		void sendMaze( MazeCell ** maze, uint_fast8_t cols, uint_fast8_t rows );
		void sendPlayerPos( uint_fast8_t player, uint_fast8_t x, uint_fast8_t y );
		void setPort( uint_fast16_t newPort );
		uint_fast16_t getPort();
		void sendGoal( Goal goal );
		void sendPlayerStarts( std::vector<PlayerStart> starts );
		void sendU8( uint_fast8_t num, std::wstring desc );
		void sendCollectables( std::vector<Collectable> stuff );
		bool receiveData();
	protected:
	private:
		GameManager * gm;
		unsigned int newPlayer;
		struct addrinfo hints;
		uint_fast16_t port;
		int rv;
		struct addrinfo *ai;
		int listener;
		struct addrinfo *p;
		int yes;
		int backlog;
		int fdmax;
		fd_set master; //Lists all the socket descriptors currently connected, as well as listener
		fd_set read_fds; //temporary file descriptor list for select(); select() modifies this to indicate which are ready to read
		char remoteIP[ INET6_ADDRSTRLEN ];
		struct sockaddr_storage remoteaddr; // client address
		struct timeval timeout;
		void *get_in_addr( struct sockaddr *sa );
		bool sendData( int sockfd, unsigned char *buf, size_t *len );
		uint_fast8_t receivedData[51 ]; //51 is the maximum possible size of a maze plus "STARTMAZE" and "ENDMAZE"

		#if defined WINDOWS
		WSADATA wsaData;   // if this doesn't work
		//WSAData wsaData; // then try this instead
		#endif
};

#endif // NETWORKMANAGER_H
