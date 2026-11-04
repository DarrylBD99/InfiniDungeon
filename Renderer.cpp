#include "Renderer.hpp"
#include <iostream>
#include "Decoder.hpp"

namespace WM908_Assessment {
	Renderer::Renderer(GamesEngineeringBase::Window& canvas_) : canvas(canvas_) {
		// Get Spritesheet Data
		spritesheetData = Decoder::ReadSpritesheetData(SPRITESHEET_FILE, spritesheetWidth, spritesheetHeight, colorPalette);

		gridHeight = spritesheetHeight / GRID_SIZE;
		gridWidth = spritesheetWidth / GRID_SIZE;
	}

	Renderer::~Renderer() {
		delete[] spritesheetData;
	}

	void Renderer::DrawCameraBackground(unsigned int colorPaletteIndex) {
		unsigned char* screenPixel = canvas.getBackBuffer();

		// Get Pixel Color
		unsigned char* pixelColor = colorPalette[colorPaletteIndex];

		// Always use the actual canvas width for buffer indexing
		const unsigned int screenWidth = canvas.getWidth();
		const unsigned int screenHeight = canvas.getHeight();

		// Camera boundaries
		const unsigned int screenTop = cameraBorderWidth, screenLeft = cameraBorderWidth,
			screenRight = cameraWidth + cameraBorderWidth, screenBottom = cameraHeight + cameraBorderWidth;

		// Draw background directly into the back buffer
		for (unsigned int y = screenTop; y < screenBottom; y++) {

			// Calculate the starting pixel of this row
			unsigned int index = (y * screenWidth + screenLeft) * 3;

			screenPixel = canvas.getBackBuffer() + index;

			for (unsigned int x = screenLeft; x < screenRight; x++) {
				if ((screenPixel[0] | screenPixel[1] | screenPixel[2]) == 0) {
					screenPixel[0] = pixelColor[0];
					screenPixel[1] = pixelColor[1];
					screenPixel[2] = pixelColor[2];
				}

				screenPixel += 3;
			}
		}
	}

