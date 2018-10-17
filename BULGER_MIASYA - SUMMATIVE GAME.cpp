//MIASYA BULGER			ICS3U			Ms. Cullum			June 2016
//Description:
//This is a desktop version of the party game Dance Dance Revolution. Instructions are provided in game.

//Include statements so I can use all my functions later on
#include <allegro.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Define constants: playing time in seconds and BPS timer to modify game length and speed easily
#define PLAYTIME 35
#define BPS 150

//----------DECLARE GLOBALS----------

//Declare my buffer as a global for smooth animation and use with multiple functions
BITMAP *buffer;

//Declare leaderboard struct and pointer for holding player info (helpful for file I/O later)
struct Leaderboard {
	char name[20];
	int score;
};
Leaderboard player[100];

//Declare filepointer for textfile which will hold leaderboard information
FILE *fptr;

//Declare scorekeeper to keep score
int iScore = 0;

//Declare hit and miss counters (for scoring purposes)
int iHit = 0;
int iMiss = 0;

//Have a long to store the values from the speed counter
volatile long lSpeedCounter = 0;

//----------PROTOTYPE FUNCTIONS----------

//Function that increments speed counter at a regular pace. Used for timing
void incrementSpeed();

//Function fills the array with different tile types, randomly, so each gameplay has a different order of tiles
void fillArray(int iArray[]);

//Function returns the tile in the array with the highest y value position (lowest on screen)
int findLowestTile(int i[]);

//Function checks if there was a tile collision with the "hit" bar based on the lowest tile's y location
bool checkCollision(int iTileY);

//Function keeps track of hit and miss counters, and returns a score based on the counters
void keepScore(bool bHit);

//Function counts how many lines are present in a file and returns it so proper file I/O can be executed
int countLines();

//Function scans in the 'i'th line of the file and puts the data into a struct
void scanIn(int i);

//Function appends player name and score (global) to file
void addPlayer(char strName[20]);

//Function bubble sorts the file (for the number of lines) by descending score
void sortByScore(int iLimit);

//Function takes in the contents of index 'i' of the struct and prints it to the buffer
void printStruct(int i);


