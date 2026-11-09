#pragma once
#include <memory>
#include <string>
#include "GamesEngineeringBase.h"


namespace WM908_Assessment {
	class SoundManager {
	private:
		std::unique_ptr<GamesEngineeringBase::SoundManager> soundManager;

	public:
		SoundManager() {
			soundManager = std::make_unique<GamesEngineeringBase::SoundManager>();
			soundManager->load("res/sfx/menu_select.wav");
			soundManager->load("res/sfx/player_select.wav");
			soundManager->load("res/sfx/enemy_hit.wav");
			soundManager->load("res/sfx/player_walk.wav");
		}

		void PlayEnemyHitSFX() {
			if (soundManager == nullptr) return;
			soundManager->play("res/sfx/enemy_hit.wav");
		}

		float counter = 0.4f;
		void PlayPlayerWalkSFX(float deltaTime) {
			if (soundManager == nullptr) return;

			if (counter >= 0.4f) {
				soundManager->play("res/sfx/player_walk.wav");
				counter = 0.0f;
			}
			else counter += deltaTime;
		}

		void PlayPlayerSelectSFX() {
			if (soundManager == nullptr) return;
			soundManager->play("res/sfx/player_select.wav");

		}

		void PlaySelectSFX() {
			if (soundManager == nullptr) return;
			soundManager->play("res/sfx/menu_select.wav");
		}
	};

	extern SoundManager soundManager;
}