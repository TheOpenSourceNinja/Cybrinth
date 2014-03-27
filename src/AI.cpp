/**
 * Copyright © 2012-2014 James Dearing.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
*/

#include "AI.h"
#include "GameManager.h"
#include "MazeManager.h"
#include "Player.h"
#include "StringConverter.h"

//TODO: Update the AI to reflect the addition of a wall dissolver item (icon: spray can labeled 'ACID')
/* Progress report:
 * DFS and IDDFS can use acid.
 * Wall-following algorithms not updated yet because the maze may not be simply connected anymore. I've given up on them for now, but I'm leaving this todo note here. */

AI::AI() : controlsPlayer(0) {
	try {
		setup( nullptr, 0, 0, nullptr, false, DEPTH_FIRST_SEARCH, 300 ); //setup( pointer to the maze, number of columns, number of rows, pointer to GameManager, whether to start solved, the algorithm to use, and the movement delay )
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::AI(): " << e.what() << std::endl;
	}
}

AI::~AI() {
	try {
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in AI::~AI(): " << e.what() << std::endl;
	}
}

void AI::allKeysFound() { //Makes the bot 'forget' that it has visited certain maze cells
	try {
		noKeysLeft = true;
		findSolution();

		for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {

			for( decltype( pathsToLockedCells.at( o ).size() ) i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {

				decltype( cellsVisited.size() ) j = 0;
				while( j < cellsVisited.size() ) {

					if( ( cellsVisited.at( j ).X == pathsToLockedCells.at( o )[ i ].X && cellsVisited.at( j ).Y == pathsToLockedCells.at( o )[ i ].Y ) ) {
						cellsVisited.erase( cellsVisited.begin() + j );
						j--;
					}
					++j;
				}
			}

			//Reduce memory usage: We're done with these now, so clear them.
			pathsToLockedCells.at( o ).clear();
			pathsToLockedCells.at( o ).shrink_to_fit(); //If your compiler doesn't support C++11 or later, comment this line and uncomment the next
			//std::vector< core::position2d< uint_fast8_t > >().swap( pathsToLockedCells.at( o ) );
		}
		//Reduce memory usage: We're done with these now, so clear them.
		pathsToLockedCells.clear();
		pathsToLockedCells.shrink_to_fit(); //If your compiler doesn't support C++11 or later, comment this line and uncomment the next
		//std::vector< std::vector< core::position2d< uint_fast8_t > > >().swap( pathsToLockedCells );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::allKeysFound(): " << e.what() << std::endl;
	}
}

bool AI::alreadyVisited( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		decltype( cellsVisited.size() ) i = 0;
		while( i < cellsVisited.size() && result != true ) {
			if( cellsVisited.at( i ).X == position.X && cellsVisited.at( i ).Y == position.Y ) {
				result = true;
			}
			++i;
		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::alreadyVisited(): " << e.what() << std::endl;
		return false;
	}
}

/*bool AI::alreadyVisitedDFS( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		decltype( DFSCellsVisited.size() ) i = 0;
		while( i < DFSCellsVisited.size() && result != true ) {
			if( DFSCellsVisited.at( i ).X == position.X && DFSCellsVisited.at( i ).Y == position.Y ) {
				result = true;
			}
			++i;
		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::alreadyVisitedDFS(): " << e.what() << std::endl;
		return false;
	}
}*/

bool AI::alreadyVisitedIDDFS( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		decltype( IDDFSCellsVisited.size() ) i = 0;
		while( i < IDDFSCellsVisited.size() && result != true ) {
			if( IDDFSCellsVisited.at( i ).X == position.X && IDDFSCellsVisited.at( i ).Y == position.Y ) {
				result = true;
			}
			++i;
		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::alreadyVisitedIDDFS(): " << e.what() << std::endl;
		return false;
	}
}

bool AI::atGoal() {
	try {
		Player* p = gm->getPlayer( controlsPlayer );
		core::position2d< uint_fast8_t > currentPosition( p->getX(), p->getY() );
		Goal* goal = gm->getGoal();
		if( currentPosition.X == goal->getX() && currentPosition.Y == goal->getY() ) {
			return true;
		} else {
			return false;
		}
	} catch( std::exception &e ) {
		std::wcout << "Error in AI::atGoal(): " << e.what() << std::endl;
		return true;
	}
}

bool AI::doneWaiting() {
	try {
		if( lastTimeMoved < gm->timer->getRealTime() - movementDelay ) {
			return true;
		} else {
			return false;
		}
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::doneWaiting(): " << e.what() << std::endl;
		return false;
	}
}

bool AI::effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y ) {
	return effectivelyNoTopWall( x, y, ( gm->getPlayer( controlsPlayer )->hasItem() && gm->getPlayer( controlsPlayer )->getItemType() == Collectable::ACID ) );
}

