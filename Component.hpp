#pragma once
#include "Vector.hpp"
#include "Dataset.hpp"

#include <string>

namespace WM908_Assessment {
	
	enum Direction {
		Down = 0,
		Right = 1,
		Left = 2,
		Up = 3
	};

	enum PlayerClass {
		Miner = 0,
		Archer = 1,
		Mage = 2
	};
	
	// World Space Components
	struct c_Position {
		Vec2 position;
	};

	struct c_Movement {
		Vec2 velocity;
		float speed = 0.0f;

		bool phaseThroughCollision = true;
	};

	// Interaction Components

	struct c_PlayerInput {
		int keyUp = 0, keyDown = 0, keyLeft = 0, keyRight = 0, primaryAttack = 0, secondaryAttack = 0;

		bool isAttacking = false;
	};

	struct c_4Direction {
		Direction facing;
	};

	// Data Components

	struct c_Health {
		unsigned int current = 0, max = 0;

		float passiveRegenerationRate = 0.0f;
		float passiveRegenerationCounter = 0.0f;
	};

	struct c_Mana {
		unsigned int current = 0, max = 0;

		float passiveRegenerationRate = 0.0f;
		float passiveRegenerationCounter = 0.0f;
	};

	enum CollectableType {
		Key = 0,
		Health = 1,
		Mana = 2
	};

	struct c_Collectable {
		bool isCollected = false;

		CollectableType type = CollectableType::Key;
	};

	struct c_Follow {
		unsigned int mask = 0;
		float lineOfSight = 0.0f;

		Vec2 targetPosition;
	};

	struct c_Savable {

	};

	enum EnemyTypes {
		Slime = 0,
		Cannon = 1,
		Bat = 2
	};

	struct c_Spawner {
		EnemyTypes type;
	};

	// Damage Types
	
	struct c_Melee {
		unsigned int collisionMask = 0;

		float instanceDuration = 0.0f;
		float timeSinceInstanced = 0.0f;
		unsigned int spritesheetID = 0;
		float positionOffset = 32.0f;
		Vec2 collisionBoxSize = { 32.0f };
		Vec2 collisionBoxOffset = { };

		unsigned int manaConsume = 0;
		unsigned int damageCount = 1;

		Dataset::Array<unsigned int> damagedEntities{ 0 };
	};

	struct c_Projectile {
		unsigned int collisionMask = 0;

		unsigned int spritesheetID = 0;
		Vec2 collisionBoxSize = { 32.0f };
		Vec2 collisionBoxOffset = {};
		bool followNearestEnemy = false;
		float followNearestEnemyRange = 0.0f;

		bool rotatesToEntityDirection = true;

		float speed = 200.0f;
		unsigned int manaConsume = 0;
		unsigned int damageCount = 1;
	};

	// Triggers
	struct c_ActiveState {
		bool isActive = true;

		unsigned int deactiveState = 0;
	};

	struct c_PressTrigger {
		Dataset::Array<unsigned int> affectedEntity;
	};

	struct c_InteractTrigger {
		Dataset::Array<unsigned int> affectedEntity;
	};

	struct c_DeathTrigger {
		Dataset::Array<unsigned int> affectedEntity;
	};

	struct c_LinkTrigger {
		Dataset::Array<unsigned int> affectedEntity;
	};

	// Collision Components

	// Use Bitwise if box can interact with multiple layers
	enum CollisionLayer {
		Wall = 1 << 0,
		Player = 1 << 1,
		Enemy = 1 << 2
	};

	struct c_CollisionBox {
		unsigned int collisionLayer = 0;
		unsigned int collisionMask = 0;

		Vec2 boxSize = { 0, 0 };
		Vec2 offset = { 0, 0 };
	};

	//struct c_CollisionCircle {
	//	float radius = 0;
	//};

	// Graphical Components
	
	struct c_Rectangle {
		unsigned char colorR, colorG, colorB;
		Vec2 size;
	};

	struct c_Sprite {
		unsigned int drawLayer = 0;

		Direction facing = Direction::Down;
		unsigned int spritesheetID = 0;
		bool flipH = false, flipV = false;

		Vec2 offset = { 0, 0 };
	};

