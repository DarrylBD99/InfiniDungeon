#include "GamesEngineeringBase.h"
#include "GameSystem.hpp"

#include <iostream>
#include <cmath>
#include <fstream>
#include <memory>
#include "SoundManager.hpp"

GamesEngineeringBase::Window window;

std::unique_ptr<WM908_Assessment::GameSystem> gameSystem = nullptr;
std::unique_ptr<WM908_Assessment::Renderer> renderer = nullptr;
Dataset::Array<WM908_Assessment::PlayerClass> playerClasses{ 2 };

bool characterSelect = false;
bool isPlaying = false;
bool isPaused = false;
bool hasPlayer2 = false;
bool isInDemo = true;
bool isGameOver = false;
bool loadSave = false;
float textCounter = 0.0f;

static constexpr std::string_view SAVE_DIRECTORY = "save.dat";

uint8_t characterSelected = 0;
std::ifstream saveFile(SAVE_DIRECTORY.data());

void ResetToMainMenu() {
	characterSelect = false;
	isPlaying = false;
	hasPlayer2 = false;
	isInDemo = true;
	isGameOver = false;
	isPaused = false;
	loadSave = false;
	characterSelected = 0;
	playerClasses = Dataset::Array<WM908_Assessment::PlayerClass>();
}

void PausedText() {
	renderer->DrawString({ 260.0f, 180.0f }, "1 - Resume\n2 - Exit and Save");

	if (window.keyPressed('1')) {
		textCounter = 0.0f;
		isPaused = false;
		WM908_Assessment::soundManager.PlaySelectSFX();
	}
	if (window.keyPressed('2')) {
		ResetToMainMenu();
		WM908_Assessment::soundManager.PlaySelectSFX();
		
		WM908_Assessment::saveSystem.SaveFile(SAVE_DIRECTORY.data());
		gameSystem.reset();
		textCounter = 0.0f;
		saveFile = std::ifstream(SAVE_DIRECTORY.data());
	}
}

void ScoreText() {
	if (!isInDemo) renderer->DrawString({ 280.0f, 240.0f }, ("Score:" + std::to_string(WM908_Assessment::saveSystem.score)).c_str());
	renderer->DrawString({ 282.0f, 280.0f }, "Main Menu:Q");

	if (window.keyPressed('Q')) {
		ResetToMainMenu();
		WM908_Assessment::soundManager.PlaySelectSFX();
	}
}

void GameOverText() {
	remove(SAVE_DIRECTORY.data());
	renderer->DrawString({ 300.0f, 196.0f }, "Game Over");
}

void EscapeText() {
	renderer->DrawString({ 322.0f, 196.0f }, "Escape");
}

void BreakOutText() {
	renderer->DrawString({ 300.0f, 196.0f }, "Break Out");
}

// Player Selection
struct PlayerControls {
	const char* moveKey;
	const char* moveKeyUp;
	const char* confirmKey;
};

static constexpr PlayerControls controls[] = {
		{ "ASD", "W", "Confirm:Q" },
		{ "JKL", "I", "Confirm:U" }
};

static constexpr char playerClassText[3][7] = {
	"Miner",
	"Archer",
	"Mage"
};

static constexpr unsigned int playerClassTextOffset[3] = {
	9,
	0,
	18
};

static constexpr unsigned int playerClassSpriteFrame[4] = {
	0,
	1,
	0,
	2
};

float frame = 0.0f;

float keyPressBuffer = 0.2f;
float keyPressCounter1 = keyPressBuffer;
float keyPressCounter2 = keyPressBuffer;

