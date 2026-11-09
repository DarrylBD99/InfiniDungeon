#pragma once

#include "ComponentSystem.hpp"

#include <random>
#include <bit>
#include "Decoder.hpp"
#include <cassert>
#include <string_view>
#include <memory>

namespace WM908_Assessment {
	class Room {
	private:
		static constexpr std::string_view ROOM_FILE_DIRECTORY = "res/maps/Room_{}.bin";
		static constexpr std::string_view TRIGGER_FILE_DIRECTORY = "res/maps/Trigger_{}.bin";

		uint8_t width = 0, height = 0;
		unsigned int worldWidth = Renderer::GRID_SIZE * width, worldHeight = Renderer::GRID_SIZE * height;

		bool visited = false;
		Vec2 worldPos;
		int posX = 0, posY = 0;

		Dataset::Array<Entity*> tileEntities = Dataset::Array<Entity*>(0);
		Dataset::Array<bool> triggerState = Dataset::Array<bool>(0);

		uint8_t* tileIDs = nullptr;
		uint8_t* triggerIDs = nullptr;
		const unsigned int roomID;
	public:
		Room(unsigned int roomID_, int posX_, int posY_, uint8_t roomType);
		Room(unsigned int roomID_, int posX_, int posY_);
		~Room();

		unsigned int GetRoomID() const { return roomID; };
		void SetVisited();
		//void FreeRoomsneighboringneighborRooms(Room* exception = nullptr);
		uint8_t FetchRoomType(unsigned int roomID_);
		void FetchRoomData(uint8_t roomIndex);
		bool IsMapVisible(const Renderer* renderer) const;
		bool IsTileVisible(const Renderer* renderer, const c_Position* position) const;
		bool IsTileXVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionX) const;
		bool IsTileYVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionY) const;
		void UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities);
		Entity* CreateTileEntity(
			unsigned int tileID,
			unsigned int triggerID,
			unsigned int index,
			int worldPositionX,
			int worldPositionY,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_EntityData& entityDataSystem,
			s_Triggers& triggerSystem,
			s_Spawner& spawnerSystem,
			s_Attacks& attackSystem);

		void CreateVisibleTiles(
			const Renderer* renderer,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_EntityData& entityDataSystem,
			s_Triggers& triggerSystem,
			s_Spawner& spawnerSystem,
			s_Attacks& attackSystem);

		bool IsPlayerInsideRoom(const Vec2 playerPos) const;

		int GetX() const { return posX; }
		int GetY() const  { return posY; }
		Vec2 GetWorldPos() const { return worldPos; }

		void OpenEntrance(Direction direction);
		void SetTile(unsigned int index, uint8_t tileID);
		void SetTrigger(unsigned int triggerID, bool triggerState, s_Triggers& triggerSystem);
	};

	class MapGenerator {
	private:
		int worldSeed = 0;
		std::unique_ptr<Room> currentRoom = nullptr;

		std::unique_ptr<Room> leftRoom = nullptr;
		std::unique_ptr<Room> rightRoom = nullptr;
		std::unique_ptr<Room> topRoom = nullptr;
		std::unique_ptr<Room> bottomRoom = nullptr;

		unsigned int GetRoomID(int posX, int posY);
		bool ShouldRoomGenerate(int posX, int posY);
		bool CanGenerateEntrances(int posX, int posY);
		uint8_t GetOpenedEntrances(int posX, int posY);
		void FreeneighborRooms(Room* exception = nullptr);
	public:
		MapGenerator();
		MapGenerator(int seed_);

		int GetWorldSeed() const { return worldSeed; }

		void StartInfiniteWorldGeneration(int posX = 0, int posY = 0);
		void Update(const Vec2& playerPos);
		void GenerateDemoRoom();
		Room* GenerateRoom(int posX, int posY);
		void GenerateNeighboringRooms(int posX, int posY);
		void UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities);
		Room* GetRoomWithID(unsigned int roomID) const;
		void CreateVisibleTiles(
			const Renderer* renderer,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_EntityData& entityDataSystem,
			s_Triggers& triggerSystem,
			s_Spawner& spawnerSystem,
			s_Attacks& attackSystem);
	};
}