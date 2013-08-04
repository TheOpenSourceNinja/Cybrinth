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
#include "NetworkManager.h"
#include "GameManager.h"
#include <algorithm/string.hpp>

// get sockaddr, IPv4 or IPv6:
void *NetworkManager::get_in_addr( struct sockaddr *sa ) {
	if( sa->sa_family == AF_INET ) {
		return &((( struct sockaddr_in* )sa )->sin_addr );
	}

	return &((( struct sockaddr_in6* )sa )->sin6_addr );
}

NetworkManager::NetworkManager() {
	//ctor
	port = 61187;
	newPlayer = 0;
	rv = 0;
	ai = NULL;
	listener = 0;
	p = NULL;
	yes = 1;
	backlog = 0;
	fdmax = 0;
	setGameManager( NULL );

	#if defined WINDOWS
	if (WSAStartup(MAKEWORD(1,1), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
    #endif
}

void NetworkManager::setGameManager( GameManager* newGM ) {
	gm = newGM;
}

void NetworkManager::setPort( uint_least16_t newPort ) {
	if( newPort <= 1023 ) {
		std::wcerr << L"Warning: Port " << newPort << L" is in the \"well-known\" range (0-1023). This may not work. Recommend using ports above 49151." << std::endl;
	} else if( newPort <= 49151 ) {
		std::wcerr << L"Warning: Port " << newPort << L" is in the \"registered ports\" range (1024-49151). This may not work. Recommend using ports above 49151." << std::endl;
	} else {
		if( gm != NULL && gm->getDebugStatus() ) {
			std::wcout << L"Setting port to " << newPort << L"." << std::endl;
		}
	}

	port = newPort;
}

int NetworkManager::setup( bool isServer ) {
	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_INET6; // AF_INET or AF_INET6 to force version, AF_UNSPEC otherwise
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //fill in my IP for me

	rv = getaddrinfo( NULL, irr::core::stringc( port ).c_str(), &hints, &ai );

	if( rv != 0 ) {
		std::wcerr << L"getaddrinfo: " << gai_strerror( rv ) << std::endl;
		return 2;
	}

	for( p = ai; p != NULL; p = p->ai_next ) {
		listener = socket( p->ai_family, p->ai_socktype, p->ai_protocol );

		if( listener < 0 ) {
			continue;
		}

		if (isServer) {
			setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

			/*if( bind( listener, p->ai_addr, p->ai_addrlen ) < 0 ) { //Commenting this out because I'm not focusing on networking right now,so don't want to debug
				std::wcerr << L"server bind() error: " << strerror( errno ) << std::endl;
				close( listener );
				continue;
			}*/
		} else { //client
			if (connect(listener, p->ai_addr, p->ai_addrlen) < 0) {
				close(listener);
				std::wcerr << L"client connect() error: " << strerror(errno) << std::endl;
				continue;
			}
		}

		break; //Cannot be reached if bind() works or if listener < 0
	}

	//Getting out of the loop means bind failed for some p.
	if( p == NULL ) {
		if (isServer) {
			std::wcerr << L"Server: Failed to bind" << std::endl;
		} else { //client
			std::wcerr << L"Client: Failed to connect" << std::endl;
		}
	}

	if (!isServer) { //Client
		inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), remoteIP, sizeof remoteIP);
		std::wcout << L"client: connecting to " << remoteIP << std::endl;
	}

	freeaddrinfo( ai ); // All done with ai

	backlog = 10;//Number of incoming connections listen() allows in its queue

	if (isServer) {
		if( listen( listener, backlog ) == -1 ) {
			std::wcerr << L"listen() error: " << strerror( errno ) << std::endl;
			return 3;
		}

		FD_ZERO( &master ); //Empty the two sets
		FD_ZERO( &read_fds );
		FD_SET( listener, &master ); //Put listener as the only (at first) item in master

		timeout.tv_sec = 0;
		timeout.tv_usec = 50000;

		fdmax = listener;
	}

	return 0;
}

NetworkManager::~NetworkManager() {
	for( int i = 0; i < fdmax; i++ ) {
		#if defined WINDOWS
		closesocket( i );
		#else
		close( i );
		#endif
	}

	#if defined WINDOWS
	WSACleanup();
	#endif
}

