#include "GameSystem.hpp"

namespace WM908_Assessment {
	unsigned int GameSystem::CreatePlayerEntityBase(PlayerClass playerClass) {
		unsigned int entityID = entity.AppendEntity();
		directions_4.AddComponent(entityID);
		movements.AddComponent(entityID, 150.0f);

		collisions.AddCollisionBox(entityID, Vec2{ 20.0f, 32.0f }, Vec2{ 6.0f, 0.0f }, CollisionLayer::Player, CollisionLayer::Wall);
		entityData.AddHealth(entityID, 100);
		entityData.AddMana(entityID, 50);
		entityData.AddSavable(entityID);


		// Wanted to make it so that way the player can move while attacking at the same time
		// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
		// I'm keeping the code here just so you know it happened
		// 
		// Add Layered AnimatedSprite Component
		//c_LayeredAnimatedSprite* layeredAnimatedSprite = sprites.AddLayeredAnimatedSprite(entityID, 1);

		//// Create one layer for top and the other for legs
		//c_AnimatedSprite* bottomLayer = layeredAnimatedSprite->AddLayer();
		//c_AnimatedSprite* topLayer = layeredAnimatedSprite->AddLayer();

		//unsigned int baseFrame = 18;
		//switch (playerClass)
		//{
		//case PlayerClass::Miner: {
		//	baseFrame = 36;
		//	attacks.AddMelee(entityID, 19, 0.2f, CollisionLayer::Enemy);
		//	break;
		//}
		//case PlayerClass::Archer:
		//	baseFrame = 72;
		//	break;
		//case PlayerClass::Mage:
		//	baseFrame = 108;
		//	break;
		//default:
		//	break;
		//}

		//unsigned int legFrames[8][4] = {
		//	{baseFrame},      // IDLE Down
		//	{baseFrame + 9},  // IDLE Right
		//	{baseFrame + 18}, // IDLE Left
		//	{baseFrame + 27}, // IDLE Up
		//	{baseFrame + 1, baseFrame, baseFrame + 2, baseFrame},              // Walk Down
		//	{baseFrame + 10, baseFrame + 9, baseFrame + 11, baseFrame + 9},    // Walk Right
		//	{baseFrame + 19, baseFrame + 18, baseFrame + 20, baseFrame + 18},  // Walk Left
		//	{baseFrame + 28, baseFrame + 27, baseFrame + 29, baseFrame + 27}   // Walk Up
		//};
		//unsigned int topFrames[12][4] = {
		//	{baseFrame + 3},  // IDLE Down
		//	{baseFrame + 12}, // IDLE Right
		//	{baseFrame + 21}, // IDLE Left
		//	{baseFrame + 30}, // IDLE Up
		//	{baseFrame + 4, baseFrame + 3, baseFrame + 5, baseFrame + 3},     // Walk Down
		//	{baseFrame + 13, baseFrame + 12, baseFrame + 14, baseFrame + 12}, // Walk Right
		//	{baseFrame + 22, baseFrame + 21, baseFrame + 23, baseFrame + 21}, // Walk Left
		//	{baseFrame + 31, baseFrame + 30, baseFrame + 32, baseFrame + 30}, // Walk Up
		//	{baseFrame + 6, baseFrame + 7, baseFrame + 8},     // Attack Down
		//	{baseFrame + 14, baseFrame + 15, baseFrame + 16},  // Attack Right
		//	{baseFrame + 24, baseFrame + 25, baseFrame + 26},  // Attack Left
		//	{baseFrame + 33, baseFrame + 34, baseFrame + 35}   // Attack Up
		//};

		//bool isLoop = true;
		//bool canOverwrite = true;
		//int frameCount = 1;
		//for (unsigned int i = 0; i < 12; i++) {
		//	// Walking Animations
		//	if (i >= 4) {
		//		frameCount = 4;
		//	}

		//	// Attack Animations
		//	if (i >= 8) {
		//		frameCount = 3;
		//		isLoop = false;
		//		canOverwrite = false;
		//	}

		//	topLayer->AddAnimation(topFrames[i], frameCount, isLoop, canOverwrite);

		//	if (i >= 8) continue;
		//	bottomLayer->AddAnimation(legFrames[i], frameCount, isLoop, canOverwrite);
		//}

		c_AnimatedSprite* animatedSprite = sprites.AddAnimatedSprite(entityID, 1);

		unsigned int baseFrame = 18;
		switch (playerClass)
		{
		case PlayerClass::Miner: {
			baseFrame = 36;
			c_Melee* meleeAttack = attacks.AddMelee(entityID, 19, 0.2f, CollisionLayer::Enemy);
			meleeAttack->positionOffset = (float)Renderer::GRID_SIZE / 1.8f;
			break;
		}
		case PlayerClass::Archer: {
			baseFrame = 72;
			c_Projectile* projectile = attacks.AddProjectile(entityID, 21, 250.0f, false, CollisionLayer::Enemy | CollisionLayer::Wall);
			projectile->collisionBoxSize = { 16.0f };
			projectile->collisionBoxOffset = { 8.0f };

			break;
		}
		case PlayerClass::Mage: {
			baseFrame = 108;
			c_Projectile* projectile = attacks.AddProjectile(entityID, 22, 150.0f, true, CollisionLayer::Enemy);
			projectile->collisionBoxSize = { 14.0f };
			projectile->collisionBoxOffset = { 9.0f };
			projectile->rotatesToEntityDirection = false;
			projectile->followNearestEnemyRange = 120.0f;
			break;
		}
		default:
			break;
		}

		unsigned int animationFrames[12][4] = {
			{baseFrame + 3},  // IDLE Down
			{baseFrame + 12}, // IDLE Right
			{baseFrame + 21}, // IDLE Left
			{baseFrame + 30}, // IDLE Up
			{baseFrame + 4, baseFrame + 3, baseFrame + 5, baseFrame + 3},     // Walk Down
			{baseFrame + 13, baseFrame + 12, baseFrame + 14, baseFrame + 12}, // Walk Right
			{baseFrame + 22, baseFrame + 21, baseFrame + 23, baseFrame + 21}, // Walk Left
			{baseFrame + 31, baseFrame + 30, baseFrame + 32, baseFrame + 30}, // Walk Up
			{baseFrame + 6, baseFrame + 7, baseFrame + 8},     // Attack Down
			{baseFrame + 15, baseFrame + 16, baseFrame + 17},  // Attack Right
			{baseFrame + 24, baseFrame + 25, baseFrame + 26},  // Attack Left
			{baseFrame + 33, baseFrame + 34, baseFrame + 35}   // Attack Up
		};

		bool isLoop = true;
		bool canOverwrite = true;
		int frameCount = 1;
		for (unsigned int i = 0; i < 12; i++) {
			// Walking Animations
			if (i >= 4) {
				frameCount = 4;
			}

			// Attack Animations
			if (i >= 8) {
				frameCount = 3;
				isLoop = false;
				canOverwrite = false;
			}

			animatedSprite->AddAnimation(animationFrames[i], frameCount, isLoop, canOverwrite);
		}
		return entityID;
	}