bool AI::effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls ) {
	if( gm != nullptr && maze != nullptr ) {
		return ( maze[ x ][ y ].getTop() == MazeCell::NONE || ( maze[ x ][ y ].getTop() != MazeCell::ACIDPROOF && canDissolveWalls ) );
	} else {
		return false;
	}
}

bool AI::effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y ) {
	return effectivelyNoLeftWall( x, y, ( gm->getPlayer( controlsPlayer )->hasItem() && gm->getPlayer( controlsPlayer )->getItemType() == Collectable::ACID ) );
}

bool AI::effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls ) {
	if( gm != nullptr && maze != nullptr ) {
		return( maze[ x ][ y ].getLeft() == MazeCell::NONE || ( maze[ x ][ y ].getLeft() != MazeCell::ACIDPROOF && canDissolveWalls ) );
	} else {
		return false;
	}
}

void AI::findSolution() {
	try {
		solved = false;
		//numKeysInSolution = 0;
		keyImSeeking = 0;
		solution.clear();
		{
			Player* p = gm->getPlayer( controlsPlayer );
			core::position2d< uint_fast8_t > currentPosition( p->getX(), p->getY() );
			switch( algorithm ) {
				case DEPTH_FIRST_SEARCH: {
					DFSCellsVisited.clear();
					findSolutionDFS( currentPosition );
					solved = true;
					break;
				}
				case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
					IDDFSCellsVisited.clear();
					findSolutionIDDFS( currentPosition );
					solved = true;
					break;
				}
				case RIGHT_HAND_RULE: 
				case LEFT_HAND_RULE: {
					//These two algorithms would work the same as if startSolved were false (i.e. bots dont' know the solution). So, instead of adding needless code, we just set startSolved to false.
					solved = false;
					startSolved = false;
					break;
				}
				default: {
					StringConverter sc;
					throw( std::wstring( L"Algorithm " ) + sc.toStdWString( algorithm ) + L" not yet added to findSolution()." );
				}
			}

			while( !solution.empty() && solution.back() == currentPosition ) {
				solution.pop_back();
			}
		}
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolution(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcout << L"Error in AI::findSolution(): " << e << std::endl;
	}
}

void AI::findSolutionDFS( irr::core::position2d< uint_fast8_t > currentPosition ) {
	try {
		std::vector< irr::core::position2d< uint_fast8_t > > partialSolution;
		//Instead of adding a bunch of code for DFS, just do IDDFS with the deepest max depth possible.
		uint_fast16_t maxDepth = ( uint_fast16_t ) cols * rows;
		IDDFSCellsVisited.clear();
		findSolutionIDDFS( partialSolution, currentPosition, maxDepth, false );

		{ //Reverses the order of the solution, so we don't start at the wrong end
			std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
			while( !solution.empty() ) {
				tempSolution.push_back( solution.back() );
				solution.pop_back();
			}
			while( !tempSolution.empty() ) {
				solution.insert( solution.begin(), tempSolution.back() );
				tempSolution.pop_back();
			}
		}
		solution.push_back( currentPosition );
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolutionDFS(): " << e.what() << std::endl;
	}
}

