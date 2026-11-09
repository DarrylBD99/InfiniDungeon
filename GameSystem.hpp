#pragma once

#include "GamesEngineeringBase.h"
#include "Dataset.hpp"
#include "ComponentSystem.hpp"
#include "MapGenerator.hpp"
#include "SaveSystem.hpp"

namespace WM908_Assessment {
	class GameSystem {
	private:
		GamesEngineeringBase::Window& window;

		EntityManager entity;

		s_Position positions;
		s_Movement movements;
		s_Directions directions;
		s_Collisions collisions;
		s_Sprites sprites;
		s_PlayerInput inputs;
		s_EntityData entityData;
		s_Triggers triggers;
		s_Attacks attacks;
		s_Spawner spawner;

		Renderer* renderer = nullptr;
		MapGenerator mapGenerator;

		Vec2 cameraBounds;

		bool reachedEndGoal = false;

		Dataset::Array<unsigned int> playerEntity = Dataset::Array<unsigned int>(2);
		Dataset::Array<bool> playerInteract = Dataset::Array<bool>(2);
		Dataset::Array<PlayerClass> playerClasses = Dataset::Array<PlayerClass>(2);

		const std::string PLAYER_DATA_TEXT = "Player %d\nHealth:";
		
		// purely for fps calculation
		unsigned int currentFPS = 0, frameCount = 0;
		float elapsed = 0.0f;

		const Vec2 startMapPos_1 = {3, 6};
		const Vec2 startMapPos_2 = {4, 6};

		float scoreSecondCounter = 0.0f;

		// Debug Stuff
		const float debugKeyBuffer = 0.5f;
		float debugKeyPressedCounter = debugKeyBuffer;
		bool showCollisionBoxes = false;

		unsigned int CreatePlayerEntityBase(PlayerClass playerClass, unsigned int health = 100, unsigned int mana = 50, unsigned int keyCount = 0);
		void CreatePlayer1(PlayerClass playerClass, Vec2 position, unsigned int health = 100, unsigned int mana = 50, unsigned int keyCount = 0);
		void CreatePlayer2(PlayerClass playerClass, Vec2 position, unsigned int health = 100, unsigned int mana = 50, unsigned int keyCount = 0);
		void CreateNewPlayerEntities(const Dataset::Array<PlayerClass> playerClasses, bool player2);
		void DisplayPlayerData();

		void UpdateCameraPosition();

		float GetFPS(int frameCount, float deltaTime);
		void SetFPS(float deltaTime);
		void DisplayFPS();

		void UnloadEntities(Dataset::Array<unsigned int>& entitiesToUnload);

		void LoadWorld(int posX = 0, int posY = 0);
		void LoadDemoWorld();

		GameSystem(GamesEngineeringBase::Window& window_, Renderer* renderer_);

	public:
		void Unload();
		void Update(float deltaTime);
		void Draw();

		bool IsGameOver();
		bool ReachedEndGoal() { return reachedEndGoal; }

		GameSystem(GamesEngineeringBase::Window& window_, Renderer* renderer_, Dataset::Array<PlayerClass> playerClasses, bool isDemoWorld = false, bool player2 = false);
		static GameSystem* LoadFromSave(GamesEngineeringBase::Window& window_, Renderer* renderer_);
	};
}