	void GameSystem::CreatePlayer1(PlayerClass playerClass) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass);
		c_Position* position = positions.AddComponent(entityID, startMapPos_1 * renderer->GRID_SIZE);

		c_PlayerInput* input = inputs.AddComponent(entityID);
		input->keyDown = 'S';
		input->keyRight = 'D';
		input->keyLeft = 'A';
		input->keyUp = 'W';

		input->primaryAttack = 'Q';
		input->secondaryAttack = 'E';

		playerEntity[0] = entityID;
	}

	void GameSystem::CreatePlayer2(PlayerClass playerClass) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass);
		c_Position* position = positions.AddComponent(entityID, startMapPos_2 * renderer->GRID_SIZE);

		c_PlayerInput* input = inputs.AddComponent(entityID);
		input->keyDown = 'K';
		input->keyRight = 'L';
		input->keyLeft = 'J';
		input->keyUp = 'I';

		input->primaryAttack = 'U';
		input->secondaryAttack = 'O';

		playerEntity[1] = entityID;
	}

	void GameSystem::CreateNewPlayerEntities(const PlayerClass playerClass[]) {
		CreatePlayer1(playerClass[0]);

		if (numberOfPlayers > 1) {
			CreatePlayer2(playerClass[1]);
		}
	}

	// Display Player(s) Data
	void GameSystem::DisplayPlayerData() {
		std::string stringOutput;

		for (unsigned int i = 0; i < numberOfPlayers; i++) {
			if (i % numberOfPlayers > 0) stringOutput += "\n\n";

			unsigned int entityID = playerEntity[i];
			c_Health* health = entityData.GetHealth(entityID);
			c_Mana* mana = entityData.GetMana(entityID);

			stringOutput += "Player " + std::to_string(i + 1)
						  + "\nHealth:" + std::to_string(health->current)
						  + "/" + std::to_string(health->max)
						  + "\nMana:" + std::to_string(mana->current)
						  + "/" + std::to_string(mana->max);
		}

		renderer->DrawString({ cameraBounds.GetX(), 12.0f }, (unsigned char*)stringOutput.c_str());
	}

	void GameSystem::UpdateCameraPosition() {
		Vec2 cameraWorldPos = { 0, 0 };
		
		for (unsigned int i = 0; i < numberOfPlayers; i++) {
			unsigned int entityID = playerEntity[i];

			c_Position* position = positions.GetComponent(entityID);
			
			if (i == 0) {
				cameraWorldPos = position->position + Vec2{ (float)Renderer::GRID_SIZE / 2 };
			} else {
				Vec2 offsetFromCamera = (position->position + Vec2{ (float)Renderer::GRID_SIZE / 2 }) - cameraWorldPos;
				cameraWorldPos += offsetFromCamera / 2;
			}
		}

		renderer->SetCameraWorldPosition(cameraWorldPos);
	}

	void GameSystem::UnloadEntities(unsigned int entitiesToUnload[], unsigned int unloadEntityCount) {
		entity.RemoveEntities(entitiesToUnload, unloadEntityCount);
		positions.DeleteComponents(entitiesToUnload, unloadEntityCount);
		movements.DeleteComponents(entitiesToUnload, unloadEntityCount);
		directions_4.DeleteComponents(entitiesToUnload, unloadEntityCount);
		collisions.DeleteCollisionComponents(entitiesToUnload, unloadEntityCount);
		sprites.DeleteSpriteComponents(entitiesToUnload, unloadEntityCount);
		inputs.DeleteComponents(entitiesToUnload, unloadEntityCount);
		
		entityData.DeleteDataComponents(entitiesToUnload, unloadEntityCount);
		triggers.DeleteTriggerComponents(entitiesToUnload, unloadEntityCount);
		attacks.DeleteAttackComponents(entitiesToUnload, unloadEntityCount);

		delete[] entitiesToUnload;
	}

	float GameSystem::GetFPS(int frameCount, float deltaTime) {
		if (deltaTime == 0) return 0; // Prevent zero-division
		return (float)frameCount / deltaTime;
	}

	void GameSystem::SetFPS(float deltaTime) {
		frameCount++;
		elapsed += deltaTime;
		if (elapsed >= 0.1f) {
			currentFPS = (int)GetFPS(frameCount, elapsed);
			frameCount = 0;
			elapsed = 0.0f;
		}
	}

	void GameSystem::DisplayFPS() {
		std::string fpsString = "FPS: " + std::to_string(currentFPS);
		renderer->DrawString({ cameraBounds.GetX(), cameraBounds.GetY() - 32 }, (unsigned char*)fpsString.c_str());
	}

	// Unload Entities
	void GameSystem::Unload() {
		unsigned int datasetSize = entity.GetDatasetSize();
		unsigned int* entitiesToUnload = new unsigned int[datasetSize];
		unsigned int unloadEntityCount = 0;
		positions.UnloadEntities(datasetSize, playerEntity, entitiesToUnload, unloadEntityCount, renderer);
		mapGenerator->UnloadTiles(entitiesToUnload, unloadEntityCount);
		
		attacks.DeleteAttackInstance(datasetSize, entitiesToUnload, unloadEntityCount, positions, movements, collisions);
		entityData.UnloadEntities(datasetSize, entitiesToUnload, unloadEntityCount);

		UnloadEntities(entitiesToUnload, unloadEntityCount);
	}

	void GameSystem::Update(float deltaTime) {
		// Debug features
		if (debugKeyPressedCounter >= debugKeyBuffer) {
			// F9: Show Collision Boxes
			if (window.keyPressed(VK_F9)) {
				showCollisionBoxes = !showCollisionBoxes;
				debugKeyPressedCounter = 0.0f;
			}
		} else {
			debugKeyPressedCounter += deltaTime;
		}

		mapGenerator->CreateVisibleTiles(renderer, entity, positions, sprites, collisions, entityData, triggers);

		unsigned int datasetSize = entity.GetDatasetSize();

		for (unsigned int entityID = 0; entityID < entity.GetDatasetSize(); entityID++) {
			inputs.Update(entityID, datasetSize, window, directions_4, entity, positions, movements, entityData, sprites, collisions, attacks);
			movements.Update(entityID, datasetSize, deltaTime, positions, collisions);
			entityData.Update(entityID, datasetSize, deltaTime, playerEntity, numberOfPlayers, positions, movements, collisions);

			directions_4.Update(entityID, movements, sprites);
			sprites.Update(entityID, deltaTime, renderer);
			attacks.Update(entityID, deltaTime);
		}
		
		UpdateCameraPosition();

		SetFPS(deltaTime);
	}

	void GameSystem::Draw() {
		for (unsigned int drawLayer = 0; drawLayer < 4; drawLayer++) {
			for (unsigned int entityID = 0; entityID < entity.GetDatasetSize(); entityID++) {
				sprites.Draw(entityID, drawLayer, positions.GetComponent(entityID), renderer);

				if (drawLayer == 3 && showCollisionBoxes) {
					collisions.DrawCollisionBox(entityID, positions, renderer, window);
				}
			}
		}

		renderer->DrawCameraBackground(33, 49, 18);
		DisplayFPS();
		DisplayPlayerData();
	}

	void GameSystem::LoadDemoWorld(const PlayerClass playerClass[]) {
		CreateNewPlayerEntities(playerClass);
		mapGenerator->GenerateDemoRoom();
	}

	GameSystem::GameSystem(GamesEngineeringBase::Window& window_, bool hasPlayer2) : window(window_) {
		cameraBounds = renderer->GetCameraBounds();
		if (hasPlayer2) numberOfPlayers = 2;

		// Start new game (Temporary)
		PlayerClass playerClasses[2] = { PlayerClass::Miner, PlayerClass::Mage };

		LoadDemoWorld(playerClasses);
	}
}