void AI::findSolutionIDDFS( irr::core::position2d< uint_fast8_t > currentPosition ) {
	try {
		std::vector< irr::core::position2d< uint_fast8_t > > partialSolution;
		
		uint_fast16_t maxDepth = ( uint_fast16_t ) cols * rows;
		
		if( noKeysLeft ) {
			IDDFSCellsVisited.clear();
			findSolutionIDDFS( partialSolution, currentPosition, maxDepth, false );
		} else {
			for( uint_fast16_t i = 1; solution.size() == 0 && i <= maxDepth; i++ ) {
				if( gm->getDebugStatus() ) {
					std::wcout << L"In IDDFS loop, i=" << i << std::endl;
				}
				IDDFSCellsVisited.clear();
				findSolutionIDDFS( partialSolution, currentPosition, i, false );
			}
		}

		{ //Reverses the order of the solution, so we don't start at the wrong end
			std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
			while( !solution.empty() ) {
				tempSolution.push_back( solution.back() );
				solution.pop_back();
			}
			while( !tempSolution.empty() ) {
				solution.insert( solution.begin(), tempSolution.back() );
				tempSolution.pop_back();
			}
		}
		solution.push_back( currentPosition );
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolutionIDDFS(): " << e.what() << std::endl;
	}
}

//TODO: Find some way to make IDDFS faster, possibly using a caching mechanism or dead-end filling.
void AI::findSolutionIDDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition, uint_fast16_t depthLimit, bool canDissolveWalls ) {
	try {
		if( gm->getDebugStatus() ) {
			std::wcout << L"findSolutionIDDFS: currentPosition: " << currentPosition.X << L"x" << currentPosition.Y << L" goal: " << gm->getGoal()->getX() << L"x" << gm->getGoal()->getY() << L" depthLimit: " << depthLimit << std::endl;
		}

		if( depthLimit == 0 ) {
			partialSolution.push_back( currentPosition );
			if( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) {
				solution = partialSolution;
				return;
			} else {
				for( decltype( gm->getNumCollectables() ) c = 0; c < gm->getNumCollectables(); ++c ) {
					if( ( currentPosition.X == gm->getCollectable( c )->getX() && currentPosition.Y == gm->getCollectable( c )->getY() ) ) {
						switch( gm->getCollectable( c )->getType() ) {
							case Collectable::ACID: {
								canDissolveWalls = true;
								//solution = partialSolution;
								break;
							}
							
							case Collectable::KEY: {
								solution = partialSolution;
								keyImSeeking = c;
								break;
							}
						}
					}
				}
			}
			
			return; //No solution found, therefore solution.size() should be zero
		} else {

			IDDFSCellsVisited.push_back( currentPosition );
			partialSolution.push_back( currentPosition );
			std::vector< direction_t > possibleDirections;
			if( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
				
				for( decltype( gm->getNumCollectables() ) c = 0; c < gm->getNumCollectables(); ++c ) {
					if( currentPosition.X == gm->getCollectable( c )->getX() && currentPosition.Y == gm->getCollectable( c )->getY() ) {
						switch( gm->getCollectable( c )->getType() ) {
							case Collectable::ACID: {
								canDissolveWalls = true;
								//solution = partialSolution;
								break;
							}
							
							case Collectable::KEY: {
								solution = partialSolution;
								keyImSeeking = c;
								break;
							}
						}
					}
				}

				//See which direction(s) the bot can move
				if( currentPosition.Y > 0 && effectivelyNoTopWall( currentPosition.X, currentPosition.Y, canDissolveWalls ) && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
					possibleDirections.push_back( UP );
				}
				if( currentPosition.X > 0 && effectivelyNoLeftWall( currentPosition.X, currentPosition.Y, canDissolveWalls ) && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( LEFT );
				}
				if( currentPosition.Y < (rows - 1) && effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1, canDissolveWalls ) && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
					possibleDirections.push_back( DOWN );
				}
				if( currentPosition.X < (cols - 1) && effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y, canDissolveWalls ) && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( RIGHT );
				}

				if( possibleDirections.size() == 0 && partialSolution.size() != 0 ) {
					partialSolution.pop_back();
				} else {
					for( uint_fast8_t i = 0; ( i < possibleDirections.size() && solution.empty() ); ++i ) { //changed decltype( possibleDirections.size() ) to uint_fast8_t because the size of possibleDirections can never exceed 4 but could be stored in a needlessly large integer type.
						direction_t choice = possibleDirections.at( i );
						uint_fast16_t newDepthLimit = depthLimit - 1;
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y - 1 );
								
								if ( maze[ currentPosition.X ][ currentPosition.Y ].getTop() != MazeCell::ACIDPROOF && canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y + 1 );
								
								if ( maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() != MazeCell::ACIDPROOF && canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X - 1, currentPosition.Y );
								
								if ( maze[ currentPosition.X ][ currentPosition.Y ].getLeft() != MazeCell::ACIDPROOF && canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X + 1, currentPosition.Y );
								
								if ( maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() != MazeCell::ACIDPROOF && canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
						}
					}
				}

			} else { //If we're at the goal
				solution = partialSolution;
				return;
			}
		}
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolutionIDDFS(): " << e.what() << std::endl;
	}
	if( partialSolution.size() > 0 ) {
		partialSolution.pop_back();
	}
}