//----------MAIN----------
int main(int argc, char *argv[]) {


	//----------INITIALIZATION----------
	//Initialize Allegro
	allegro_init();

	//Initialize timer routines
	install_timer();

	//Setting my timer
	LOCK_VARIABLE(lSpeedCounter);
	LOCK_FUNCTION(incrementSpeed);

	//Setting my BPS
	install_int_ex(incrementSpeed, BPS_TO_TIMER(BPS));

	//Initialize sound
	install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);

	//Set colour depth
	set_color_depth(desktop_color_depth());

	//Change our graphics mode to 960 x 640
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 960, 640, 0, 0);

	//Seed from time
	srand(time(NULL));

	//Initialize keyboard routines
	install_keyboard();

	//Initialize mouse
	install_mouse();

	//Show mouse on screen
	show_mouse(screen);

	//----------DECLARING VARIABLES FOR USE IN MAIN----------
	//Set my buffer as a bitmap for double buffering for smooth animation
	buffer = create_bitmap(960, 640);

	//Declare variables
	int i = 0;
	int j = 0;
	int iPlaceholder = 0;

	//Declare a lock so that when a key is pressed, it only works once
	bool bLock = false;

	//Declare a bool to allow for pregame, ingame, and endgame procedures
	bool bInGame = false;

	//Declares a bool that is false until a song is selected
	bool bSongSelected = false;

	//Declare an int which represents the song number
	int iSong;

	//Declare a bool for keyboard input
	bool bEnterName = true;

	//Declare a bool to check if the user wants to replay
	bool bReplay = true;

	//Declare placeholder string
	char strPlaceholder[20] = "";

	//Declare a placeholder char
	char cPlaceholder;

	//Declare number of lines in file and set it to the number of lines in file
	int iLines;

	//Declare frame counter
	unsigned int long iFrame = 1;

	//Declare tile array
	int iTileType[100];
	int iTileY[100];

	//Verify that the file exists
	fptr = fopen("BULGER_Miasya Leaderboard.txt", "r");
	if (fptr == NULL) {

		//Print error message if the textfile can't be found
		allegro_message("BULGER_Miasya Leaderboard.txt failed to load");
	}
	fclose(fptr);

	//Declare array of samples (music)
	SAMPLE *sSong[2] = {NULL, NULL};

	//Load three songs
	sSong[0]= load_sample("SONG_Demons_Imagine_Dragons.wav");
	sSong[1]= load_sample("SONG_Good_For_You_Selena_Gomez.wav");

	for (i = 0; i < 2; i++) {
		//Check if songs successfully loaded
		if (sSong[i] == NULL) {
			allegro_message("SONG %d failed to load", i);
		}
	}

	//Declare a bitmap for the screens
	BITMAP *background[7] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	background[0] = load_bitmap("BACKGROUND_start.bmp", NULL);
	background[1]= load_bitmap("BACKGROUND_credits.bmp", NULL);
	background[2] = load_bitmap("BACKGROUND_leaderboard.bmp", NULL);
	background[3] = load_bitmap("BACKGROUND_leaderboard2.bmp", NULL);
	background[4] = load_bitmap("BACKGROUND_select_song.bmp", NULL);
	background[5] = load_bitmap("BACKGROUND_play.bmp", NULL);
	background[6] = load_bitmap("BACKGROUND_choose_name.bmp", NULL);

	//Declare an array of bitmaps for the tiles
	BITMAP *tile[4] = {NULL, NULL, NULL, NULL};
	tile[0] = load_bitmap("TILE_left.bmp", NULL);
	tile[1] = load_bitmap("TILE_down.bmp", NULL);
	tile[2] = load_bitmap("TILE_up.bmp", NULL);
	tile[3] = load_bitmap("TILE_right.bmp", NULL);

	//Check if BACKGROUND and TILE bitmaps successfully loaded
	for (i = 0; i < 6; i++) {
		if (background[i] == NULL) {
			allegro_message("BACKGROUND %d failed to load", i);
		}
	}
	for (i = 0; i < 4; i++) {
		if (tile[i] == NULL) {
			allegro_message("TILE %d failed to load", i);
		}
	}

	//----------INITIALIZE THE STRUCTURE----------
	//Set number of lines to read in
	iLines = countLines();

	//Open file to read
	fptr = fopen("BULGER_Miasya Leaderboard.txt", "r");

	//Read in the textfile contents for the number of lines
	for (i = 0; i < iLines; i++) {
		scanIn(i);
	}

	//Close file
	fclose(fptr);

	//----------BEGIN THE GAME----------
	while (bReplay && !key[KEY_ESC]) {

		//----------RESET GAME COMPONENTS BEFORE PLAYING----------
		//Reset score and hit and miss counters at the start of every game
		iScore = 0;
		iHit = 0;
		iMiss = 0;

		//Set the tiles Y location as just above the top of the screen
		for (i = 0; i < 100; i++) {
			iTileY[i] = - (i * 100) - 300;
		}

		//Fill iTileType with random numbers
		fillArray(iTileType);

		//----------MAIN MENU NAVIGATION----------
		while (!bInGame && !key[KEY_ESC]) {

			//Blit the main menu screen so the user can see the main menu
			blit(background[0], buffer, 0, 0, 0, 0, 960, 640);
			blit(buffer, screen, 0, 0, 0, 0, 960, 640);

			//When the user clicks somewhere on the main menu screen, check what they clicked
			if (mouse_b & 1) {

				//If the user clicks on the CREDITS button, go to credits
				if (mouse_x > 264 && mouse_x < 374 && mouse_y > 468 && mouse_y < 568) {

					//Once the user hits enter, go back to main menu
					while(!key[KEY_ENTER] && !key[KEY_ESC]) {

						//Blit credits page to screen
						blit(background[1], buffer, 0, 0, 0, 0, 960, 640);
						blit(buffer, screen, 0, 0, 0, 0, 960, 640);
					}

					//Clear keybuffer and screen
					clear_keybuf();
					clear(buffer);

					//Blit main menu o screen
					blit(background[0], buffer, 0, 0, 0, 0, 960, 640);
				}


				//If the user clicks on LEADERBOARD button, go to leaderboard
				else if (mouse_x > 603 && mouse_x < 715 && mouse_y > 467 && mouse_y < 564) {

					//Blit leaderboard page to buffer
					blit(background[2], buffer, 0, 0, 0, 0, 960, 640);

					//Print the leaderboard to the buffer (10 lines)
					for (i = 0; i < 10; i++) {
						printStruct(i);
					}

					//Blit buffer to screen
					blit(buffer, screen, 0, 0, 0, 0, 960, 640);

					//If the user hits enter, go back to main menu
					while(!key[KEY_ENTER] && !key[KEY_ESC]) {
					}

					//Clear keybuffer, buffer, and screen so everythings ready to move on
					clear_keybuf();
					clear(buffer);
					clear(screen);
					blit(background[0], screen, 0, 0, 0, 0, 960, 640);
				}

				//----------HITTING PLAY----------
				//Once the user clicks on the PLAY button, begin game
				else if (mouse_x > 415 && mouse_x < 560 && mouse_y > 460 && mouse_y < 580) {

					//Reset song selected to false
					bSongSelected = false;

					//Clear main menu off screen
					clear(screen);


					//----------SELECTING A SONG----------
					//Allow user to select song by clicking
					while (!bSongSelected) {

						//Blit select song page to buffer then screen
						blit(background[4], buffer, 0, 0, 0, 0, 960, 640);
						blit(buffer, screen, 0, 0, 0, 0, 960, 640);

						//When the user clicks, see which song they choose
						if (mouse_b & 1) {

							//Good for you
							if (mouse_x > 191 && mouse_x < 441 && mouse_y > 216 && mouse_y < 446) {

								//Set iSong to the selected song
								iSong = 0;

								//Set song selected bool to true to exit loop
								bSongSelected = true;
							}
							//Demons
							else if (mouse_x > 483 && mouse_x < 735 && mouse_y > 216 && mouse_y < 446) {

								//Set iSong to the selected song
								iSong = 1;

								//Set song selected bool to true to exit loop
								bSongSelected = true;
							}
						}
					}

					//Clear screen for clarity
					clear(screen);

					//Set game play bool to true to begin game
					bInGame = true;
				}
			}
		}

		//Reset speed counter to get ready to play
		lSpeedCounter = 0;

		//----------PLAYING THE GAME----------
		//Start game after PLAY button is clicked. Stop game if ESC is hit
		while (bInGame && !key[KEY_ESC]) {

			//Reset iFrame
			iFrame = 0;

			//Begin playing selected sample / CUE THE TUNES!
			play_sample(sSong[iSong], 100, 128, 1000, 0);

			//----------DRAWING THE TILES, TIME LEFT, AND SCORE TO THE SCREEN---------
			//Run game while the game isn't done
			while ((iFrame / BPS) < PLAYTIME && !key[KEY_ESC]) {

				//Draw the play background screen to the buffer
				blit(background[5], buffer, 0, 0, 0, 0, 960, 640);

				// Draw tiles to the buffer
				for (i = 0; i < 100; i++) {

					//If there is isnt much time left, don't draw the tile unless it's already begun to fall
					if (PLAYTIME * 1.0 - iFrame / (BPS * 1.0) <= 640 / (BPS * 1.0)) {

						//Every beat, increase the required y position from the top by one pixel (so no new tiles spawn)
						if (iTileY[i] + 75 > iFrame - (PLAYTIME * BPS - 640) && iTileY[i]) {

							//Draw the appropriate tile to the buffer
							draw_sprite(buffer, tile[iTileType[i]], (iTileType[i] * 100) + 275, iTileY[i]);
						}
					}

					//If there's enough time left, draw the tile
					else {

						//Draw the appropriate tile to the buffer
						draw_sprite(buffer, tile[iTileType[i]], (iTileType[i] * 100) + 275, iTileY[i]);
					}
				}

				//Print countdown timer and score in the corners
				textprintf_ex(buffer, font, 114, 100, makecol(0, 0, 0), -1, "%d", PLAYTIME - (iFrame / BPS));
				textprintf_ex(buffer, font, 838, 100, makecol(0, 0, 0), -1, "%d", iScore);

				//Blit the buffer to the screen
				blit(buffer, screen, 0, 0, 0, 0, 960, 640);

				//Clear the buffer
				clear(buffer);

				//----------DROPPING THE TILES AT A CONSTANT RATE----------
				//Execute the logic loop once per beat
				while (lSpeedCounter > 0) {

					//Lower the all tiles in the array by one pixel every beat
					for (i = 0; i < 100; i++) {
						//Lower the tiles by one pixel
						iTileY[i]++;
					}

					//Every 8 beats, make the lock false
					if (iFrame > 8 && iFrame % 8 == 0) {
						//When the lock is false, any keypress will be read
						bLock = false;
					}

					//Decrement speed counter
					lSpeedCounter--;

					//Increment frame counter
					iFrame++;
				}

				//----------READING USER KEYPRESSES AND SCORING----------
				//If a key is pressed, read the key once every 8 beats
				if (keypressed() && !bLock) {

					//When the user hits a left
					if (key[KEY_LEFT]) {

						//Check if the lowest tile is a left
						if (iTileType[findLowestTile(iTileY)] == 0) {

							//If it's a collision
							if (checkCollision(iTileY[findLowestTile(iTileY)])) {

								//Modify the score if there is a hit
								keepScore(true);

								//Ensure that this only happens once per tile by moving hit tiles to below the screen
								iTileY[findLowestTile(iTileY)] = 640;
							}
						}

						//If the wrong tile was hit, record a miss
						else {
							keepScore(false);
						}
					}

					//When the user hits a down
					else if (key[KEY_DOWN]) {

						//Check if the lowest tile is a down
						if (iTileType[findLowestTile(iTileY)] == 1) {

							//If it's a collision
							if (checkCollision(iTileY[findLowestTile(iTileY)])) {

								//Modify the score if there is a hit
								keepScore(true);

								//Ensure that this only happens once per tile by moving hit tiles to below the screen
								iTileY[findLowestTile(iTileY)] = 640;
							}
						}

						//If the wrong tile was hit, record a miss
						else {
							keepScore(false);
						}
					}

					//When the user hits an up
					else if (key[KEY_UP]) {

						//Check if the lowest tile is an up
						if (iTileType[findLowestTile(iTileY)] == 2) {

							//If it's a collision
							if (checkCollision(iTileY[findLowestTile(iTileY)])) {

								//Modify the score if there is a hit
								keepScore(true);

								//Ensure that this only happens once per tile by moving hit tiles to below the screen
								iTileY[findLowestTile(iTileY)] = 640;
							}
						}

						//If the wrong tile was hit, record a miss
						else {
							keepScore(false);
						}
					}

					//When the user hits a right
					else if (key[KEY_RIGHT]) {

						//Check if the lowest tile is a right
						if (iTileType[findLowestTile(iTileY)] == 3) {

							//If it's a collision
							if (checkCollision(iTileY[findLowestTile(iTileY)])) {

								//Modify the score if there is a hit
								keepScore(true);

								//Ensure that this only happens once per tile by moving hit tiles to below the screen
								iTileY[findLowestTile(iTileY)] = 640;
							}
						}

						//If the wrong tile was hit, record a miss
						else {
							keepScore(false);
						}
					}

					//Make the lock true again
					bLock = true;

					//Clear keyboard buffer
					clear_keybuf();
				}

			}

			//Clear screen
			clear(buffer);

			//Stop the song from playing once the game is done
			stop_sample(sSong[iSong]);
			rest(250);


			//----------END OF GAME PROCEDURES----------
			while (bInGame && !key[KEY_ESC]) {

				//Draw the get name background screen to the buffer
				blit(background[6], buffer, 0, 0, 0, 0, 960, 640);

				//Print score once the game is done, ask for name
				textprintf_ex(buffer, font, 450, 260, makecol(255, 255, 255), -1, "%d!", iScore);

				//Blit the buffer to the screen
				blit(buffer, screen, 0, 0, 0, 0, 960, 640);
				//Set variable to 0
				j = 0;

				// reset bEnterName
				bEnterName = true;


				//----------GETTING THE USERS NAME----------
				//Get a string from the user
				while (bEnterName) {

					while (lSpeedCounter > 0) {

						//Every 10 beats, make the lock false
						if (iFrame > 10 && iFrame % 10 == 0) {
							//When the lock is false, any keypress will be read
							bLock = false;
						}

						lSpeedCounter--;
						iFrame++;
					}

					//Read in a keypress
					if (keypressed() && !bLock) {
						iPlaceholder = readkey();

						//Convert the keyboard press into an ascii character
						cPlaceholder = iPlaceholder & 0xff;

						//Clear keyboard buffer
						clear_keybuf();

						//If the user hits a letter or number, add it to the string
						if(cPlaceholder >= 32 && cPlaceholder <= 126 && j < 19) {

							//Put the char into the string
							strPlaceholder[j] = cPlaceholder;

							//Increase the variable
							j++;

							//Put an end of string char at the end of the string
							strPlaceholder[j] = '\0';

							//Draw the letters to the screen
							textprintf_ex(screen, font, 8 * j + 430, 480, makecol(255, 255, 255), -1, "%c", strPlaceholder[j-1]);
						}

						//If the user enters something other than a letter, quit
						else {
							bEnterName = false;
						}
					}
				}

				//----------MODIFYING THE LEADERBOARD FILE AND REREADING IT IN----------
				//Add player to database
				addPlayer(strPlaceholder);

				//Set number of lines to read in
				iLines = countLines();

				//Open file to read
				fptr = fopen("BULGER_Miasya Leaderboard.txt", "r");

				//Read in the textfile contents for the number of lines
				for (i = 0; i < iLines; i++) {
					scanIn(i);
				}

				//Close file
				fclose(fptr);

				//Sort the structure by descending score
				sortByScore(iLines);

				//Open file to read
				fptr = fopen("BULGER_Miasya Leaderboard.txt", "r");

				//Read in the textfile contents for the number of lines
				for (i = 0; i < iLines; i++) {
					scanIn(i);
				}

				//Close file
				fclose(fptr);

				//----------PRINTING THE LEADERBOARD FILE CONTENTS TO THE SCREEN----------
				//Blit leaderboard page to buffer
				blit(background[3], buffer, 0, 0, 0, 0, 960, 640);

				//Print the leaderboard to the screen (10 lines)
				for (i = 0; i < 10; i++) {
					printStruct(i);
				}

				blit(buffer, screen, 0, 0, 0, 0, 960, 640);

				//Clear keybuf and wait one second before accepting input
				clear_keybuf();
				rest(1000);


				//----------GIVE USER OPTION TO REPLAY OR QUIT----------
				//If the user hits enter, go back to main menu. If the user hits escape, quit the game
				while(bInGame) {

					//ESC means quit
					if (key[KEY_ESC]) {
						//Set bools to false to quit loops
						bReplay = false;
						bInGame = false;
					}

					//Enter means replay
					if (key[KEY_ENTER]) {
						//Set replay to true to play again
						bReplay = true;
						bInGame = false;
					}
				}
			}
		}
	}


	//----------QUITTING PROCEDURES----------

	//If user hits ESC, say byebye and thank you
	if(key[KEY_ESC]) {
		allegro_message("Thank you for playing.");
	}

	//Release sample data
	for (i = 0; i < 2; i++) {
		destroy_sample(sSong[i]);
	}

	//Release bitmap data (BACKGROUNDS, TILES, BUFFER)
	for (i = 0; i < 7; i++) {
		destroy_bitmap(background[i]);
	}
	for (i = 0; i < 4; i++) {
		destroy_bitmap(tile[i]);
	}
	destroy_bitmap(buffer);

	return 0;
}
END_OF_MAIN()