void CharacterSelection(float deltaTime) {
	renderer->DrawString({ 237.0f, 100.0f }, "Select Character");
	float upKeyOffset = 18.0f;
	float confirmOffset = -58.0f;
	float nextLineOffset = 20.0f;
	float basePositionY = 248.0f;
	float basePositionX = hasPlayer2 ? 249.0f : 349.0f;
	unsigned int baseFrame = 0;

	frame = std::fmod(frame + deltaTime / 0.15f, 4.0f);

	for (unsigned int i = 0; i < playerClasses.GetMaxSize(); i++) {
		WM908_Assessment::PlayerClass playerClass = playerClasses.Get(i);

		switch (playerClass)
		{
		case WM908_Assessment::PlayerClass::Miner:
			baseFrame = 39;
			break;
		case WM908_Assessment::PlayerClass::Archer:
			baseFrame = 75;
			break;
		case WM908_Assessment::PlayerClass::Mage:
			baseFrame = 111;
			break;
		default:
			break;
		}

		if (i != 0) {
			if (!hasPlayer2) continue;
			basePositionX += 180.0f;
		}
		
		baseFrame += playerClassSpriteFrame[(unsigned int)frame];

		renderer->DrawFromSpritesheetIDScreenPosition({ basePositionX, basePositionY }, baseFrame, { 18.0f, -48.0f });
		renderer->DrawString({ basePositionX + playerClassTextOffset[playerClass] - 27.0f, basePositionY - 90.0f}, playerClassText[playerClass]);
		renderer->DrawString({ basePositionX + upKeyOffset, basePositionY }, controls[i].moveKeyUp);
		renderer->DrawString({ basePositionX, basePositionY + nextLineOffset }, controls[i].moveKey);

		if ((characterSelected & (0b01 << i)) == 0) renderer->DrawString({ basePositionX + confirmOffset, basePositionY + nextLineOffset * 3 }, controls[i].confirmKey);
		else renderer->DrawString({ basePositionX + confirmOffset, basePositionY + nextLineOffset * 3 }, "Confirmed");
	}

	int newPlayerClass;
	if (keyPressCounter1 >= keyPressBuffer) {
		if ((characterSelected & 0b01) == 0) {
			newPlayerClass = playerClasses.Get(0);
			if (window.keyPressed('A') || window.keyPressed('W')) {
				if (newPlayerClass <= 0) newPlayerClass = 2;
				else newPlayerClass--;
				keyPressCounter1 = 0;
				WM908_Assessment::soundManager.PlayPlayerSelectSFX();
			}
			if (window.keyPressed('D') || window.keyPressed('S')) {
				newPlayerClass++;
				newPlayerClass %= 3;
				keyPressCounter1 = 0;
				WM908_Assessment::soundManager.PlayPlayerSelectSFX();
			}

			playerClasses.SetValue((WM908_Assessment::PlayerClass)newPlayerClass, 0);

			if (window.keyPressed('Q')) {
				characterSelected |= 0b01;
				keyPressCounter1 = 0;
				WM908_Assessment::soundManager.PlaySelectSFX();
			}
		}
	}
	else {
		keyPressCounter1 += deltaTime;
	}
	if (hasPlayer2 && keyPressCounter2 >= keyPressBuffer) {
		if (hasPlayer2 && (characterSelected & 0b10) == 0) {
			newPlayerClass = playerClasses.Get(1);
			if (window.keyPressed('J') || window.keyPressed('I')) {
				if (newPlayerClass <= 0) newPlayerClass = 2;
				else newPlayerClass--;
				keyPressCounter2 = 0;
				WM908_Assessment::soundManager.PlayPlayerSelectSFX();
			}
			if (window.keyPressed('L') || window.keyPressed('K')) {
				newPlayerClass++;
				newPlayerClass %= 3;
				keyPressCounter2 = 0;
				WM908_Assessment::soundManager.PlayPlayerSelectSFX();
			}

			playerClasses.SetValue((WM908_Assessment::PlayerClass)newPlayerClass, 1);

			if (window.keyPressed('U')) {
				characterSelected |= 0b10;
				keyPressCounter2 = 0;
				WM908_Assessment::soundManager.PlaySelectSFX();
			}
		}
	} else {
		keyPressCounter2 += deltaTime;
	}
	

	if ((characterSelected & 0b01) != 0 && (!hasPlayer2 || (characterSelected & 0b10) != 0)) {
		isPlaying = true;
		textCounter = 0.0f;
	}
}

void Splash() {
	renderer->DrawString({ 210.0f, 196.0f }, "DarrylBD99 Presents");
}

