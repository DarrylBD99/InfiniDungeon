#include "SaveSystem.hpp"
#include <iomanip>

namespace WM908_Assessment {
	SaveSystem saveSystem;

	sv_RoomData* SaveSystem::GetRoomData(const unsigned int roomID) {
		Dataset::Node<sv_RoomData>* node = mapData.GetHead();
		if (node == nullptr) return nullptr;

		while (node != nullptr) {
			sv_RoomData& data = node->data;
			if (data.roomID == roomID) {
				return &data;
			}

			node = node->next;
		}

		return nullptr;
	}

	bool SaveSystem::GetMapData(const unsigned int roomID, bool& visited, Dataset::Array<bool>& triggerState, uint8_t*& tileIDs) {
		sv_RoomData* data = GetRoomData(roomID);
		if (data == nullptr) return false;

		visited = data->visited;
		triggerState = data->triggerState;
		for (unsigned int deadTileIndex : data->deadTileIndex)
			tileIDs[deadTileIndex] = 0;

		return true;
	}

	void SaveSystem::CreateMapData(const unsigned int roomID, Dataset::Array<bool> triggerState) {
		sv_RoomData data{};
		data.roomID = roomID;
		data.triggerState = triggerState;

		mapData.Enqueue(data);
	}

	void SaveSystem::AddDeadTile(const unsigned int roomID, const unsigned int index) {
		sv_RoomData* data = GetRoomData(roomID);
		if (data == nullptr) return;

		data->deadTileIndex.SetValue(index, data->deadTileIndex.GetSize());
	}

	void SaveSystem::SetVisited(const unsigned int roomID) {
		sv_RoomData* data = GetRoomData(roomID);
		if (data == nullptr) return;

		data->visited = true;
	}

	void SaveSystem::ChangeTriggerState(const unsigned int roomID, const unsigned int triggerID, bool triggerState) {
		sv_RoomData* data = GetRoomData(roomID);
		if (data == nullptr) return;

		data->triggerState.SetValue(triggerState, triggerID);
	}

	void SaveSystem::CreateSave(bool player2) {
		unsigned int playerCount = player2 ? 2 : 1;
		playerData = Dataset::Array<sv_PlayerData>(playerCount);
	}

	void SaveSystem::KillPlayerData(const unsigned int index) {
		if (index >= playerData.GetMaxSize()) return;

		sv_PlayerData data{};
		data.isDead = true;
		playerData.SetValue(data, index);
	}

	void SaveSystem::SavePlayerData(const unsigned int index, const c_Position* positon, const c_Health* health, const c_Mana* mana, const c_KeyCount* keyCount, const PlayerClass playerClass) {
		sv_PlayerData data{};

		data.health = health->current;
		data.mana = mana->current;
		data.position = positon->position;
		data.keyCount = keyCount->keyCount;
		data.playerClass = playerClass;

		playerData.SetValue(data, index);
	}

	void SaveSystem::SaveFile(const char* saveDir) {
		std::ofstream saveFile(saveDir);
		saveFile << score << std::endl;
		saveFile << playerData.GetSize() << std::endl;
		for (sv_PlayerData data : playerData) {
			saveFile << data.isDead << " ";
			if (!data.isDead) {
				saveFile << data.playerClass << " ";
				saveFile << data.health << " ";
				saveFile << data.mana << " ";
				saveFile << data.keyCount << " ";
				saveFile << std::fixed << std::setprecision(0) << data.position.GetX() << " ";
				saveFile << std::fixed << std::setprecision(0) << data.position.GetY() << " ";
			}
			saveFile << std::endl;
		}

		saveFile << isDemo << std::endl;

		if (!isDemo) {
			saveFile << worldSeed << std::endl;
			saveFile << mapPosX << " " << mapPosY << std::endl;
		}

		sv_RoomData roomData;
		while (mapData.Dequeue(roomData)) {
			saveFile << roomData.roomID << std::endl;
			saveFile << roomData.visited << std::endl;
			saveFile << roomData.triggerState.GetSize() << std::endl;
			for (bool triggerState : roomData.triggerState) {
				saveFile << triggerState << " ";
			}
			saveFile << std::endl;

			saveFile << roomData.deadTileIndex.GetSize() << std::endl;
			for (unsigned int deadTileIndex : roomData.deadTileIndex) {
				saveFile << deadTileIndex << " ";
			}
		}

		saveFile.close();
	}

	void SaveSystem::LoadFile(const char* saveDir) {
		std::ifstream saveFile(saveDir);
		if (!saveFile.is_open()) return;
		*this = {};

		saveFile >> score;

		unsigned int playerCount;
		saveFile >> playerCount;

		for (unsigned int i = 0; i < playerCount; i++) {
			sv_PlayerData data{};

			saveFile >> data.isDead;

			if (!data.isDead) {
				int playerClass;
				float x, y;

				saveFile >> playerClass;
				saveFile >> data.health;
				saveFile >> data.mana;
				saveFile >> data.keyCount;
				saveFile >> x;
				saveFile >> y;

				data.playerClass = static_cast<PlayerClass>(playerClass);
				data.position = { x, y };
			}

			playerData.SetValue(data, i);
		}

		saveFile >> isDemo;

		if (!isDemo) {
			saveFile >> worldSeed;
			saveFile >> mapPosX >> mapPosY;
		}

		while (true) {
			sv_RoomData roomData{};
			if (!(saveFile >> roomData.roomID)) break;
			saveFile >> roomData.visited;

			unsigned int triggerCount;
			saveFile >> triggerCount;
			roomData.triggerState = Dataset::Array<bool>(triggerCount);

			for (unsigned int i = 0; i < triggerCount; i++) {
				bool trigger;
				saveFile >> trigger;

				roomData.triggerState.SetValue(trigger, i);
			}

			unsigned int deadTileCount;
			saveFile >> deadTileCount;
			for (unsigned int i = 0; i < deadTileCount; i++) {
				unsigned int tileIndex;
				saveFile >> tileIndex;

				roomData.deadTileIndex.SetValue(tileIndex, i);
			}
			mapData.Enqueue(roomData);
		}

		saveFile.close();
	}
}