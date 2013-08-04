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

		bool canGetTo( uint_least8_t startX, uint_least8_t startY, uint_least8_t goalX, uint_least8_t goalY );
		bool canGetToAllCollectables( uint_least8_t startX, uint_least8_t startY );

		void draw( irr::video::IVideoDriver* driver, uint_least16_t cellWidth, uint_least16_t cellHeight );

		bool existsAnythingAt( uint_least8_t x, uint_least8_t y );

		bool loadFromFile();
		bool loadFromFile( boost::filesystem::path src );

		void makeRandomLevel();

		void recurseRandom( uint_least8_t x, uint_least8_t y, uint_least16_t depth, uint_least16_t numSoFar );
		void resizeMaze( uint_least8_t newCols, uint_least8_t newRows );

		bool saveToFile();
		bool saveToFile( boost::filesystem::path dest );

		void setGameManager( GameManager* newGM );

		uint_least8_t cols;

		GameManager* gameManager;

		uint_least8_t rows;
	protected:
	private:
};

#endif // MAZEMANAGER_H
