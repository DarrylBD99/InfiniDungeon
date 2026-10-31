#include "GameSystem.hpp"

namespace WM908_Assessment {
	unsigned int GameSystem::CreatePlayerEntityBase(PlayerCLass playerClass) {
		unsigned int entityID = entity.AppendEntity();
		directions_4.AddComponent(entityID);

		c_Movement* movement = movements.AddComponent(entityID);
		movement->speed = 150.0f;

		collisions.AddCollisionBox(entityID, Vec2{ (float)Renderer::GRID_SIZE }, false);

		// Add Layered AnimatedSprite Component
		c_LayeredAnimatedSprite* layeredAnimatedSprite = sprites.AddLayeredAnimatedSprite(entityID, 1);

		// Create one layer for top and the other for legs
		c_AnimatedSprite* bottomLayer = layeredAnimatedSprite->AddLayer();
		c_AnimatedSprite* topLayer = layeredAnimatedSprite->AddLayer();

		unsigned int baseFrame = 18;
		switch (playerClass)
		{
		case PlayerCLass::Miner:
			baseFrame = 36;
			break;
		case PlayerCLass::Archer:
			baseFrame = 72;
			break;
		case PlayerCLass::Mage:
			baseFrame = 108;
			break;
		default:
			break;
		}

		unsigned int legFrames[8][4] = {
			{baseFrame},      // IDLE Down
			{baseFrame + 9},  // IDLE Right
			{baseFrame + 18}, // IDLE Left
			{baseFrame + 27}, // IDLE Up
			{baseFrame + 1, baseFrame, baseFrame + 2, baseFrame},              // Walk Down
			{baseFrame + 10, baseFrame + 9, baseFrame + 11, baseFrame + 9},    // Walk Right
			{baseFrame + 19, baseFrame + 18, baseFrame + 20, baseFrame + 18},  // Walk Left
			{baseFrame + 28, baseFrame + 27, baseFrame + 29, baseFrame + 27}   // Walk Up
		};
		unsigned int topFrames[12][4] = {
			{baseFrame + 3},  // IDLE Down
			{baseFrame + 12}, // IDLE Right
			{baseFrame + 21}, // IDLE Left
			{baseFrame + 30}, // IDLE Up
			{baseFrame + 4, baseFrame + 3, baseFrame + 5, baseFrame + 3},     // Walk Down
			{baseFrame + 13, baseFrame + 12, baseFrame + 14, baseFrame + 12}, // Walk Right
			{baseFrame + 22, baseFrame + 21, baseFrame + 23, baseFrame + 21}, // Walk Left
			{baseFrame + 31, baseFrame + 30, baseFrame + 32, baseFrame + 30}, // Walk Up
			{baseFrame + 6, baseFrame + 7, baseFrame + 8},     // Attack Down
			{baseFrame + 14, baseFrame + 15, baseFrame + 16},  // Attack Right
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

			topLayer->AddAnimation(topFrames[i], frameCount, isLoop, canOverwrite);

			if (i >= 8) continue;
			bottomLayer->AddAnimation(legFrames[i], frameCount, isLoop, canOverwrite);
		}
		return entityID;
	}

	void GameSystem::CreatePlayer1(PlayerCLass playerClass) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass);
		c_Position* position = positions.AddComponent(entityID);
		position->position = startMapPos_1 * renderer->GRID_SIZE;

		c_Input* input = inputs.AddComponent(entityID);
		input->keyDown = 'S';
		input->keyRight = 'D';
		input->keyLeft = 'A';
		input->keyUp = 'W';

		input->primaryAttack = 'Q';
		input->secondaryAttack = 'E';

		playerEntity[0] = entityID;
	}

	void GameSystem::CreatePlayer2(PlayerCLass playerClass) {
		const unsigned int entityID = CreatePlayerEntityBase(playerClass);
		c_Position* position = positions.AddComponent(entityID);
		position->position = startMapPos_2 * renderer->GRID_SIZE;

		c_Input* input = inputs.AddComponent(entityID);
		input->keyDown = 'K';
		input->keyRight = 'L';
		input->keyLeft = 'J';
		input->keyUp = 'I';

		input->primaryAttack = 'U';
		input->secondaryAttack = 'O';

		playerEntity[1] = entityID;
	}

	float GameSystem::GetFPS(int frameCount, float deltaTime) {
		if (deltaTime == 0) return 0; // Prevent zero-division
		return (float)frameCount / deltaTime;
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
		sprites.DeleteSpriteComponents(entitiesToUnload, unloadEntityCount);
		inputs.DeleteComponents(entitiesToUnload, unloadEntityCount);
		collisions.DeleteCollisionComponents(entitiesToUnload, unloadEntityCount);

		delete[] entitiesToUnload;
	}

	void GameSystem::Update(float deltaTime) {

		unsigned int* entitiesToUnload = new unsigned int[entity.GetDatasetSize()];
		unsigned int unloadEntityCount = 0;
		mapGenerator->AddTilesToUnload(renderer, positions, entitiesToUnload, unloadEntityCount);
		UnloadEntities(entitiesToUnload, unloadEntityCount);

		mapGenerator->CreateVisibleTiles(renderer, entity, positions, sprites, collisions);

		unsigned int datasetSize = entity.GetDatasetSize();

		renderer->DrawCameraBackground(33, 49, 18);

		directions_4.Update(datasetSize, movements, sprites);


		sprites.Update(datasetSize, deltaTime, renderer);

		inputs.Update(datasetSize, window, movements);
		movements.Update(datasetSize, deltaTime, positions, collisions);

		sprites.Draw(datasetSize, positions, renderer);
		collisions.DrawCollisionBox(datasetSize, positions, renderer, window);

		UpdateCameraPosition();
		
		// Get and Display FPS
		frameCount++;
		elapsed += deltaTime;
		if (elapsed >= 0.1f) {
			currentFPS = roundf(GetFPS(frameCount, elapsed));
			frameCount = 0;
			elapsed = 0.0f;
		}

		std::string fpsString = "FPS: " + std::to_string(currentFPS);
		renderer->DrawString({ cameraBounds.GetX(), cameraBounds.GetY() - 32 }, (unsigned char*)fpsString.c_str());
		
		// Display Player(s) Data
		renderer->DrawString({ cameraBounds.GetX(), 12.0f }, (unsigned char*)"Player 1\nHealth:50/50\nMana:50/50\n\nPlayer 2\nHealth:50/50\nMana:50/50");
	}

	GameSystem::GameSystem(GamesEngineeringBase::Window& window_, bool hasPlayer2) : window(window_) {
		renderer = new Renderer(window);
		cameraBounds = renderer->GetCameraBounds();

		mapGenerator = new MapGenerator();

		// Temporary (Initialize Player Class)
		CreatePlayer1(PlayerCLass::Miner);

		if (hasPlayer2) {
			CreatePlayer2(PlayerCLass::Archer);
			numberOfPlayers++;
		}

		// Temporary (Create Basic Map)
		mapGenerator->GenerateDemoRoom();
	}
}