	void Renderer::DrawFromSpritesheetIDScreenPosition(const Vec2& screenPosition, const unsigned int gridID, const Vec2& offset, Direction facing, bool flipH, bool flipV, bool insideCamera) {
		unsigned int screenHeight = canvas.getHeight(), screenWidth = canvas.getWidth();
		const unsigned int gridPosX = gridID % gridWidth, gridPosY = gridID / gridWidth;
		int screenPosX = (int)roundf(screenPosition.GetX() + offset.GetX());
		int screenPosY = (int)roundf(screenPosition.GetY() + offset.GetY());

		// New render code that is 2x faster (thank you Immie)
		// Some modification needed to be made to work with camera system, rotations and spritesheet code
		// Please check out his original work: https://github.com/im-e/Cryptmarch/blob/main/src/Rendering/Graphics.cpp

		int screenTop = 0, screenLeft = 0, screenBottom = screenHeight, screenRight = screenWidth;

		// Use camera properties if inside camera is true
		if (insideCamera) {
			screenTop = screenLeft = cameraBorderWidth;
			screenRight = cameraWidth + cameraBorderWidth;
			screenBottom = cameraHeight + cameraBorderWidth;
		}
		
		// Check if image is completely out of bounds
		if (screenPosX >= screenRight || screenPosY >= screenBottom || screenPosX + (int)GRID_SIZE <= screenLeft || screenPosY + (int)GRID_SIZE <= screenTop) return;
		
		if (spritesheetData == nullptr) return;

		// Image Clipping Check
		// Ensure unnecesary draw attempts aren't called at all if pixel is out of bounds
		int renderWidth = GRID_SIZE, renderHeight = GRID_SIZE;
		int clippedX = 0, clippedY = 0;

		if (screenPosX < screenLeft) {
			clippedX = screenLeft - screenPosX;
			renderWidth -= clippedX;
			screenPosX = screenLeft;
		}

		if (screenPosX + renderWidth > screenRight) {
			renderWidth = screenRight - screenPosX;
		}

		if (screenPosY < screenTop) {
			clippedY = screenTop - screenPosY;
			renderHeight -= clippedY;
			screenPosY = screenTop;
		}

		if (screenPosY + renderHeight > screenBottom) {
			renderHeight = screenBottom - screenPosY;
		}

		for (int y = 0; y < renderHeight; y++) {
			// Flip horizontal and vertical orientation
			unsigned int sourceX = 0, sourceY = 0;

			// How many pixels in spritesheet should they jump
			// Mainly for Left and Right facing sprites as they go on the x axis instead of y;
			int pixelStep = 0;

			// Rotation Logic
			switch (facing)
			{
			case Direction::Down: {
				sourceX = flipH ? GRID_SIZE - 1 - clippedX : clippedX;
				sourceY = flipV ? GRID_SIZE - 1 - (clippedY + y) : clippedY + y;

				pixelStep = flipH ? -1 : 1;
				break;
			}
			case Direction::Up: {
				sourceX = flipH ? clippedX : GRID_SIZE - 1 - clippedX;
				sourceY = flipV ? clippedY + y : GRID_SIZE - 1 - (clippedY + y);

				pixelStep = flipH ? 1 : -1;
				break;
			}
			case Direction::Right: {
				sourceX = flipH ? clippedY + y : GRID_SIZE - 1 - (clippedY + y);
				sourceY = flipV ? GRID_SIZE - 1 - clippedX : clippedX;

				pixelStep = flipV ? -(int)spritesheetWidth : spritesheetWidth;
				break;
			}
			case Direction::Left: {
				sourceX = flipV ? GRID_SIZE - 1 - (clippedY + y) : clippedY + y;
				sourceY = flipH ? clippedX : GRID_SIZE - 1 - clippedX;

				pixelStep = flipV ? spritesheetWidth : -(int)spritesheetWidth;
				break;
			}
			}

			// Work out pixel data and positions only once before getting x value
			uint8_t* pixelIndex = &spritesheetData[((gridPosY * GRID_SIZE) + sourceY) * spritesheetWidth + (gridPosX * GRID_SIZE) + sourceX];

			// Get Screen Position to set color directly
			int index = ((screenPosY + y) * screenWidth + screenPosX) * 3;
			unsigned char* screenPixel = canvas.getBackBuffer() + index;

			for (int x = 0; x < renderWidth; x++) {
				if (*pixelIndex > 0) {
					// Get Pixel Color from Color Palette
					uint8_t* pixelColor = colorPalette[*pixelIndex];

					screenPixel[0] = pixelColor[0];
					screenPixel[1] = pixelColor[1];
					screenPixel[2] = pixelColor[2];
				}

				// Move through spritesheet
				pixelIndex += pixelStep;

				// Move through screen (only need 3 since screen uses RGB instead of RGBA)
				screenPixel += 3;
			}
		}

		// My newer draw code that isn't as obsolete, but is slower
		//
		//for (int gridRow = 0; gridRow < GRID_SIZE; gridRow++) {
		//	int screenPixelY = screenPosY + gridRow;

		//	if (screenPixelY >= (int)screenHeight || screenPixelY < 0) continue;
		//	else if (insideCamera && !IsScreenYWithinCamera(screenPixelY)) continue;
		//	 

		//	for (int gridCol = 0; gridCol < GRID_SIZE; gridCol++) {
		//		int screenPixelX = screenPosX + gridCol;
		//		if (screenPixelX >= (int)screenWidth || screenPixelX < 0) continue;
		//		else if (insideCamera && !IsScreenXWithinCamera(screenPixelX)) continue;

		//		unsigned int pixelX = 0, pixelY = 0;

		//		// Rotates sprite based on facing direction
		//		switch (facing)
		//		{
		//		case Direction::Down: {
		//			pixelX = gridCol;
		//			pixelY = gridRow;
		//			break;
		//		}

		//		case Direction::Right: {
		//			pixelX = ((GRID_SIZE - 1) - gridRow);
		//			pixelY = gridCol;
		//			break;
		//		}

		//		case Direction::Left: {
		//			pixelX = gridRow;
		//			pixelY = ((GRID_SIZE - 1) - gridCol);
		//			break;
		//		}

		//		case Direction::Up: {
		//			pixelX = ((GRID_SIZE - 1) - gridCol);
		//			pixelY = ((GRID_SIZE - 1) - gridRow);
		//			break;
		//		}
		//		}

		//		// flips sprite based on flipH and flipV
		//		if (flipH) pixelX = (GRID_SIZE - 1) - pixelX;
		//		if (flipV) pixelY = (GRID_SIZE - 1) - pixelY;

		//		pixelX += (gridPosX * GRID_SIZE);
		//		pixelY += (gridPosY * GRID_SIZE);

		//		unsigned char* spritesheetPixel = spritesheet->atUnchecked(pixelX, pixelY);

		//		const unsigned int pixelIndex = (pixelY * spritesheet->width + pixelX) * spritesheet->channels;

		//		if (spritesheet->channels != 3 && spritesheet->data[pixelIndex + 3] == 0) continue;
		//		canvas.draw(screenPixelX, screenPixelY, &spritesheet->data[pixelIndex]);
		//	}
		//}

		// Old code: checks pixel only when x and y of screen pos are found
		// Updated code now checks if pixel is already out of bound at the start of each for loop
		//
		//for (unsigned int gridRow = 0; gridRow < GRID_SIZE; gridRow++) {
		//	for (unsigned int gridCol = 0; gridCol < GRID_SIZE; gridCol++) {
		//		int screenPosX = int(screenPosition.GetX() + offset.GetX());
		//		int screenPosY = int(screenPosition.GetY() + offset.GetY());

		//		int gridPixelX = 0, gridPixelY = 0;

		//		switch (facing)
		//		{
		//		case Direction::Down: {
		//			gridPixelX = gridCol;
		//			gridPixelY = gridRow;
		//			break;
		//		}

		//		case Direction::Right: {
		//			gridPixelX = gridRow;
		//			gridPixelY = ((GRID_SIZE - 1) - gridCol);
		//			break;
		//		}

		//		case Direction::Left: {
		//			gridPixelX = ((GRID_SIZE - 1) - gridRow);
		//			gridPixelY = gridCol;
		//			break;
		//		}

		//		case Direction::Up: {
		//			gridPixelX = ((GRID_SIZE - 1) - gridCol);
		//			gridPixelY = ((GRID_SIZE - 1) - gridRow);
		//			break;
		//		}
		//		}

		//		int pixelPosX = screenPosX + (flipH ? (GRID_SIZE - 1) - gridPixelX : gridPixelX);
		//		int pixelPosY = screenPosY + (flipV ? (GRID_SIZE - 1) - gridPixelY : gridPixelY);

		//		if (insideCamera) {
		//			if (pixelPosX >= cameraWidth + cameraBorderWidth || pixelPosX < cameraBorderWidth) continue;
		//			if (pixelPosY >= cameraHeight + cameraBorderWidth || pixelPosY < cameraBorderWidth) continue;
		//		}
		//		else {
		//			if (pixelPosX >= (int)screenWidth  || pixelPosX < 0) continue;
		//			if (pixelPosY >= (int)screenHeight || pixelPosY < 0) continue;
		//		}
		//		

		//		unsigned int pixelX = gridCol + (gridPosX * GRID_SIZE);
		//		unsigned int pixelY = gridRow + (gridPosY * GRID_SIZE);

		//		const unsigned int pixelIndex = (pixelY * spritesheet->width + pixelX) * spritesheet->channels;

		//		if (spritesheet->channels != 3 && spritesheet->data[pixelIndex + 3] == 0) continue;
		//		canvas.draw(pixelPosX, pixelPosY, &spritesheet->data[pixelIndex]);
		//	}
		//}
	}

