#pragma once

#include "ComponentSystem.hpp"

#include <iostream>
#include <fstream>

namespace WM908_Assessment {
	class Room {
	private:
		const std::string ROOM_FILE_DIRECTORY = "res/maps/Room_";
		const std::string FILE_TYPE = ".txt";

		unsigned int width = 12, height = 10;
		unsigned int worldWidth = Renderer::GRID_SIZE * width, worldHeight = Renderer::GRID_SIZE * height;

		int posX, posY;
		Vec2 worldPos;

		Dataset::Array<Entity*>* tileEntities = nullptr;
		unsigned int** tileIDs = nullptr;

		Room() : posX(0), posY(0) {}

	public:
		Room* leftRoom = nullptr;
		Room* rightRoom = nullptr;
		Room* topRoom = nullptr;
		Room* bottomRoom = nullptr;

		Room(int seed, int posX_, int posY_) : posX(posX_), posY(posY_) {

			// Map Data is dependent on x and y position as well as world seed
			// I would try a complicated algorithm, but for testing purposes, I'm doing something linear
			int mapData = (seed ^ posX * 31 ^ posY * 17) & 0xF;


			// Create Map
			worldPos = { (float)posX * worldWidth, (float)posY * worldHeight};
			CreateTileEntityIDArray();
		}

		Vec2 GetWorldPos() const {
			return worldPos;
		}

		void FetchRoomData(unsigned int roomIndex) {
			std::ifstream file((ROOM_FILE_DIRECTORY + std::to_string(roomIndex) + FILE_TYPE));

			tileIDs = new unsigned int* [height];
			for (unsigned int y = 0; y < height; y++) {
				tileIDs[y] = new unsigned int[width];

				for (unsigned int x = 0; x < width; x++) {
					file >> tileIDs[y][x];
				}
			}
		}

		bool IsMapVisible(const Renderer* renderer) const {
			Vec2 cameraTopLeft, cameraBottomRight;
			renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

			Vec2 screenPosition = renderer->ConvertWorldToScreen(worldPos);
			
			if (cameraTopLeft.GetX() > screenPosition.GetX() + worldWidth) return false;
			if (cameraTopLeft.GetY() > screenPosition.GetY() + worldHeight) return false;
			if (cameraBottomRight.GetX() < screenPosition.GetX()) return false;
			if (cameraBottomRight.GetY() < screenPosition.GetY()) return false;

			return true;
		}

		bool IsTileVisible(const Renderer* renderer, const c_Position* position) {
			Vec2 cameraTopLeft, cameraBottomRight;
			renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

			Vec2 screenPosition = renderer->ConvertWorldToScreen(position->position);
			return IsTileXVisible(cameraTopLeft, cameraBottomRight, screenPosition.GetX())
				&& IsTileYVisible(cameraTopLeft, cameraBottomRight, screenPosition.GetY());
		}

		bool IsTileXVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionX) {
			return ((screenPositionX + Renderer::GRID_SIZE) > cameraTopLeft.GetX()
				&& screenPositionX <= cameraBottomRight.GetX());
		}

