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
		
		bool canGetTo( uint8_t startX, uint8_t startY, uint8_t goalX, uint8_t goalY );
		bool canGetToAllCollectables( uint8_t startX, uint8_t startY );
		
		void draw( irr::video::IVideoDriver* driver, uint32_t cellWidth, uint32_t cellHeight );
		
		bool existsAnythingAt( uint8_t x, uint8_t y );
		
		bool loadFromFile();
		bool loadFromFile( boost::filesystem::path src );
		
		void makeRandomLevel();
		
		void recurseRandom( uint8_t x, uint8_t y, uint16_t depth, uint16_t numSoFar );
		void resizeMaze( uint8_t newCols, uint8_t newRows );
		
		bool saveToFile();
		bool saveToFile( boost::filesystem::path dest );
		
		void setGameManager( GameManager* newGM );
		
		uint8_t cols;
		
		GameManager* gameManager;
		
		uint8_t rows;
	protected:
	private:
};

#endif // MAZEMANAGER_H