//Function that increments speed counter at a regular pace. Used for timing
void incrementSpeed() {

	//Increment speed counter by one
	lSpeedCounter++;
}
END_OF_FUNCTION(incrementSpeed)

//Function fills the array with different tile types, randomly, so each gameplay has a different order of tiles
void fillArray(int iArray[]) {

	//Declare variables
	int i = 0;

	//Do this loop for 100 tiles
	for (i = 0; i < 100; i++) {

		//Set tile to either 1, 2, 3, or 4, which corresponds to either left, down, up, or right respectively
		iArray[i] = rand() % 4;
	}
}
END_OF_FUNCTION(fillArray)

//Function returns the tile in the array with the highest y value position (lowest on screen)
int findLowestTile(int i[]) {

	//Declare variable for loop
	int j;

	//Declare the index of the array with the highest y value
	int iHighest = 0;
	int iTile = 0;

	//For all one hundred tiles
	for (j = 0; j < 100; j++) {

		//Check the y coordinate of each tile to see which is lowest on the screen but not too far past the hit line (at 490)
		if (i[j] > iHighest && i[j] < 500) {
			iHighest = i[j];
			iTile = j;
		}
	}
	//Return the index of the tile
	return (iTile);
}
END_OF_FUNCTION(findLowestTile)

//Function checks if there was a tile collision with the "hit" bar based on the lowest tile's y location
bool checkCollision(int iTileY) {

	//If the hit line at (x, 490) is between the top and bottom, collision is true
	if (iTileY < 490 && (iTileY + 75) > 490) {
		return true;
	}
	//If not, collision is false
	else {
		return false;
	}
}
END_OF_FUNCTION(checkCollision)

