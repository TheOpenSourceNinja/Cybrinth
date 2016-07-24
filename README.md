#Cybrinth
##What is it?
Cybrinth is a simple game. You play a circle (Maybe you're from Flatland or something; make up your own story). You must find your way through the maze to the goal (a door with an arrow pointing into it). But to get there, you've got to unlock the gates, and to do that you need keys. The keys, scattered throughout the maze, don't unlock specific gates. The gates will be automatically unlocked if and only if all the keys have been collected. If you're playing with friends or AI bots, it doesn't matter who gets each key.

The mazes are pseudorandomly generated based on a seed number (called a "random seed"). If you don't know what that means, see the Wikipedia articles on [pseudorandom number generators](https://en.wikipedia.org/wiki/Pseudorandom_number_generator) and [random seeds](https://en.wikipedia.org/wiki/Random_seed). When saving a maze to a file or sending it over a network (for multiplayer games), all that needs to get saved/sent is the seed. This unfortunately means that different versions of the game would likely be incompatible. Everyone playing a network game or reading the saved file must therefore use the same game version.

##Multiplayer support
Currently Cybrinth supports any number of players using controllers, plus up to 4 human players on one keyboard. LAN network play is also possible but may be buggy. You can also play against anywhere from 0 to 255 bots as long as the total number of players (human and bot) does not exceed 255. Don't want to actually play at all? Fine, just set the number of bots equal to the number of players and watch the game play itself.

##Configuration
The game stores all its settings in a file called prefs.cfg, except for controls which are stored in controls.cfg. They are standard UTF-8 text files editable with any text editor.

##Music
You can place any music you want in the ./music folder. Cybrinth will scan ./music and its subfolders for any playable files and will play them in random order. It should be able to take any common music file type, like FLAC or MP3 or Ogg Vorbis; the specific types will depend on how your [SDL_Mixer](https://www.libsdl.org/projects/SDL_mixer/) library is configured.

##Copyrights
Cybrinth was created by James Dearing, copyright 2012-2016 and licensed under the GNU AGPL (see the file COPYING). Fonts, music, and images distributed with Cybrinth may be licensed under separate licenses.