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

		bool subtractVelocityOnCollision = true;
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
		unsigned int current, max;
	};

	struct c_Mana {
		unsigned int current, max;
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
	};

	// Triggers
	struct c_ActiveState {
		bool isActive = true;

		unsigned int deactiveState = 0;
	};

	struct c_PressTrigger {
		unsigned int* affectedEntity;
	};

	struct c_InteractTrigger {
		unsigned int* affectedEntity;
	};

	struct c_DeathTrigger {
		unsigned int* affectedEntity;
	};

	struct c_LinkTrigger {
		unsigned int* affectedEntity;
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
		
		unsigned int numberOfAnimations = 0;
		unsigned int* animationFrameCount = new unsigned int[MAX_ANIMATIONS];
		unsigned int* animationIsLoop = new unsigned int[MAX_ANIMATIONS];
		unsigned int* animationCanOverwrite = new unsigned int[MAX_ANIMATIONS];

		unsigned int** animations = new unsigned int* [MAX_ANIMATIONS];

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
			return numberOfAnimations;
		}

		unsigned int GetCurrentFrameID() const {
			return animations[currentAnimation][animationFrameIndex];
		}
		
		unsigned int GetCurrentAnimationFrameCount() const {
			return animationFrameCount[currentAnimation];
		}

		bool IsCurrentAnimationLoop() const {
			return animationIsLoop[currentAnimation];
		}

		bool CanOverwriteCurrentAnimation() const {
			return animationCanOverwrite[currentAnimation];
		}

		void AddAnimation(unsigned int frames[], unsigned int numberOfFrames, bool isLoop = false, bool canOverwrite = true) {
			if (numberOfAnimations >= MAX_ANIMATIONS) return;

			animationFrameCount[numberOfAnimations] = numberOfFrames;
			animationIsLoop[numberOfAnimations] = isLoop;
			animationCanOverwrite[numberOfAnimations] = canOverwrite;

			unsigned int* framesTemp = new unsigned int[numberOfFrames];
			for (unsigned int i = 0; i < numberOfFrames; i++) {
				framesTemp[i] = frames[i];
			}
			animations[numberOfAnimations] = framesTemp;

			numberOfAnimations++;
		}

		~c_AnimatedSprite() {
			for (unsigned int i = 0; i < numberOfAnimations; i++) {
				delete[] animations[i];
			}

			delete[] animations;
			delete[] animationFrameCount;
			delete[] animationIsLoop;
			delete[] animationCanOverwrite;
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