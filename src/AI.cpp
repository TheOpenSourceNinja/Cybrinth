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
 * The AI class is responsible for controlling computer players. It contains the maze-solving algorithms.
 */

#include "AI.h"
#include "MainGame.h"
#include "MazeManager.h"
#include "Player.h"
#include "SpellChecker.h"
#include "StringConverter.h"

///TODO: Update the AI to reflect the addition of a wall dissolver item (icon: spray can labeled 'ACID')
/* Progress report:
 * DFS and IDDFS can use acid.
 * Wall-following algorithms not updated yet because the maze may not be simply connected anymore. I've given up on them for now, but I'm leaving this todo note here. */

AI::AI() : controlsPlayer(0) {
	try {
		//setup( nullptr, 0, 0, nullptr, false, DEPTH_FIRST_SEARCH, 300 ); //setup( pointer to the maze, number of columns, number of rows, pointer to MainGame, whether to start solved, the algorithm to use, and the movement delay )
		setup( nullptr, false, DEPTH_FIRST_SEARCH, 300 ); //setup( pointer to MainGame, whether to start solved, the algorithm to use, and the movement delay )
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::AI(): " << e.what() << std::endl;
	}
}

AI::~AI() {
	try {
		pathsToLockedCells.clear();
		pathTaken.clear();
		pretendCellsVisited.clear();
		pretendCellsUnvisited.clear();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::~AI(): " << e.what() << std::endl;
	}
}

AI::algorithm_t AI::algorithmFromString( std::wstring input ) {
	std::vector< std::wstring > possibleChoices = { stringFromAlgorithm( DEPTH_FIRST_SEARCH ), stringFromAlgorithm( ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH ), stringFromAlgorithm( RIGHT_HAND_RULE ), stringFromAlgorithm( LEFT_HAND_RULE ), stringFromAlgorithm( DIJKSTRA ) };
	
	std::wstring choice;
	{
		SpellChecker spellChecker;
		choice = possibleChoices.at( spellChecker.indexOfClosestString( input, possibleChoices ) );
	}
	
	algorithm_t result = ALGORITHM_DO_NOT_USE;
	
	if( choice == possibleChoices.at( 0 ) ) { //DFS
		if( not( mg == nullptr or mg == NULL ) and mg->getDebugStatus() ) {
			std::wcout << L"Bots will use Depth-First Search" << std::endl;
		}
		result = DEPTH_FIRST_SEARCH;
	} else if( choice == possibleChoices.at( 1 ) ) { //IDDFS
		if( not( mg == nullptr or mg == NULL ) and mg->getDebugStatus() ) {
			std::wcout << L"Bots will use Iterative Deepening Depth-First Search" << std::endl;
		}
		result = ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH;
	} else if( choice == possibleChoices.at( 2 ) ) {
		if( not( mg == nullptr or mg == NULL ) and mg->getDebugStatus() ) {
			std::wcout << L"Bots will use the Right Hand Rule" << std::endl;
		}
		result = RIGHT_HAND_RULE;
	} else if( choice == possibleChoices.at( 3 ) ) {
		if( not( mg == nullptr or mg == NULL ) and mg->getDebugStatus() ) {
			std::wcout << L"Bots will use the Left Hand Rule" << std::endl;
		}
		result = LEFT_HAND_RULE;
	} else if( choice == possibleChoices.at( 4 ) ) {
		if( not( mg == nullptr or mg == NULL ) and mg->getDebugStatus() ) {
			std::wcout << L"Bots will use Dijkstra's algorithm" << std::endl;
		}
		result = DIJKSTRA;
	}
	
	return result;
}

