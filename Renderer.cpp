#include "Renderer.hpp"
#include <iostream>
#include "Decoder.hpp"

namespace WM908_Assessment {
	Renderer::Renderer(GamesEngineeringBase::Window& canvas_) : canvas(canvas_) {
		// Get Spritesheet Data
		spritesheetData = Decoder::ReadSpritesheetData(SPRITESHEET_FILE, spritesheetWidth, spritesheetHeight, colorPalette, colorPaletteSize);

		gridHeight = spritesheetHeight / GRID_SIZE;
		gridWidth = spritesheetWidth / GRID_SIZE;
	}

	Renderer::~Renderer() {
		delete[] spritesheetData;

		for (uint8_t i = 0; i < colorPaletteSize; i++)
			delete[] colorPalette[i];
		delete[] colorPalette;
	}

	void Renderer::DrawCameraBackground(unsigned int colorPaletteIndex) {
		// Draw Square within Camera Bounds
		DrawSquare(colorPaletteIndex, cameraBorderWidth, cameraHeight + cameraBorderWidth, cameraBorderWidth, cameraWidth + cameraBorderWidth);
	}

	void Renderer::DrawSquare(unsigned int colorPaletteIndex, const unsigned int& screenTop, const unsigned int& screenBottom, const unsigned int& screenLeft, const unsigned int& screenRight) {
		// Get Pixel Color
		uint8_t* pixelColor = colorPalette[colorPaletteIndex];

		// Always use the actual canvas width for buffer indexing
		const unsigned int screenWidth = canvas.getWidth();
		const unsigned int screenHeight = canvas.getHeight();

		for (unsigned int y = screenTop; y < screenBottom; y++) {
			unsigned char* screenPixel = canvas.getBackBuffer() + ((y * screenWidth) + screenLeft) * 3;

			for (unsigned int x = screenLeft; x < screenRight; x++) {
				// Draw background if pixel index is 0
				if (screenPixel[0] == 0 && screenPixel[1] == 0 && screenPixel[2] == 0) {
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
		int screenPosX = (int)roundf(screenPosition.GetX() + offset.GetX()), screenPosY = (int)roundf(screenPosition.GetY() + offset.GetY());

		int screenTop = 0, screenLeft = 0, screenBottom = screenHeight, screenRight = screenWidth;

		// Use camera properties if inside camera is true
		if (insideCamera) {
			screenTop = screenLeft = cameraBorderWidth;
			screenRight = cameraWidth + cameraBorderWidth;
			screenBottom = cameraHeight + cameraBorderWidth;
		}

		int screenPixelX = screenPosX, screenPixelY = screenPosY;
		
		int gridStartCol = std::max(0, screenLeft - screenPosX);
		int gridStartRow = std::max(0, screenTop - screenPosY);
		int gridEndCol = std::min((int)GRID_SIZE, screenRight - screenPosX);
		int gridEndRow = std::min((int)GRID_SIZE, screenBottom - screenPosY);

		// Skip if the sprite is completely out of bounds
		if (gridStartCol >= gridEndCol || gridStartRow >= gridEndRow) {
			return;
		}

		// How many pixels in spritesheet should they jump
		// Seperate variables to account for rotation logic
		int pixelStartX = 0, pixelStartY = 0, pixelStepX = 0, pixelStepY = 0;

		// Rotation Logic
		switch (facing)
		{
		case Direction::Down: {
			pixelStartX = flipH ? GRID_SIZE - 1 - gridStartCol : gridStartCol;
			pixelStartY = flipV ? GRID_SIZE - 1 - gridStartRow : gridStartRow;

			pixelStepX = flipH ? -1 : 1;
			pixelStepY = (flipV ? -1 : 1) * (int)spritesheetWidth;
			break;
		}
		case Direction::Up: {
			pixelStartX = flipH ? gridStartCol : GRID_SIZE - 1 - gridStartCol;
			pixelStartY = flipV ? gridStartRow : GRID_SIZE - 1 - gridStartRow;

			pixelStepX = flipH ? 1 : -1;
			pixelStepY = (flipV ? 1 : -1) * (int)spritesheetWidth;
			break;
		}
		case Direction::Right: {
			pixelStartX = flipH ? gridStartRow : GRID_SIZE - 1 - gridStartRow;
			pixelStartY = flipV ? GRID_SIZE - 1 - gridStartCol : gridStartCol;

			pixelStepX = (flipV ? -1 : 1) * (int)spritesheetWidth;
			pixelStepY = flipH ? 1 : -1;
			break;
		}
		case Direction::Left: {
			pixelStartX = flipH ? GRID_SIZE - 1 - gridStartRow : gridStartRow;
			pixelStartY = flipV ? gridStartCol : GRID_SIZE - 1 - gridStartCol;

			pixelStepX = (flipV ? 1 : -1) * (int)spritesheetWidth;
			pixelStepY = flipH ? -1 : 1;
			break;
		}
		}

		int spritesheetIndex = (((gridPosY * GRID_SIZE + pixelStartY) * spritesheetWidth) + (gridPosX * GRID_SIZE + pixelStartX));
		
		for (int gridRow = gridStartRow; gridRow < gridEndRow; gridRow++) {
			uint8_t* screenIndex = canvas.getBackBuffer() + ((screenPosY + gridRow) * screenWidth + screenPosX + gridStartCol) * 3;
			uint8_t* pixelIndex = &spritesheetData[spritesheetIndex];

			for (int gridCol = gridStartCol; gridCol < gridEndCol; gridCol++) {
				if (*pixelIndex > 0) {
					uint8_t* pixelData = colorPalette[*pixelIndex];

					screenIndex[0] = pixelData[0];
					screenIndex[1] = pixelData[1];
					screenIndex[2] = pixelData[2];
				}

				pixelIndex += pixelStepX;
				screenIndex += 3;
			}

			spritesheetIndex += pixelStepY;
		}

		// Old code: extremely unoptimized and too simple
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
	void Renderer::DrawString(const Vec2& screenPosition, const char* string) {
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
		return std::round(worldPosition - cameraWorldPosition + (Vec2{ float(cameraWidth) , float(cameraHeight) } / 2) + Vec2{ (float)cameraBorderWidth });
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
}