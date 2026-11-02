#include "GamesEngineeringBase.h"
#include "GameSystem.hpp"

#include <iostream>

WM908_Assessment::GameSystem* gameSystem = nullptr;

int main() {
	// Create Canvas
	GamesEngineeringBase::Window window;
	window.create(752, 423, "InfiniDungeon");

	// Timer object to manage time-based events, such as movement speed
	GamesEngineeringBase::Timer timer;
	
	gameSystem = new WM908_Assessment::GameSystem(window, true);
	
	while (true) {
		// Clear the window for the next frame rendering
		window.clear();
		window.checkInput();

		// If the Escape key is pressed, exit the loop and close the window
		if (window.keyPressed(VK_ESCAPE))
			break; // Exits the game loop

		// Get the delta time for the frame
		float dt = timer.dt();

		if (gameSystem != nullptr) {
			gameSystem->Update(dt);
			gameSystem->Draw();
			gameSystem->Unload();
		}

		// Display the frame on the screen. This must be called once the frame is finished in order to display the frame.
		window.present();
	}

	return 0;
}