int NetworkManager::checkForConnections() {
	read_fds = master;

	if( select( fdmax + 1, &read_fds, NULL, NULL, &timeout ) < 0 ) {
		std::wcerr << L"select() error: " << strerror( errno ) << std::endl;
		return 4;
	} else {
		return 0;
	}
}

int NetworkManager::sendData( int sockfd, unsigned char *buf, size_t *len ) {
	size_t bytesSent = 0;
	int bytesLeft = *len;
	int n = 0;

	while( bytesSent < *len ) {
		n = send( sockfd, buf + bytesSent, bytesLeft, 0 );

		if( n < 0 ) {
			break;
		}

		bytesSent += n;
		bytesLeft -= n;
	}

	*len = bytesSent; //Sets the variable pointed to by len to bytesSent, so that the caller can see how many bytes were sent.

	if( n < 0 ) {
		return -1;
	} else {
		return 0;
	}
}


bool NetworkManager::hasNewPlayerConnected() {
	//std::wcout << L"read_fds count: " << read_fds.fd_count << std::endl;
	//getchar();
	//checkForConnections();
	if( FD_ISSET( listener, &read_fds ) ) {
		socklen_t addrlen = sizeof remoteaddr;
		int newfd = accept( listener, ( struct sockaddr * ) &remoteaddr, &addrlen );

		if( newfd < 0 ) {
			std::wcerr << L"accept() error: " << strerror( errno ) << std::endl;
			return false;
		} else {
			FD_SET( newfd, &master ); //add newfd to master set

			if( newfd > fdmax ) {
				fdmax = newfd;
			}

			std::wcout << L"Server: New connection from " << inet_ntop( remoteaddr.ss_family, get_in_addr(( struct sockaddr* )&remoteaddr ), remoteIP, INET6_ADDRSTRLEN ) << L" on socket " << newfd << std::endl;
			//getchar();
			return true;
		}
	} else {
		//std::wcout << L"No new connections at this time." << std::endl;
		return false;
	}
}

