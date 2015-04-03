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

#include "MainGame.h"
#include "Integers.h"
#include "NetworkManager.h"
#include "RakNet/MessageIdentifiers.h"
#include "RakNet/RakNetStatistics.h"
#include "StringConverter.h"
#include <iostream>
#include <vector>

//All that matters about these numbers is that they not be the same, and I'm not even sure about that. See RakNet/RakPeerInterface.h
#define CLIENT_SEND_CHANNEL 0
#define SERVER_SEND_CHANNEL 1

NetworkManager::NetworkManager() {
	try {
		password = PACKAGE_STRING;
		serverPort = "61187";
		serverIP = "::1";
		me = nullptr;
		isConnected = false;
		mg = nullptr;
		clientID = RakNet::UNASSIGNED_SYSTEM_ADDRESS; // Record the first client that connects to us so we can pass it to the ping function
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in NetworkManager::NetworkManager(): " << e.what() << std::endl;
	}
}

NetworkManager::~NetworkManager() {
	try {
		if( me != nullptr ) {
			me->Shutdown( 300 ); //The number of milliseconds to wait for remaining messages to go out. Both the client and server examples used 300, so that's what I put here too.
			RakNet::RakPeerInterface::DestroyInstance( me );
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in NetworkManager::~NetworkManager(): " << e.what() << std::endl;
	}
}

bool NetworkManager::getConnectionStatus() {
	return isConnected;
}

void NetworkManager::processPackets() {
	for( RakNet::Packet* p = me->Receive(); p != 0; me->DeallocatePacket( p ), p = me->Receive() ) {
		if( p == 0 ) {
			return;
		}
		
		unsigned char messageID = p->data[ 0 ];
		if( messageID == ID_TIMESTAMP ) {
			auto skipLength = sizeof( RakNet::MessageID ) + sizeof( RakNet::Time );
			if( p->length <= skipLength ) {
				std::wcerr << L"Received packet of invalid length " << p->length << L" > " << skipLength << std::endl;
			}
			messageID = p->data[ skipLength ]; //The timestamp, which we ignore, is followed by another message ID.
		}
		
		StringConverter sc;
		
		switch( messageID ) {
			case ID_DISCONNECTION_NOTIFICATION: {
				if( isServer ) {
					if( mg != nullptr and mg->getDebugStatus() ) {
						std::wcout << L"Client disconnected: " << sc.toStdWString( p->systemAddress.ToString() ) << std::endl;
					}
				} else {
					isConnected = false;
				}
				break;
			}
			case ID_ALREADY_CONNECTED: {
				if( mg != nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Already connected." << std::endl;
				}
				break;
			}
			case ID_NEW_INCOMING_CONNECTION: { //Only the server should receive incoming connections
				if( mg not_eq nullptr ) {
					if( mg->getDebugStatus() ) {
						std::wcout << L"New incoming connection from " << sc.toStdWString( p->systemAddress.ToString() ) << L" with GUID " << sc.toStdWString( p->guid.ToString() ) << L". ";
						clientID=p->systemAddress;
						
						std::wcout << L"Remote IDs: " << std::endl;
						for ( decltype( MAXIMUM_NUMBER_OF_INTERNAL_IDS ) i = 0; i < MAXIMUM_NUMBER_OF_INTERNAL_IDS; ++i ) {
							RakNet::SystemAddress internalID = me->GetInternalID( p->systemAddress, i );
							if ( internalID not_eq RakNet::UNASSIGNED_SYSTEM_ADDRESS ) {
								std::wcout << i << L". " << sc.toStdWString( internalID.ToString() ) << std::endl;
							}
						}
					}
					
					latestClientAddress = p->systemAddress;
					mg->networkHasNewConnection();
				}
				break;
			}
			case ID_INCOMPATIBLE_PROTOCOL_VERSION: {
				std::wcerr << L"Error: Incompatible RakNet protocol versions. Ensure that the server and client are using the same version of the software." << std::endl;
				break;
			}
			case ID_REMOTE_DISCONNECTION_NOTIFICATION: { //Should only be received by clients
				if( mg not_eq nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Another client has disconnected." << std::endl;
				}
				break;
			}
			case ID_REMOTE_CONNECTION_LOST: { //Should only be received by clients
				if( mg not_eq nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Another client has lost its connection to the server." << std::endl;
				}
				break;
			}
			case ID_REMOTE_NEW_INCOMING_CONNECTION: { //Should only be received by clients
				if( mg not_eq nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Another client has connected to the server." << std::endl;
				}
				break;
			}
			case ID_CONNECTION_BANNED: { //Should only be received by clients
				std::wcerr << L"The server has banned this client. Cannot connect." << std::endl;
				break;
			}
			case ID_CONNECTION_ATTEMPT_FAILED: { //Should only be received by clients
				std::wcerr << L"Connection attempt failed." << std::endl;
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS: { //Should only be received by clients
				std::wcerr << L"The server is full." << std::endl;
				break;
			}
			case ID_INVALID_PASSWORD: { //Should only be received by clients
				std::wcerr << L"Cannot connect to server: Invalid  program name and/or version." << std::endl;
				break;
			}
			case ID_CONNECTED_PING: //Should only be received by the server
			case ID_UNCONNECTED_PING: { //Should only be received by the server
				if( mg not_eq nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Ping received from " << sc.toStdWString( p->systemAddress.ToString() ) << std::endl;
				}
				break;
			}
			case ID_CONNECTION_LOST: {
				std::wcerr << L"Connection to " << sc.toStdWString( p->systemAddress.ToString() ) << L" lost." << std::endl;
				break;
			}
			case ID_CONNECTION_REQUEST_ACCEPTED: { //Should only be received by clients
				if( mg not_eq nullptr and mg->getDebugStatus() ) {
					std::wcout << L"Connection request accepted by " << sc.toStdWString( p->systemAddress.ToString() ) << std::endl;
				}
				break;
			}
			case ID_IP_RECENTLY_CONNECTED: {
				std::wcerr << L"Connected too recently; can't connect again so soon." << std::endl;
				break;
			}
			default: { //Message received, the server should forward it to the clients. Clients should react accordingly.
				std::wcout << L"Message received, the server should forward it to the clients. Clients should react accordingly." << std::endl;
				if( isServer ) {
					me->Send( ( const char * ) p->data, p->length, HIGH_PRIORITY, RELIABLE_ORDERED, SERVER_SEND_CHANNEL, p->systemAddress, true ); //p->systemAddress tells it who (not) to send to (the address we just received from), and the bool means broadcast to all other connected systems.
				} else {
					std::wcout << sc.toStdWString( p->data ) << std::endl;
				}
			}
		}
	}
}

void NetworkManager::sendMaze( std::minstd_rand::result_type randomSeed ) {
	if( me not_eq nullptr and isConnected ) {
		auto data = serializeU32( randomSeed );
		char channel;
		if( isServer ) {
			channel = SERVER_SEND_CHANNEL;
		} else {
			channel = CLIENT_SEND_CHANNEL;
		}
		me->Send( data.c_str(), data.length(), MEDIUM_PRIORITY, RELIABLE_ORDERED, channel, latestClientAddress, false ); //The Boolean being false means we will send only to the specified address.
	} else {
		std::wcerr << L"Error: Cannot send data, not yet connected to anything." << std::endl;
	}
}

std::string NetworkManager::serializeS16( int16_t input ) {
	std::stringstream ss;
	ss << htons( input );
	return ss.str();
}

std::string NetworkManager::serializeU16( uint16_t input ) {
	std::stringstream ss;
	ss << htons( input );
	return ss.str();
}

std::string NetworkManager::serializeS32( int32_t input ) {
	std::stringstream ss;
	ss << htonl( input );
	return ss.str();
}

std::string NetworkManager::serializeU32( uint32_t input ) {
	std::stringstream ss;
	ss << htonl( input );
	return ss.str();
}

void NetworkManager::setup( MainGame* newGM, bool newIsServer ) {
	isServer = newIsServer;
	mg = newGM;
	me = RakNet::RakPeerInterface::GetInstance(); //In RakNet's examples, this is called "client" in the client program and "server" in the server program.
	RakNet::RakNetStatistics* rss;
	
	if( isServer ) {
		me->SetIncomingPassword( password.c_str(), password.length() );
		me->SetTimeoutTime( 30000, RakNet::UNASSIGNED_SYSTEM_ADDRESS ); //30000 comes from the chat server example
	}
	
	unsigned char packetIdentifier;
	
	std::vector< RakNet::SocketDescriptor > socketDescriptors;
	
	if( isServer ) {
		socketDescriptors.resize( UINT_FAST8_MAX ); //A server can have many clients
	} else {
		socketDescriptors.resize( 1 ); //A client only connects to one server
	}
	
	for( decltype( socketDescriptors.size() ) sd = 0; sd < socketDescriptors.size(); ++sd ) {
		if( isServer ) {
			socketDescriptors.at( sd ).port = atoi( serverPort.c_str() );
		} else {
			socketDescriptors.at( sd ).port = atoi( clientPort.c_str() );
		}
		socketDescriptors.at( sd ).socketFamily = AF_INET6;
	}
	
	uint_fast8_t maxConnections;
	if( isServer ) {
		maxConnections = UINT_FAST8_MAX;
	} else {
		maxConnections = 1;
	}
	
	bool b = me->Startup( maxConnections, socketDescriptors.data(), socketDescriptors.size() ) == RakNet::RAKNET_STARTED;
	if( b ) {
		if( isServer ) {
			me->SetMaximumIncomingConnections( maxConnections ); //Ensures that all the allowed connections ( set with Startup() above ) are for incoming use only.
		}
		
		me->SetUnreliableTimeout( 5000 ); //The number of milliseconds to wait before timing out an unreliable connection. Set to 0 or less to disable timeout. No magic number here; I just guessed that 5 seconds would be good.
		
		if( not isServer ) {
			RakNet::ConnectionAttemptResult car = me->Connect( serverIP.c_str(), atoi( serverPort.c_str() ), password.c_str(), password.length() );
			isConnected = ( car == RakNet::CONNECTION_ATTEMPT_STARTED );
		}
		
		if( mg->getDebugStatus() ) {
			StringConverter sc;
			std::wcout << L"My IP addresses:" << std::endl;
			for( decltype( me->GetNumberOfAddresses() ) a = 0; a < me->GetNumberOfAddresses(); ++a ) {
				std::wcout << a << L". ";
				if( isServer ) {
					RakNet::SystemAddress sa = me->GetInternalID( RakNet::UNASSIGNED_SYSTEM_ADDRESS, a ); //Gets both local and non-local IPs
					std::wcout << sc.toStdWString( sa.ToString() );
					if( sa.IsLANAddress() ) {
						std::wcout << L" which is a LAN address";
					}
				} else {
					std::wcout << sc.toStdWString( me->GetLocalIP( a ) ); //Clients only need to know their local IP(s). Actually not even that really.
				}
				std::wcout << std::endl;
			}
			
			std::wcout << L"My GUID is " << sc.toStdWString( me->GetGuidFromSystemAddress( RakNet::UNASSIGNED_SYSTEM_ADDRESS ).ToString() ) << std::endl;
		}
	}
}
