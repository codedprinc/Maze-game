#include "terms.h"

bool init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}
	gWindow = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}
	gRenderer = SDL_CreateRenderer(gWindow, -1, 1);
	if (gRenderer == NULL) {
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
	if (gTexture == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Texture could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}
	

	return true;
}
	
	

void render_wall()
{
	for (int x = 0; x < SCREEN_WIDTH; x++)
	{
		//calculate ray position and direction
		double cameraX = 2 * x / (double)SCREEN_WIDTH - 1; //x-coordinate in camera space
		double rayDirX = dirX + planeX * cameraX;
		double rayDirY = dirY + planeY * cameraX;

		// which box of the map we're in
		int mapX = (int)posX;
		int mapY = (int)posY;

		//length of ray from current position to next x or y-side
		double sideDistX;
		double sideDistY;

		//length of ray from one x or y-side to next x or y-side
		double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
		double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
		double perpWallDist;

		//what direction to step in x or y-direction (either +1 or -1)
		int stepX;
		int stepY;

		int hit = 0;  //was there a wall hit?
		int side; //was a NS or a EW wall hit?


		//calculate step and initial sideDist
		if (rayDirX < 0) {
			stepX = -1;
			sideDistX = (posX - mapX) * deltaDistX;
		}
		else {
			stepX = 1;
			sideDistX = (mapX + 1.0 - posX) * deltaDistX;
		}
		if (rayDirY < 0) {
			stepY = -1;
			sideDistY = (posY - mapY) * deltaDistY;
		}
		else {
			stepY = 1;
			sideDistY = (mapY + 1.0 - posY) * deltaDistY;
		}

		// Perform DDA
		while (hit == 0)
		{
			//jump to next map square, either in x-direction, or in y-direction
			if (sideDistX < sideDistY) {
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			}
			else {
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			}
			//jump to next map square, either in x-direction, or in y-direction
			if (worldMap[mapX][mapY] > 0) hit = 1;
		}

		//Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
		if (side == 0) perpWallDist = (sideDistX - deltaDistX);
		else perpWallDist = (sideDistY - deltaDistY);

		//Calculate height of line to draw on screen
		int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

		//calculate lowest and highest pixel to fill in current stripe
		int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
		if (drawStart < 0) drawStart = 0;
		int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
		if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

		SDL_Color color;
		switch (worldMap[mapX][mapY]) {
		case 1: color = (SDL_Color){ 255, 0, 0, 255 }; break;
		case 2: color = (SDL_Color){ 0, 255, 0, 255 }; break;
		case 3: color = (SDL_Color){ 0, 0, 255, 255 }; break;
		case 4: color = (SDL_Color){ 255, 255, 255, 255 }; break;
		default: color = (SDL_Color){ 255, 255, 0, 255 }; break;
		}

		if (side == 1) {
			color.r /= 2;
			color.g /= 2;
			color.b /= 2;
		}

		SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, 255);
		SDL_RenderDrawLine(gRenderer, x, drawStart, x, drawEnd);


	}
}


// Function to generate a simple checkerboard texture
void generateTextures() {
	for (int i = 0; i < 10; i++) {
		for (int y = 0; y < texHeight; y++) {
			for (int x = 0; x < texWidth; x++) {
				int xorcolor = (x * 256 / texWidth) ^ (y * 256 / texHeight);
				int ycolor = y * 256 / texHeight;
				int xycolor = y * 128 / texHeight + x * 128 / texWidth;
				Uint32 color;
				switch (i) {
				case 0: color = 65536 * 254 * (x != y && x != texWidth - y); break;
				case 1: color = xycolor + 256 * xycolor + 65536 * xycolor; break;
				case 2: color = 256 * xycolor + 65536 * xycolor; break;
				case 3: color = xorcolor + 256 * xorcolor + 65536 * xorcolor; break;
				case 4: color = 256 * xorcolor; break;
				case 5: color = 65536 * 192 * (x % 16 && y % 16); break;
				case 6: color = 65536 * ycolor; break;
				case 7: color = 128 + 256 * 128 + 65536 * 128; break;
				case 8: color = 0xFFFFFFFF; break; // White
				case 9: color = 0xFFFF00FF; break; // Magenta
				default: color = 0x00000000; break; // Black (default case)
				}
				texture[i][texWidth * y + x] = color;
			}
		}
	}
}
void close() {
	SDL_DestroyTexture(gTexture);
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	
	SDL_Quit();
}


