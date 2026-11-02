#include "ComponentSystem.hpp"
#include <iostream>
namespace WM908_Assessment {
	bool EntityManager::EntityExist(unsigned int id) const {
		return (entityDataset.Get(id) == nullptr);
	}

	unsigned int EntityManager::GetDatasetSize() const {
		return datasetSize;
	}

	unsigned int EntityManager::AppendEntity() {
		unsigned int id;
		for (id = 0; id < entityDataset.maxSize; id++) {
			if (entityDataset.Get(id) != nullptr) continue;

			if (id >= datasetSize) {
				datasetSize = id + 1;
			}

			entityDataset.SetValue(new Entity(id), id);
			entityDataset.Get(id)->SetEntityID(id);
			break;
		}

		return id;
	}

	void EntityManager::RemoveEntity(unsigned int id) {
		entityDataset.RemoveAtIndex(id);

		ResetDatasetSize();
	}

	void EntityManager::RemoveEntities(unsigned int id[], unsigned int size) {
		for (unsigned int idx = 0; idx < size; idx++) {
			entityDataset.RemoveAtIndex(id[idx]);
		}

		ResetDatasetSize();
	}

	Entity* EntityManager::GetEntityData(unsigned int entityID) const {
		return entityDataset.Get(entityID);
	}

	void EntityManager::ResetDatasetSize() {
		while (datasetSize > 0 && entityDataset.Get(datasetSize - 1) == nullptr) {
			datasetSize--;
		}
	}