uint_fast8_t AI::getPlayer() {
	try {
		return controlsPlayer;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::AI(): " << e.what() << std::endl;
		return UINT_FAST8_MAX; //This should never be reached.
	}
}

void AI::keyFound( uint_fast8_t key ) {
	try {
		//std::wcout << L"Bot acknowledging key " << key << L" found." << std::endl;
		if( startSolved ) {
			//std::wcout << L"Seeking key " << keyImSeeking << std::endl;
			if( key == keyImSeeking ) {
				//std::wcout << L"They're the same" << std::endl;
				findSolution();
			}
		}

	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::keyFound(): " << e.what() << std::endl;
	}
}

void AI::move() {
	try {
		lastTimeMoved = gm->timer->getRealTime();
		irr::core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );

		if( startSolved ) {
			if( !solved ) {
				findSolution();
			}

			if( !solution.empty() ) {
				if( solution.back().X > currentPosition.X ) {
					gm->movePlayerOnX( controlsPlayer, 1 );
				} else if( solution.back().X < currentPosition.X ) {
					gm->movePlayerOnX( controlsPlayer, -1 );
				} else if( solution.back().Y > currentPosition.Y ) {
					gm->movePlayerOnY( controlsPlayer, 1 );
				} else if( solution.back().Y < currentPosition.Y ) {
					gm->movePlayerOnY( controlsPlayer, -1 );
				}
				solution.pop_back();
			} else {
				solved = false;
				startSolved = false;
				std::wcout << L"Could not find a solution" << std::endl;
				move();
			}

		} else {

			switch( algorithm ) {
				case DEPTH_FIRST_SEARCH: {
					core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );
					
					if( pathTaken.size() == 0 ) { //Ensures that the player's start position is marked as visited
						pathTaken.push_back( currentPosition );
					}
					
					if( !alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					std::vector< direction_t > possibleDirections;
					if( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {

						//Check for locks
						if( maze[ currentPosition.X ][ currentPosition.Y ].hasLock() ) {
							pathsToLockedCells.push_back( std::vector< core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
						}
						if( currentPosition.X < ( cols - 1 ) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].hasLeftLock() ) {
							pathsToLockedCells.push_back( std::vector< core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
							pathsToLockedCells.back().push_back( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) );
						}
						if( currentPosition.Y < ( rows - 1 ) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].hasTopLock() ) {
							pathsToLockedCells.push_back( std::vector< core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
							pathsToLockedCells.back().push_back( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) );
						}

						//See which direction(s) the bot can move
						if( currentPosition.Y > 0 && effectivelyNoTopWall( currentPosition.X, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 && effectivelyNoLeftWall( currentPosition.X, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (rows - 1) && effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1 ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (cols - 1) && effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}

					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.size() == 0 && pathTaken.size() != 0 && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						core::position2d< uint_fast8_t > oldPosition = pathTaken.back();
						for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
							if( pathsToLockedCells.at( o ).back() != oldPosition ) {
								pathsToLockedCells.at( o ).push_back( oldPosition );
							}
						}
						if( oldPosition.X < currentPosition.X ) {
							gm->movePlayerOnX( controlsPlayer, -1 );
						} else if( oldPosition.X > currentPosition.X ) {
							gm->movePlayerOnX( controlsPlayer, 1 );
						} else if( oldPosition.Y < currentPosition.Y ) {
							gm->movePlayerOnY( controlsPlayer, -1 );
						} else { //if( oldPosition.Y > currentPosition.Y ) {
							gm->movePlayerOnY( controlsPlayer, 1 );
						}
					} else if ( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) { //Go to next position
						
						direction_t choice = possibleDirections.at( rand() % possibleDirections.size() );
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, -1 );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, 1 );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnX( controlsPlayer, -1 );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnX( controlsPlayer, 1 );
							} break;
						}
					}
					break;
				}
				case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
					
					core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );
					
					if( pathTaken.empty() ) {//currentPosition.X == gm->getStart( controlsPlayer )->getX() && currentPosition.Y == gm->getStart( controlsPlayer )->getY() ) {
						IDDFSDepthLimit += 1;
						cellsVisited.clear();
						pathTaken.push_back( currentPosition );
					}
					
					if( !alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					std::vector< direction_t > possibleDirections;
					if( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {

						//See which direction(s) the bot can move
						if( currentPosition.Y > 0 && effectivelyNoTopWall( currentPosition.X, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 && effectivelyNoLeftWall( currentPosition.X, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (rows - 1) && effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1 ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (cols - 1) && effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y ) && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}
					
					if( pathTaken.size() == IDDFSDepthLimit ) {
						possibleDirections.clear();
					}

					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.empty() && !pathTaken.empty() && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						core::position2d< uint_fast8_t > oldPosition = pathTaken.back();

						if( oldPosition.X < currentPosition.X ) {
							gm->movePlayerOnX( controlsPlayer, -1 );
						} else if( oldPosition.X > currentPosition.X ) {
							gm->movePlayerOnX( controlsPlayer, 1 );
						} else if( oldPosition.Y < currentPosition.Y ) {
							gm->movePlayerOnY( controlsPlayer, -1 );
						} else { //if( oldPosition.Y > currentPosition.Y ) {
							gm->movePlayerOnY( controlsPlayer, 1 );
						}
					} else if ( !possibleDirections.empty() && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) { //Go to next position
						uint_fast8_t choiceNum = 0;//rand() % possibleDirections.size();
						direction_t choice = possibleDirections.at( choiceNum );
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								gm->movePlayerOnY( controlsPlayer, -1 );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								gm->movePlayerOnY( controlsPlayer, 1 );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								gm->movePlayerOnX( controlsPlayer, -1 );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								gm->movePlayerOnX( controlsPlayer, 1 );
							} break;
						}
					}
					break;
				}
				case RIGHT_HAND_RULE: {
					
					if( !alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 && noOriginalTopWall( currentPosition.X, currentPosition.Y ) ) {
								gm->movePlayerOnY( controlsPlayer, -1 );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 && noOriginalLeftWall( currentPosition.X, currentPosition.Y ) ) {
								gm->movePlayerOnX( controlsPlayer, -1 );
								hand = RIGHT;
							} else {
								hand = LEFT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < ( rows - 1 ) && noOriginalTopWall( currentPosition.X, currentPosition.Y + 1 ) ) {
								gm->movePlayerOnY( controlsPlayer, 1 );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < ( cols - 1 ) && noOriginalLeftWall( currentPosition.X + 1, currentPosition.Y ) ) {
								gm->movePlayerOnX( controlsPlayer, 1 );
								hand = LEFT;
							} else {
								hand = RIGHT;
								move();
							}
							break;
						}
					}
					break;
				}
				case LEFT_HAND_RULE: {
									
					if( !alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 && noOriginalTopWall( currentPosition.X, currentPosition.Y ) ) {
								gm->movePlayerOnY( controlsPlayer, -1 );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 && noOriginalLeftWall( currentPosition.X, currentPosition.Y ) ) {
								gm->movePlayerOnX( controlsPlayer, -1 );
								hand = LEFT;
							} else {
								hand = RIGHT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < (rows - 1) && noOriginalTopWall( currentPosition.X, currentPosition.Y + 1 ) ) {
								gm->movePlayerOnY( controlsPlayer, 1 );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < (cols - 1) && noOriginalLeftWall( currentPosition.X + 1, currentPosition.Y ) ) {
								gm->movePlayerOnX( controlsPlayer, 1 );
								hand = RIGHT;
							} else {
								hand = LEFT;
								move();
							}
							break;
						}
					}
					break;
				}
				default: {
					std::wcout << L"The current algorithm cannot be used when bots don't know the solution." << std::endl;
					break;
				}
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in AI::move(): " << e.what() << std::endl;
	}
}

