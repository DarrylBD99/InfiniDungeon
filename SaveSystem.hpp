#pragma once

#include "Dataset.hpp"
#include "Vector.hpp"
#include "Component.hpp"

#include <fstream>

namespace WM908_Assessment{
	
	struct sv_RoomData {
		unsigned int roomID = 0;
		bool visited = false;
		Dataset::Array<bool> triggerState{ 0 };
		Dataset::Array<unsigned int> deadTileIndex{};
	};

	struct sv_PlayerData {
		Vec2 position{};
		unsigned int health = 100;
		unsigned int mana = 50;
		unsigned int keyCount = 0;
		bool isDead = false;

		PlayerClass playerClass{};
	};
	
	class SaveSystem {
	private:
		int worldSeed;
		int mapPosX, mapPosY;

		Dataset::Array<sv_PlayerData> playerData;
		Dataset::Queue<sv_RoomData> mapData;
	public:
		int score = 0;

		bool isDemo = false;
		void SetCurrentMapPos(int mapPosX_, int mapPosY_) { mapPosX = mapPosX_; mapPosY = mapPosY_; }
		void GetCurrentMapPos(int& mapPosX_, int& mapPosY_) const { mapPosX_ = mapPosX; mapPosY_ = mapPosY; }

		int GetWorldSeed() { return worldSeed; }
		void SetWorldSeed(const int worldSeed_) { worldSeed = worldSeed_; }
		sv_PlayerData GetPlayerData(unsigned int index) const { return playerData.Get(index); }
		unsigned int GetPlayerCount() { return playerData.GetSize(); }

		sv_RoomData* GetRoomData(const unsigned int roomID);
		bool GetMapData(const unsigned int roomID, bool& visited, Dataset::Array<bool>& triggerState, uint8_t*& tileIDs);
		void CreateMapData(const unsigned int roomID, Dataset::Array<bool> triggerState);
		void AddDeadTile(const unsigned int roomID, const unsigned int index);
		void SetVisited(const unsigned int roomID);
		void ChangeTriggerState(const unsigned int roomID, const unsigned int triggerID, bool triggerState);
		void CreateSave(bool player2);
		void KillPlayerData(const unsigned int index);
		void SavePlayerData(const unsigned int index, const c_Position* position, const c_Health* health, const c_Mana* mana, const c_KeyCount* keyCount, const PlayerClass playerClass);
		void SaveFile(const char* saveDir);
		void LoadFile(const char* saveDir);
	};

	extern SaveSystem saveSystem;
}