		bool IsTileYVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionY) {
			return ((screenPositionY + Renderer::GRID_SIZE) > cameraTopLeft.GetY()
				&& screenPositionY <= cameraBottomRight.GetY());

		}

		void AddTilesToUnload(const Renderer* renderer, const s_Position& positionSystem, unsigned int* unloadedEntities, unsigned int& unloadEntityCount) {
			Vec2 cameraTopLeft, cameraBottomRight;
			renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

			for (unsigned int idx = 0; idx < tileEntities->maxSize; idx++) {
				Entity* entity = tileEntities->Get(idx);
				if (entity == nullptr) continue;

				unsigned int entityID = entity->GetEntityID();
				c_Position* position = positionSystem.GetComponent(entityID);
				if (position == nullptr) continue;

				if (IsTileVisible(renderer, position)) continue;
				tileEntities->SetValue(nullptr, idx);
				unloadedEntities[unloadEntityCount] = entityID;
				unloadEntityCount++;
			}
			// temporary print for testing purposes
			//for (unsigned int idx = 0; idx < height * width; idx++) {
			//	if (idx == 0) std::cout << "[";
			//	else if (idx % width == 0) std::cout << std::endl;
			//	Entity* entity = tileEntities->Get(idx);
			//	
			//	if (entity == nullptr) {
			//		std::cout << "-1, ";
			//	} else {
			//		std::cout << entity->GetEntityID() << ", ";
			//	}

			//}
			//std::cout << "]" << std::endl;
		}

		Entity* CreateTileEntity(unsigned int tileID, int worldPositionX, int worldPositionY, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem) {
			unsigned int entityID = entityManager.AppendEntity();
			c_Position* position = positionSystem.AddComponent(entityID);
			position->position = { worldPositionX, worldPositionY };
			spriteSystem.AddSprite(entityID, tileID, 0);

			if (tileID == 2) collisionSystem.AddCollisionBox(entityID, { (float)Renderer::GRID_SIZE }, true);

			return entityManager.GetEntityData(entityID);
		}

		void CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem) {
			if (!IsMapVisible(renderer)) return;

			Vec2 cameraTopLeft, cameraBottomRight;
			renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

			for (unsigned int tileY = 0; tileY < height; tileY++) {
				unsigned int worldPositionY = (tileY * Renderer::GRID_SIZE) + worldPos.GetY();
				if (!IsTileYVisible(cameraTopLeft, cameraBottomRight, renderer->ConvertWorldToScreenY(worldPositionY))) continue;

				for (unsigned int tileX = 0; tileX < width; tileX++) {
					unsigned int worldPositionX = (tileX * Renderer::GRID_SIZE) + worldPos.GetX();
					if (!IsTileXVisible(cameraTopLeft, cameraBottomRight, renderer->ConvertWorldToScreenX(worldPositionX))) continue;

					unsigned int idx = tileY * width + tileX;
					if (tileEntities->Get(idx) != nullptr) continue;

					Entity* entity = CreateTileEntity(tileIDs[tileY][tileX], worldPositionX, worldPositionY, entityManager, positionSystem, spriteSystem, collisionSystem);
					tileEntities->SetValue(entity, idx);
				}
			}
		}

		void CreateTileEntityIDArray() {
			tileEntities = new Dataset::Array<Entity*>(width * height);
		}

		static Room* CreateDemoRoom() {
			Room* map = new Room();

			map->width = 15;
			map->height = 20;

			map->worldWidth = Renderer::GRID_SIZE * map->width;
			map->worldHeight = Renderer::GRID_SIZE * map->height;

			map->CreateTileEntityIDArray();

			map->FetchRoomData(0);

			return map;
		}
	};

	class MapGenerator {
	private:
		int seed = 0;
		Room* currentRoom = nullptr;

	public:
		MapGenerator() {}
		~MapGenerator() {}

		void GenerateDemoRoom() {
			currentRoom = Room::CreateDemoRoom();
		}

		void AddTilesToUnload(const Renderer* renderer, const s_Position& positionSystem, unsigned int* unloadedEntities, unsigned int& unloadEntityCount) {
			currentRoom->AddTilesToUnload(renderer, positionSystem, unloadedEntities, unloadEntityCount);
		}

		void CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem) {
			Vec2 cameraTopLeft, cameraBottomRight;
			renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);
			
			currentRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem);
			if (currentRoom->bottomRoom != nullptr) currentRoom->bottomRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem);
			if (currentRoom->rightRoom != nullptr) currentRoom->rightRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem);
			if (currentRoom->leftRoom != nullptr) currentRoom->leftRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem);
			if (currentRoom->topRoom != nullptr) currentRoom->topRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem);
		}
	};
}