	// Wanted to make it so that way the player can move while attacking at the same time
	// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
	// I'm keeping the code here just so you know it happened
	// 
	//struct c_LayeredSprite {
	//private:
	//	static constexpr unsigned int MAX_LAYERS = 4;
	//public:
	//	unsigned int drawLayer = 0;

	//	unsigned int parentLayerIdx = 0;
	//	unsigned int numberOfLayers = 0;
	//	c_Sprite** layers = new c_Sprite * [MAX_LAYERS];

	//	~c_LayeredSprite() {
	//		delete[] layers;
	//	}

	//	c_Sprite* AddLayer(unsigned int spritesheetID) {
	//		if (numberOfLayers >= MAX_LAYERS) return nullptr;

	//		c_Sprite* layer = new c_Sprite();
	//		layer->spritesheetID = spritesheetID;
	//		layers[numberOfLayers] = layer;

	//		numberOfLayers++;

	//		return layer;
	//	}
	//};

	class c_AnimatedSprite {
	private:
		static constexpr unsigned int MAX_ANIMATIONS = 16;

		Dataset::Array<unsigned int> frameCount{ MAX_ANIMATIONS };
		Dataset::Array<unsigned int> animationIsLoop{ MAX_ANIMATIONS };
		Dataset::Array<unsigned int> animationCanOverwrite{ MAX_ANIMATIONS };

		Dataset::Array<unsigned int*> animations{ MAX_ANIMATIONS };

	public:
		unsigned int drawLayer = 0;
		bool flipH = false, flipV = false;

		Direction facing = Direction::Down;

		bool isPlaying = false;
		unsigned int currentAnimation = 0;
		unsigned int animationFrameIndex = 0;

		const float secondsPerFrame = 0.1f;
		float timePassed = 0.0f;

		Vec2 offset = { 0, 0 };

		unsigned int GetNumberOfAnimations() const {
			return animations.GetSize();
		}

		unsigned int GetCurrentFrameID() const {
			return animations.Get(currentAnimation)[animationFrameIndex];
		}
		
		unsigned int GetCurrentAnimationFrameCount() const {
			return frameCount.Get(currentAnimation);
		}

		bool IsCurrentAnimationLoop() const {
			return animationIsLoop.Get(currentAnimation);
		}

		bool CanOverwriteCurrentAnimation() const {
			return animationCanOverwrite.Get(currentAnimation);
		}

		void AddAnimation(unsigned int frames[], unsigned int numberOfFrames, bool isLoop = false, bool canOverwrite = true) {
			unsigned int numberOfAnimations = animations.GetSize();
			if (numberOfAnimations >= MAX_ANIMATIONS || numberOfFrames == 0 || frames == nullptr) return;

			frameCount.SetValue(numberOfFrames, numberOfAnimations);
			animationIsLoop.SetValue(isLoop, numberOfAnimations);
			animationCanOverwrite.SetValue(canOverwrite, numberOfAnimations);

			unsigned int* framesArray = new unsigned int[numberOfFrames];

			for (unsigned int i = 0; i < numberOfFrames; i++) {
				framesArray[i] = frames[i];
			}
			animations.SetValue(framesArray, animations.GetSize());
		}

		~c_AnimatedSprite() {
			for (unsigned int* animation : animations) {
				delete[] animation;
			}
		}
	};

	// Wanted to make it so that way the player can move while attacking at the same time
	// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
	// I'm keeping the code here just so you know it happened
	// 
	//class c_LayeredAnimatedSprite {
	//private:
	//	static constexpr unsigned int MAX_LAYERS = 4;
	//public:
	//	unsigned int drawLayer = 0;

	//	unsigned int parentLayerIdx = 0;
	//	unsigned int numberOfLayers = 0;
	//	c_AnimatedSprite** layers = new c_AnimatedSprite* [MAX_LAYERS];

	//	~c_LayeredAnimatedSprite() {
	//		delete[] layers;
	//	}

	//	c_AnimatedSprite* AddLayer() {
	//		if (numberOfLayers >= MAX_LAYERS) return nullptr;

	//		c_AnimatedSprite* layer = new c_AnimatedSprite();
	//		layers[numberOfLayers] = layer;

	//		numberOfLayers++;

	//		return layer;
	//	}
	//};
}