void AI::allKeysFound() {
	try {
		noKeysLeft = true;

		for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {

			for( decltype( pathsToLockedCells.at( o ).size() ) i = 0; i < pathsToLockedCells.at( o ).size(); ++i ) {

				decltype( cellsVisited.size() ) j = 0;
				while( j < cellsVisited.size() ) {

					if( ( cellsVisited.at( j ).X == pathsToLockedCells.at( o )[ i ].X and cellsVisited.at( j ).Y == pathsToLockedCells.at( o )[ i ].Y ) ) {
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
		
		if( startSolved ) {
			findSolution();
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::allKeysFound(): " << e.what() << std::endl;
	}
}

bool AI::alreadyVisited( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		decltype( cellsVisited.size() ) i = 0;
		while( i < cellsVisited.size() and result not_eq true ) {
			if( cellsVisited.at( i ).X == position.X and cellsVisited.at( i ).Y == position.Y ) {
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

bool AI::alreadyVisitedPretend( irr::core::position2d< uint_fast8_t > position ) {
	try {
		bool result = false;

		decltype( pretendCellsVisited.size() ) i = 0;
		while( i < pretendCellsVisited.size() and result not_eq true ) {
			if( pretendCellsVisited.at( i ).X == position.X and pretendCellsVisited.at( i ).Y == position.Y ) {
				result = true;
			}
			++i;
		}
		return result;
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::alreadyVisitedPretend(): " << e.what() << std::endl;
		return false;
	}
}

bool AI::atGoal() {
	try {
		Player* p = mg->getPlayer( controlsPlayer );
		irr::core::position2d< uint_fast8_t > currentPosition( p->getX(), p->getY() );
		Goal* goal = mg->getGoal();
		if( currentPosition.X == goal->getX() and currentPosition.Y == goal->getY() ) {
			return true;
		} else {
			return false;
		}
	} catch( std::exception &e ) {
		std::wcerr << "Error in AI::atGoal(): " << e.what() << std::endl;
		return true;
	}
}

bool AI::doneWaiting() {
	try {
		if( lastTimeMoved < mg->timer->getRealTime() - movementDelay ) {
			return true;
		} else {
			return false;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::doneWaiting(): " << e.what() << std::endl;
		return false;
	}
}

bool AI::effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y ) {
	return effectivelyNoTopWall( x, y, ( mg->getPlayer( controlsPlayer )->hasItem() and mg->getPlayer( controlsPlayer )->getItemType() == Collectable::ACID ) );
}

bool AI::effectivelyNoTopWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls ) {
	if( mg not_eq nullptr and mg->getMazeManager() not_eq nullptr and mg->getMazeManager()->maze not_eq nullptr ) {
		auto topWall = mg->getMazeManager()->maze[ x ][ y ].getTop();
		return ( topWall == MazeCell::NONE or ( ( topWall not_eq MazeCell::LOCK and topWall not_eq MazeCell::ACIDPROOF )  and canDissolveWalls ) );
	} else {
		return false;
	}
}

bool AI::effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y ) {
	return effectivelyNoLeftWall( x, y, ( mg->getPlayer( controlsPlayer )->hasItem() and mg->getPlayer( controlsPlayer )->getItemType() == Collectable::ACID ) );
}

bool AI::effectivelyNoLeftWall( uint_fast8_t x, uint_fast8_t y, bool canDissolveWalls ) {
	if( mg not_eq nullptr and mg->getMazeManager()->maze not_eq nullptr ) {
		auto leftWall = mg->getMazeManager()->maze[ x ][ y ].getLeft();
		return( leftWall == MazeCell::NONE or ( ( leftWall not_eq MazeCell::LOCK and leftWall not_eq MazeCell::ACIDPROOF ) and canDissolveWalls ) );
	} else {
		return false;
	}
}

void AI::findSolution() {
	try {
		solved = false;
		//numKeysInSolution = 0;
		keyImSeeking = UINT_FAST8_MAX;
		solution.clear();
		{
			Player* p = mg->getPlayer( controlsPlayer );
			irr::core::position2d< uint_fast8_t > currentPosition( p->getX(), p->getY() );
			switch( algorithm ) {
				case DEPTH_FIRST_SEARCH: {
					pretendCellsVisited.clear();
					findSolutionDFS( currentPosition );
					solved = true;
					break;
				}
				case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
					pretendCellsVisited.clear();
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
				case DIJKSTRA: {
					findSolutionDijkstra( currentPosition );
					break;
				}
				default: {
					StringConverter sc;
					throw( std::wstring( L"Algorithm " ) + sc.toStdWString( algorithm ) + L" not yet added to findSolution()." );
				}
			}

			while( not solution.empty() and solution.back() == currentPosition ) {
				solution.pop_back();
			}
			
			if( mg->getDebugStatus() ) {
				std::wcout << L"Solution: ";
				for( decltype( solution.size() ) i = 0; i < solution.size(); ++i ) {
					std::wcout << L"(" << solution.at( i ).X << L"," << solution.at( i ).Y << L") ";
				}
				std::wcout << std::endl;
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::findSolution(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in AI::findSolution(): " << e << std::endl;
	}
}

void AI::findSolutionDFS( irr::core::position2d< uint_fast8_t > startPosition ) {
	try {
		std::vector< irr::core::position2d< uint_fast8_t > > partialSolution;
		//Instead of adding a bunch of code for DFS, just do IDDFS with the deepest max depth possible.
		uint_fast16_t maxDepth = static_cast< uint_fast16_t > ( mg->getMazeManager()->cols ) * static_cast< uint_fast16_t > ( mg->getMazeManager()->rows );
		pretendCellsVisited.clear();
		IDDFSDeadEnds.clear();
		findSolutionIDDFS( partialSolution, startPosition, maxDepth, false );

		{ //Reverses the order of the solution, so we don't start at the wrong end
			std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
			while( not solution.empty() ) {
				tempSolution.push_back( solution.back() );
				solution.pop_back();
			}
			while( not tempSolution.empty() ) {
				solution.insert( solution.begin(), tempSolution.back() );
				tempSolution.pop_back();
			}
		}
		solution.push_back( startPosition );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::findSolutionDFS(): " << e.what() << std::endl;
	}
}

void AI::findSolutionDijkstra( irr::core::position2d< uint_fast8_t > startPosition ) {
	try {
		if( mg->getDebugStatus() ) {
			std::wcout << L"Solving the maze using Dijkstra's algorithm. Start position is (" << startPosition.X << L"," << startPosition.Y << L")" << std::endl;
		}
		
		auto maze = mg->getMazeManager();
		if( maze->rows > 0 and maze->cols > 0 ) { //The maze size can be zero when the game first starts.
			std::vector< std::vector< irr::core::position2d< uint_fast8_t > > > previous;
			DijkstraDistance.clear();
			DijkstraDistance.resize( maze->cols );
			previous.clear();
			previous.resize( DijkstraDistance.size() );
			for( decltype( DijkstraDistance.size() ) i = 0; i < DijkstraDistance.size(); ++i ) {
				DijkstraDistance.at( i ).clear();
				DijkstraDistance.at( i ).resize( maze->rows );
				previous.at( i ).clear();
				previous.at( i ).resize( maze->rows );
			}
			auto undefined = irr::core::position2d< uint_fast8_t >( UINT_FAST8_MAX, UINT_FAST8_MAX ); //An approximation for previous[v]  := undefined
			//irr::core::position2d< uint_fast8_t > target;
			std::vector< irr::core::position2d< uint_fast8_t > > targets;
			decltype( targets.size() ) targetNumber = 0; //If a target gets found, targetNumber will identify it
			
			for( decltype( mg->getNumCollectables() ) c = 0; c < mg->getNumCollectables(); ++c ) {
				auto collectable = mg->getCollectable( c );
				targets.push_back( irr::core::position2d< uint_fast8_t >( collectable->getX(), collectable->getY() ) );
			}
			targets.push_back( irr::core::position2d< uint_fast8_t >( mg->getGoal()->getX(), mg->getGoal()->getY() ) );
			
			DijkstraDistance.at( startPosition.X ).at( startPosition.Y ) = 0; //dist[source]  := 0 // Distance from source to source
			previous.at( startPosition.X ).at( startPosition.Y ) = undefined;
			
			for( decltype( maze->cols ) x = 0; x < maze->cols; ++x ) {
				for( decltype( maze->rows ) y = 0; y < maze->rows; ++y ) { //for each vertex v in Graph: // Initializations
					auto v = irr::core::position2d< decltype( x ) >( x, y );
					if( v not_eq startPosition ) { //if v ≠ source
						DijkstraDistance.at( v.X ).at( v.Y ) = UINT_FAST16_MAX; //dist[v] := infinity // Unknown distance function from source to v
						previous.at( v.X ).at( v.Y ) = undefined; //previous[v]  := undefined
					}
					pretendCellsUnvisited.push_back( v ); //add v to Q // All nodes initially in Q (unvisited nodes)
				}
			}
			
			bool haveAcid = false;
			
			while( not pretendCellsUnvisited.empty() ) { //while Q is not empty: // The main loop
				
				if( mg->getDebugStatus() ) {
					std::wcout << L"Distances: " << std::endl;
					for( decltype( maze->rows ) y = 0; y < maze->rows; ++y ) {
						for( decltype( maze->cols ) x = 0; x < maze->cols; ++x ) {
							std::wcout << DijkstraDistance.at( x ).at( y ) << L"\t";
						}
						std::wcout << std::endl;
					}
				}
				
				uint_fast16_t minDistance = UINT_FAST16_MAX;
				auto u = pretendCellsUnvisited.at( 0 );
				for( decltype( pretendCellsUnvisited.size() ) i = 0; i < pretendCellsUnvisited.size(); ++i ) {
					if( DijkstraDistance.at( pretendCellsUnvisited.at( i ).X ).at( pretendCellsUnvisited.at( i ).Y ) <= minDistance ) {
						minDistance = DijkstraDistance.at( pretendCellsUnvisited.at( i ).X ).at( pretendCellsUnvisited.at( i ).Y );
						u = pretendCellsUnvisited.at( i ); //u := vertex in Q with min dist[u] // Source node in first case
					}
				}
				
				for( decltype( pretendCellsUnvisited.size() ) i = 0; i < pretendCellsUnvisited.size(); ++i ) {
					if( pretendCellsUnvisited.at( i ) == u ) {
						pretendCellsUnvisited.erase( pretendCellsUnvisited.begin() + i ); //remove u from Q
						break;
					}
				}
				
				bool targetFound = false;
				for( decltype( targets.size() ) t = 0; not targetFound and t < targets.size(); ++t ) {
					if( u == targets.at( t ) ) {
						targetFound = true;
						/*for( decltype( mg->getNumCollectables() ) c = 0; c < mg->getNumCollectables(); ++c ) { //TODO: Dijkstra's algorithm seems to go into an infinite loop if it recognizes acid. Fix it.
							auto collectable = mg->getCollectable( c );
							if( collectable->getX() == u.X and collectable->getY() == u.Y ) {
								if( collectable->getType() == Collectable::ACID ) {
									haveAcid = true;
								}
								break;
							}
						}*/
						break;
					}
				}
				
				if( not targetFound ) {
					decltype( pretendCellsUnvisited ) possibleNeighbors;
					if( u.X > 0 and effectivelyNoLeftWall( u.X, u.Y, haveAcid ) ) {
						possibleNeighbors.push_back( irr::core::position2d< uint_fast8_t >( u.X - 1, u.Y ) );
					}
					if( u.X < maze->cols - 1 and effectivelyNoLeftWall( u.X + 1, u.Y, haveAcid ) ) {
						possibleNeighbors.push_back( irr::core::position2d< uint_fast8_t >( u.X + 1, u.Y ) );
					}
					if( u.Y > 0 and effectivelyNoTopWall( u.X, u.Y, haveAcid ) ) {
						possibleNeighbors.push_back( irr::core::position2d< uint_fast8_t >( u.X, u.Y - 1 ) );
					}
					if( u.Y < maze->rows - 1 and effectivelyNoTopWall( u.X, u.Y + 1, haveAcid ) ) {
						possibleNeighbors.push_back( irr::core::position2d< uint_fast8_t >( u.X, u.Y + 1 ) );
					}
					
					uint_fast16_t alt = 0;
					
					if( possibleNeighbors.size() > 0 ) {
						for( decltype( possibleNeighbors.size() ) i = 0; i < possibleNeighbors.size(); ++i ) {
							bool stillUnvisited = false;
							irr::core::position2d< uint_fast8_t > v = undefined; //for each neighbor v of u: // where v has not yet been removed from Q.
							for( decltype( pretendCellsUnvisited.size() ) j = 0; not stillUnvisited and j < pretendCellsUnvisited.size(); ++j ) {
								if( pretendCellsUnvisited.at( j ) == possibleNeighbors.at( i ) ) {
									stillUnvisited = true;
									v = possibleNeighbors.at( i );
								}
							}
							
							if( v not_eq undefined ) {
								alt = DijkstraDistance.at( u.X ).at( u.Y ) + 1; //alt := dist[u] + length(u, v)
								if( alt < DijkstraDistance.at( v.X ).at( v.Y ) ) { //if alt < dist[v]: // A shorter path to v has been found
									DijkstraDistance.at( v.X ).at( v.Y ) = alt;
									previous.at( v.X ).at( v.Y ) = u; //previous[v]  := u
								} //end if
							}
						} //end for
					}
				} //end if u not_eq target
			} //end while
			
			solution.clear(); //S := empty sequence
			auto u = targets.at( targetNumber ); //u := target
			while( previous.at( u.X ).at( u.Y ) not_eq undefined ) { //while previous[u] is defined: // Construct the shortest path with a stack S
				//solution.insert( solution.begin(), u ); //insert u at the beginning of S // Push the vertex into the stack
				solution.push_back( u );
				u = previous.at( u.X ).at( u.Y ); //u := previous[u] // Traverse from target to source
			} //end while

		} else { //Maze size is zero
		}
		
		if( solution.size() > 0 ) {
			solved = true;
		}
		
		if( mg->getDebugStatus() ) {
			std::wcout << L"Done solving using Dijkstra's algorithm. Solution size: " << solution.size() << std::endl;
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::findSolutionDijkstra(): " << e.what() << std::endl;
	} catch( std::wstring &e ) {
		std::wcerr << L"Error in AI::findSolutionDijkstra(): " << e << std::endl;
	}
}

void AI::findSolutionIDDFS( irr::core::position2d< uint_fast8_t > startPosition ) {
	try {
		std::vector< irr::core::position2d< uint_fast8_t > > partialSolution;
		
		uint_fast16_t maxDepth = static_cast< uint_fast16_t >( mg->getMazeManager()->cols ) * static_cast< uint_fast16_t >( mg->getMazeManager()->rows );
		
		if( noKeysLeft ) { //If there aren't any keys left, the only thing left to try for is the goal. There's no point in using a less-than-maximum depth limit in that case.
			pretendCellsVisited.clear();
			IDDFSDeadEnds.clear();
			findSolutionIDDFS( partialSolution, startPosition, maxDepth, false );
		} else {
			IDDFSDeadEnds.clear();
			for( decltype( maxDepth ) i = 1; solution.empty() and i <= maxDepth; ++i ) {
				if( mg->getDebugStatus() ) {
					std::wcout << L"In IDDFS loop, i=" << i << std::endl;
				}
				pretendCellsVisited.clear();
				findSolutionIDDFS( partialSolution, startPosition, i, false );
			}
		}

		{ //Reverses the order of the solution, so we don't start at the wrong end
			std::vector< irr::core::position2d< uint_fast8_t > > tempSolution;
			while( not solution.empty() ) {
				tempSolution.push_back( solution.back() );
				solution.pop_back();
			}
			while( not tempSolution.empty() ) {
				solution.insert( solution.begin(), tempSolution.back() );
				tempSolution.pop_back();
			}
		}
		solution.push_back( startPosition );
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::findSolutionIDDFS(): " << e.what() << std::endl;
	}
}

void AI::findSolutionIDDFS( std::vector< irr::core::position2d< uint_fast8_t > > partialSolution, irr::core::position2d< uint_fast8_t > currentPosition, uint_fast16_t depthLimit, bool canDissolveWalls ) {
	try {
		if( mg->getDebugStatus() ) {
			std::wcout << L"findSolutionIDDFS: currentPosition: " << currentPosition.X << L"x" << currentPosition.Y << L" goal: " << mg->getGoal()->getX() << L"x" << mg->getGoal()->getY() << L" depthLimit: " << depthLimit << std::endl;
		}

		if( depthLimit == 0 ) {
			partialSolution.push_back( currentPosition );
			if( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) {
				solution = partialSolution;
				return;
			} else {
				for( decltype( mg->getNumCollectables() ) c = 0; c < mg->getNumCollectables(); ++c ) {
					if( ( currentPosition.X == mg->getCollectable( c )->getX() and currentPosition.Y == mg->getCollectable( c )->getY() ) ) {
						switch( mg->getCollectable( c )->getType() ) {
							case Collectable::ACID: {
								//canDissolveWalls = true; //Commenting this out because Clang's static analyzer says canDissolveWalls is never read after this point.
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
			
			//No solution found, therefore solution.size() should be zero
			//IDDFSDeadEnds.push_back( currentPosition );
			return;
		} else {

			pretendCellsVisited.push_back( currentPosition );
			partialSolution.push_back( currentPosition );
			std::vector< direction_t > possibleDirections;
			if( not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) {
				
				for( decltype( mg->getNumCollectables() ) c = 0; c < mg->getNumCollectables(); ++c ) {
					if( currentPosition.X == mg->getCollectable( c )->getX() and currentPosition.Y == mg->getCollectable( c )->getY() ) {
						switch( mg->getCollectable( c )->getType() ) {
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
				if( currentPosition.Y > 0 and effectivelyNoTopWall( currentPosition.X, currentPosition.Y, canDissolveWalls ) and not alreadyVisitedPretend( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) and not IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
					possibleDirections.push_back( UP );
				}
				if( currentPosition.X > 0 and effectivelyNoLeftWall( currentPosition.X, currentPosition.Y, canDissolveWalls ) and not alreadyVisitedPretend( irr::core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) and not IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( LEFT );
				}
				if( currentPosition.Y < (mg->getMazeManager()->rows - 1) and effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1, canDissolveWalls ) and not alreadyVisitedPretend( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) and not IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
					possibleDirections.push_back( DOWN );
				}
				if( currentPosition.X < (mg->getMazeManager()->cols - 1) and effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y, canDissolveWalls ) and not alreadyVisitedPretend( irr::core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) and not IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
					possibleDirections.push_back( RIGHT );
				}
				
				if( possibleDirections.empty() ) {
					IDDFSDeadEnds.push_back( currentPosition );
				}
				
				if( possibleDirections.empty() and partialSolution.size() not_eq 0 ) {
					partialSolution.pop_back();
				} else {
					while( not possibleDirections.empty() ) { //for( uint_fast8_t i = 0; ( i < possibleDirections.size() and solution.empty() ); ++i ) { //changed decltype( possibleDirections.size() ) to uint_fast8_t because the size of possibleDirections can never exceed 4 but could be stored in a needlessly large integer type.
						uint_fast8_t choiceInt = mg->getRandomNumber() % possibleDirections.size(); //rand() % possibleDirections.size();
						direction_t choice = possibleDirections.at( choiceInt );
						//direction_t choice = possibleDirections.at( i );
						auto newDepthLimit = depthLimit - 1;
						switch( choice ) {
							case UP: {
								irr::core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y - 1 );
								
								if ( mg->getMazeManager()->maze[ currentPosition.X ][ currentPosition.Y ].getTop() not_eq MazeCell::ACIDPROOF and canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case DOWN: {
								irr::core::position2d< uint_fast8_t > newPosition( currentPosition.X, currentPosition.Y + 1 );
								
								if ( mg->getMazeManager()->maze[ currentPosition.X ][ currentPosition.Y + 1 ].getTop() not_eq MazeCell::ACIDPROOF and canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case LEFT: {
								irr::core::position2d< uint_fast8_t > newPosition( currentPosition.X - 1, currentPosition.Y );
								
								if ( mg->getMazeManager()->maze[ currentPosition.X ][ currentPosition.Y ].getLeft() not_eq MazeCell::ACIDPROOF and canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
							case RIGHT: {
								irr::core::position2d< uint_fast8_t > newPosition( currentPosition.X + 1, currentPosition.Y );
								
								if ( mg->getMazeManager()->maze[ currentPosition.X + 1 ][ currentPosition.Y ].getLeft() not_eq MazeCell::ACIDPROOF and canDissolveWalls ) {
									canDissolveWalls = false;
								}
								
								findSolutionIDDFS( partialSolution, newPosition, newDepthLimit, canDissolveWalls );
							} break;
						}
						possibleDirections.erase( possibleDirections.begin() + choiceInt );
					}
				}

			} else { //If we're at the goal
				solution = partialSolution;
				return;
			}
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::findSolutionIDDFS(): " << e.what() << std::endl;
	}
	if( partialSolution.size() > 0 ) {
		partialSolution.pop_back();
	}
}

uint_fast8_t AI::getPlayer() {
	return controlsPlayer;
}

bool AI::IDDFSIsDeadEnd( irr::core::position2d< uint_fast8_t > position ) {
	for( decltype( IDDFSDeadEnds.size() ) i = 0; i < IDDFSDeadEnds.size(); ++i ) {
		if( IDDFSDeadEnds.at( i ) == position ) {
			return true;
		}
	}
	return false;
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
		std::wcerr << L"Error in AI::keyFound(): " << e.what() << std::endl;
	}
}

void AI::move() {
	try {
		lastTimeMoved = mg->timer->getRealTime();
		irr::core::position2d< uint_fast8_t > currentPosition( mg->getPlayer( controlsPlayer )->getX(), mg->getPlayer( controlsPlayer )->getY() );

		if( startSolved ) {
			while( not solved or solution.empty() ) {
				findSolution();
			}
			
			if( not solution.empty() ) {
				if( solution.back().X > currentPosition.X ) {
					mg->movePlayerOnX( controlsPlayer, 1, false );
				} else if( solution.back().X < currentPosition.X ) {
					mg->movePlayerOnX( controlsPlayer, -1, false );
				} else if( solution.back().Y > currentPosition.Y ) {
					mg->movePlayerOnY( controlsPlayer, 1, false );
				} else if( solution.back().Y < currentPosition.Y ) {
					mg->movePlayerOnY( controlsPlayer, -1, false );
				}
				solution.pop_back();
			} else {
				solved = false;
				startSolved = false;
				std::wcerr << L"Could not find a solution" << std::endl;
				move();
			}

		} else {

			switch( algorithm ) {
				case DEPTH_FIRST_SEARCH: {
					irr::core::position2d< uint_fast8_t > currentPosition( mg->getPlayer( controlsPlayer )->getX(), mg->getPlayer( controlsPlayer )->getY() );
					
					if( pathTaken.size() == 0 ) { //Ensures that the player's start position is marked as visited
						pathTaken.push_back( currentPosition );
					}
					
					if( not alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					std::vector< direction_t > possibleDirections;
					if( not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) {

						//Check for locks
						if( mg->getMazeManager()->maze[ currentPosition.X ][ currentPosition.Y ].hasLock() ) {
							pathsToLockedCells.push_back( std::vector< irr::core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
							pathsToLockedCells.back().push_back( currentPosition );
						}
						if( currentPosition.X < ( mg->getMazeManager()->cols - 1 ) and mg->getMazeManager()->maze[ currentPosition.X + 1 ][ currentPosition.Y ].hasLeftLock() ) {
							pathsToLockedCells.push_back( std::vector< irr::core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
							pathsToLockedCells.back().push_back( irr::core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) );
						}
						if( currentPosition.Y < ( mg->getMazeManager()->rows - 1 ) and mg->getMazeManager()->maze[ currentPosition.X ][ currentPosition.Y + 1 ].hasTopLock() ) {
							pathsToLockedCells.push_back( std::vector< irr::core::position2d< uint_fast8_t > >() );
							pathsToLockedCells.back().push_back( currentPosition );
							pathsToLockedCells.back().push_back( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) );
						}

						//See which direction(s) the bot can move
						if( currentPosition.Y > 0 and effectivelyNoTopWall( currentPosition.X, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 and effectivelyNoLeftWall( currentPosition.X, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (mg->getMazeManager()->rows - 1) and effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1 ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (mg->getMazeManager()->cols - 1) and effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}
					
					
					
					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.size() == 0 and pathTaken.size() not_eq 0 and not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						irr::core::position2d< uint_fast8_t > oldPosition = pathTaken.back();
						
						for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
							if( pathsToLockedCells.at( o ).back() not_eq currentPosition ) {
								pathsToLockedCells.at( o ).push_back( currentPosition );
							} else {
								pathsToLockedCells.at( o ).pop_back();
								//pathsToLockedCells.at( o ).push_back( currentPosition );
							}
						}
						if( oldPosition.X < currentPosition.X ) {
							mg->movePlayerOnX( controlsPlayer, -1, false );
						} else if( oldPosition.X > currentPosition.X ) {
							mg->movePlayerOnX( controlsPlayer, 1, false );
						} else if( oldPosition.Y < currentPosition.Y ) {
							mg->movePlayerOnY( controlsPlayer, -1, false );
						} else { //if( oldPosition.Y > currentPosition.Y ) {
							mg->movePlayerOnY( controlsPlayer, 1, false );
						}
					} else if ( not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) { //Go to next position
						
						direction_t choice = possibleDirections.at( mg->getRandomNumber() % possibleDirections.size() ); //rand() % possibleDirections.size() );
						switch( choice ) {
							case UP: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									if( pathsToLockedCells.at( o ).back() not_eq position ) {
										pathsToLockedCells.at( o ).push_back( position );
									} else {
										pathsToLockedCells.at( o ).pop_back();
									}
								}
								mg->movePlayerOnY( controlsPlayer, -1, false );
							} break;
							case DOWN: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									if( pathsToLockedCells.at( o ).back() not_eq position ) {
										pathsToLockedCells.at( o ).push_back( position );
									} else {
										pathsToLockedCells.at( o ).pop_back();
									}
								}
								mg->movePlayerOnY( controlsPlayer, 1, false );
							} break;
							case LEFT: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									if( pathsToLockedCells.at( o ).back() not_eq position ) {
										pathsToLockedCells.at( o ).push_back( position );
									} else {
										pathsToLockedCells.at( o ).pop_back();
									}
								}
								mg->movePlayerOnX( controlsPlayer, -1, false );
							} break;
							case RIGHT: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								for( decltype( pathsToLockedCells.size() ) o = 0; o < pathsToLockedCells.size(); ++o ) {
									if( pathsToLockedCells.at( o ).back() not_eq position ) {
										pathsToLockedCells.at( o ).push_back( position );
									} else {
										pathsToLockedCells.at( o ).pop_back();
									}
								}
								mg->movePlayerOnX( controlsPlayer, 1, false );
							} break;
						}
					}
					
					if( mg->getDebugStatus() ) {
						for( decltype( pathsToLockedCells.size() ) d = 0; d < pathsToLockedCells.size(); ++d ) {
							std::wcout << L"pathsToLockedCells.at( " << d << L" ).size(): " << pathsToLockedCells.at( d ).size() << std::endl;
						}
					}
					
					break;
				}
				case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
					
					irr::core::position2d< uint_fast8_t > currentPosition( mg->getPlayer( controlsPlayer )->getX(), mg->getPlayer( controlsPlayer )->getY() );
					
					if( pathTaken.empty() ) {//currentPosition.X == mg->getStart( controlsPlayer )->getX() and currentPosition.Y == mg->getStart( controlsPlayer )->getY() ) {
						IDDFSDepthLimit += 1;
						cellsVisited.clear();
						pathTaken.push_back( currentPosition );
					}
					
					if( not alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					std::vector< direction_t > possibleDirections;
					if( not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) {

						//See which direction(s) the bot can move
						if( currentPosition.Y > 0 and effectivelyNoTopWall( currentPosition.X, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y - 1 ) ) ) {
							possibleDirections.push_back( UP );
						}
						if( currentPosition.X > 0 and effectivelyNoLeftWall( currentPosition.X, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X - 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( LEFT );
						}
						if( currentPosition.Y < (mg->getMazeManager()->rows - 1) and effectivelyNoTopWall( currentPosition.X, currentPosition.Y + 1 ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X, currentPosition.Y + 1 ) ) ) {
							possibleDirections.push_back( DOWN );
						}
						if( currentPosition.X < (mg->getMazeManager()->cols - 1) and effectivelyNoLeftWall( currentPosition.X + 1, currentPosition.Y ) and not alreadyVisited( irr::core::position2d< uint_fast8_t >( currentPosition.X + 1, currentPosition.Y ) ) ) {
							possibleDirections.push_back( RIGHT );
						}
					}
					
					if( pathTaken.size() == IDDFSDepthLimit ) {
						possibleDirections.clear();
					}

					//If we can't go anywhere new, go back to previous position
					if( possibleDirections.empty() and not pathTaken.empty() and not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) {
						pathTaken.pop_back();
						irr::core::position2d< uint_fast8_t > oldPosition = pathTaken.back();

						if( oldPosition.X < currentPosition.X ) {
							mg->movePlayerOnX( controlsPlayer, -1, false );
						} else if( oldPosition.X > currentPosition.X ) {
							mg->movePlayerOnX( controlsPlayer, 1, false );
						} else if( oldPosition.Y < currentPosition.Y ) {
							mg->movePlayerOnY( controlsPlayer, -1, false );
						} else { //if( oldPosition.Y > currentPosition.Y ) {
							mg->movePlayerOnY( controlsPlayer, 1, false );
						}
					} else if ( not possibleDirections.empty() and not ( currentPosition.X == mg->getGoal()->getX() and currentPosition.Y == mg->getGoal()->getY() ) ) { //Go to next position
						uint_fast8_t choiceNum = 0;//rand() % possibleDirections.size();
						direction_t choice = possibleDirections.at( choiceNum );
						switch( choice ) {
							case UP: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y - 1 );
								pathTaken.push_back( position );
								mg->movePlayerOnY( controlsPlayer, -1, false );
							} break;
							case DOWN: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X, currentPosition.Y + 1 );
								pathTaken.push_back( position );
								mg->movePlayerOnY( controlsPlayer, 1, false );
							} break;
							case LEFT: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X - 1, currentPosition.Y );
								pathTaken.push_back( position );
								mg->movePlayerOnX( controlsPlayer, -1, false );
							} break;
							case RIGHT: {
								irr::core::position2d< uint_fast8_t > position( currentPosition.X + 1, currentPosition.Y );
								pathTaken.push_back( position );
								mg->movePlayerOnX( controlsPlayer, 1, false );
							} break;
						}
					}
					break;
				}
				case RIGHT_HAND_RULE: {
					
					if( not alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 and noOriginalTopWall( currentPosition.X, currentPosition.Y ) ) {
								mg->movePlayerOnY( controlsPlayer, -1, false );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 and noOriginalLeftWall( currentPosition.X, currentPosition.Y ) ) {
								mg->movePlayerOnX( controlsPlayer, -1, false );
								hand = RIGHT;
							} else {
								hand = LEFT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < ( mg->getMazeManager()->rows - 1 ) and noOriginalTopWall( currentPosition.X, currentPosition.Y + 1 ) ) {
								mg->movePlayerOnY( controlsPlayer, 1, false );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < ( mg->getMazeManager()->cols - 1 ) and noOriginalLeftWall( currentPosition.X + 1, currentPosition.Y ) ) {
								mg->movePlayerOnX( controlsPlayer, 1, false );
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
									
					if( not alreadyVisited( currentPosition ) ) {
						cellsVisited.push_back( currentPosition );
					}
					
					switch( hand ) {
						case RIGHT: {
							if( currentPosition.Y > 0 and noOriginalTopWall( currentPosition.X, currentPosition.Y ) ) {
								mg->movePlayerOnY( controlsPlayer, -1, false );
								hand = UP;
							} else {
								hand = DOWN;
								move();
							}
							break;
						}
						case UP: {
							if( currentPosition.X > 0 and noOriginalLeftWall( currentPosition.X, currentPosition.Y ) ) {
								mg->movePlayerOnX( controlsPlayer, -1, false );
								hand = LEFT;
							} else {
								hand = RIGHT;
								move();
							}
							break;
						}
						case LEFT: {
							if( currentPosition.Y < (mg->getMazeManager()->rows - 1) and noOriginalTopWall( currentPosition.X, currentPosition.Y + 1 ) ) {
								mg->movePlayerOnY( controlsPlayer, 1, false );
								hand = DOWN;
							} else {
								hand = UP;
								move();
							}
							break;
						}
						case DOWN: {
							if( currentPosition.X < (mg->getMazeManager()->cols - 1) and noOriginalLeftWall( currentPosition.X + 1, currentPosition.Y ) ) {
								mg->movePlayerOnX( controlsPlayer, 1, false );
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
					std::wcerr << L"The current algorithm cannot be used when bots don't know the solution." << std::endl;
					break;
				}
			}
		}
	} catch ( std::exception &e ) {
		std::wcerr << L"Error in AI::move(): " << e.what() << std::endl;
	}
}

bool AI::noOriginalLeftWall( uint_fast8_t x, uint_fast8_t y ) {
	return( mg->getMazeManager()->maze[ x ][ y ].getOriginalLeft() == MazeCell::NONE or ( mg->getMazeManager()->maze[ x ][ y ].getOriginalLeft() == MazeCell::LOCK and mg->getMazeManager()->maze[ x ][ y ].getLeft() == MazeCell::NONE ) );
}

bool AI::noOriginalTopWall( uint_fast8_t x, uint_fast8_t y ) {
	return( mg->getMazeManager()->maze[ x ][ y ].getOriginalTop() == MazeCell::NONE or ( mg->getMazeManager()->maze[ x ][ y ].getOriginalTop() == MazeCell::LOCK and mg->getMazeManager()->maze[ x ][ y ].getTop() == MazeCell::NONE ) );
}

void AI::reset() {
	try {
		noKeysLeft = false;
		keyImSeeking = UINT_FAST8_MAX;
		IDDFSDepthLimit = 1;
		hand = RIGHT; //Arbitrarily chosen direction
		lastTimeMoved = 0;
		solution.clear();
		solved = false;
		pretendCellsVisited.clear();
		for( decltype( pathsToLockedCells.size() ) i = 0; i < pathsToLockedCells.size(); ++i ) {
			pathsToLockedCells.at( i ).clear();
		}
		pathsToLockedCells.clear();
		pathTaken.clear();
		cellsVisited.clear();

		if( startSolved and mg not_eq nullptr ) {
			findSolution();
		}
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::reset(): " << e.what() << std::endl;
	}
}

void AI::setPlayer( uint_fast8_t newPlayer ) {
	controlsPlayer = newPlayer;
}

void AI::setup( MainGame *newGM, bool newStartSolved, algorithm_t newAlgorithm, uint_fast16_t newMovementDelay ) {
	try {
		movementDelay = newMovementDelay;
		mg = newGM;
		startSolved = newStartSolved;
		algorithm = newAlgorithm;
		reset();
	} catch( std::exception &e ) {
		std::wcerr << L"Error in AI::setup(): " << e.what() << std::endl;
	}
}

std::wstring AI::stringFromAlgorithm( algorithm_t input ) {
	switch( input ) {
		case DEPTH_FIRST_SEARCH: {
			return L"depth-first search";
		}
		case ITERATIVE_DEEPENING_DEPTH_FIRST_SEARCH: {
			return L"iterative deepening depth-first search";
		}
		case RIGHT_HAND_RULE: {
			return L"right hand rule";
		}
		case LEFT_HAND_RULE: {
			return L"left hand rule";
		}
		case DIJKSTRA: {
			return L"dijkstra";
		}
		default: {
			return L"Unrecognized algorithm";
		}
	}
}