void NetworkManager::sendMaze( MazeCell ** maze, uint_least8_t cols, uint_least8_t rows ) {
	//char buffer[sizeof(cols) + sizeof(rows) + (cols * rows)];
	std::vector<uint_least8_t> toSend;
	toSend.reserve( 9 + 7 + 2 + ( cols * rows * 4 ) );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	toSend.push_back( 'M' );
	toSend.push_back( 'A' );
	toSend.push_back( 'Z' );
	toSend.push_back( 'E' );
	toSend.push_back( cols );
	toSend.push_back( rows );

	for( uint_fast8_t c = 0; c < cols; c++ ) {
		for( uint_fast8_t r = 0; r < rows; r++ ) {
			toSend.push_back( maze[c][r].getTop() );
			toSend.push_back( maze[c][r].getLeft() );
			toSend.push_back( maze[c][r].getBottom() );
			toSend.push_back( maze[c][r].getRight() );
		}
	}

	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );
	toSend.push_back( 'M' );
	toSend.push_back( 'A' );
	toSend.push_back( 'Z' );
	toSend.push_back( 'E' );

	for( int i = 0; i <= fdmax; i++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( i, &master ) && i != listener ) {
			size_t size = toSend.size();

			if( sendData( i, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

void NetworkManager::sendPlayerPos( uint_least8_t player, uint_least8_t x, uint_least8_t y ) {
	//std::wcout << L"Sending player " << player << L" position " << x << L"," << y << std::endl;

	std::vector<uint_least8_t> toSend;
	toSend.reserve( 6 + 3 + 4 );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	toSend.push_back( 'P' );
	toSend.push_back( player );
	toSend.push_back( x );
	toSend.push_back( y );
	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );
	toSend.push_back( 'P' );

	for( int j = 0; j <= fdmax; j++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( j, &master ) && j != listener ) {
			size_t size = toSend.size();

			if( sendData( j, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

void NetworkManager::sendGoal( Goal goal ) {
	std::vector<uint_least8_t> toSend;
	toSend.reserve( 6 + 2 + 4 );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	toSend.push_back( 'G' );
	toSend.push_back( goal.getX() );
	toSend.push_back( goal.getY() );
	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );
	toSend.push_back( 'G' );

	std::wcout << L"Sending goal at " << goal.getX() << L"," << goal.getY() << std::endl;

	for( int j = 0; j <= fdmax; j++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( j, &master ) && j != listener ) {
			size_t size = toSend.size();

			if( sendData( j, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

void NetworkManager::sendPlayerStarts( std::vector<PlayerStart> starts ) {
	std::vector<uint_least8_t> toSend;
	toSend.reserve( 7 + starts.size() + 5 );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	toSend.push_back( 'S' );
	toSend.push_back( 'S' );
	toSend.push_back( starts.size() );

	for( uint_fast8_t i = 0; i < starts.size(); i++ ) {
		toSend.push_back( i );
		toSend.push_back( starts[i].getX() );
		toSend.push_back( starts[i].getY() );
		std::wcout << L"Send player start #" << i << L" at " << starts[i].getX() << L"," << starts[i].getY() << std::endl;
	}

	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );
	toSend.push_back( 'S' );
	toSend.push_back( 'S' );

	for( int j = 0; j <= fdmax; j++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( j, &master ) && j != listener ) {
			size_t size = toSend.size();

			if( sendData( j, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

void NetworkManager::sendU8( uint_least8_t num, std::wstring desc ) {
	std::wcout << L"Sending uint_least8_t " << desc << L": " << num << std::endl;

	std::vector<uint_least8_t> toSend;
	toSend.reserve( 5 + ( desc.size() * 2 ) + 1 + 3 );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	boost::algorithm::to_upper( desc );

	for( uint_fast8_t i = 0; i < desc.size(); i++ ) {
		toSend.push_back( desc[i] );
	}

	toSend.push_back( num );

	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );

	for( uint_fast8_t i = 0; i < desc.size(); i++ ) {
		toSend.push_back( desc[i] );
	}

	for( int j = 0; j <= fdmax; j++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( j, &master ) && j != listener ) {
			size_t size = toSend.size();

			if( sendData( j, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

void NetworkManager::sendCollectables( std::vector<Collectable> stuff ) {
	std::vector<uint_least8_t> toSend;
	toSend.reserve( 6 + stuff.size() + 4 );
	toSend.push_back( 'S' );
	toSend.push_back( 'T' );
	toSend.push_back( 'A' );
	toSend.push_back( 'R' );
	toSend.push_back( 'T' );
	toSend.push_back( 'C' );

	for( uint_fast8_t i = 0; i < stuff.size(); i++ ) {
		toSend.push_back( i );
		toSend.push_back( stuff[i].getType() );
		toSend.push_back( stuff[i].getX() );
		toSend.push_back( stuff[i].getY() );
		std::wcout << L"Sending collectable #" << i << L" of type " << stuff[i].getType() << L" at " << stuff[i].getX() << L"," << stuff[i].getY() << std::endl;
	}

	toSend.push_back( 'E' );
	toSend.push_back( 'N' );
	toSend.push_back( 'D' );
	toSend.push_back( 'C' );

	for( int j = 0; j <= fdmax; j++ ) {
		//send to everyone! except the listener
		if( FD_ISSET( j, &master ) && j != listener ) {
			size_t size = toSend.size();

			if( sendData( j, &toSend[0], &size ) < 0 ) {
				std::wcerr << L"sendData() " << strerror( errno ) << std::endl;
			}

			if( size != toSend.size() ) {
				std::wcerr << L"sendData() error: Not all data was sent" << std::endl;
			}
		}
	}
}

bool NetworkManager::receiveData() {
	int numBytesReceived = recv(listener, receivedData, 50, 0); //See comment in networkManager.h next to receivedData. Subtract 1 from that.
	if (numBytesReceived < 1) {
		//std::wcerr << L"recv() error: " << strerror(errno) << std::endl;
		return false;
	} else if (numBytesReceived == 0) {
		return false;
	} else {
		return true;
	}
}