int main(int argc, char* args[]) {
	if (!init()) {
		printf("Failed to initialize!\n");
		return -1;
	}
	else {
		printf("Init works");
	}

	generateTextures();
	// Main loop
	
	while (!quit) {
		printf("Entered main loop\n\n");
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}

		
		}

		//FLOOR CASTING
		for (int y = 0; y < h; y++)
		{
			// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
			double rayDirX0 = dirX - planeX;
			double rayDirY0 = dirY - planeY;
			double rayDirX1 = dirX + planeX;
			double rayDirY1 = dirY + planeY;

			// Current y position compared to the center of the screen (the horizon)
			int p = y - screenHeight / 2;

			// Vertical position of the camera.
			double posZ = 0.5 * screenHeight;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			double rowDistance = posZ / p;

			// calculate the real world step vector we have to add for each x (parallel to camera plane)
			// adding step by step avoids multiplications with a weight in the inner loop
			double floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
			double floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

			// real world coordinates of the leftmost column. This will be updated as we step to the right.
			double floorX = posX + rowDistance * rayDirX0;
			double floorY = posY + rowDistance * rayDirY0;

			for (int x = 0; x < screenWidth; ++x)
			{
				// the cell coord is simply got from the integer parts of floorX and floorY
				int cellX = (int)(floorX);
				int cellY = (int)(floorY);

				// get the texture coordinate from the fractional part
				int tx = (int)(texWidth * (floorX - cellX)) & (texWidth - 1);
				int ty = (int)(texHeight * (floorY - cellY)) & (texHeight - 1);

				floorX += floorStepX;
				floorY += floorStepY;

				// choose texture and draw the pixel
				int floorTexture = 8;
				int ceilingTexture = 9;
				Uint32 color;

				// floor
				color = texture[floorTexture][texWidth * ty + tx];
				color = (color >> 1) & 8355711; // make a bit darker
				buffer[y][x] = color;

				//ceiling (symmetrical, at screenHeight - y - 1 instead of y)
				color = texture[ceilingTexture][texWidth * ty + tx];
				color = (color >> 1) & 8355711; // make a bit darker
				buffer[screenHeight - y - 1][x] = color;
			}
		}

		//Wall casting
		for (int x = 0; x < SCREEN_WIDTH; x++)
		{
			
			printf("No of rays drawn, %d\n\n", x);
			//calculate ray position and direction
			double cameraX = 2 * x / (double)SCREEN_WIDTH - 1; //x-coordinate in camera space
			double rayDirX = dirX + planeX * cameraX;
			double rayDirY = dirY + planeY * cameraX;

			// which box of the map we're in
			int mapX = (int)posX;
			int mapY = (int)posY;

			//length of ray from current position to next x or y-side
			double sideDistX;
			double sideDistY;

			//length of ray from one x or y-side to next x or y-side
			double deltaDistX = (rayDirX == 0) ? 1e30 : fabs(1 / rayDirX);
			double deltaDistY = (rayDirY == 0) ? 1e30 : fabs(1 / rayDirY);
			double perpWallDist;

			//what direction to step in x or y-direction (either +1 or -1)
			int stepX;
			int stepY;

			int hit = 0;  //was there a wall hit?
			int side; //was a NS or a EW wall hit?


			//calculate step and initial sideDist
			if (rayDirX < 0) {
				stepX = -1;
				sideDistX = (posX - mapX) * deltaDistX;
			}
			else {
				stepX = 1;
				sideDistX = (mapX + 1.0 - posX) * deltaDistX;
			}
			if (rayDirY < 0) {
				stepY = -1;
				sideDistY = (posY - mapY) * deltaDistY;
			}
			else {
				stepY = 1;
				sideDistY = (mapY + 1.0 - posY) * deltaDistY;
			}

			// Perform DDA
			while (hit == 0)
			{
				//jump to next map square, either in x-direction, or in y-direction
				if (sideDistX < sideDistY) {
					sideDistX += deltaDistX;
					mapX += stepX;
					side = 0;
				}
				else {
					sideDistY += deltaDistY;
					mapY += stepY;
					side = 1;
				}
				//jump to next map square, either in x-direction, or in y-direction
				if (worldMap[mapX][mapY] > 0) hit = 1;
			}

			//Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
			if (side == 0) perpWallDist = (sideDistX - deltaDistX);
			else perpWallDist = (sideDistY - deltaDistY);

			//Calculate height of line to draw on screen
			int lineHeight = (int)(SCREEN_HEIGHT / perpWallDist);

			//calculate lowest and highest pixel to fill in current stripe
			int drawStart = -lineHeight / 2 + SCREEN_HEIGHT / 2;
			if (drawStart < 0) drawStart = 0;
			int drawEnd = lineHeight / 2 + SCREEN_HEIGHT / 2;
			if (drawEnd >= SCREEN_HEIGHT) drawEnd = SCREEN_HEIGHT - 1;

			//texturing calculations
			int texNum = worldMap[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!

			//calculate value of wallX
			double wallX; //where exactly the wall was hit
			if (side == 0) wallX = posY + perpWallDist * rayDirY;
			else           wallX = posX + perpWallDist * rayDirX;
			wallX -= floor((wallX));

			//x coordinate on the texture
			int texX = (int)(wallX * (double)(texWidth));
			if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
			if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

			// How much to increase the texture coordinate per screen pixel
			double step = 1.0 * texHeight / lineHeight;
			// Starting texture coordinate
			double texPos = (drawStart - screenHeight / 2 + lineHeight / 2) * step;
			for (int y = drawStart; y < drawEnd; y++)
			{
				// Cast the texture coordinate to integer, and mask with (texHeight - 1) in case of overflow
				int texY = (int)texPos & (texHeight - 1);
				texPos += step;
				Uint32 color = texture[texNum][texHeight * texY + texX];
				//make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
				if (side == 1) color = (color >> 1) & 8355711;
				buffer[y][x] = color;
			}
		
			
		}
		//SDL_RenderClear(gRenderer);
		SDL_UpdateTexture(gTexture, NULL, buffer, screenWidth * sizeof(Uint32));
		for (int y = 0; y < screenHeight; y++) {
			for (int x = 0; x < screenWidth; x++) {
				buffer[y][x] = 0;
			}
		}
		
		
		// Timing for input and fps counter
		oldTime = time;
		time = SDL_GetTicks();
		double frameTime = (time - oldTime) / 1000.0; // Frametime is the time this frame has taken, in seconds
		double FPS_counter = 1.0 / frameTime; // FPS COUNTER

		SDL_RenderClear(gRenderer);
		SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
		SDL_RenderPresent(gRenderer);
		// Speed modifiers
		double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
		double rotSpeed = frameTime * 3.0; //the constant value is in radians/second
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

		//move forward if no wall in front of you
		if (currentKeyStates[SDL_SCANCODE_W] || currentKeyStates[SDL_SCANCODE_UP]) {
			if (worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == false) posX += dirX * moveSpeed;
			if (worldMap[(int)(posX)][(int)(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
		}
		//move backwards if no wall behind you
		if (currentKeyStates[SDL_SCANCODE_S] || currentKeyStates[SDL_SCANCODE_DOWN]) {
			if (worldMap[(int)(posX - dirX * moveSpeed)][(int)posY] == false) posX -= dirX * moveSpeed;
			if (worldMap[(int)posX][(int)(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;

		}
		//rotate to the left
		if (currentKeyStates[SDL_SCANCODE_A] || currentKeyStates[SDL_SCANCODE_LEFT]) {
			double oldDirX = dirX;
			dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
			dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
			planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
		}
		//rotate to the right
		if (currentKeyStates[SDL_SCANCODE_D] || currentKeyStates[SDL_SCANCODE_RIGHT]) {
			double oldDirX = dirX;
			dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
			dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
			planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
		}
		if (currentKeyStates[SDL_SCANCODE_ESCAPE]) {
			break;
		}
		
	}
	
	//SDL_DestroyTexture(screenTexture);
	
	close();
	return 0;
}



