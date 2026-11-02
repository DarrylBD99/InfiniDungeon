#pragma once

#include "GamesEngineeringBase.h"
#include "Dataset.hpp"
#include "ComponentSystem.hpp"
#include "MapGenerator.hpp"

namespace WM908_Assessment {
	class GameSystem {
	private:
		GamesEngineeringBase::Window& window;

		EntityManager entity;

		s_Position positions;
		s_Movement movements;
		s_4Direction directions_4;
		s_Collisions collisions;
		s_Sprites sprites;
		s_PlayerInput inputs;
		s_EntityData entityData;
		s_Triggers triggers;
		s_Attacks attacks;

		Renderer* renderer = new Renderer(window);
		MapGenerator* mapGenerator = new MapGenerator();

		Vec2 cameraBounds;

		unsigned int* playerEntity = new unsigned int[2];
		unsigned int numberOfPlayers = 1;

		const std::string PLAYER_DATA_TEXT = "Player %d\nHealth:";
		
		// purely for fps calculation
		unsigned int currentFPS = 0, frameCount = 0;
		float elapsed = 0.0f;

		Vec2 startMapPos_1 = {3, 6};
		Vec2 startMapPos_2 = {4, 6};


		// Debug Stuff
		const float debugKeyBuffer = 0.5f;
		float debugKeyPressedCounter = debugKeyBuffer;
		bool showCollisionBoxes = false;

		unsigned int CreatePlayerEntityBase(PlayerClass playerClass);
		void CreatePlayer1(PlayerClass playerClass);
		void CreatePlayer2(PlayerClass playerClass);
		void CreateNewPlayerEntities(const PlayerClass playerClass[2]);
		void DisplayPlayerData();

		void UpdateCameraPosition();

		float GetFPS(int frameCount, float deltaTime);
		void SetFPS(float deltaTime);
		void DisplayFPS();

		void UnloadEntities(unsigned int entitiesToUnload[], unsigned int unloadEntityCount);

		void LoadDemoWorld(const PlayerClass playerClass[2]);

	public:
		void Unload();
		void Update(float deltaTime);
		void Draw();

		GameSystem(GamesEngineeringBase::Window& window_, bool hasPlayer2 = false);
	};
}