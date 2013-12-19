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

#include "AI.h"
#include "GameManager.h"
#include "MazeManager.h"
#include "Player.h"
#include "StringConverter.h"

AI::AI() : algorithm( DEPTH_FIRST_SEARCH ), cols(0), controlsPlayer(0), gm(nullptr), keyImSeeking(0), lastTimeMoved(0), maze(nullptr), movementDelay(300), rows(0), solved(false), startSolved(true), IDDFSDepthLimit(1), IDDFSMaxDepthLimit(1), noKeysLeft(false), hand(RIGHT) {
	try {
		//TODO: Allow custom AI movement delays for increased/decreased challenge
		//setup( nullptr, 0, 0, nullptr );
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
		//pathTaken.clear();
		//cellsVisited.clear();
		if( gm->getDebugStatus() ) {
			//std::wcout << L"Bot " << controlsPlayer << L" acknowledging all keys found" << std::endl;
		}
		core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );

		for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {

			for( auto i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {

				std::vector< core::position2d< uint_fast8_t > >::size_type j = 0;
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

		std::vector< core::position2d< uint_fast8_t > >::size_type i = 0;
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

bool AI::alreadyVisitedDFS( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		std::vector< core::position2d< uint_fast8_t > >::size_type i = 0;
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
}

bool AI::alreadyVisitedIDDFS( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		std::vector< core::position2d< uint_fast8_t > >::size_type i = 0;
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
		core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );
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

void AI::findSolution() {
	try {
		solved = false;
		//numKeysInSolution = 0;
		keyImSeeking = 0;
		solution.clear();
		irr::core::position2d< uint_fast8_t > currentPosition( gm->getPlayer( controlsPlayer )->getX(), gm->getPlayer( controlsPlayer )->getY() );
		switch( algorithm ) {
			case DEPTH_FIRST_SEARCH: {
				DFSCellsVisited.clear();
				findSolutionDFS( currentPosition );
				solved = true;
				break;
			}
			case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
				DFSCellsVisited.clear();
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

		if( !solution.empty() ) {
			//while( solution.back().X == currentPosition.X && solution.back().Y == currentPosition.Y ) {
			while( solution.back() == currentPosition ) {
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
		findSolutionDFS( partialSolution, currentPosition );

		//Reverses the order of the solution, so we don't start and the wrong end
		std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
		while( !solution.empty() ) {
			tempSolution.push_back( solution.back() );
			solution.pop_back();
		}
		while( !tempSolution.empty() ) {
			solution.insert( solution.begin(), tempSolution.back() );
			tempSolution.pop_back();
		}
		solution.push_back( currentPosition );
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolutionDFS(): " << e.what() << std::endl;
	}
}

void AI::findSolutionDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition ) {
	try {
		uint_fast16_t maxDepth = ( uint_fast16_t ) cols * rows;
		IDDFSCellsVisited.clear();
		findSolutionIDDFS( partialSolution, currentPosition, maxDepth );
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
			findSolutionIDDFS( partialSolution, currentPosition, maxDepth );
		} else {
			for( uint_fast16_t i = 1; solution.size() == 0 && i <= maxDepth; i++ ) {
				if( gm->getDebugStatus() ) {
					std::wcout << L"In IDDFS loop, i=" << i << std::endl;
				}
				IDDFSCellsVisited.clear();
				findSolutionIDDFS( partialSolution, currentPosition, i );
			}
		}

		//Reverses the order of the solution, so we don't start at the wrong end
		std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
		while( !solution.empty() ) {
			tempSolution.push_back( solution.back() );
			solution.pop_back();
		}
		while( !tempSolution.empty() ) {
			solution.insert( solution.begin(), tempSolution.back() );
			tempSolution.pop_back();
		}
		solution.push_back( currentPosition );
	} catch( std::exception &e ) {
		std::wcout << L"Error in AI::findSolutionIDDFS(): " << e.what() << std::endl;
	}
}

//TODO: Find some way to make IDDFS faster, possibly using a caching mechanism
void AI::findSolutionIDDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition, uint_fast16_t depthLimit ) {
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
				for( uint_fast8_t k = 0; k < gm->getNumKeys(); ++k ) {
					if( ( currentPosition.X == gm->getKey( k )->getX() && currentPosition.Y == gm->getKey( k )->getY() ) ) {
						solution = partialSolution;
						keyImSeeking = k;
						return;
					}
				}
			}
			
			return; //No solution found, therefore solution.size() should be zero
		} else {

			IDDFSCellsVisited.push_back( currentPosition );
			partialSolution.push_back( currentPosition );
			std::vector< direction_t > possibleDirections;
			if( !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {

				for( uint_fast8_t k = 0; k < gm->getNumKeys(); ++k ) {
					if( ( currentPosition.X == gm->getKey( k )->getX() && currentPosition.Y == gm->getKey( k )->getY() ) ) {
						
						solution = partialSolution;
						
						keyImSeeking = k;
						
						break;
					}
				}

				//See which direction(s) the bot can move
				if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == MazeCell::NONE && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
					possibleDirections.push_back( UP );
				}
				if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( LEFT );
				}
				if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == MazeCell::NONE && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
					possibleDirections.push_back( DOWN );
				}
				if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisitedIDDFS( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( RIGHT );
				}

				if( possibleDirections.size() == 0 && partialSolution.size() != 0 ) {
					partialSolution.pop_back();
				} else {
					for( auto i = 0; ( i < possibleDirections.size() && solution.empty() ); ++i ) {
						direction_t choice = possibleDirections.at( i );
						uint_fast16_t newDepthLimit = depthLimit - 1;
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y - 1 );
								//partialSolution.push_back( newPosition );
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y + 1 );
								//partialSolution.push_back( newPosition );
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X - 1, currentPosition.Y );
								//partialSolution.push_back( newPosition );
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > newPosition( currentPosition.X + 1, currentPosition.Y );
								//partialSolution.push_back( newPosition );
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit );
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
			}

		} else {
			//if( algorithm == DEPTH_FIRST_SEARCH ) {
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
						if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}

					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.size() == 0 && pathTaken.size() != 0 && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						core::position2d< uint_fast8_t > oldPosition = pathTaken.back();
						for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
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
						//uint_fast8_t choiceNum = rand() % possibleDirections.size();
						direction_t choice = possibleDirections.at( rand() % possibleDirections.size() );
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, -1 );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, 1 );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnX( controlsPlayer, -1 );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
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
					
					/*if( IDDFSDepthLimit == 0 && pathTaken.size() > 0 ) {
						IDDFSMaxDepthLimit += 1;
						IDDFSDepthLimit = IDDFSMaxDepthLimit;
					} else if( IDDFSDepthLimit > 0 ) {
						IDDFSDepthLimit -= 1;
					}*/
					
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
						/*if( maze[ currentPosition.X ][ currentPosition.Y ].hasLock() ) {
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
						}*/

						//See which direction(s) the bot can move
						if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == MazeCell::NONE && !alreadyVisited( core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}
					
					if( pathTaken.size() >= IDDFSDepthLimit ) {
						possibleDirections.clear();
					}

					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.size() == 0 && pathTaken.size() > 1 && !( currentPosition.X == gm->getGoal()->getX() && currentPosition.Y == gm->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						core::position2d< uint_fast8_t > oldPosition = pathTaken.back();
						/*for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
							if( pathsToLockedCells.at( o ).back() != oldPosition ) {
								pathsToLockedCells.at( o ).push_back( oldPosition );
							}
						}*/
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
						//uint_fast8_t choiceNum = rand() % possibleDirections.size();
						direction_t choice = possibleDirections.at( rand() % possibleDirections.size() );
						switch( choice ) {
							case UP: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, -1 );
							} break;
							case DOWN: {
								core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnY( controlsPlayer, 1 );
							} break;
							case LEFT: {
								core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
									//for( std::vector< core::dimension2d< uint_fast8_t > >::size_type i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {
										pathsToLockedCells.at( o ).push_back( position );
									//}
								}
								gm->movePlayerOnX( controlsPlayer, -1 );
							} break;
							case RIGHT: {
								core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( auto o = 0; o < pathsToLockedCells.size(); ++o ) {
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
				case RIGHT_HAND_RULE: {
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == MazeCell::NONE ) {
								gm->movePlayerOnY( controlsPlayer, -1 );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == MazeCell::NONE ) {
								gm->movePlayerOnX( controlsPlayer, -1 );
								hand = RIGHT;
							} else {
								hand = LEFT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == MazeCell::NONE ) {
								gm->movePlayerOnY( controlsPlayer, 1 );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == MazeCell::NONE ) {
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
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getTop() == MazeCell::NONE ) {
								gm->movePlayerOnY( controlsPlayer, -1 );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 && maze[ currentPosition.X ][ currentPosition.Y ].getLeft() == MazeCell::NONE ) {
								gm->movePlayerOnX( controlsPlayer, -1 );
								hand = LEFT;
							} else {
								hand = RIGHT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < (rows - 1) && maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() == MazeCell::NONE ) {
								gm->movePlayerOnY( controlsPlayer, 1 );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < (cols - 1) && maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() == MazeCell::NONE ) {
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

void AI::reset() {
	try {
		hand = RIGHT;
		lastTimeMoved = 0;
		solution.clear();
		solved = false;
		DFSCellsVisited.clear();
		for( auto i = 0; i < pathsToLockedCells.size(); ++i ) {
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

void AI::setup( MazeCell ** newMaze, uint_fast8_t newCols, uint_fast8_t newRows, GameManager *newGM, bool newStartSolved, algorithm_t newAlgorithm ) {
	try {
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