	void Renderer::DrawFromSpritesheetIDWorldPosition(const Vec2& worldPosition, const unsigned int gridID, const Vec2& offset, Direction facing, bool flipH, bool flipV) {
		DrawFromSpritesheetIDScreenPosition(ConvertWorldToScreen(worldPosition), gridID, offset, facing, flipH, flipV, true);
	}

	//The first character in the spritesheet is '!', which corresponds to gridID 144. Each character is 18 pixels wide and 32 pixels tall, with a 2-pixel gap between characters. The space character (' ') is represented by an empty space of 8 pixels wide.
	void Renderer::DrawString(const Vec2& screenPosition, const unsigned char* string) {
		unsigned int screenOffsetX = 0;
		unsigned int screenOffsetY = 0;
		
		// Loops until there is no character in string (null terminator '\0' is reached)
		for (int i = 0; string[i] != '\0'; ++i) {
			char character = string[i];

			// Resets Offset X and increases Offset Y by GRID_SIZE when making a new line
			if (character == '\n') {
				screenOffsetX = 0;
				screenOffsetY += GRID_SIZE;
				continue;
			}

			// Increases Offset X by 8 when a space is entered
			if (character == ' ') {
				screenOffsetX += 8;
				continue;
			}

			// Characters are stored in ASCII order in the spritesheet, from '!' (ASCII 33) to '~' (ASCII 126).
			// Therefore, able to calculate spritesheetID just by subtracting '!' and adding 144 (the spritesheetID of '!').
			if (character < '!' || character > '~') continue;
			
			unsigned int spritesheetID = 144 + (character  - '!');
			DrawFromSpritesheetIDScreenPosition(screenPosition, spritesheetID, { (float)screenOffsetX, (float)screenOffsetY });
			
			// Increase Offset X to move to next character position.
			screenOffsetX += 18;
		}
	}

