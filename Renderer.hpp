#pragma once
#include "GamesEngineeringBase.h"
#include "Component.hpp"

namespace WM908_Assessment {
	class Renderer {
	private:
		const char* SPRITESHEET_FILE = "res/Spritesheet.bin";
		const char* PALETTE_FILE = "res/Palette.png";
		Vec2 cameraWorldPosition;

		GamesEngineeringBase::Window& canvas;
		uint8_t** colorPalette = nullptr;

		uint16_t spritesheetHeight = 0, spritesheetWidth = 0;
		uint8_t* spritesheetData = nullptr;

		unsigned int gridHeight, gridWidth;
		uint8_t colorPaletteSize = 0;
	public:
		const unsigned int
			cameraWidth = 383,
			cameraHeight = 383,
			cameraBorderWidth = 20;

		static constexpr unsigned int GRID_SIZE = 32, MAX_DRAW_LAYERS = 4;
		
		Renderer(GamesEngineeringBase::Window& canvas_);
		~Renderer();

		void DrawCameraBackground(unsigned int colorPaletteIndex);
		void DrawSquare(unsigned int colorPaletteIndex, const unsigned int& screenTop, const unsigned int& screenBottom, const unsigned int& screenLeft, const unsigned int& screenRight);

		void DrawFromSpritesheetIDScreenPosition(
			const Vec2& screenPosition,
			const unsigned int gridID,
			const Vec2& offset = { 0.0f },
			Direction facing = Direction::Down,
			bool flipH = false,
			bool flipV = false,
			bool insideCamera = false
		);

		void DrawFromSpritesheetIDWorldPosition(
			const Vec2& worldPosition,
			const unsigned int gridID,
			const Vec2& offset,
			Direction facing = Direction::Down,
			bool flipH = false,
			bool flipV = false
		);

		void DrawString(const Vec2& screenPosition, const char* value);

		bool IsWorldPositionWithinCamera(const Vec2& worldPosition) const;
		bool IsScreenPositionWithinCamera(const Vec2& screenPosition) const;
		bool IsScreenXWithinCamera(const int screenPixelX) const;
		bool IsScreenYWithinCamera(const int screenPixelY) const;

		void GetCameraCorners(Vec2& topLeft, Vec2& bottomRight) const;
		void SetCameraWorldPosition(const Vec2& cameraWorldPosition_);
		Vec2 ConvertWorldToScreen(const Vec2& worldPosition) const;
		int ConvertWorldToScreenX(const int worldPositionX) const;
		int ConvertWorldToScreenY(const int worldPositionY) const;
		Vec2 GetCameraBounds() const;
		Vec2 GetCameraWorldPosition() const;
	};
}