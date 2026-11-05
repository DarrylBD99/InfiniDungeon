#include "ComponentSystem.hpp"
#include <iostream>
#include <cassert>

namespace WM908_Assessment {
	bool EntityManager::EntityExist(unsigned int id) const {
		return (entityDataset.Get(id) == nullptr);
	}

	unsigned int EntityManager::GetDatasetSize() const {
		return entityDataset.GetSize();
	}

	unsigned int EntityManager::AppendEntity() {
		unsigned int id = entityDataset.AddToClosestEmptyIndex(new Entity(0));
		assert(id < entityDataset.GetMaxSize() && "Entity limit (1024) reached");
		entityDataset.Get(id)->SetEntityID(id);
		return id;
	}

	void EntityManager::RemoveEntity(unsigned int id) {
		entityDataset.RemoveAtIndex(id);
		entityDataset.ResetSize();
	}

	void EntityManager::RemoveEntities(Dataset::Array<unsigned int>* ids) {
		for (unsigned int id : *ids) {
			entityDataset.RemoveAtIndex(id);
		}
		entityDataset.ResetSize();
	}

	Entity* EntityManager::GetEntityData(unsigned int entityID) const {
		return entityDataset.Get(entityID);
	}

	void s_Position::UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>* unloadedEntities, Renderer* renderer) {
		if (unloadedEntities->HasData(entityID)) return;

		c_Position* position = GetComponent(entityID);
		if (position == nullptr || renderer->IsWorldPositionWithinCamera(position->position) || renderer->IsWorldPositionWithinCamera(position->position + Vec2{ (float) renderer->GRID_SIZE })) return;

		unloadedEntities->SetValue(entityID, unloadedEntities->GetSize());
	}

	void s_Collisions::DrawCollisionBox(unsigned int entityID, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) const {
		c_CollisionBox* collisionBox = GetCollisionBox(entityID);
		if (collisionBox == nullptr) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;
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

	bool s_Collisions::CanCollide(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox) const {
		return collisionBox->collisionMask & targetCollisionBox->collisionLayer;
	}

	bool s_Collisions::AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const {
		return (entityTopLeft.GetX() < targetBottomRight.GetX() && entityBottomRight.GetX() > targetTopLeft.GetX()
			&& entityTopLeft.GetY() < targetBottomRight.GetY() && entityBottomRight.GetY() > targetTopLeft.GetY());
	}

	bool s_Collisions::IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition) const {
		Vec2 entityTopLeft = position + collisionBox->offset;
		Vec2 entityTargetBottomRight = entityTopLeft + collisionBox->boxSize;
		Vec2 targetTopLeft = targetPosition + targetCollisionBox->offset;
		Vec2 targetBottomRight = targetPosition + targetCollisionBox->boxSize;

		// Use Basic AABB to check collision between boxes
		return AABB(entityTopLeft, entityTargetBottomRight, targetTopLeft, targetBottomRight);
	}

	void s_Collisions::SubtractVelocity(const unsigned int entityID, const unsigned int targetEntityID, const s_Position& positionSystem, Vec2& velocity) const {
		c_Position* positionComponent = positionSystem.GetComponent(entityID);
		c_Position* targetPositionComponent = positionSystem.GetComponent(targetEntityID);

		c_CollisionBox* collisionBox = GetCollisionBox(entityID);
		c_CollisionBox* targetCollisionBox = GetCollisionBox(targetEntityID);

		Vec2 entityPosition = positionComponent->position + collisionBox->offset;
		Vec2 targetPosition = targetPositionComponent->position + targetCollisionBox->offset;

		for (unsigned int i = 0; i < 2; i++) {
			Vec2 targetTopLeft = targetPosition;
			Vec2 targetBottomRight = targetTopLeft + targetCollisionBox->boxSize;
			
			// Adds velocity into positions
			Vec2 entityTopLeft = entityPosition;
			if (i == 0) entityTopLeft += Vec2{ velocity.GetX(), 0.0f }; // Check X Axis first, then Y axis
			else entityTopLeft += Vec2{ 0.0f, velocity.GetY() }; // (prevent kick back behaviour when hugging wall)

			Vec2 entityTargetBottomRight = entityTopLeft + collisionBox->boxSize;
			
			// Check if space between has collided to prevent speeding bullet from phasing
			float boxSizeAxis = (i == 0) ? targetCollisionBox->boxSize.GetX() : targetCollisionBox->boxSize.GetY();

			if (entityPosition.DistanceTo(entityTopLeft) > boxSizeAxis) {
				entityTopLeft.SetX(std::min(entityTopLeft.GetX(), entityPosition.GetX()));
				entityTopLeft.SetY(std::min(entityTopLeft.GetY(), entityPosition.GetY()));

				entityTargetBottomRight.SetX(std::max(entityTargetBottomRight.GetX(), (entityPosition + collisionBox->boxSize).GetX()));
				entityTargetBottomRight.SetY(std::max(entityTargetBottomRight.GetY(), (entityPosition + collisionBox->boxSize).GetY()));
			}
			
			// Use Basic AABB to check collision between boxes
			if (!AABB(entityTopLeft, entityTargetBottomRight, targetTopLeft, targetBottomRight)) continue;

			if (i == 0) { // Set New Position X Axis
				float newPositionX = entityPosition.GetX();

				if (velocity.GetX() > 0) newPositionX = targetPosition.GetX() - collisionBox->boxSize.GetX();
				else if (velocity.GetX() < 0) newPositionX = targetBottomRight.GetX();

				// subtract velocity X based on collision on X Axis
				velocity.SetX(newPositionX - entityPosition.GetX());
			} else { // Set New Position Y Axis
				float newPositionY = entityPosition.GetY();

				if (velocity.GetY() > 0) newPositionY = targetPosition.GetY() - collisionBox->boxSize.GetY();
				else if (velocity.GetY() < 0) newPositionY = targetBottomRight.GetY();

				// subtract velocity Y based on collision on Y Axis
				velocity.SetY(newPositionY - entityPosition.GetY());
			}
		}
	}

	Dataset::Array<unsigned int> s_Collisions::GetCollidingEntities(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, const Vec2 velocity,const Dataset::Array<unsigned int>* entityToIgnore) const {
		c_CollisionBox* collisionBox = GetCollisionBox(entityID);
		if (collisionBox == nullptr) return Dataset::Array<unsigned int>(0);

		c_Position* position1 = positionSystem.GetComponent(entityID);
		if (position1 == nullptr) return Dataset::Array<unsigned int>(0);

		unsigned int* collidingEntitiesTemp = new unsigned int[datasetSize];
		unsigned int collidingEntityCount = 0;

		for (unsigned int otherEntityID = 0; otherEntityID < datasetSize; otherEntityID++) {
			if (entityToIgnore != nullptr) {
				if (entityToIgnore->HasData(otherEntityID))
					continue;
			}

			if (otherEntityID == entityID) continue;
			c_Position* positionTarget = positionSystem.GetComponent(otherEntityID);
			if (positionTarget == nullptr) continue;
			
			c_CollisionBox* collisionBoxTarget = GetCollisionBox(otherEntityID);
			if (collisionBoxTarget == nullptr || !CanCollide(collisionBox, collisionBoxTarget)) continue;

			if (IsBoxColliding(collisionBox, collisionBoxTarget, position1->position + velocity, positionTarget->position)) {
				collidingEntitiesTemp[collidingEntityCount] = otherEntityID;
				collidingEntityCount++;
			}
		}

		Dataset::Array<unsigned int> collidingEntities(collidingEntityCount);

		for (unsigned int i = 0; i < collidingEntityCount; i++)
			collidingEntities.SetValue(collidingEntitiesTemp[i], i);

		delete[] collidingEntitiesTemp;

		return collidingEntities;
	}

	void s_Movement::Update(unsigned int entityID, unsigned int datasetSize, float deltaTime, s_Position& positionSystem, s_Collisions& collisionSystem) {
		c_Movement* movement = GetComponent(entityID);
		if (movement == nullptr) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		Vec2 velocity = movement->velocity * movement->speed * deltaTime;

		if (movement->phaseThroughCollision) {
			Dataset::Array<unsigned int> collidingEntities = collisionSystem.GetCollidingEntities(datasetSize, entityID, positionSystem, velocity);

			for (unsigned int collidingEntity : collidingEntities) {
				collisionSystem.SubtractVelocity(entityID, collidingEntity, positionSystem, velocity);
			}
		}
		
		position->position += velocity;
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

	void s_4Direction::Update(unsigned int entityID, s_Movement& movementSystem, s_Sprites& spriteSystem) {
		c_4Direction* direction = GetComponent(entityID);
		if (direction == nullptr) return;

		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;

		UpdateDirection(direction, movement);

		c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
		if (animatedSprite != nullptr) {
			spriteSystem.PlayAnimation(animatedSprite, GetAnimationIndex(direction, movement));
		}

		// Wanted to make it so that way the player can move while attacking at the same time
		// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
		// I'm keeping the code here just so you know it happened
		// 
		//c_LayeredAnimatedSprite* layeredAnimatedSprite = spriteSystem.GetLayeredAnimatedSprite(entityID);
		//if (layeredAnimatedSprite != nullptr) {
		//	spriteSystem.PlayAnimationAll(layeredAnimatedSprite, GetAnimationIndex(direction, movement));
		//}
	}

	void s_Sprites::PlayAnimation(c_AnimatedSprite* animatedSprite, unsigned int animationIdx, unsigned int startFrame) {
		if (animatedSprite->currentAnimation == animationIdx) return;
		if (animationIdx >= animatedSprite->GetNumberOfAnimations()) return;
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

	void s_Sprites::UpdateAnimationFrame(float deltaTime, c_AnimatedSprite* animatedSprite) {
		if (!animatedSprite->isPlaying) return;
		animatedSprite->timePassed += deltaTime;

		while (animatedSprite->timePassed > animatedSprite->secondsPerFrame) {
			animatedSprite->animationFrameIndex++;
			animatedSprite->timePassed -= animatedSprite->secondsPerFrame;

			if (animatedSprite->animationFrameIndex < animatedSprite->GetCurrentAnimationFrameCount()) continue;

			if (animatedSprite->IsCurrentAnimationLoop()) {
				animatedSprite->animationFrameIndex = 0;
				continue;
			}
			animatedSprite->animationFrameIndex = animatedSprite->GetCurrentAnimationFrameCount() - 1;
			animatedSprite->isPlaying = false;
			animatedSprite->timePassed = 0.0f;
			return;
		}
	}

	void s_Sprites::Update(unsigned int entityID, float deltaTime, Renderer* renderer) {
		c_AnimatedSprite* animatedSprite = GetAnimatedSprite(entityID);
		if (animatedSprite != nullptr) {
			UpdateAnimationFrame(deltaTime, animatedSprite);
			return;
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
		//		UpdateAnimationFrame(deltaTime, layeredAnimatedSprite->layers[i]);
		//	}

		//	return;
		//}
	}

	void s_Sprites::Draw(unsigned int entityID, unsigned int drawLayer, const c_Position* position, Renderer* renderer) {
		c_Sprite* sprite = GetSprite(entityID);
		if (sprite != nullptr) {
			if (sprite->drawLayer != drawLayer) return;

			DrawSprite(sprite, position, renderer);
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
			if (animatedSprite->drawLayer != drawLayer) return;

			DrawAnimatedSprite(
				animatedSprite,
				position,
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

	void s_Triggers::DeactivateState(const unsigned int entityID, s_Sprites& spriteSystem) const {
		c_ActiveState* activeState = GetActiveState(entityID);
		if (activeState == nullptr) return;
		activeState->isActive = false;

		c_Sprite* sprite = spriteSystem.GetSprite(entityID);
		if (sprite == nullptr) return;
		sprite->spritesheetID = activeState->deactiveState;
	}

	void s_PlayerInput::InstancePrimaryAttack(
		unsigned int entityID,
		unsigned int datasetSize,
		s_4Direction& directionSystem,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
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
			attackSystem.InstanceMeleeAttack(entityID, datasetSize, meleeAttack, position->position, direction->facing, entityManager, positionSystem, entityDataSystem, spriteSystem, collisionSystem);
		}

		c_Projectile* projectile = attackSystem.GetProjectile(entityID);
		if (projectile != nullptr) {
			attackSystem.InstanceProjectile(datasetSize, entityID, projectile, position->position, direction->facing, entityManager, positionSystem, movementSystem, entityDataSystem, spriteSystem, collisionSystem);
		}
	}

	void s_PlayerInput::Update(
		unsigned int entityID,
		unsigned int datasetSize,
		GamesEngineeringBase::Window& window,
		s_4Direction& directionSystem,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem)
	{
		c_PlayerInput* input = GetComponent(entityID);
		if (input == nullptr) return;

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
		if (animatedSprite == nullptr) return;

		c_4Direction* direction_4 = directionSystem.GetComponent(entityID);
		if (direction_4 == nullptr) return;
			
		if (window.keyPressed(input->primaryAttack)) {
			unsigned int direction = direction_4->facing;

			if (animatedSprite->currentAnimation < 8) {
				spriteSystem.PlayAnimation(animatedSprite, 8 + direction);
				input->isAttacking = true;
			}
		}

		if (animatedSprite->animationFrameIndex == 1 && input->isAttacking) {
			InstancePrimaryAttack(
				entityID,
				datasetSize,
				directionSystem,
				entityManager,
				positionSystem,
				movementSystem,
				entityDataSystem,
				spriteSystem,
				collisionSystem,
				attackSystem
			);
			input->isAttacking = false;
		}

		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;
		Vec2 velocity = { 0, 0 };


		if (animatedSprite->currentAnimation < 8) {
			velocity += {window.keyPressed(input->keyRight), window.keyPressed(input->keyDown)};
			velocity -= {window.keyPressed(input->keyLeft), window.keyPressed(input->keyUp)};
		}
			
		movement->velocity = velocity.Normalize();
	}

	void s_Attacks::InstanceMeleeAttack(
		const unsigned int entityID,
		const unsigned int datasetSize,
		c_Melee* meleeAttack,
		const Vec2 entityPosition,
		const Direction facing,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem)
	{
		if (meleeAttack->manaConsume > 0 && !entityDataSystem.DrainMana(entityID, meleeAttack->manaConsume)) {
			return;
		}

		meleeAttack->damagedEntities = Dataset::Array<unsigned int>(datasetSize);

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

	void s_Attacks::InstanceProjectile(
		const unsigned int datasetSize,
		const unsigned int entityID,
		c_Projectile* projectile,
		const Vec2 entityPosition,
		const Direction facing,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem)
	{
		if (projectile->manaConsume > 0 && !entityDataSystem.DrainMana(entityID, projectile->manaConsume)) {
			return;
		}

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
		c_Position* position = positionSystem.AddComponent(attackEntityID, entityPosition);
		c_Movement* movement = movementSystem.AddComponent(attackEntityID, projectile->speed);
		movement->velocity = rangedDirection;
		movement->phaseThroughCollision = false;

		c_CollisionBox* collisionBox = collisionSystem.AddCollisionBox(attackEntityID, projectile->collisionBoxSize, projectile->collisionBoxOffset, 0, projectile->collisionMask);
		c_Sprite* sprite = spriteSystem.AddSprite(attackEntityID, projectile->spritesheetID, 2);

		if (projectile->followNearestEnemy) {
			entityDataSystem.AddFollow(attackEntityID, projectile->followNearestEnemyRange, projectile->collisionMask);
			Vec2 nearestOpp = entityDataSystem.FollowGetNearestEntityPosition(datasetSize, projectile->followNearestEnemyRange, position->position, collisionBox, positionSystem, collisionSystem);

			if (position->position.DistanceTo(nearestOpp) > 0) {
				movement->velocity = (nearestOpp - position->position).Normalize();
			}
		}
		if (projectile->rotatesToEntityDirection) sprite->facing = facing;

		attackInstanceEntityID.SetValue(entityManager.GetEntityData(entityID), attackEntityID);

	}

	void s_Attacks::Update(
		unsigned int entityID,
		unsigned int datasetSize,
		float deltaTime,
		const s_Position& positionSystem,
		const s_Collisions& collisionSystem,
		s_EntityData& entityDataSystem
	) {
		Entity* parent = attackInstanceEntityID.Get(entityID);
		if (parent == nullptr) return;

		unsigned int parentEntityID = parent->GetEntityID();

		c_Melee* meleeAttack = GetMelee(parentEntityID);
		if (meleeAttack != nullptr) {
			meleeAttack->timeSinceInstanced += deltaTime;

			Dataset::Array<unsigned int> collidingEntities = collisionSystem.GetCollidingEntities(datasetSize, entityID, positionSystem, { 0.0f }, &meleeAttack->damagedEntities);

			for (unsigned int collidingEntity : collidingEntities) {
				if (!entityDataSystem.DamageHealth(collidingEntity, meleeAttack->damageCount)) continue;

				meleeAttack->damagedEntities.SetValue(collidingEntity, meleeAttack->damagedEntities.GetSize());
			}
		}
	}

	void s_Attacks::DeleteAttackInstance(
		unsigned int entityID,
		unsigned int datasetSize,
		Dataset::Array<unsigned int>* unloadedEntities,
		const s_Position& positionSystem,
		const s_Movement& movementSystem,
		const s_Collisions& collisionSystem,
		s_EntityData& entityDataSystem
	) 
	{
		if (unloadedEntities->HasData(entityID)) return;

		Entity* parent = attackInstanceEntityID.Get(entityID);
		if (parent == nullptr) return;

		unsigned int parentEntityID = parent->GetEntityID();

		c_Melee* meleeAttack = GetMelee(parentEntityID);
		c_Projectile* projectile = GetProjectile(parentEntityID);

		if (meleeAttack != nullptr) {
			if (meleeAttack->timeSinceInstanced < meleeAttack->instanceDuration) return;

		}
		
		if (projectile != nullptr) {
			c_Position* position = positionSystem.GetComponent(entityID);
			if (position == nullptr) return;

			c_Movement* movement = movementSystem.GetComponent(entityID);
			if (movement == nullptr) return;

			Dataset::Array<unsigned int> collidingEntities = collisionSystem.GetCollidingEntities(datasetSize, entityID, positionSystem);
			
			// Deletes and Damage entities when hit
			if (collidingEntities.GetMaxSize() == 0) return;
			for (unsigned int collidingEntity : collidingEntities) {
				entityDataSystem.DamageHealth(collidingEntity, projectile->damageCount);
			}

		}

		attackInstanceEntityID.SetValue(nullptr, entityID);
		unloadedEntities->SetValue(entityID, unloadedEntities->GetSize());
	}

	void s_EntityData::UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>* unloadedEntities) {
		if (unloadedEntities->HasData(entityID)) return;

		c_Health* health = GetHealth(entityID);
		c_Collectable* collectable = GetCollectable(entityID);

		if (health != nullptr) {
			if (health->current > 0) return;
		} else if (collectable != nullptr) {
			if (!collectable->isCollected) return;
		} else return;

		unloadedEntities->SetValue(entityID, unloadedEntities->GetSize());
	}

	bool s_EntityData::DamageHealth(unsigned int entityID, unsigned int damage) {
		c_Health* health = GetHealth(entityID);

		if (health == nullptr) return false;

		health->current = std::max(int(health->current - damage), 0);

		return true;
	}

	bool s_EntityData::DrainMana(unsigned int entityID, unsigned int manaDrained, bool forceDrain) {
		c_Mana* mana = GetMana(entityID);

		if (mana == nullptr) return false;

		if (int(mana->current - manaDrained) < 0 && !forceDrain) return false;
		
		mana->current = std::max(int(mana->current - manaDrained), 0);
		mana->passiveRegenerationCounter = 0.0f;

		return true;
	}

	Vec2 s_EntityData::FollowGetNearestEntityPosition(unsigned int datasetSize, float lineOfSight, const Vec2& position, c_CollisionBox* collisionBox, s_Position& positionSystem, s_Collisions& collisionSystem) {
		Vec2 closestTargetPos = position;
		float closestDistance = -1;

		for (unsigned int targetEntityID = 0; targetEntityID < datasetSize; targetEntityID++) {
			c_CollisionBox* targetCollisionBox = collisionSystem.GetCollisionBox(targetEntityID);
			if (targetCollisionBox == nullptr || !collisionSystem.CanCollide(collisionBox, targetCollisionBox)) continue;

			c_Position* targetPosition = positionSystem.GetComponent(targetEntityID);
			if (targetPosition == nullptr) continue;

			float distance = position.DistanceTo(targetPosition->position);
			if (lineOfSight > 0 && distance > lineOfSight) continue;
			
			if (closestDistance > 0 && distance > closestDistance) continue;

			closestTargetPos = targetPosition->position;
			closestDistance = distance;
		}

		return closestTargetPos;
	}

	void s_EntityData::Update(unsigned int entityID, unsigned int datasetSize, float deltaTime, Dataset::Array<unsigned int>* playerIDs, unsigned int numberOfPlayers, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem) {
		c_Health* health = GetHealth(entityID);
		if (health != nullptr) {
			if (health->current > health->max) {
				health->current = health->max;
			} else if (health->current < health->max && health->passiveRegenerationRate > 0) {
				health->passiveRegenerationCounter += deltaTime;
				if (health->passiveRegenerationCounter >= health->passiveRegenerationRate) {
					health->current += 1;
					health->passiveRegenerationCounter -= health->passiveRegenerationRate;
				}
			}
		}
		
		c_Mana* mana = GetMana(entityID);
		if (mana != nullptr) {
			if (mana->current > mana->max) {
				mana->current = mana->max;
			}
			else if (mana->current < mana->max && mana->passiveRegenerationRate > 0) {
				mana->passiveRegenerationCounter += deltaTime;
				if (mana->passiveRegenerationCounter >= mana->passiveRegenerationRate) {
					mana->current += 1;
					mana->passiveRegenerationCounter -= mana->passiveRegenerationRate;
				}
			}
		}

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		c_CollisionBox* collisionBox = collisionSystem.GetCollisionBox(entityID);
		if (collisionBox == nullptr) return;

		c_Collectable* collectable = GetCollectable(entityID);
		if (collectable != nullptr) {
			if (collectable->isCollected) return;

			for (unsigned int i = 0; i < numberOfPlayers; i++) {
				unsigned int playerID = playerIDs->Get(i);

				c_Position* playerPosition = positionSystem.GetComponent(playerID);
				if (playerPosition == nullptr) continue;

				c_CollisionBox* playerCollisionBox = collisionSystem.GetCollisionBox(playerID);
				if (playerCollisionBox == nullptr || !collisionSystem.IsBoxColliding(collisionBox, playerCollisionBox, position->position, playerPosition->position)) continue;
				collectable->isCollected = true;
				break;
			}
		}
			
		followUpdateCounter += deltaTime;
		if (followUpdateCounter < runFollowCode) return;

		followUpdateCounter -= runFollowCode;
			
		c_Follow* follow = GetFollow(entityID);
		if (follow != nullptr) {
			c_Movement* movement = movementSystem.GetComponent(entityID);
			if (movement == nullptr) return;

			Vec2 closestTargetPos = FollowGetNearestEntityPosition(datasetSize, follow->lineOfSight, position->position, collisionBox, positionSystem, collisionSystem);

			if (position->position.DistanceTo(closestTargetPos) <= 0) return;
			movement->velocity = (closestTargetPos - position->position).Normalize();
		}
	}
}