	void Renderer::GetCameraCorners(Vec2& topLeft, Vec2& bottomRight) const {;
		topLeft = Vec2{ (float)cameraBorderWidth };
		bottomRight = Vec2{ (float)cameraWidth, (float)cameraHeight } + Vec2{ (float)cameraBorderWidth } * 2;
	}

	void Renderer::SetCameraWorldPosition(const Vec2& cameraWorldPosition_) {
		cameraWorldPosition = cameraWorldPosition_;
	}

	Vec2 Renderer::ConvertWorldToScreen(const Vec2& worldPosition) const {
		return worldPosition - cameraWorldPosition + (Vec2{ float(cameraWidth) , float(cameraHeight) } / 2) + Vec2{ (float)cameraBorderWidth };
	}

	int Renderer::ConvertWorldToScreenX(const int worldPositionX) const {
		return worldPositionX - (int)cameraWorldPosition.GetX() + (cameraWidth / 2) + cameraBorderWidth;
	}

	int Renderer::ConvertWorldToScreenY(const int worldPositionY) const {
		return worldPositionY - (int)cameraWorldPosition.GetY() + (cameraHeight / 2) + cameraBorderWidth;
	}

	bool Renderer::IsWorldPositionWithinCamera(const Vec2& worldPosition) const {
		return IsScreenPositionWithinCamera(ConvertWorldToScreen(worldPosition));
	}

	bool Renderer::IsScreenPositionWithinCamera(const Vec2& screenPosition) const {
		return IsScreenXWithinCamera((int)screenPosition.GetX()) && IsScreenYWithinCamera((int)screenPosition.GetY());
	}

	bool Renderer::IsScreenXWithinCamera(const int screenPixelX) const {
		return screenPixelX <= int(cameraWidth + cameraBorderWidth) && screenPixelX >= (int)cameraBorderWidth;
	}

	bool Renderer::IsScreenYWithinCamera(const int screenPixelY) const {
		return screenPixelY <= int(cameraHeight + cameraBorderWidth) && screenPixelY >= (int)cameraBorderWidth;
	}

	Vec2 Renderer::GetCameraBounds() const {
		return { (float)cameraBorderWidth + cameraWidth , (float)cameraBorderWidth + cameraHeight };
	}

	Vec2 Renderer::GetCameraWorldPosition() const {
		return cameraWorldPosition;
	}

	void Renderer::UpdateAnimationFrame(float deltaTime, c_AnimatedSprite* animatedSprite) {
		if (!animatedSprite->isPlaying) return;
		animatedSprite->timePassed += deltaTime;

		while (animatedSprite->timePassed > animatedSprite->secondsPerFrame) {
			animatedSprite->animationFrameIndex++;
			animatedSprite->timePassed -= animatedSprite->secondsPerFrame;

			if (animatedSprite->animationFrameIndex < animatedSprite->GetCurrentAnimationFrameCount()) continue;

			if (animatedSprite->IsCurrentAnimationLoop()) {
				animatedSprite->animationFrameIndex = 0;
				continue;
			}
			animatedSprite->animationFrameIndex = animatedSprite->GetCurrentAnimationFrameCount() - 1;
			animatedSprite->isPlaying = false;
			animatedSprite->timePassed = 0.0f;
			return;
		}
	}
}