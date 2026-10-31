#include "Renderer.hpp"

namespace WM908_Assessment {
	Renderer::Renderer(GamesEngineeringBase::Window& canvas_) : canvas(canvas_) {
		spritesheet = new GamesEngineeringBase::Image();
		spritesheet->load(SPRITESHEET_FILE);

		gridHeight = spritesheet->height / GRID_SIZE;
		gridWidth = spritesheet->width / GRID_SIZE;
	}

	Renderer::~Renderer() {
		delete spritesheet;
	}

	void Renderer::DrawCameraBackground(unsigned char r, unsigned char g, unsigned char b) {
		// Clear the entire canvas with a blue background (RGB: 0, 0, 255)
		for (unsigned int y = cameraBorderWidth; y < cameraBorderWidth + cameraHeight; y++) {
			for (unsigned int x = cameraBorderWidth; x < cameraBorderWidth + cameraWidth; x++) {
				canvas.draw(x, y, r, g, b); // Draw the blue background pixel by pixel.
			}
		}
	}

	void Renderer::DrawFromSpritesheetIDWorldPosition(const Vec2& worldPosition, const unsigned int gridID, const Vec2& offset, Direction facing, bool flipH, bool flipV) {
		DrawFromSpritesheetIDScreenPosition(ConvertWorldToScreen(worldPosition), gridID, offset, facing, flipH, flipV, true);
	}

	void Renderer::DrawFromSpritesheetIDScreenPosition(const Vec2& screenPosition, const unsigned int gridID, const Vec2& offset, Direction facing, bool flipH, bool flipV, bool insideCamera) {
		if (spritesheet->data == nullptr) return;

		const unsigned int gridPosX = gridID % gridWidth;
		const unsigned int gridPosY = gridID / gridWidth;
		const unsigned int screenHeight = canvas.getHeight();
		const unsigned int screenWidth = canvas.getWidth();

		const int screenPosX = (int)roundf(screenPosition.GetX() + offset.GetX());
		const int screenPosY = (int)roundf(screenPosition.GetY() + offset.GetY());

		for (int gridRow = 0; gridRow < GRID_SIZE; gridRow++) {
			int screenPixelY = screenPosY + gridRow;

			if (screenPixelY >= (int)screenHeight || screenPixelY < 0) continue;
			else if (insideCamera && !IsScreenYWithinCamera(screenPixelY)) continue;
			 

			for (int gridCol = 0; gridCol < GRID_SIZE; gridCol++) {
				int screenPixelX = screenPosX + gridCol;
				if (screenPixelX >= (int)screenWidth || screenPixelX < 0) continue;
				else if (insideCamera && !IsScreenXWithinCamera(screenPixelX)) continue;

				unsigned int pixelX = 0, pixelY = 0;

				switch (facing)
				{
				case Direction::Down: {
					pixelX = gridCol;
					pixelY = gridRow;
					break;
				}

				case Direction::Right: {
					pixelX = gridRow;
					pixelY = ((GRID_SIZE - 1) - gridCol);
					break;
				}

				case Direction::Left: {
					pixelX = ((GRID_SIZE - 1) - gridRow);
					pixelY = gridCol;
					break;
				}

				case Direction::Up: {
					pixelX = ((GRID_SIZE - 1) - gridCol);
					pixelY = ((GRID_SIZE - 1) - gridRow);
					break;
				}
				}

				if (flipH) pixelX = (GRID_SIZE - 1) - pixelX;
				if (flipV) pixelY = (GRID_SIZE - 1) - pixelY;

				pixelX += (gridPosX * GRID_SIZE);
				pixelY += (gridPosY * GRID_SIZE);

				const unsigned int pixelIndex = (pixelY * spritesheet->width + pixelX) * spritesheet->channels;

				if (spritesheet->channels != 3 && spritesheet->data[pixelIndex + 3] == 0) continue;
				canvas.draw(screenPixelX, screenPixelY, &spritesheet->data[pixelIndex]);
			}
		}

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

	void Renderer::DrawString(const Vec2& screenPosition, const unsigned char* string) {
		unsigned int screenOffsetX = 0;
		unsigned int screenOffsetY = 0;
		
		for (int i = 0; string[i] != '\0'; ++i) {
			char character = string[i];

			if (character == '\n') {
				screenOffsetX = 0;
				screenOffsetY += GRID_SIZE;
				continue;
			}

			if (character == ' ') {
				screenOffsetX += 8;
				continue;
			}

			// only accepts characters within ASCII range of 33 to 126
			if (character < '!' || character > '~') continue;
			
			unsigned int charIdx = 144 + (character  - '!');
			DrawFromSpritesheetIDScreenPosition(screenPosition, charIdx, { (float)screenOffsetX, (float)screenOffsetY });
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
		return worldPositionX - (int)cameraWorldPosition.GetX() + (cameraWidth / 2);
	}

	int Renderer::ConvertWorldToScreenY(const int worldPositionY) const {
		return worldPositionY - (int)cameraWorldPosition.GetY() + (cameraHeight / 2);
	}

	bool Renderer::IsScreenPositionWithinCamera(const Vec2& screenPosition) const {
		return IsScreenXWithinCamera((int)screenPosition.GetX()) && IsScreenYWithinCamera((int)screenPosition.GetY());
	}

	bool Renderer::IsScreenXWithinCamera(const int screenPixelX) const {
		return screenPixelX < int(cameraWidth + cameraBorderWidth) && screenPixelX >= (int)cameraBorderWidth;
	}

	bool Renderer::IsScreenYWithinCamera(const int screenPixelY) const {
		return screenPixelY < int(cameraHeight + cameraBorderWidth) && screenPixelY >= (int)cameraBorderWidth;
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