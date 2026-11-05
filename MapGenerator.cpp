#include "MapGenerator.hpp"

namespace WM908_Assessment {

	Room::Room(uint8_t roomType, int posX_, int posY_) : posX(posX_), posY(posY_) {
		FetchRoomData(roomType);

		worldWidth = Renderer::GRID_SIZE * width;
		worldHeight = Renderer::GRID_SIZE * height;

		// Create Map
		worldPos = { (float)posX * worldWidth, (float)posY * worldHeight };
		tileEntities = Dataset::Array<Entity*>(width * height);
	}

	// Players can look over at the corners and may find an empty void until
	// the player actually enters the current room.
	// The original purpose of this function is to instead redirect the
	// freeing to the neighbours of the neighbour
	// 
	// Of course, this was scrapped as it will make the game much more expensive.
	// And since each room has their own neighbours, the logic will just be too complicated.
	// 
	// I decided to keep the function here cause the name I gave it is out of pocket
	// 
	//void Room::FreeRoomsNeighbouringNeighbourRooms(Room* exception = nullptr) { 
	//	if (leftRoom != exception) leftRoom->FreeNeighbourRooms(this);
	//	if (rightRoom != exception) rightRoom->FreeNeighbourRooms(this);
	//	if (topRoom != exception) topRoom->FreeNeighbourRooms(this);
	//	if (bottomRoom != exception) bottomRoom->FreeNeighbourRooms(this);
	//}

	void Room::FetchRoomData(uint8_t roomIndex) {
		tileIDs = Decoder::ReadRoomData((ROOM_FILE_DIRECTORY + std::to_string(roomIndex) + FILE_TYPE)
			, width, height);

	}

	bool Room::IsMapVisible(const Renderer* renderer) const {
		Vec2 cameraTopLeft, cameraBottomRight;
		renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

		Vec2 screenPosition = renderer->ConvertWorldToScreen(worldPos);

		if (cameraTopLeft.GetX() > screenPosition.GetX() + worldWidth) return false;
		if (cameraTopLeft.GetY() > screenPosition.GetY() + worldHeight) return false;
		if (cameraBottomRight.GetX() < screenPosition.GetX()) return false;
		if (cameraBottomRight.GetY() < screenPosition.GetY()) return false;

		return true;
	}

	bool Room::IsTileVisible(const Renderer* renderer, const c_Position* position) const {
		Vec2 cameraTopLeft, cameraBottomRight;
		renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

		Vec2 screenPosition = renderer->ConvertWorldToScreen(position->position);
		return IsTileXVisible(cameraTopLeft, cameraBottomRight, (int)screenPosition.GetX())
			&& IsTileYVisible(cameraTopLeft, cameraBottomRight, (int)screenPosition.GetY());
	}

