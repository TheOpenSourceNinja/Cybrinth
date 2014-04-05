#ifndef MAZEMANAGER_H
#define MAZEMANAGER_H

//#include "GameManager.h"
#include "MazeCell.h"
#include "PreprocessorCommands.h"

#include <boost/filesystem.hpp>

#include <irrlicht/irrlicht.h>

class GameManager;

class MazeManager
{
	public:
		/** Default constructor */
		MazeManager();
		/** Default destructor */
		virtual ~MazeManager();

		MazeCell** maze;

		bool canGetTo( uint_fast8_t startX, uint_fast8_t startY, uint_fast8_t goalX, uint_fast8_t goalY );
		bool canGetToAllCollectables( uint_fast8_t startX, uint_fast8_t startY );

		void draw( irr::video::IVideoDriver* driver, uint_fast16_t cellWidth, uint_fast16_t cellHeight );

		void makeRandomLevel();
		
		void newMaze( uint_fast8_t newCols, uint_fast8_t newRows );

		void recurseRandom( uint_fast8_t x, uint_fast8_t y, uint_fast16_t depth, uint_fast16_t numSoFar );

		bool saveToFile();
		bool saveToFile( boost::filesystem::path dest );

		void setGameManager( GameManager* newGM );

		uint_fast8_t cols;

		GameManager* gameManager;

		uint_fast8_t rows;
	protected:
	private:
};

#endif // MAZEMANAGER_H
