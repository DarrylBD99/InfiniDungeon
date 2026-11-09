#include "GameSystem.hpp"
#include "SoundManager.hpp"

#include <string>

namespace WM908_Assessment {
	unsigned int GameSystem::CreatePlayerEntityBase(PlayerClass playerClass, unsigned int health_, unsigned int mana_, unsigned int keyCount_) {
		unsigned int entityID = entity.AppendEntity();
		directions.Add4Direction(entityID);
		movements.AddComponent(entityID, 150.0f);

		collisions.AddCollisionBox(entityID, Vec2{ 20.0f, 26.0f }, Vec2{ 6.0f, 4.0f }, CollisionLayer::Player, CollisionLayer::Wall | CollisionLayer::Interactable);
		entityData.AddNoOutsideCamera(entityID);

		c_Health* health = entityData.AddHealth(entityID, 100, health_);
		c_Mana* mana = entityData.AddMana(entityID, 50, mana_);
		c_KeyCount* keyCount = entityData.AddKeyCount(entityID);
		keyCount->keyCount = keyCount_;

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
			meleeAttack->damageCount = 3;
			attacks.AddImmunity(entityID, 15, 10);
			break;
		}
		case PlayerClass::Archer: {
			baseFrame = 72;
			c_Projectile* projectile = attacks.AddProjectile(entityID, 21, 250.0f, false, CollisionLayer::Enemy | CollisionLayer::Wall);
			projectile->collisionBoxSize = { 16.0f };
			projectile->collisionBoxOffset = { 8.0f };
			c_Dash* dash = attacks.AddDash(entityID, 300.0f, 0.5f, 1);
			dash->oldSpeed = 150.0f;
			break;
		}
		case PlayerClass::Mage: {
			baseFrame = 108;
			c_Projectile* projectile = attacks.AddProjectile(entityID, 22, 150.0f, true, CollisionLayer::Enemy);
			projectile->collisionBoxSize = { 14.0f };
			projectile->collisionBoxOffset = { 9.0f };
			projectile->rotatesToEntityDirection = false;
			projectile->followNearestEnemyRange = 120.0f;
			projectile->manaConsume = 2;
			projectile->damageCount = 2;
			//mana->current = 0; // temporary test
			mana->passiveRegenerationRate = 0.5f;
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

	void GameSystem::CreatePlayer1(PlayerClass playerClass, Vec2 position_, unsigned int health, unsigned int mana, unsigned int keyCount) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass, health, mana, keyCount);
		c_Position* position = positions.AddComponent(entityID, position_);

		c_PlayerInput* input = inputs.AddComponent(entityID);
		input->keyDown = 'S';
		input->keyRight = 'D';
		input->keyLeft = 'A';
		input->keyUp = 'W';

		input->primaryAttack = 'Q';
		input->secondaryAttack = 'E';
		input->interact = 'R';