//Function keeps track of hit and miss counters, and returns a score based on the counters
void keepScore(bool bHit) {

	//Increase hit or miss counters respectively
	if (bHit) {
		iHit++;
	} else {
		iMiss++;
	}

	//Output score
	iScore = (iHit * 117 - iMiss * 33);

}
END_OF_FUNCTION(keepScore)

//Function counts how many lines are present in a file and returns it so proper file I/O can be executed
int countLines() {

	//Open file to read
	fptr = fopen("BULGER_Miasya Leaderboard.txt", "r");

	//Declare a character holder and the number of lines
	int iLines = 1;
	char cChar;

	//Reads in a char and stores it
	for (cChar = getc(fptr); cChar != EOF; cChar = getc(fptr)) {

		//If the char is newline, increase iLimit
		if (cChar == '\n')
			iLines++;
	}

	//Close file
	fclose(fptr);

	//Return limit of lines
	return (iLines);
}
END_OF_FUNCTION(countLines);

//Function scans in the 'i'th line of the file and puts the data into a struct
void scanIn(int i) {

	//Scan in one line of (one score and one name) and assign it to struct
	fscanf(fptr, "%s", player[i].name);
	fscanf(fptr, "%d", &player[i].score);

}
END_OF_FUNCTION(scanIn)