	bool Room::IsTileXVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionX) const {
		return ((screenPositionX + Renderer::GRID_SIZE) > cameraTopLeft.GetX()
			&& screenPositionX <= cameraBottomRight.GetX());
	}

	bool Room::IsTileYVisible(const Vec2& cameraTopLeft, const Vec2& cameraBottomRight, const int screenPositionY) const {
		return ((screenPositionY + Renderer::GRID_SIZE) > cameraTopLeft.GetY()
			&& screenPositionY <= cameraBottomRight.GetY());

	}

	void Room::UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities) {
		for (Entity*& entity : tileEntities) {
			if (entity == nullptr) continue;
			unsigned int entityID = entity->GetEntityID();

			if (unloadedEntities->HasData(entityID)) entity = nullptr;
		}
		// temporary print for testing purposes
		//for (unsigned int idx = 0; idx < height * width; idx++) {
		//	if (idx == 0) std::cout << "[";
		//	else if (idx % width == 0) std::cout << std::endl;
		//	Entity* entity = tileEntities.Get(idx);
		//	
		//	if (entity == nullptr) {
		//		std::cout << "-1, ";
		//	} else {
		//		std::cout << entity->GetEntityID() << ", ";
		//	}

		//}
		//std::cout << "]" << std::endl;
	}

	Entity* Room::CreateTileEntity(unsigned int tileID, int worldPositionX, int worldPositionY, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem) {
		if (tileID > 14) return nullptr; // There exists only 15 types of map tiles
		if (tileID == 14) tileID = 2; // Assumes entrance tile isn't open, set to wall

		unsigned int entityID = entityManager.AppendEntity();
		Entity* entity = entityManager.GetEntityData(entityID);

		positionSystem.AddComponent(entityID, { worldPositionX, worldPositionY });

		c_Sprite* sprite = spriteSystem.AddSprite(entityID, tileID, 0);

		// Add new properties for tiles that are walls or have external properties
		if (tileID < 2) return entity;

		// Create Collision for all tiles that aren't floor
		c_CollisionBox* collisionBox = collisionSystem.AddCollisionBox(entityID, { (float)Renderer::GRID_SIZE }, { 0.0f }, CollisionLayer::Wall);

		// Only continue if tile has extrenal behaviour that needs to be defined
		if (tileID == 2) return entity;

		// Add active state component to chests, pressure pads, and traps
		// They are placed in sequential order in spritesheetID for seamless implementation
		if (tileID >= 3 && tileID <= 6) {
			triggerSystem.AddActiveState(entityID, tileID + 12);

			// Individual interactable properties
			switch (tileID)
			{
				// Pressure Pads
			case 3:
				// Reduce hitbox size
				collisionBox->boxSize = { 24.0f, 14.0f };
				collisionBox->offset = { 4.0f, 9.0f };
				break;
			default:
				break;
			}
		}

		// Set Sprite Layer of spawners and collectibles to 1 and add another tile entity below
		// Spawners are given a tile below as they have health and can be destroyed by player
		else if (tileID >= 8 && tileID <= 13) {
			sprite->drawLayer = 1;
			unsigned int floorEntityID = entityManager.AppendEntity();
			positionSystem.AddComponent(floorEntityID, { worldPositionX, worldPositionY });
			spriteSystem.AddSprite(floorEntityID, 0, 0);

			// Collectibles
			if (tileID >= 11) {
				entityDataSystem.AddCollectable(entityID, CollectableType(tileID - 11));
				collisionBox->boxSize = { 22.0f };
				collisionBox->offset = { 5.0f };
				collisionBox->collisionLayer = 0;
				collisionBox->collisionMask = CollisionLayer::Player;

			}
			else { // Spawners
				entityDataSystem.AddHealth(entityID, 5);
				collisionBox->collisionLayer = collisionBox->collisionLayer | CollisionLayer::Enemy;
			}

			return entityManager.GetEntityData(floorEntityID);
		}

		return entity;
	}

	void Room::CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem) {
		if (!IsMapVisible(renderer)) return;

		Vec2 cameraTopLeft, cameraBottomRight;
		renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

		for (unsigned int tileY = 0; tileY < height; tileY++) {
			int worldPositionY = int(tileY * Renderer::GRID_SIZE) + (int)worldPos.GetY();
			if (!IsTileYVisible(cameraTopLeft, cameraBottomRight, renderer->ConvertWorldToScreenY(worldPositionY))) continue;

			for (unsigned int tileX = 0; tileX < width; tileX++) {
				int worldPositionX = int(tileX * Renderer::GRID_SIZE) + (int)worldPos.GetX();
				if (!IsTileXVisible(cameraTopLeft, cameraBottomRight, renderer->ConvertWorldToScreenX(worldPositionX))) continue;

				unsigned int idx = tileY * width + tileX;
				if (tileEntities.Get(idx) != nullptr) continue;

				Entity* entity = CreateTileEntity(tileIDs[(tileY * width) + tileX], worldPositionX, worldPositionY, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
				tileEntities.SetValue(entity, idx);
			}
		}
	}

	void Room::OpenEntrance(Direction direction) {
		// Allows addition straight to pointer
		unsigned int dataAdvance = 1;

		// Duplicate pointer so actual tileIDs doesn't move
		uint8_t* tileIDs_ = tileIDs;

		// Ensure it doesn't go beyond the array
		std::size_t loopCount = 0;

		switch (direction)
		{
		case Direction::Down: {
			tileIDs_ += (height - 1) * width;
			loopCount = width;
			break;
		}
		case Direction::Right: {
			tileIDs_ += width - 1;
			dataAdvance = width;
			loopCount = height;
			break;
		}
		case Direction::Left: {
			dataAdvance = width;
			loopCount = height;
			break;
		}
		case Direction::Up: {
			loopCount = width;
			break;
		}
		}

		// Tile ID 14 is entrance tile
		for (int i = 0; i < loopCount; i++) {
			if (*tileIDs_ == 14) *tileIDs_ = 0;

			tileIDs_ += dataAdvance;
		}
	}

	bool Room::IsPlayerInsideRoom(const Vec2 playerPos) const {
		return playerPos.GetX() >= worldPos.GetX() && playerPos.GetX() + Renderer::GRID_SIZE < worldPos.GetX() + worldWidth
			&& playerPos.GetY() >= worldPos.GetY() && playerPos.GetY() + Renderer::GRID_SIZE < worldPos.GetY() + worldHeight;
	}

	// Map Data is dependent on x and y position as well as world seed
	uint8_t MapGenerator::GetRoomTemplate(int posX, int posY) {
		std::seed_seq seedSequence{ worldSeed, posX, posY };
		std::mt19937 rng(seedSequence);
		std::uniform_int_distribution<unsigned int> distribution(1, 4);

		return (uint8_t)distribution(rng);
	}

	// Doesn't load room if no room is inaccessible
	bool MapGenerator::ShouldRoomGenerate(int posX, int posY) {
		if (CanGenerateEntrances(posX, posY)) return true;

		if ((GetOpenedEntrances(posX, posY - 1) & 0b1000) != 0) return true; // Is connected to room above
		if ((GetOpenedEntrances(posX - 1, posY) & 0b0100) != 0) return true; // Is connected to room to the left
		if ((GetOpenedEntrances(posX + 1, posY) & 0b0010) != 0) return true; // Is connected to room to the right
		if ((GetOpenedEntrances(posX, posY + 1) & 0b0001) != 0) return true; // Is connected to room below

		return false;
	}

	// Only rooms where their x and y values are even can generate entrances
	bool MapGenerator::CanGenerateEntrances(int posX, int posY) {
		return ((posX + posY) % 2 == 0);
	}

	// Each bit corresponds to an entrance (Down, Right, Left, Up)
	uint8_t MapGenerator::GetOpenedEntrances(int posX, int posY) {
		if (!CanGenerateEntrances(posX, posY)) return 0;

		std::seed_seq seedSequence{ worldSeed, posX, posY };
		std::mt19937 rng(seedSequence);
		std::uniform_int_distribution<unsigned int> distribution(0b0000, 0b1111);

		// Ensures at least 2 entrances are generated at each position
		uint8_t bitmask;
		do {
			bitmask = (uint8_t)distribution(rng);
		} while (std::popcount(bitmask) < 2);
		return bitmask;
	}

	// Free Neighbour Rooms as they are out of bounds and (for the most part) shouldn't be seen
	void MapGenerator::FreeNeighbourRooms(Room* exception) {
		if (leftRoom != exception) {
			delete leftRoom;
			leftRoom = nullptr;
		}

		if (rightRoom != exception) {
			delete rightRoom;
			rightRoom = nullptr;
		}

		if (topRoom != exception) {
			delete topRoom;
			topRoom = nullptr;
		}

		if (bottomRoom != exception) {
			delete bottomRoom;
			bottomRoom = nullptr;
		}
	}

	MapGenerator::MapGenerator() {
		std::random_device randomDevice;
		std::mt19937 rng(randomDevice());
		std::uniform_int_distribution<int> distribution(INT_MIN, INT_MAX);

		worldSeed = distribution(rng);
	}

	MapGenerator::MapGenerator(int seed_) {
		worldSeed = seed_;
	}

	MapGenerator::~MapGenerator() {}

	void MapGenerator::StartInfiniteWorldGeneration() {
		currentRoom = GenerateRoom(0, 0);
		GenerateNeighbouringRooms(0, 0);
	}

	void MapGenerator::Update(const Vec2& playerPos) {
		if (currentRoom->IsPlayerInsideRoom(playerPos)) return;

		Room* newRoom = nullptr;

		// Check which room player is inside
		if (bottomRoom != nullptr && bottomRoom->IsPlayerInsideRoom(playerPos)) {
			newRoom = bottomRoom;
			bottomRoom = nullptr;
			topRoom = currentRoom;
		}
		else if (rightRoom != nullptr && rightRoom->IsPlayerInsideRoom(playerPos)) {
			newRoom = rightRoom;
			rightRoom = nullptr;
			leftRoom = currentRoom;
		}
		else if (leftRoom != nullptr && leftRoom->IsPlayerInsideRoom(playerPos)) {
			newRoom = leftRoom;
			leftRoom = nullptr;
			rightRoom = currentRoom;
		}
		else if (topRoom != nullptr && topRoom->IsPlayerInsideRoom(playerPos)) {
			newRoom = topRoom;
			topRoom = nullptr;
			bottomRoom = currentRoom;
		}

		// Should be impossible to reach, but it's here just in case some miracle happens
		if (newRoom == nullptr || newRoom == currentRoom) {
			assert(true);
			return;
		}

		FreeNeighbourRooms(currentRoom);
		currentRoom = newRoom;

		GenerateNeighbouringRooms(currentRoom->GetX(), currentRoom->GetY());
	}

	void MapGenerator::GenerateDemoRoom() {
		currentRoom = new Room(0, 0, 0);
	}

	Room* MapGenerator::GenerateRoom(int posX, int posY) {
		if (!ShouldRoomGenerate(posX, posY)) return nullptr;
		return new Room(GetRoomTemplate(posX, posY), posX, posY);
	}

	void MapGenerator::GenerateNeighbouringRooms(int posX, int posY) {
		if (bottomRoom == nullptr) bottomRoom = GenerateRoom(posX, posY + 1);
		if (rightRoom == nullptr) rightRoom = GenerateRoom(posX + 1, posY);
		if (leftRoom == nullptr) leftRoom = GenerateRoom(posX - 1, posY);
		if (topRoom == nullptr) topRoom = GenerateRoom(posX, posY - 1);

		uint8_t entrances = GetOpenedEntrances(posX, posY);
		if (entrances == 0) {
			entrances |= (GetOpenedEntrances(posX, posY + 1) & 0b0001) << 3; // Down
			entrances |= (GetOpenedEntrances(posX + 1, posY) & 0b0010) << 1; // Right
			entrances |= (GetOpenedEntrances(posX - 1, posY) & 0b0100) >> 1; // Left
			entrances |= (GetOpenedEntrances(posX, posY - 1) & 0b1000) >> 3; // Up
		}

		// Open entrance tiles
		if ((entrances & 0b1000) != 0 && bottomRoom != nullptr) {
			currentRoom->OpenEntrance(Direction::Down);
			bottomRoom->OpenEntrance(Direction::Up);
		}

		if ((entrances & 0b0100) != 0 && rightRoom != nullptr) {
			currentRoom->OpenEntrance(Direction::Right);
			rightRoom->OpenEntrance(Direction::Left);
		}

		if ((entrances & 0b0010) != 0 && leftRoom != nullptr) {
			currentRoom->OpenEntrance(Direction::Left);
			leftRoom->OpenEntrance(Direction::Right);
		}

		if ((entrances & 0b0001) != 0 && topRoom != nullptr) {
			currentRoom->OpenEntrance(Direction::Up);
			topRoom->OpenEntrance(Direction::Down);
		}
	}

	void MapGenerator::UnloadTiles(Dataset::Array<unsigned int>* unloadedEntities) {
		currentRoom->UnloadTiles(unloadedEntities);
		if (bottomRoom != nullptr) bottomRoom->UnloadTiles(unloadedEntities);
		if (rightRoom != nullptr) rightRoom->UnloadTiles(unloadedEntities);
		if (leftRoom != nullptr) leftRoom->UnloadTiles(unloadedEntities);
		if (topRoom != nullptr) topRoom->UnloadTiles(unloadedEntities);
	}

	void MapGenerator::CreateVisibleTiles(const Renderer* renderer, EntityManager& entityManager, s_Position& positionSystem, s_Sprites& spriteSystem, s_Collisions& collisionSystem, s_EntityData& entityDataSystem, s_Triggers& triggerSystem) {
		Vec2 cameraTopLeft, cameraBottomRight;
		renderer->GetCameraCorners(cameraTopLeft, cameraBottomRight);

		currentRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
		if (bottomRoom != nullptr) bottomRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
		if (rightRoom != nullptr) rightRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
		if (leftRoom != nullptr) leftRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
		if (topRoom != nullptr) topRoom->CreateVisibleTiles(renderer, entityManager, positionSystem, spriteSystem, collisionSystem, entityDataSystem, triggerSystem);
	}
}