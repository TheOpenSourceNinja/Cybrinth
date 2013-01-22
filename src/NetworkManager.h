/**
 * Copyright © 2013 James Dearing.
 * This file is part of Cybrinth.
 * 
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * 
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <irrlicht.h>
#include <vector>

#if defined WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "Goal.h"
#include "MazeCell.h"
#include "PlayerStart.h"
#include "Collectable.h"

using namespace irr;

class NetworkManager {
	public:
		NetworkManager();
		virtual ~NetworkManager();
		int setup(bool isServer);
		bool hasNewPlayerConnected();
		u8 getNewPlayer();
		int checkForConnections();
		void sendMaze( MazeCell ** maze, u8 cols, u8 rows );
		void sendPlayerPos( u8 player, u8 x, u8 y );
		void setPort( u16 newPort );
		void sendGoal( Goal goal );
		void sendPlayerStarts( std::vector<PlayerStart> starts );
		void sendU8( u8 num, std::wstring desc );
		void sendCollectables( std::vector<Collectable> stuff );
		bool receiveData();
	protected:
	private:
		unsigned int newPlayer;
		struct addrinfo hints;
		std::string port; //Does not need to be a wstring because it only ever contains numbers
		int rv;
		struct addrinfo *ai;
		int listener;
		struct addrinfo *p;
		int yes;
		int backlog;
		int fdmax;
		fd_set master; //Lists all the socket descriptors currently connected, as well as listener
		fd_set read_fds; //temporary file descriptor list for select(); select() modifies this to indicate which are ready to read
		char remoteIP[INET6_ADDRSTRLEN];
		struct sockaddr_storage remoteaddr; // client address
		struct timeval timeout;
		void *get_in_addr( struct sockaddr *sa );
		int sendData( int sockfd, unsigned char *buf, size_t *len );
		u8 receivedData[51]; //51 is the maximum possible size of a maze plus "STARTMAZE" and "ENDMAZE"

		#if defined WINDOWS
		WSADATA wsaData;   // if this doesn't work
		//WSAData wsaData; // then try this instead
		#endif
};

#endif // NETWORKMANAGER_H
