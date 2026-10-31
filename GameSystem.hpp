#pragma once

#include "GamesEngineeringBase.h"
#include "Dataset.hpp"
#include "ComponentSystem.hpp"
#include "MapGenerator.hpp"

namespace WM908_Assessment {
	class GameSystem {
	private:
		EntityManager entity;

		s_Position positions;
		s_Movement movements;
		s_4Direction directions_4;
		s_Sprites sprites;
		s_Input inputs;

		Vec2 cameraBounds;

		Renderer* renderer = nullptr;
		MapGenerator* mapGenerator = nullptr;

		GamesEngineeringBase::Window& window;

		unsigned int* playerEntity = new unsigned int[2]();
		unsigned int numberOfPlayers = 1;

		const std::string PLAYER_DATA_TEXT = "Player %d\nHealth:";
		
		// purely for fps calculation
		int currentFPS, frameCount;
		float elapsed;

		Vec2 startMapPos_1 = {3, 6};
		Vec2 startMapPos_2 = {4, 6};
	public:
		unsigned int CreatePlayerEntityBase(PlayerCLass playerClass);
		void CreatePlayer1(PlayerCLass playerClass);
		void CreatePlayer2(PlayerCLass playerClass);

		void UpdateCameraPosition();

		float GetFPS(int frameCount, float deltaTime);
		void Update(float deltaTime);

		void UnloadEntities(unsigned int entitiesToUnload[], unsigned int unloadEntityCount);
		GameSystem(GamesEngineeringBase::Window& window_, bool hasPlayer2 = false);
	};
}