bool AI::noOriginalLeftWall( uint_fast8_t x, uint_fast8_t y ) {
	return( maze[ x ][ y ].getOriginalLeft() == MazeCell::NONE || ( maze[ x ][ y ].getOriginalLeft() == MazeCell::LOCK && maze[ x ][ y ].getLeft() == MazeCell::NONE ) );
}

bool AI::noOriginalTopWall( uint_fast8_t x, uint_fast8_t y ) {
	return( maze[ x ][ y ].getOriginalTop() == MazeCell::NONE || ( maze[ x ][ y ].getOriginalTop() == MazeCell::LOCK && maze[ x ][ y ].getTop() == MazeCell::NONE ) );
}

void AI::reset() {
	try {
		noKeysLeft = false;
		keyImSeeking = 0;
		IDDFSDepthLimit = 1;
		hand = RIGHT; //Arbitrarily chosen direction
		lastTimeMoved = 0;
		solution.clear();
		solved = false;
		DFSCellsVisited.clear();
		IDDFSCellsVisited.clear();
		for( decltype( pathsToLockedCells.size() ) i = 0; i < pathsToLockedCells.size(); ++i ) {
			pathsToLockedCells.at( i ).clear();
		}
		pathsToLockedCells.clear();
		pathTaken.clear();
		cellsVisited.clear();

		if( startSolved && gm != nullptr ) {
			findSolution();
		}
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::reset(): " << e.what() << std::endl;
	}
}

void AI::setPlayer( uint_fast8_t newPlayer ) {
	try {
		controlsPlayer = newPlayer;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::setPlayer(): " << e.what() << std::endl;
	}
}

void AI::setup( MazeCell ** newMaze, uint_fast8_t newCols, uint_fast8_t newRows, GameManager *newGM, bool newStartSolved, algorithm_t newAlgorithm, uint_fast16_t newMovementDelay ) {
	try {
		movementDelay = newMovementDelay;
		maze = newMaze;
		cols = newCols;
		rows = newRows;
		gm = newGM;
		startSolved = newStartSolved;
		algorithm = newAlgorithm;
		reset();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::setup(): " << e.what() << std::endl;
	}
}