//Function appends player name and score (global) to file
void addPlayer(char strName[20]) {

	//Open file to append
	fptr = fopen("BULGER_Miasya Leaderboard.txt", "a");
	fprintf(fptr, "\n%s %d", strName, iScore);

	//Close file pointer
	fclose(fptr);

}
END_OF_FUNCTION(addPlayer)

//Function bubble sorts the file (for the number of lines) by descending score
void sortByScore(int iLimit) {

	//Declare variables and set limit to the number of lines
	int i;
	int j;

	//Bubble sort by descending score
	for (j = 0; j < iLimit; j++) {
		for (i = 0; i < iLimit - 1; i++) {

			//Swap if n element is greater than the n + 1 element to sort
			if (player[i].score < player[i + 1].score) {

				//Swap position of player
				player[iLimit + 1] = player[i + 1];
				player[i + 1] = player[i];
				player[i] = player[iLimit + 1];

				//Clear the placeholder
				player[iLimit + 1].score = 0;
				strcpy(player[iLimit + 1].name, "");
			}
		}
	}

	//Open file to write
	fptr = fopen("BULGER_Miasya Leaderboard.txt", "w");

	for (i = 0; i < 10; i++) {

		//If line isn't first line, prints newline before beginning
		if (i != 0) {
			fprintf(fptr, "\n");
		}

		//Write struct info back to file
		fprintf(fptr, "%s ", player[i].name);
		fprintf(fptr, "%d ", player[i].score);
	}

	//Close file
	fclose(fptr);
}
END_OF_FUNCTION(sortByScore)

//Function takes in the contents of index 'i' of the struct and prints it to the buffer
void printStruct(int i) {

	//Prints the place of the player
	textprintf_ex(buffer, font, 360, (i + 1) * 20 + 250, makecol(255, 255, 255), -1, "#%d", i + 1);

	//Prints the name of the player
	textprintf_ex(buffer, font, 430, (i + 1) * 20 + 250, makecol(255, 255, 255), -1, "%s", player[i].name);

	//Prints score of the player
	textprintf_ex(buffer, font, 550, (i + 1) * 20 + 250, makecol(255, 255, 255), -1, "%d", player[i].score);

}
END_OF_FUNCTION(printStruct)