		playerClasses.SetValue(playerClass, 0);
		playerEntity.SetValue(entityID + 1, 0);
	}

	void GameSystem::CreatePlayer2(PlayerClass playerClass, Vec2 position_, unsigned int health, unsigned int mana, unsigned int keyCount) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass, health, mana, keyCount);
		c_Position* position = positions.AddComponent(entityID, position_);

		c_PlayerInput* input = inputs.AddComponent(entityID);
		input->keyDown = 'K';
		input->keyRight = 'L';
		input->keyLeft = 'J';
		input->keyUp = 'I';

		input->primaryAttack = 'U';
		input->secondaryAttack = 'O';
		input->interact = 'P';

		playerClasses.SetValue(playerClass, 1);
		playerEntity.SetValue(entityID + 1, 1);
	}

	void GameSystem::CreateNewPlayerEntities(const Dataset::Array<PlayerClass> playerClasses, bool player2) {
		CreatePlayer1(playerClasses.Get(0), startMapPos_1 * renderer->GRID_SIZE);

		if (player2) {
			CreatePlayer2(playerClasses.Get(1), startMapPos_2 * renderer->GRID_SIZE);
		}
	}

	// Display Player(s) Data
	void GameSystem::DisplayPlayerData() {
		std::string stringOutput;

		for (unsigned int i = 0; i < playerEntity.GetSize(); i++) {
			if (i != 0) stringOutput += "\n\n";

			unsigned int entityID = playerEntity.Get(i);
			if (entityID == 0) {
				stringOutput += "Player " + std::to_string(i + 1) + "\nDead\n\n";
				continue;
			}

			entityID = entityID - 1;

			c_Health* health = entityData.GetHealth(entityID);
			c_Mana* mana = entityData.GetMana(entityID);
			c_KeyCount* keyCount = entityData.GetKeyCount(entityID);
			c_Position* position = positions.GetComponent(entityID);

			saveSystem.SavePlayerData(i, position, health, mana, keyCount, playerClasses.Get(i));

			stringOutput += "Player " + std::to_string(i + 1)
						  + "\nHealth:" + std::to_string(health->current)
						  + "/" + std::to_string(health->max)
						  + "\nMana:" + std::to_string(mana->current)
						  + "/" + std::to_string(mana->max)
						  + "\nKeys:" + std::to_string(keyCount->keyCount);
		}

		renderer->DrawString({ cameraBounds.GetX(), 12.0f }, stringOutput.c_str());
	}

	void GameSystem::UpdateCameraPosition() {
		Vec2 cameraWorldPos = renderer->GetCameraWorldPosition();
		unsigned int counter = 0;
		for (unsigned int entityID : playerEntity) {
			if (entityID == 0) continue;
			entityID -= 1;

			c_Position* position = positions.GetComponent(entityID);
			
			if (counter == 0) {
				cameraWorldPos = position->position + Vec2{ (float)Renderer::GRID_SIZE / 2 };
			} else {
				Vec2 offsetFromCamera = (position->position + Vec2{ (float)Renderer::GRID_SIZE / 2 }) - cameraWorldPos;
				cameraWorldPos += offsetFromCamera / 2;
			}
			counter++;
		}

		renderer->SetCameraWorldPosition(cameraWorldPos);
	}

	void GameSystem::UnloadEntities(Dataset::Array<unsigned int>& entitiesToUnload) {
		for (unsigned int i = 0; i < playerEntity.GetSize(); i++) {
			if (playerEntity.Get(i) == 0) continue;
			if (!entitiesToUnload.HasData(playerEntity.Get(i) - 1)) continue;
			saveSystem.KillPlayerData(i);
			playerEntity.SetValue(0, i);
		}

		entity.RemoveEntities(entitiesToUnload);
		positions.DeleteComponents(entitiesToUnload);
		movements.DeleteComponents(entitiesToUnload);
		directions.DeleteDirectionComponents(entitiesToUnload);
		collisions.DeleteCollisionComponents(entitiesToUnload);
		sprites.DeleteSpriteComponents(entitiesToUnload);
		inputs.DeleteComponents(entitiesToUnload);
		
		entityData.DeleteDataComponents(entitiesToUnload);
		triggers.DeleteTriggerComponents(entitiesToUnload);
		attacks.DeleteAttackComponents(entitiesToUnload);
		spawner.DeleteComponents(entitiesToUnload);
	}

	float GameSystem::GetFPS(int frameCount, float deltaTime) {
		if (deltaTime == 0) return 0; // Prevent zero-division
		return (float)frameCount / deltaTime;
	}

	void GameSystem::SetFPS(float deltaTime) {
		frameCount++;
		elapsed += deltaTime;
		if (elapsed >= 0.5f) {
			currentFPS = (int)GetFPS(frameCount, elapsed);
			frameCount = 0;
			elapsed -= 0.5f;
		}
	}

	void GameSystem::DisplayFPS() {
		char fpsString[16];
		snprintf(fpsString, sizeof(fpsString), "FPS: %d", currentFPS);

		renderer->DrawString({ cameraBounds.GetX(), cameraBounds.GetY() - 32 }, fpsString);
	}

	// Unload Entities
	void GameSystem::Unload() {
		unsigned int datasetSize = entity.GetDatasetSize();
		Dataset::Array<unsigned int> entitiesToUnload(datasetSize);

		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			if (!playerEntity.HasData(entityID + 1)) positions.UnloadEntities(entityID, entitiesToUnload, renderer);
			
			attacks.UnloadDamageBox(entityID, entitiesToUnload);
			entityData.UnloadEntities(entityID, entitiesToUnload);
			triggers.UnloadKillTrigger(entityID, entitiesToUnload);

			if (entityData.SaveDeathState(entityID, entitiesToUnload, triggers)) {
				c_DeathSavable* deathSavable = entityData.GetDeathSavable(entityID);
				Room* room = mapGenerator.GetRoomWithID(deathSavable->roomID);
				if (room != nullptr) room->SetTile(deathSavable->tileIndex, 0);
			}
		}

		mapGenerator.UnloadTiles(&entitiesToUnload);
		UnloadEntities(entitiesToUnload);
	}

	void GameSystem::Update(float deltaTime) {
		scoreSecondCounter += deltaTime;
		while (scoreSecondCounter >= 1.0f) {
			saveSystem.score++;
			scoreSecondCounter -= 1.0f;
		}

		// Debug features
		if (debugKeyPressedCounter >= debugKeyBuffer) {
			// F9: Show Collision Boxes
			if (window.keyPressed('9')) {
				showCollisionBoxes = !showCollisionBoxes;
				debugKeyPressedCounter = 0.0f;
			}
		} else {
			debugKeyPressedCounter += deltaTime;
		}


		mapGenerator.CreateVisibleTiles(renderer, entity, positions, sprites, collisions, entityData, triggers, spawner, attacks);

		unsigned int datasetSize = entity.GetDatasetSize();
		entityData.UpdateFollowCounter(deltaTime);

		for (unsigned int i = 0; i < playerEntity.GetSize(); i++) {
			if (playerEntity.Get(i) == 0) continue;

			unsigned int entityID = playerEntity.Get(i) - 1;
			unsigned int interactEntityID = 0;
			bool canInteract = inputs.CanInteract(entityID, datasetSize, positions, entityData, triggers, collisions, interactEntityID);
			playerInteract.SetValue(canInteract, i);
			
			inputs.Update(entityID, window, directions, entity, positions, movements, entityData, sprites, collisions, triggers, attacks, canInteract, interactEntityID);
		}

		for (unsigned int entityID = 0; entityID < entity.GetDatasetSize(); entityID++) {
			Dataset::Array<unsigned int> collidingEntities = collisions.GetCollidingEntities(entity.GetDatasetSize(), entityID, positions, movements.GetComponent(entityID));

			entityData.UpdateHealth(entityID, deltaTime);
			entityData.UpdateMana(entityID, deltaTime);
			entityData.UpdateCollectible(entityID, collidingEntities);
			entityData.UpdateFollow(entityID, entity.GetDatasetSize(), positions, movements, collisions);
			entityData.CheckOutsideCamera(entityID, renderer, positions, movements, collisions);

			directions.Update(entityID, movements, sprites);
			sprites.Update(entityID, deltaTime, renderer);

			attacks.UpdateAttackProximity(entityID, deltaTime, entity, positions, collisions, movements, entityData, sprites);
			attacks.UpdateDamageBox(entityID, deltaTime, collidingEntities, entityData, triggers);
			attacks.UpdateDash(entityID, deltaTime, movements);
			attacks.UpdateAutoProjectiles(entityID, deltaTime, entity, positions, movements, entityData, sprites, collisions, triggers);

			spawner.Update(entityID, deltaTime, entity, positions, movements, sprites, directions, collisions, attacks, entityData);
			movements.Update(entityID, deltaTime, collidingEntities, positions, collisions);

			if (playerEntity.HasData(entityID + 1)) {
				c_Movement* movement = movements.GetComponent(entityID);
				if (movement != nullptr && movement->velocity.Magnitude() > 0)
					soundManager.PlayPlayerWalkSFX(deltaTime);
			}

			if (!reachedEndGoal) reachedEndGoal = triggers.IsEndGoalReached(entityID);

			if (triggers.UpdatePressTrigger(entityID, collidingEntities)) {
				bool triggerState = triggers.TriggerAction(entityID);
				c_PressTrigger* pressTrigger = triggers.GetPressTrigger(entityID);
				Room* room = mapGenerator.GetRoomWithID(pressTrigger->roomID);
				if (room != nullptr) room->SetTrigger(pressTrigger->triggerID, triggerState, triggers);
			}
		}

		entityData.ResetFollowCounter();
		mapGenerator.Update(renderer->GetCameraWorldPosition());
		UpdateCameraPosition();
		SetFPS(deltaTime);
	}

	void GameSystem::Draw() {

		for (unsigned int drawLayer = 0; drawLayer < 3; drawLayer++) {
			for (unsigned int entityID = 0; entityID < entity.GetDatasetSize(); entityID++) {
				sprites.Draw(entityID, drawLayer, positions.GetComponent(entityID), triggers.GetActiveState(entityID), renderer);

				if (drawLayer < 2) continue;
				attacks.DrawImmunityVFX(entityID, positions, renderer);

				if (!showCollisionBoxes) continue;
				collisions.DrawCollisionBox(entityID, positions, renderer, window);
			}
		}

		for (unsigned int i = 0; i < playerEntity.GetSize(); i++) {
			if (playerEntity.Get(i) == 0) continue;
			if (!playerInteract.Get(i)) continue;

			unsigned int entityID = playerEntity.Get(i) - 1;
			inputs.DrawInteractIndicator(entityID, renderer, positions);
		}

		renderer->DrawCameraBackground(1);
		DisplayFPS();
		DisplayPlayerData();
	}

	void GameSystem::LoadWorld(int posX, int posY) {
		mapGenerator.StartInfiniteWorldGeneration(posX, posY);
	}

	void GameSystem::LoadDemoWorld() {
		mapGenerator.GenerateDemoRoom();
	}
	
	GameSystem::GameSystem(GamesEngineeringBase::Window& window_, Renderer* renderer_) : window(window_), renderer(renderer_) {
		cameraBounds = renderer->GetCameraBounds();
	}

	GameSystem::GameSystem(GamesEngineeringBase::Window& window_, Renderer* renderer_, Dataset::Array<PlayerClass> playerClasses, bool isDemoWorld, bool player2) : GameSystem(window_, renderer_) {
		CreateNewPlayerEntities(playerClasses, player2);

		// Start new game (Temporary)
		//PlayerClass playerClasses[2] = { PlayerClass::Miner, PlayerClass::Mage };
		//LoadWorld(playerClasses, player2);

		if (isDemoWorld) LoadDemoWorld();
		else LoadWorld();
		saveSystem.SetWorldSeed(mapGenerator.GetWorldSeed());

		
		// Temp Enemy Spawn Test
		//spawner.SpawnEnemy({ 64, 64 }, entity, positions, movements, sprites, directions, collisions, attacks, entityData, EnemyTypes::Slime);
	}

	bool GameSystem::IsGameOver() {
		for (unsigned int playerEntityID : playerEntity) {
			if (playerEntityID != 0)
				return false;
		}
		return true;
	}

	GameSystem* GameSystem::LoadFromSave(GamesEngineeringBase::Window& window_, Renderer* renderer_) {
		GameSystem* instance = new GameSystem(window_, renderer_);

		for (unsigned int i = 0; i < saveSystem.GetPlayerCount(); i++) {
			sv_PlayerData playerData = saveSystem.GetPlayerData(i);
			if (playerData.isDead) {
				instance->playerEntity.SetValue(0, i);
				continue;
			}
			if (i == 0) instance->CreatePlayer1(playerData.playerClass, playerData.position, playerData.health, playerData.mana, playerData.keyCount);
			if (i == 1) instance->CreatePlayer2(playerData.playerClass, playerData.position, playerData.health, playerData.mana, playerData.keyCount);
		}

		instance->mapGenerator = MapGenerator(saveSystem.GetWorldSeed());
		
		int posX, posY;
		saveSystem.GetCurrentMapPos(posX, posY);

		if (saveSystem.isDemo) instance->LoadDemoWorld();
		else instance->LoadWorld(posX, posY);

		return instance;
	}
}