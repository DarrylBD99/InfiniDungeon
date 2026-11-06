#pragma once

#include "ComponentSystem.hpp"

#include <random>
#include <bit>
#include "Decoder.hpp"
#include <cassert>

namespace WM908_Assessment {
	class Room {
	private:
		const std::string ROOM_FILE_DIRECTORY = "res/maps/Room_";
		const std::string FILE_TYPE = ".txt";

		uint8_t width = 0, height = 0;
		unsigned int worldWidth = Renderer::GRID_SIZE * width, worldHeight = Renderer::GRID_SIZE * height;

		Vec2 worldPos;
		int posX = 0, posY = 0;

		Dataset::Array<Entity*> tileEntities = Dataset::Array<Entity*>(0);
		uint8_t* tileIDs = nullptr;

	public:
		Room(uint8_t roomType, int posX_, int posY_);

		//void FreeRoomsneighboringneighborRooms(Room* exception = nullptr);
		void FetchRoomData(uint8_t roomIndex);
		bool IsMapVisible(const Renderer* renderer) const;
		bool IsTileVisible(const Renderer* renderer, const c_Position* position) const;
		bool IsTileXVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionX) const;
		bool IsTileYVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionY) const;
		void UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities);
		Entity* CreateTileEntity(unsigned int tileID, int worldPositionX, int worldPositionY, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem);
		void CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem);
		bool IsPlayerInsideRoom(const Vec2 playerPos) const;

		int GetX() const { return posX; }
		int GetY() const  { return posY; }
		Vec2 GetWorldPos() const { return worldPos; }

		void OpenEntrance(Direction direction);
	};

	class MapGenerator {
	private:
		int worldSeed = 0;
		Room* currentRoom = nullptr;

		Room* leftRoom = nullptr;
		Room* rightRoom = nullptr;
		Room* topRoom = nullptr;
		Room* bottomRoom = nullptr;

		uint8_t GetRoomTemplate(int posX, int posY);
		bool ShouldRoomGenerate(int posX, int posY);
		bool CanGenerateEntrances(int posX, int posY);
		uint8_t GetOpenedEntrances(int posX, int posY);
		void FreeneighborRooms(Room* exception = nullptr);
	public:
		MapGenerator();
		MapGenerator(int seed_);
		~MapGenerator();
		void StartInfiniteWorldGeneration();
		void Update(const Vec2& playerPos);
		void GenerateDemoRoom();
		Room* GenerateRoom(int posX, int posY);
		void GenerateneighboringRooms(int posX, int posY);
		void UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities);
		void CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem);
	};
}