#ifndef MAZEMANAGER_H
#define MAZEMANAGER_H

//#include "GameManager.h"
#include "MazeCell.h"

#define BOOST_FILESYSTEM_NO_DEPRECATED //Recommended by the Boost filesystem library documentation to prevent us from using functions which will be removed in later versions
#include <filesystem.hpp>

#include <irrlicht.h>

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

		bool existsAnythingAt( uint_fast8_t x, uint_fast8_t y );

		bool loadFromFile();
		bool loadFromFile( boost::filesystem::path src );

		void makeRandomLevel();

		void recurseRandom( uint_fast8_t x, uint_fast8_t y, uint_fast16_t depth, uint_fast16_t numSoFar );
		void resizeMaze( uint_fast8_t newCols, uint_fast8_t newRows );

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