void MainMenu() {
	renderer->DrawString({ 259.0f, 80.0f }, "InfiniDungeon");
	renderer->DrawString({ 216.0f, 270.0f }, "Press 1 for 1 Player\nPress 2 for 2 Player");

	if (saveFile.is_open()) {
		renderer->DrawString({ 224.0f, 340.0f }, "Enter to Load Save");
		if (window.keyPressed(VK_RETURN)) {
			WM908_Assessment::saveSystem.LoadFile(SAVE_DIRECTORY.data());
			isInDemo = WM908_Assessment::saveSystem.isDemo;
			loadSave = true;
			isPlaying = true;
			textCounter = 0.0f;
		}
	}

	if (window.keyPressed('2')) {
		hasPlayer2 = true;
	}

	if (window.keyPressed('1') || window.keyPressed('2')) {
		characterSelect = true;
		WM908_Assessment::soundManager.PlaySelectSFX();
	}
}

void MainMenuBackground() {
	renderer->DrawSquare(1, 20, 403, 184, 568);
}

const float textWaitBuffer = 1.5f;

int main() {
	// Create Window
	window.create(752, 423, "InfiniDungeon");

	WM908_Assessment::soundManager = WM908_Assessment::SoundManager();

	// Timer object to manage time-based events, such as movement speed
	GamesEngineeringBase::Timer timer;
	
	renderer = std::make_unique<WM908_Assessment::Renderer>(window);
	//gameSystem = new WM908_Assessment::GameSystem(window, renderer, false);
	
	while (true) {
		// Clear the window for the next frame rendering
		window.clear();
		window.checkInput();

		// If the Escape key is pressed, pause/resume game if playing, else break game loop
		if (textCounter >= textWaitBuffer && window.keyPressed(VK_ESCAPE)) {
			if (isPlaying) {
				isPaused = !isPaused;
				textCounter = 0.0f;
			} else break;
		}

		// Get the delta time for the frame
		float dt = timer.dt();

		if (textCounter < textWaitBuffer) {
			textCounter += dt;
		}

		if (isPaused) {
			if (textCounter >= textWaitBuffer) PausedText();
			window.present();
			continue;
		}

		if (isGameOver) {
			if (isPlaying) {
				textCounter = 0.0f;
				isPlaying = false;
				gameSystem.reset();
				gameSystem = nullptr;
			}
			if (textCounter >= 0.5f) GameOverText();
			if (textCounter >= textWaitBuffer) ScoreText();

		} else if (!isPlaying) {
			if (characterSelect) {
				CharacterSelection(dt);

			} else { // Main Menu Screen
				if (textCounter >= textWaitBuffer) MainMenu();
				else if (textCounter >= 0.5f) Splash();
			}

			MainMenuBackground();
		} else if (gameSystem == nullptr) {
			if (saveFile.is_open()) saveFile.close();

			if (textCounter >= 0.5f && textCounter < textWaitBuffer)
				(isInDemo) ? BreakOutText() : EscapeText();

			if (textCounter >= textWaitBuffer) {
				if (loadSave) gameSystem.reset(WM908_Assessment::GameSystem::LoadFromSave(window, renderer.get()));
				else {
					WM908_Assessment::saveSystem = WM908_Assessment::SaveSystem();
					WM908_Assessment::saveSystem.isDemo = isInDemo;
					gameSystem = std::make_unique<WM908_Assessment::GameSystem>(window, renderer.get(), playerClasses, isInDemo, hasPlayer2);
				}
			}
		}
		
		if (gameSystem != nullptr && textCounter >= textWaitBuffer) {
			gameSystem->Update(dt);
			gameSystem->Draw();
			gameSystem->Unload();

			isGameOver = gameSystem->IsGameOver();

			if (gameSystem->ReachedEndGoal()) {
				gameSystem.reset();
				isInDemo = false;
				textCounter = 0.0f;
			}
		}

		// Display the frame on the screen. This must be called once the frame is finished in order to display the frame.
		window.present();
	}

	return 0;
}