	void s_Collisions::DrawCollisionBox(unsigned int datasetSize, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) const {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_CollisionBox* collisionBox = GetCollisionBox(entityID);
			if (collisionBox == nullptr) continue;

			c_Position* position = positionSystem.GetComponent(entityID);
			if (position == nullptr) continue;
			Vec2 screenPosition = renderer->ConvertWorldToScreen(position->position + collisionBox->offset);

			for (unsigned int y = 0; y < collisionBox->boxSize.GetY(); y++) {
				int pixelY = (int)screenPosition.GetY() + y;
				if (!renderer->IsScreenYWithinCamera(pixelY)) continue;

				for (unsigned int x = 0; x < collisionBox->boxSize.GetX(); x++) {
					if (x > 0 && x < (collisionBox->boxSize.GetX() - 1) && y > 0 && y < (collisionBox->boxSize.GetY() - 1)) continue;

					int pixelX = (int)screenPosition.GetX() + x;
					if (!renderer->IsScreenXWithinCamera(pixelX)) continue;

					canvas.draw(pixelX, pixelY, 255, 0, 0);
				}
			}
		}
	}

	bool s_Collisions::CanCollide(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox) const {
		return collisionBox->collisionMask & targetCollisionBox->collisionLayer;
	}

	bool s_Collisions::AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const {
		return (entityTopLeft.GetX() < targetBottomRight.GetX() && entityBottomRight.GetX() > targetTopLeft.GetX()
			&& entityTopLeft.GetY() < targetBottomRight.GetY() && entityBottomRight.GetY() > targetTopLeft.GetY());
	}

	bool s_Collisions::IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition, Vec2& velocity, bool subtractVelocity) const {
		bool isColliding = false;

		for (unsigned int i = 0; i < 2; i++) {
			Vec2 targetBottomRight = targetPosition + targetCollisionBox->boxSize;
			
			// Adds velocity into positions
			Vec2 entityTopLeft = position;
			if (i == 0) entityTopLeft += Vec2{ velocity.GetX(), 0.0f }; // Check X Axis first, then Y axis
			else entityTopLeft += Vec2{ 0.0f, velocity.GetY() }; // (prevent kick back behaviour when hugging wall)

			Vec2 entityTargetBottomRight = entityTopLeft + collisionBox->boxSize;
			
			// Check if space between has collided to prevent speeding bullet from phasing
			float boxSizeAxis = (i == 0) ? targetCollisionBox->boxSize.GetX() : targetCollisionBox->boxSize.GetY();
			if (position.DistanceTo(entityTopLeft) > boxSizeAxis) {
				entityTopLeft.SetX(std::min(entityTopLeft.GetX(), position.GetX()));
				entityTopLeft.SetY(std::min(entityTopLeft.GetY(), position.GetY()));

				entityTargetBottomRight.SetX(std::max(entityTargetBottomRight.GetX(), (position + collisionBox->boxSize).GetX()));
				entityTargetBottomRight.SetY(std::max(entityTargetBottomRight.GetY(), (position + collisionBox->boxSize).GetY()));
			}
			
			// Use Basic AABB to check collision between boxes
			if (!AABB(entityTopLeft, entityTargetBottomRight, targetPosition, targetBottomRight)) continue;
			isColliding = true; // Toggles on if collision exists in either direction

			if (!subtractVelocity) continue;
			if (i % 2 == 0) { // Set New Position X Axis
				float newPositionX = position.GetX();

				if (velocity.GetX() > 0) newPositionX = targetPosition.GetX() - collisionBox->boxSize.GetX();
				else if (velocity.GetX() < 0) newPositionX = targetBottomRight.GetX();

				// subtract velocity X based on collision on X Axis
				velocity.SetX(newPositionX - position.GetX());
			} else { // Set New Position Y Axis
				float newPositionY = position.GetY();

				if (velocity.GetY() > 0) newPositionY = targetPosition.GetY() - collisionBox->boxSize.GetY();
				else if (velocity.GetY() < 0) newPositionY = targetBottomRight.GetY();

				// subtract velocity Y based on collision on Y Axis
				velocity.SetY(newPositionY - position.GetY());
			}
		}
		
		return isColliding;
	}

	bool s_Collisions::IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition) const {
		Vec2 targetBottomRight = targetPosition + targetCollisionBox->boxSize;

		Vec2 entityTopLeft = position;
		Vec2 entityTargetBottomRight = entityTopLeft + collisionBox->boxSize;

		// Use Basic AABB to check collision between boxes
		return AABB(entityTopLeft, entityTargetBottomRight, targetPosition, targetBottomRight);
	}

	bool s_Collisions::CheckCollision(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, Vec2& velocity, bool subtractVelocity) const {
		c_Position* position1 = positionSystem.GetComponent(entityID);
		
		if (position1 == nullptr) return false;

		bool isColliding = false;

		for (unsigned int otherEntityID = 0; otherEntityID < datasetSize; otherEntityID++) {
			if (otherEntityID == entityID) continue;
			c_Position* position2 = positionSystem.GetComponent(otherEntityID);

			if (position2 == nullptr) continue;
			c_CollisionBox* collisionBox1 = GetCollisionBox(entityID);
			c_CollisionBox* collisionBox2 = GetCollisionBox(otherEntityID);

			if (collisionBox1 == nullptr || collisionBox2 == nullptr || !CanCollide(collisionBox1, collisionBox2)) continue;
			
			if (!IsBoxColliding(collisionBox1, collisionBox2, (position1->position + collisionBox1->offset), (position2->position + collisionBox2->offset), velocity, subtractVelocity)) continue;
			isColliding = true;
		}

		return isColliding;
	}

	void s_Movement::Update(unsigned int datasetSize, float deltaTime, s_Position& positionSystem, s_Collisions& collisionSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_Movement* movement = GetComponent(entityID);
			if (movement == nullptr) continue;

			c_Position* position = positionSystem.GetComponent(entityID);
			if (position == nullptr) continue;

			Vec2 velocity = movement->velocity * movement->speed * deltaTime;
			collisionSystem.CheckCollision(datasetSize, entityID, positionSystem, velocity, movement->subtractVelocityOnCollision);

			position->position += velocity;
		}
	}

	void s_4Direction::UpdateDirection(c_4Direction* direction, c_Movement* movement) {
		bool isMoving = movement->velocity.Magnitude() > 0;

		if (movement->velocity.Magnitude() > 0) {
			float x = movement->velocity.GetX();
			float y = movement->velocity.GetY();
			if (movement->velocity.GetX() > 0) {
				direction->facing = Direction::Right;
			}
			else if (movement->velocity.GetX() < 0) {
				direction->facing = Direction::Left;
			}
			else if (movement->velocity.GetY() > 0) {
				direction->facing = Direction::Down;
			}
			else if (movement->velocity.GetY() < 0) {
				direction->facing = Direction::Up;
			}
		}
	}

	unsigned int s_4Direction::GetAnimationIndex(c_4Direction* direction, c_Movement* movement) {
		bool isMoving = movement->velocity.Magnitude() > 0;
		unsigned int animationIndex = direction->facing;

		if (isMoving) {
			animationIndex += 4;
		}

		return animationIndex;
	}

	void s_4Direction::Update(unsigned int datasetSize, s_Movement& movementSystem, s_Sprites& spriteSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_4Direction* direction = GetComponent(entityID);
			if (direction == nullptr) continue;


			c_Movement* movement = movementSystem.GetComponent(entityID);
			if (movement == nullptr) continue;

			UpdateDirection(direction, movement);
			unsigned int animationIdx = GetAnimationIndex(direction, movement);

			c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
			if (animatedSprite != nullptr) {
				spriteSystem.PlayAnimation(animatedSprite, animationIdx);
			}

			// Wanted to make it so that way the player can move while attacking at the same time
			// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
			// I'm keeping the code here just so you know it happened
			// 
			//c_LayeredAnimatedSprite* layeredAnimatedSprite = spriteSystem.GetLayeredAnimatedSprite(entityID);
			//if (layeredAnimatedSprite != nullptr) {
			//	spriteSystem.PlayAnimationAll(layeredAnimatedSprite, animationIdx);
			//}
		}
	}

	void s_Sprites::PlayAnimation(c_AnimatedSprite* animatedSprite, unsigned int animationIdx, unsigned int startFrame) {
		if (animatedSprite->currentAnimation == animationIdx) return;
		if (animationIdx > animatedSprite->GetNumberOfAnimations()) return;
		if (startFrame >= animatedSprite->GetCurrentAnimationFrameCount()) return;

		if (animatedSprite->isPlaying && !animatedSprite->CanOverwriteCurrentAnimation()) return;

		animatedSprite->currentAnimation = animationIdx;
		animatedSprite->animationFrameIndex = startFrame;
		animatedSprite->isPlaying = true;
		animatedSprite->timePassed = 0.0f;
	}
	// Wanted to make it so that way the player can move while attacking at the same time
	// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
	// I'm keeping the code here just so you know it happened
	// 
	//void s_Sprites::PlayAnimationLayer(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int layerID, unsigned int animationIdx, unsigned int startFrame) {
	//	c_AnimatedSprite* layer = layeredAnimatedSprite->layers[layerID];

	//	PlayAnimation(layer, animationIdx, startFrame);
	//}

	//void s_Sprites::PlayAnimationAll(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int animationIdx, unsigned int startFrame) {
	//	for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
	//		PlayAnimationLayer(layeredAnimatedSprite, i, animationIdx, startFrame);
	//	}
	//}

	//void s_Sprites::SyncAnimationLayerFrames(c_LayeredAnimatedSprite* layeredAnimatedSprite) {
	//	unsigned int parentLayerIdx = layeredAnimatedSprite->parentLayerIdx;

	//	c_AnimatedSprite* parentLayer = layeredAnimatedSprite->layers[parentLayerIdx];

	//	for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
	//		if (i == parentLayerIdx) continue;

	//		PlayAnimationLayer(layeredAnimatedSprite, i, parentLayer->currentAnimation, parentLayer->animationFrameIndex);
	//	}
	//}

	void s_Sprites::DrawSprite(const c_Sprite* spriteComponent, const c_Position* positionComponent, Renderer* renderer) {
		if (positionComponent == nullptr) return;
		
		renderer->DrawFromSpritesheetIDWorldPosition(positionComponent->position,
			spriteComponent->spritesheetID,
			spriteComponent->offset,
			spriteComponent->facing,
			spriteComponent->flipH,
			spriteComponent->flipV);
	}

	void s_Sprites::DrawAnimatedSprite(const c_AnimatedSprite* animatedSpriteComponent, const c_Position* positionComponent, Renderer* renderer) {
		if (positionComponent == nullptr) return;

		renderer->DrawFromSpritesheetIDWorldPosition(positionComponent->position,
			animatedSpriteComponent->GetCurrentFrameID(),
			animatedSpriteComponent->offset,
			animatedSpriteComponent->facing,
			animatedSpriteComponent->flipH,
			animatedSpriteComponent->flipV);
	}

	void s_Sprites::Update(unsigned int datasetSize, float deltaTime, Renderer* renderer) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_AnimatedSprite* animatedSprite = GetAnimatedSprite(entityID);
			if (animatedSprite != nullptr) {
				renderer->UpdateAnimationFrame(deltaTime, animatedSprite);

				continue;
			}
			// Wanted to make it so that way the player can move while attacking at the same time
			// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
			// I'm keeping the code here just so you know it happened
			// 
			//c_LayeredAnimatedSprite* layeredAnimatedSprite = GetLayeredAnimatedSprite(entityID);
			//if (layeredAnimatedSprite != nullptr) {
			//	SyncAnimationLayerFrames(layeredAnimatedSprite);
			//	
			//	for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
			//		renderer->UpdateAnimationFrame(deltaTime, layeredAnimatedSprite->layers[i]);
			//	}

			//	continue;
			//}
		}
	}

	void s_Sprites::Draw(unsigned int datasetSize, const s_Position& positionSystem, Renderer* renderer) {
		for (unsigned int drawLayer = 0; drawLayer < 4; drawLayer++) {
			for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
				c_Sprite* sprite = GetSprite(entityID);
				if (sprite != nullptr) {
					if (sprite->drawLayer != drawLayer) continue;

					DrawSprite(sprite, positionSystem.GetComponent(entityID), renderer);
				}
				// Wanted to make it so that way the player can move while attacking at the same time
				// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
				// I'm keeping the code here just so you know it happened
				// 
				//c_LayeredSprite* layeredSprite = GetLayeredSprite(entityID);
				//if (layeredSprite != nullptr) {
				//	if (layeredSprite->drawLayer != drawLayer) continue;

				//	for (unsigned int idx = 0; idx < layeredSprite->numberOfLayers; idx++) {
				//		DrawSprite(layeredSprite->layers[idx], positionSystem.GetComponent(entityID), renderer);
				//	}
				//}

				c_AnimatedSprite* animatedSprite = GetAnimatedSprite(entityID);
				if (animatedSprite != nullptr) {
					if (animatedSprite->drawLayer != drawLayer) continue;

					DrawAnimatedSprite(
						animatedSprite,
						positionSystem.GetComponent(entityID),
						renderer
					);
				}

				// Wanted to make it so that way the player can move while attacking at the same time
				// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
				// I'm keeping the code here just so you know it happened
				// 
				//c_LayeredAnimatedSprite* layeredAnimatedSprite = GetLayeredAnimatedSprite(entityID);
				//if (layeredAnimatedSprite != nullptr) {
				//	if (layeredAnimatedSprite->drawLayer != drawLayer) continue;

				//	for (unsigned int idx = 0; idx < layeredAnimatedSprite->numberOfLayers; idx++) {
				//		DrawAnimatedSprite(layeredAnimatedSprite->layers[idx], positionSystem.GetComponent(entityID), renderer);
				//	}
				//}
			}
		}
	}

	void s_Triggers::DeactivateState(const unsigned int entityID, s_Sprites& spriteSystem) {
		c_ActiveState* activeState = GetActiveState(entityID);
		if (activeState == nullptr) return;
		activeState->isActive = false;

		c_Sprite* sprite = spriteSystem.GetSprite(entityID);
		if (sprite == nullptr) return;
		sprite->spritesheetID = activeState->deactiveState;
	}

	void s_PlayerInput::InstancePrimaryAttack(unsigned int entityID,
		s_4Direction& directionSystem,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem)
	{
		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		c_4Direction* direction = directionSystem.GetComponent(entityID);
		if (direction == nullptr) return;

		c_Melee* meleeAttack = attackSystem.GetMelee(entityID);
		if (meleeAttack != nullptr) {
			attackSystem.InstanceMeleeAttack(entityID, meleeAttack, position->position, direction->facing, entityManager, positionSystem, spriteSystem, collisionSystem);
		}

		c_Projectile* projectile = attackSystem.GetProjectile(entityID);
		if (projectile != nullptr) {
			attackSystem.InstanceProjectile(entityID, projectile, position->position, direction->facing, entityManager, positionSystem, movementSystem, spriteSystem, collisionSystem);
		}
	}

	void s_PlayerInput::Update(unsigned int datasetSize,
		GamesEngineeringBase::Window& window,
		s_4Direction& directionSystem,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem)
	{
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_PlayerInput* input = GetComponent(entityID);
			if (input == nullptr) continue;

			// Wanted to make it so that way the player can move while attacking at the same time
			// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
			// I'm keeping the code here just so you know it happened
			// 
			//c_LayeredAnimatedSprite* layeredAnimatedSprite = spriteSystem.GetLayeredAnimatedSprite(entityID);
			//c_AnimatedSprite* topLayer = nullptr;
			//
			//if (layeredAnimatedSprite != nullptr) topLayer = layeredAnimatedSprite->layers[layeredAnimatedSprite->numberOfLayers - 1];
			
			//if (topLayer != nullptr) {
			//	if (window.keyPressed(input->primaryAttack)) {
			//		unsigned int direction = topLayer->currentAnimation % 4;

			//		if (topLayer->currentAnimation < 8) {
			//			spriteSystem.PlayAnimation(topLayer, 8 + direction);
			//			input->isAttacking = true;
			//		}
			//	}

			//	if (topLayer->animationFrameIndex == 1 && input->isAttacking) {
			//		InstancePrimaryAttack(entityID,
			//			directionSystem,
			//			entityManager,
			//			positionSystem,
			//			movementSystem,
			//			spriteSystem,
			//			collisionSystem,
			//			attackSystem
			//		);
			//		input->isAttacking = false;
			//	}
			//}
			
			c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
			if (animatedSprite == nullptr) continue;

			c_4Direction* direction_4 = directionSystem.GetComponent(entityID);
			if (direction_4 == nullptr) continue;
			
			if (window.keyPressed(input->primaryAttack)) {
				unsigned int direction = direction_4->facing;

				if (animatedSprite->currentAnimation < 8) {
					spriteSystem.PlayAnimation(animatedSprite, 8 + direction);
					input->isAttacking = true;
				}
			}

			if (animatedSprite->animationFrameIndex == 1 && input->isAttacking) {
				InstancePrimaryAttack(entityID,
					directionSystem,
					entityManager,
					positionSystem,
					movementSystem,
					spriteSystem,
					collisionSystem,
					attackSystem
				);
				input->isAttacking = false;
			}

			c_Movement* movement = movementSystem.GetComponent(entityID);
			if (movement == nullptr) continue;
			Vec2 velocity = { 0, 0 };


			if (animatedSprite->currentAnimation < 8) {
				velocity += {window.keyPressed(input->keyRight), window.keyPressed(input->keyDown)};
				velocity -= {window.keyPressed(input->keyLeft), window.keyPressed(input->keyUp)};
			}
			
			movement->velocity = velocity.Normalize();
		}
	}

	void s_Attacks::InstanceMeleeAttack(const unsigned int entityID,
		c_Melee* meleeAttack,
		const Vec2 entityPosition,
		const Direction facing,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem)
	{
		Vec2 meleePosition = { 0.0f, 1.0f };
		
		switch (facing)
		{
		case Direction::Right:
			meleePosition = meleePosition.RotateDeg(-90.0f);
			break;
		case Direction::Left:
			meleePosition = meleePosition.RotateDeg(90.0f);
			break;
		case Direction::Up:
			meleePosition = meleePosition.RotateDeg(180.0f);
			break;
		default:
			break;
		}

		meleePosition = (meleePosition * meleeAttack->positionOffset) + entityPosition;
		unsigned int attackEntityID = entityManager.AppendEntity();
		positionSystem.AddComponent(attackEntityID, meleePosition);
		collisionSystem.AddCollisionBox(attackEntityID, meleeAttack->collisionBoxSize, meleeAttack->collisionBoxOffset, 0, meleeAttack->collisionMask);
		c_Sprite* sprite = spriteSystem.AddSprite(attackEntityID, meleeAttack->spritesheetID, 2);
		sprite->facing = facing;

		attackInstanceEntityID.SetValue(entityManager.GetEntityData(entityID), attackEntityID);
		meleeAttack->timeSinceInstanced = 0.0f;
	}

	void s_Attacks::InstanceProjectile(const unsigned int entityID,
		c_Projectile* projectile,
		const Vec2 entityPosition,
		const Direction facing,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem)
	{
		Vec2 rangedDirection = { 0.0f, 1.0f };

		switch (facing)
		{
		case Direction::Right:
			rangedDirection = rangedDirection.RotateDeg(-90.0f);
			break;
		case Direction::Left:
			rangedDirection = rangedDirection.RotateDeg(90.0f);
			break;
		case Direction::Up:
			rangedDirection = rangedDirection.RotateDeg(180.0f);
			break;
		default:
			break;
		}

		unsigned int attackEntityID = entityManager.AppendEntity();
		positionSystem.AddComponent(attackEntityID, entityPosition);
		c_Movement* movement = movementSystem.AddComponent(attackEntityID, projectile->speed);
		movement->velocity = rangedDirection;
		
		collisionSystem.AddCollisionBox(attackEntityID, projectile->collisionBoxSize, projectile->collisionBoxOffset, 0, projectile->collisionMask);
		c_Sprite* sprite = spriteSystem.AddSprite(attackEntityID, projectile->spritesheetID, 2);

		if (projectile->rotatesToEntityDirection) sprite->facing = facing;

		attackInstanceEntityID.SetValue(entityManager.GetEntityData(entityID), attackEntityID);

	}

	void s_Attacks::Update(unsigned int datasetSize, float deltaTime) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			Entity* parent = attackInstanceEntityID.Get(entityID);
			if (parent == nullptr) continue;

			unsigned int parentEntityID = parent->GetEntityID();

			c_Melee* meleeAttack = GetMelee(parentEntityID);
			if (meleeAttack != nullptr) {
				meleeAttack->timeSinceInstanced += deltaTime;
			}
		}
	}

	void s_Attacks::DeleteAttackInstance(unsigned int datasetSize, unsigned int* unloadedEntities, unsigned int& unloadEntityCount, Renderer* renderer, const s_Position& positionSystem, const s_Movement& movementSystem, const s_Collisions& collisionSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			Entity* parent = attackInstanceEntityID.Get(entityID);
			if (parent == nullptr) continue;

			unsigned int parentEntityID = parent->GetEntityID();

			c_Melee* meleeAttack = GetMelee(parentEntityID);
			c_Projectile* projectile = GetProjectile(parentEntityID);

			if (meleeAttack != nullptr) {
				if (meleeAttack->timeSinceInstanced < meleeAttack->instanceDuration) continue;

			} else if (projectile != nullptr) {
				c_Position* position = positionSystem.GetComponent(entityID);
				if (position == nullptr) continue;

				c_Movement* movement = movementSystem.GetComponent(entityID);
				if (movement == nullptr) continue;

				bool isColliding = collisionSystem.CheckCollision(datasetSize, entityID, positionSystem, movement->velocity, false);
				bool isOnScreen = renderer->IsWorldPositionWithinCamera(position->position);

				if (!isColliding && isOnScreen) continue;
			} else continue;

			attackInstanceEntityID.SetValue(nullptr, entityID);
			unloadedEntities[unloadEntityCount] = entityID;
			unloadEntityCount++;
		}
	}

	void s_EntityData::DeleteCollectables(unsigned int datasetSize, unsigned int* unloadedEntities, unsigned int& unloadEntityCount, Renderer* renderer, const s_Position& positionSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_Collectable* collectable = GetCollectable(entityID);
			if (collectable != nullptr) {
				bool isVisible = renderer->IsWorldPositionWithinCamera(positionSystem.GetComponent(entityID)->position);

				if (!collectable->isCollected && isVisible) continue;
			} else continue;

			unloadedEntities[unloadEntityCount] = entityID;
			unloadEntityCount++;

		}
	}

	void s_EntityData::Update(unsigned int datasetSize, unsigned int* playerIDs, unsigned int numberOfPlayers, s_Position& positionSystem, s_Collisions& collisionSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_Collectable* collectable = GetCollectable(entityID);
			if (collectable == nullptr) continue;
			if (collectable->isCollected) continue;

			c_Position* position = positionSystem.GetComponent(entityID);
			if (position == nullptr) continue;

			c_CollisionBox* collisionBox = collisionSystem.GetCollisionBox(entityID);
			if (collisionBox == nullptr) continue;

			for (unsigned int i = 0; i < numberOfPlayers; i++) {
				unsigned int playerID = playerIDs[i];

				c_Position* playerPosition = positionSystem.GetComponent(playerID);
				if (playerPosition == nullptr) continue;

				c_CollisionBox* playerCollisionBox = collisionSystem.GetCollisionBox(playerID);
				if (playerCollisionBox == nullptr) continue;

				if (!collisionSystem.IsBoxColliding(collisionBox, playerCollisionBox, position->position, playerPosition->position)) continue;
				collectable->isCollected = true;
				break;
			}
		}
	}
}