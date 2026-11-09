#include "ComponentSystem.hpp"
#include "SaveSystem.hpp"
#include "SoundManager.hpp"

#include <cassert>
#include <stdexcept>
#include <random>

namespace WM908_Assessment {
	bool EntityManager::EntityExist(unsigned int id) const {
		return (entityDataset.Get(id) != nullptr);
	}

	unsigned int EntityManager::GetDatasetSize() const {
		return entityDataset.GetSize();
	}

	unsigned int EntityManager::AppendEntity() {
		Entity* entity = new Entity(0);
		unsigned int id = entityDataset.AddToClosestEmptyIndex(entity);
		
		if (id >= entityDataset.GetMaxSize()) {
			delete entity;
			assert(id < entityDataset.GetMaxSize() && "Entity limit (1024) reached");
			throw std::runtime_error("Entity limit reached");
		}

		entityDataset.Get(id)->SetEntityID(id);
		return id;
	}

	void EntityManager::RemoveEntity(unsigned int id) {
		entityDataset.RemoveAtIndex(id);
		entityDataset.ResetSize();
	}

	void EntityManager::RemoveEntities(Dataset::Array<unsigned int>& ids) {
		for (unsigned int id : ids) {
			entityDataset.RemoveAtIndex(id);
		}
		entityDataset.ResetSize();
	}

	Entity* EntityManager::GetEntityData(unsigned int entityID) const {
		return entityDataset.Get(entityID);
	}

	Vec2 s_Position::GetCenterPosition(unsigned int entityID) {
		c_Position* position = GetComponent(entityID);
		if (position == nullptr) return {};

		return GetCenterPosition(position->position);
	}
	
	Vec2 s_Position::GetCenterPosition(Vec2 position) {
		return position + Vec2{ (float)Renderer::GRID_SIZE };
	}

	Vec2 s_Position::GetCenterPosition(Vec2 position, c_CollisionBox* collisionBox) {
		return position + collisionBox->offset + (collisionBox->boxSize / 2);
	}

	void s_Position::UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities, Renderer* renderer) {
		// beauty of doing for loop outside function
		// we can just get the last entry of unloaded entity
		if (unloadedEntities.GetSize() > 0 && unloadedEntities.Get(unloadedEntities.GetSize() - 1) == entityID) return;

		c_Position* position = GetComponent(entityID);
		if (position == nullptr || renderer->IsWorldPositionWithinCamera(position->position) || renderer->IsWorldPositionWithinCamera(position->position + Vec2{ (float) renderer->GRID_SIZE })) return;

		unloadedEntities.SetValue(entityID, unloadedEntities.GetSize());
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

	bool s_Collisions::CanCollide(const unsigned int collisionMask, const unsigned int targetCollisionLayer) const {
		return collisionMask & targetCollisionLayer;
	}

	bool s_Collisions::AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const {
		return (entityTopLeft.GetX() < targetBottomRight.GetX() && entityBottomRight.GetX() > targetTopLeft.GetX()
			&& entityTopLeft.GetY() < targetBottomRight.GetY() && entityBottomRight.GetY() > targetTopLeft.GetY());
	}

	bool s_Collisions::IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition) const {
		Vec2 entityTopLeft = position + collisionBox->offset;
		Vec2 entityTargetBottomRight = entityTopLeft + collisionBox->boxSize;
		Vec2 targetTopLeft = targetPosition + targetCollisionBox->offset;
		Vec2 targetBottomRight = targetTopLeft + targetCollisionBox->boxSize;

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

	Dataset::Array<unsigned int> s_Collisions::GetCollidingEntities(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, const c_Movement* movement) const {
		c_CollisionBox* collisionBox = GetCollisionBox(entityID);
		if (collisionBox == nullptr) return Dataset::Array<unsigned int>(0);

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return Dataset::Array<unsigned int>(0);

		unsigned int* collidingEntitiesTemp = new unsigned int[datasetSize];
		unsigned int collidingEntityCount = 0;

		Vec2 velocity{};

		if (movement != nullptr) {
			velocity = movement->velocity;
		}

		for (unsigned int otherEntityID = 0; otherEntityID < datasetSize; otherEntityID++) {
			if (collisionBox->ignoreEntities.HasData(otherEntityID + 1))
				continue;

			if (otherEntityID == entityID) continue;
			c_Position* positionTarget = positionSystem.GetComponent(otherEntityID);
			if (positionTarget == nullptr) continue;
			
			c_CollisionBox* collisionBoxTarget = GetCollisionBox(otherEntityID);
			if (collisionBoxTarget == nullptr) continue;
			if (!CanCollide(collisionBox->collisionMask, collisionBoxTarget->collisionLayer)) continue;

			if (IsBoxColliding(collisionBox, collisionBoxTarget, position->position + velocity, positionTarget->position)) {
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

	void s_Movement::Update(unsigned int entityID, float deltaTime, Dataset::Array<unsigned int> collidingEntities, s_Position& positionSystem, s_Collisions& collisionSystem) {
		c_Movement* movement = GetComponent(entityID);
		if (movement == nullptr) return;

		if (movement->velocity.Magnitude() == 0) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		Vec2 velocity = movement->velocity * movement->speed * deltaTime;

		if (!movement->phaseThroughCollision) {
			for (unsigned int collidingEntity : collidingEntities) {
				collisionSystem.SubtractVelocity(entityID, collidingEntity, positionSystem, velocity);
			}
		}

		position->position += velocity;
	}

	void s_Directions::UpdateDirection(unsigned int entityID, c_Movement* movement) {
		if (movement->velocity.Magnitude() <= 0) return;
		float x = movement->velocity.GetX();
		float y = movement->velocity.GetY();

		c_2Direction* direction2 = Get2Direction(entityID);
		if (direction2 != nullptr) {
			if (movement->velocity.GetX() > 0) {
				direction2->isFacingLeft = false;
			}
			else if (movement->velocity.GetX() < 0) {
				direction2->isFacingLeft = true;
			}
		}

		c_4Direction* direction4 = Get4Direction(entityID);
		if (direction4 != nullptr) {
			if (std::abs(movement->velocity.GetX()) > std::abs(movement->velocity.GetY())) {
				if (movement->velocity.GetX() > 0) {
					direction4->facing = Direction::Right;
				}
				else if (movement->velocity.GetX() < 0) {
					direction4->facing = Direction::Left;
				}
			} else {
				if (movement->velocity.GetY() > 0) {
					direction4->facing = Direction::Down;
				}
				else if (movement->velocity.GetY() < 0) {
					direction4->facing = Direction::Up;
				}
			}
		}
	}

	unsigned int s_Directions::GetAnimationIndex(c_4Direction* direction, c_Movement* movement) {
		unsigned int animationIndex = direction->facing;

		if (movement->velocity.Magnitude() > 0) {
			animationIndex += 4;
		}

		return animationIndex;
	}

	Vec2 s_Directions::DirectionToVec2(Direction dir) {
		switch (dir)
		{
		case Direction::Down:
			return {  0.0f,  1.0f };
		case Direction::Right:
			return {  1.0f,  0.0f };
		case Direction::Left:
			return { -1.0f,  0.0f };
		case Direction::Up:
			return {  0.0f, -1.0f };
		}
		return { 0.0f };
	}

	void s_Directions::Update(unsigned int entityID, s_Movement& movementSystem, s_Sprites& spriteSystem) {
		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;

		UpdateDirection(entityID, movement);

		c_2Direction* direction2 = Get2Direction(entityID);
		if (direction2 != nullptr) {
			c_Sprite* sprite = spriteSystem.GetSprite(entityID);
			if (sprite != nullptr) {
				sprite->flipH = direction2->isFacingLeft;
			}

			c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
			if (animatedSprite != nullptr) {
				animatedSprite->flipH = direction2->isFacingLeft;
			}
		}

		c_4Direction* direction4 = Get4Direction(entityID);
		if (direction4 != nullptr) {

			c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
			if (animatedSprite != nullptr) {
				unsigned int animationIndex = GetAnimationIndex(direction4, movement) % animatedSprite->GetNumberOfAnimations();
				spriteSystem.PlayAnimation(animatedSprite, animationIndex);
			}
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

	void s_Sprites::DrawSprite(const c_Sprite* spriteComponent, const c_Position* positionComponent, c_ActiveState* activeState, Renderer* renderer) {
		if (positionComponent == nullptr) return;
		unsigned int spritesheetID = spriteComponent->spritesheetID;

		if (activeState != nullptr && activeState->isActive) spritesheetID = activeState->activeStateSprite;

		renderer->DrawFromSpritesheetIDWorldPosition(positionComponent->position,
			spritesheetID,
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

	void s_Sprites::Draw(unsigned int entityID, unsigned int drawLayer, const c_Position* position, c_ActiveState* activeState, Renderer* renderer) {
		c_Sprite* sprite = GetSprite(entityID);
		if (sprite != nullptr) {
			if (sprite->drawLayer != drawLayer) return;

			DrawSprite(sprite, position, activeState, renderer);
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

	bool s_Triggers::TriggerAction(const unsigned int entityID) {
		c_EndGoal* endGoal = GetEndGoal(entityID);
		if (endGoal != nullptr) {
			endGoal->reachedEndGoal = true;
			return true;
		}

		c_ActiveState* activeState = GetActiveState(entityID);
		if (activeState != nullptr) {
			activeState->isActive = !activeState->isActive;
			return activeState->isActive;
		}

		c_TriggerKill* kill = GetTriggerKill(entityID);
		if (kill != nullptr) {
			kill->isDead = !kill->isDead;
			return kill->isDead;
		}

		return false;
	}

	bool s_Triggers::UpdatePressTrigger(const unsigned int entityID, Dataset::Array<unsigned int> collidingEntities) {
		c_PressTrigger* pressTrigger = GetPressTrigger(entityID);
		if (pressTrigger == nullptr) return false;

		c_ActiveState* activeState = GetActiveState(entityID);
		if (activeState != nullptr && activeState->isActive) return false;

		return (collidingEntities.GetSize() > 0);
	}
	
	void s_Triggers::UnloadKillTrigger(const unsigned int entityID, Dataset::Array<unsigned int>&unloadedEntities) {
		// beauty of doing for loop outside function
		// we can just get the last entry of unloaded entity
		if (unloadedEntities.GetSize() > 0 && unloadedEntities.Get(unloadedEntities.GetSize() - 1) == entityID) return;

		c_TriggerKill* kill = GetTriggerKill(entityID);
		if (kill == nullptr) return;
		if (!kill->isDead) return;
		unloadedEntities.SetValue(entityID, unloadedEntities.GetSize());
	}

	bool s_Triggers::IsEndGoalReached(const unsigned int entityID) {
		c_EndGoal* endGoal = GetEndGoal(entityID);
		return (endGoal != nullptr && endGoal->reachedEndGoal);
	}

	void s_PlayerInput::InstancePrimaryAttack(
		unsigned int entityID,
		s_Directions& directionSystem,
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

		c_4Direction* direction = directionSystem.Get4Direction(entityID);
		if (direction == nullptr) return;

		c_Melee* meleeAttack = attackSystem.GetMelee(entityID);
		if (meleeAttack != nullptr) {
			attackSystem.InstanceMeleeAttack(entityID, meleeAttack, position->position, direction->facing, entityManager, positionSystem, entityDataSystem, spriteSystem, collisionSystem);
		}

		c_Projectile* projectile = attackSystem.GetProjectile(entityID);
		if (projectile != nullptr) {
			Vec2 attackDirection = { 0, 0 };
			switch (direction->facing)
			{
			case Direction::Down: {
				attackDirection.SetY(1);
				break;
			}
			case Direction::Right: {
				attackDirection.SetX(1);
				break;
			}
			case Direction::Left: {
				attackDirection.SetX(-1);
				break;
			}
			case Direction::Up: {
				attackDirection.SetY(-1);
				break;
			}
			}

			attackSystem.InstanceProjectile(entityID, projectile, position->position, attackDirection, entityManager, positionSystem, movementSystem, entityDataSystem, spriteSystem, collisionSystem);
		}
	}
	void s_PlayerInput::InstanceSecondaryAction(
		unsigned int entityID,
		s_Movement& movementSystem,
		s_Directions& directionSystem,
		s_EntityData& entityDataSystem,
		s_Attacks& attackSystem)
	{
		c_Mana* mana = entityDataSystem.GetMana(entityID);
		if (mana == nullptr) return;

		c_Dash* dash = attackSystem.GetDash(entityID);
		if (dash != nullptr && entityDataSystem.DrainMana(entityID, dash->manaConsume)) {
			c_Movement* movement = movementSystem.GetComponent(entityID);
			c_4Direction* direction_4 = directionSystem.Get4Direction(entityID);
			if (movement != nullptr && direction_4 != nullptr) {
				dash->oldSpeed = movement->speed;
				movement->velocity = (movement->velocity.Magnitude() > 0) ? movement->velocity : directionSystem.DirectionToVec2(direction_4->facing);
			}

			dash->isActive = true;
			dash->counter = 0.0f;
		}

		c_Immunity* immunity = attackSystem.GetImmunity(entityID);
		if (immunity != nullptr && immunity->hitCount == 0 && entityDataSystem.DrainMana(entityID, immunity->manaConsume)) {
			immunity->hitCount = immunity->maxHits;
		}
	}

	bool s_PlayerInput::CanInteract(
		unsigned int entityID,
		unsigned int datasetSize,
		s_Position& positionSystem,
		s_EntityData& entityDataSystem,
		s_Triggers& triggerSystem,
		s_Collisions& collisionSystem,
		unsigned int& closestEntityID)
	{
		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return false;

		c_CollisionBox* collisionBox = collisionSystem.GetCollisionBox(entityID);
		if (collisionBox == nullptr) return false;

		bool haveInteractable = entityDataSystem.GetNearestEntity(datasetSize, 32.0f, positionSystem.GetCenterPosition(position->position, collisionBox), CollisionLayer::Interactable, positionSystem, collisionSystem, closestEntityID);

		if (!haveInteractable) return false;

		c_InteractTrigger* interact = triggerSystem.GetInteractTrigger(closestEntityID);
		if (interact == nullptr) return false;

		c_KeyCount* keyCount = entityDataSystem.GetKeyCount(entityID);
		if (interact->requireKey && (keyCount == nullptr || keyCount->keyCount == 0)) return false;

		return true;
	}

	void s_PlayerInput::DrawInteractIndicator(
		unsigned entityID,
		Renderer* renderer,
		s_Position& positionSystem)
	{
		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		renderer->DrawFromSpritesheetIDWorldPosition(position->position, 23, { 0.0f, -16.0f });
	}

	void s_PlayerInput::Update(
		unsigned int entityID,
		GamesEngineeringBase::Window& window,
		s_Directions& directionSystem,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem,
		s_Triggers& triggerSystem,
		s_Attacks& attackSystem,
		bool canInteract,
		unsigned int interactEntityID)
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

		c_Dash* dash = attackSystem.GetDash(entityID);
		if (dash != nullptr && dash->isActive) return;

		c_AnimatedSprite* animatedSprite = spriteSystem.GetAnimatedSprite(entityID);
		if (animatedSprite == nullptr) return;

		c_4Direction* direction_4 = directionSystem.Get4Direction(entityID);
		if (direction_4 == nullptr) return;
			
		if (window.keyPressed(input->primaryAttack)) {
			unsigned int direction = direction_4->facing;

			if (animatedSprite->currentAnimation < 8) {
				spriteSystem.PlayAnimation(animatedSprite, 8 + direction);
				input->isAttacking = true;
			}
		}

		if (window.keyPressed(input->interact) && canInteract) {
			c_InteractTrigger* interactTrigger = triggerSystem.GetInteractTrigger(interactEntityID);
			if (interactTrigger != nullptr && interactTrigger->requireKey) {
				entityDataSystem.GetKeyCount(entityID)->keyCount -= 1;
			}

			triggerSystem.TriggerAction(interactEntityID);
		}

		if (window.keyPressed(input->secondaryAttack)) {
			InstanceSecondaryAction(entityID, movementSystem, directionSystem, entityDataSystem, attackSystem);
		}

		if (animatedSprite->animationFrameIndex == 1 && input->isAttacking) {
			InstancePrimaryAttack(
				entityID,
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

	void s_Attacks::DrawImmunityVFX(unsigned int entityID, s_Position& positionSystem, Renderer* renderer) {
		c_Immunity* immunity = GetImmunity(entityID);
		if (immunity == nullptr) return;
		if (immunity->hitCount == 0) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		renderer->DrawFromSpritesheetIDWorldPosition(position->position, immunity->vfxSprite, { 0.0f, -24.0f });
	}

	void s_Attacks::InstanceMeleeAttack(
		const unsigned int entityID,
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
		c_CollisionBox* collisionBox = collisionSystem.AddCollisionBox(attackEntityID, meleeAttack->collisionBoxSize, meleeAttack->collisionBoxOffset, 0, meleeAttack->collisionMask);
		collisionBox->ignoreEntities = Dataset::Array<unsigned int>(1);
		collisionBox->ignoreEntities.SetValue(entityID + 1, 0);
		
		c_Sprite* sprite = spriteSystem.AddSprite(attackEntityID, meleeAttack->spritesheetID, 2);
		sprite->facing = facing;

		c_DamageBox* damageBox = AddDamageBox(attackEntityID, meleeAttack->damageCount);
		damageBox->isTemporary = true;
		damageBox->timeCounter = 0.0f;
		damageBox->timeMax = meleeAttack->instanceDuration;
		damageBox->manaDrain = meleeAttack->enemyManaDrain;
	}

	void s_Attacks::InstanceProjectile(
		const unsigned int entityID,
		c_Projectile* projectile,
		const Vec2 entityPosition,
		Vec2 attackDirection,
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

		attackDirection = attackDirection.Normalize();

		Direction facing;
		if (std::abs(attackDirection.GetX()) > std::abs(attackDirection.GetY()))
			facing = (attackDirection.GetX() > 0) ? Direction::Right : Direction::Left;
		else
			facing = (attackDirection.GetY() > 0) ? Direction::Down : Direction::Up;

		unsigned int attackEntityID = entityManager.AppendEntity();
		c_Position* position = positionSystem.AddComponent(attackEntityID, entityPosition);
		c_Movement* movement = movementSystem.AddComponent(attackEntityID, projectile->speed);
		movement->velocity = attackDirection;
		movement->phaseThroughCollision = true;

		c_CollisionBox* collisionBox = collisionSystem.AddCollisionBox(attackEntityID, projectile->collisionBoxSize, projectile->collisionBoxOffset, 0, projectile->collisionMask);
		collisionBox->ignoreEntities = Dataset::Array<unsigned int>(1);
		collisionBox->ignoreEntities.SetValue(entityID + 1, 0);

		c_Sprite* sprite = spriteSystem.AddSprite(attackEntityID, projectile->spritesheetID, 2);

		if (projectile->followNearestEnemy) {
			entityDataSystem.AddFollow(attackEntityID, projectile->followNearestEnemyRange, projectile->collisionMask);
		}
		if (projectile->rotatesToEntityDirection) sprite->facing = facing;

		c_DamageBox* damageBox = AddDamageBox(attackEntityID, projectile->damageCount, true);
		damageBox->manaDrain = projectile->enemyManaDrain;
	}

	void s_Attacks::UpdateAttackProximity(
		unsigned int entityID,
		float deltaTime,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Collisions& collisionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem
	) {
		c_AttackProximity* attackProximity = GetAttackProximity(entityID);
		if (attackProximity == nullptr) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		c_CollisionBox* collision = collisionSystem.GetCollisionBox(entityID);
		if (collision == nullptr) return;

		c_Follow* follow = entityDataSystem.GetFollow(entityID);
		Vec2 centerPos = positionSystem.GetCenterPosition(position->position, collision);

		if (follow != nullptr) attackProximity->targetPosition = follow->targetPosition;
		if (centerPos.DistanceTo(attackProximity->targetPosition) > attackProximity->attackRange) return;
		
		if (attackProximity->attackCooldownCounter > attackProximity->attackCooldown) {
			attackProximity->attackCooldownCounter -= attackProximity->attackCooldown;

			Vec2 attackDirection = attackProximity->targetPosition - centerPos;
			attackDirection = attackDirection.Normalize();
			Direction facing;

			if (std::abs(attackDirection.GetX()) > std::abs(attackDirection.GetY()))
				facing = (attackDirection.GetX() > 0) ? Direction::Right : Direction::Left;
			else
				facing = (attackDirection.GetY() > 0) ? Direction::Down : Direction::Up;

			c_Melee* melee = GetMelee(entityID);
			if (melee != nullptr) {
				InstanceMeleeAttack(entityID, melee, position->position, facing, entityManager, positionSystem, entityDataSystem, spriteSystem, collisionSystem);
			}

			c_Projectile* projectile= GetProjectile(entityID);
			if (projectile != nullptr) {
				InstanceProjectile(entityID, projectile, position->position, attackDirection, entityManager, positionSystem, movementSystem, entityDataSystem, spriteSystem, collisionSystem);
			}

		} else if (attackProximity->attackCooldownCounter > attackProximity->attackCooldown) return;


		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement != nullptr) movement->velocity = { 0.0f };

		attackProximity->attackCooldownCounter += deltaTime;
	}

	void s_Attacks::UpdateDash(unsigned int entityID, float deltaTime, s_Movement& movementSystem) {
		c_Dash* dash = GetDash(entityID);
		if (dash == nullptr) return;
		if (!dash->isActive) return;

		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;

		if (dash->duration > dash->counter) {
			dash->counter += deltaTime;
			movement->speed = dash->speed;
		}
		else {
			dash->isActive = false;
			movement->speed = dash->oldSpeed;
		}
	}

	void s_Attacks::UpdateAutoProjectiles(
		const unsigned int entityID,
		float deltaTime,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_EntityData& entityDataSystem,
		s_Sprites& spriteSystem,
		s_Collisions& collisionSystem,
		s_Triggers& triggerSystem) {
		c_Projectile* projectile = GetProjectile(entityID);
		if (projectile == nullptr) return;

		c_ActiveState* activeState = triggerSystem.GetActiveState(entityID);
		if (activeState != nullptr && activeState->isActive) return;
		if (!projectile->autoFire) return;
		projectile->autoFireCounter += deltaTime;
		
		if (projectile->autoFireCounter < projectile->autoFireRate) return;
		projectile->autoFireCounter -= projectile->autoFireRate;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		InstanceProjectile(entityID, projectile, position->position, Vec2{ 0.0f, 1.0f }, entityManager, positionSystem, movementSystem, entityDataSystem, spriteSystem, collisionSystem);
	}

	void s_Attacks::UpdateDamageBox(
		unsigned int entityID,
		float deltaTime,
		Dataset::Array<unsigned int> collidingEntities,
		s_EntityData& entityDataSystem,
		s_Triggers& triggerSystem)
	{
		c_DamageBox* damageBox = GetDamageBox(entityID);
		if (damageBox == nullptr) return;
		if (damageBox->isTemporary) damageBox->timeCounter += deltaTime;

		c_ActiveState* activeState = triggerSystem.GetActiveState(entityID);
		if (activeState != nullptr) damageBox->isDisabled = activeState->isActive; // reversed as it is synced with map trigger id

		if (damageBox->isDisabled) return;

		// Deletes and Damage entities when hit
		if (collidingEntities.GetMaxSize() > 0) {
			damageBox->isHit = true;

			for (unsigned int collidingEntity : collidingEntities) {
				if (damageBox->damagedEntities.HasData(collidingEntity + 1))
					continue;
				c_Immunity* immunity = GetImmunity(collidingEntity);
				if (immunity != nullptr && immunity->hitCount > 0) {
					immunity->hitCount -= 1;
				}
				else if (!entityDataSystem.DamageHealth(collidingEntity, damageBox->damageCount)) {
					if (damageBox->manaDrain > 0) entityDataSystem.DrainMana(collidingEntity, damageBox->manaDrain, true);
					continue;
				}

				damageBox->damagedEntities.SetValue(collidingEntity + 1, damageBox->damagedEntities.GetSize());
				damageBox->damageCounter.SetValue(0.0f, damageBox->damagedEntities.GetSize());
			}
		}

		for (unsigned int i = 0; i < damageBox->damageCounter.GetSize(); i++) {
			float& damageCounter = damageBox->damageCounter.Get(i);

			damageCounter += deltaTime;
			if (damageCounter < damageBox->hitEnemyBuffer) continue;
			damageBox->damagedEntities.RemoveAtIndex(i);
			damageBox->damageCounter.RemoveAtIndex(i);
		}
		
		damageBox->damagedEntities.ResetSize();
		damageBox->damageCounter.ResetSize();
	}

	void s_Attacks::UnloadDamageBox(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities)
	{
		c_DamageBox* damageBox = GetDamageBox(entityID);
		if (damageBox == nullptr) return;

		if (damageBox->isTemporary && damageBox->timeCounter >= damageBox->timeMax) {
			unloadedEntities.SetValue(entityID, unloadedEntities.GetSize());
			return;
		}

		if (damageBox->isOneTime && damageBox->isHit) {
			unloadedEntities.SetValue(entityID, unloadedEntities.GetSize());
			return;
		}
	}

	void s_EntityData::UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities) {
		// beauty of doing for loop outside function
		// we can just get the last entry of unloaded entity
		if (unloadedEntities.GetSize() > 0 && unloadedEntities.Get(unloadedEntities.GetSize() - 1) == entityID) return;

		c_Health* health = GetHealth(entityID);
		c_Collectable* collectable = GetCollectable(entityID);

		if (health != nullptr) {
			if (health->current > 0) { return; }
		} else if (collectable != nullptr) {
			if (!collectable->isCollected)
				{ return; }
		} else { return; }

		unloadedEntities.SetValue(entityID, unloadedEntities.GetSize());
	}

	bool s_EntityData::DamageHealth(unsigned int entityID, unsigned int damage) {
		c_Health* health = GetHealth(entityID);
		if (health == nullptr) return false;
		health->current = std::max(int(health->current - damage), 0);

		soundManager.PlayEnemyHitSFX();
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

	bool s_EntityData::GetNearestEntity(
		unsigned int datasetSize,
		float lineOfSight,
		const Vec2& centerPosition,
		const unsigned int collisionLayer,
		s_Position& positionSystem,
		s_Collisions& collisionSystem,
		unsigned int& closestEntityID)
	{
		float closestDistance = -1;
		bool foundTarget = false;
		Vec2 targetCenterPosition;

		for (unsigned int targetEntityID = 0; targetEntityID < datasetSize; targetEntityID++) {
			c_CollisionBox* targetCollisionBox = collisionSystem.GetCollisionBox(targetEntityID);
			if (targetCollisionBox == nullptr || !collisionSystem.CanCollide(collisionLayer, targetCollisionBox->collisionLayer)) continue;

			c_Position* targetPosition = positionSystem.GetComponent(targetEntityID);
			if (targetPosition == nullptr) continue;

			Vec2 targetCenterPos = positionSystem.GetCenterPosition(targetPosition->position, targetCollisionBox);
			float distance = centerPosition.DistanceTo(targetCenterPos);
			if (lineOfSight > 0 && distance > lineOfSight) continue;

			if (closestDistance > 0 && distance > closestDistance) continue;

			targetCenterPosition = targetCenterPos;
			closestDistance = distance;
			closestEntityID = targetEntityID;
			foundTarget = true;
		}

		return foundTarget;
	}

	bool s_EntityData::GetNearestEntityPosition(
		unsigned int datasetSize,
		float lineOfSight,
		const Vec2& centerPosition,
		const unsigned int collisionLayer,
		s_Position& positionSystem,
		s_Collisions& collisionSystem,
		Vec2& targetCenterPosition)
	{
		unsigned int closestEntityID = 0;
		bool foundTarget = GetNearestEntity(datasetSize, lineOfSight, centerPosition, collisionLayer, positionSystem, collisionSystem, closestEntityID);
		if (foundTarget) {
			c_Position* targetPosition = positionSystem.GetComponent(closestEntityID);
			if (targetPosition == nullptr) return false;

			c_CollisionBox* targetCollisionBox = collisionSystem.GetCollisionBox(closestEntityID);
			if (targetCollisionBox == nullptr) return false;

			targetCenterPosition = positionSystem.GetCenterPosition(targetPosition->position, targetCollisionBox);
		}

		return foundTarget;
	}

	void s_Spawner::Update(
		unsigned int entityID,
		float deltaTime,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Directions& directionSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem,
		s_EntityData& entityDataSystem)
	{
		c_Spawner* spawner = GetComponent(entityID);
		if (spawner == nullptr) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		spawner->spawnCooldownCounter += deltaTime;
		if (spawner->spawnCooldownCounter > spawner->spawnCooldown) {
			SpawnEnemies(position->position, entityManager, positionSystem, movementSystem, spriteSystem, directionSystem, collisionSystem, attackSystem, entityDataSystem, spawner->type, spawner->maxCount);
			spawner->spawnCooldownCounter -= spawner->spawnCooldown;
		}
	}

	void s_Spawner::SpawnEnemy(
		Vec2 spawnPosition,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Directions& directionSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem,
		s_EntityData& entityDataSystem,
		EnemyTypes enemyType) {

		unsigned int entityID = entityManager.AppendEntity();
		positionSystem.AddComponent(entityID, spawnPosition);
		c_CollisionBox* collision = collisionSystem.AddCollisionBox(entityID, { (float)Renderer::GRID_SIZE }, { 0.0f }, CollisionLayer::Enemy, CollisionLayer::Wall);
		c_Health* health = entityDataSystem.AddHealth(entityID, 5);
		c_AttackProximity* attackProximity = attackSystem.AddAttackProximity(entityID, 30.0f);
		switch (enemyType)
		{
		case EnemyTypes::Slime: {
			c_Melee* melee = attackSystem.AddMelee(entityID, 24, 0.2f, CollisionLayer::Player);
			melee->damageCount = 2;
			entityDataSystem.AddScoreIncrease(entityID, 5);
			spriteSystem.AddSprite(entityID, 28, 1);
			directionSystem.Add2Direction(entityID);
			movementSystem.AddComponent(entityID, 100.0f);
			break;
		}
		case EnemyTypes::Cannon: {
			c_Projectile* projectile = attackSystem.AddProjectile(entityID, 25, 200.0f, false, CollisionLayer::Player | CollisionLayer::Wall);
			projectile->damageCount = 5;
			projectile->collisionBoxSize = { 16.0f };
			projectile->collisionBoxOffset = { 8.0f };

			health->max = 8;
			health->current = 8;

			entityDataSystem.AddScoreIncrease(entityID, 10);
			c_AnimatedSprite* animatedSprite = spriteSystem.AddAnimatedSprite(entityID, 1);
			unsigned int animations[4][1] = {
				{ 29 },
				{ 30 },
				{ 31 },
				{ 32 }
			};

			for (unsigned int i = 0; i < 4; i++) {
				animatedSprite->AddAnimation(animations[i], 1);
			}

			directionSystem.Add4Direction(entityID);
			movementSystem.AddComponent(entityID, 80.0f);
			attackProximity->attackRange = 120.0f;
			attackProximity->attackCooldownCounter = 1.4f;
			attackProximity->attackCooldown = 1.4f;
			break;
		}
		case EnemyTypes::Bat: {
			c_Melee* melee = attackSystem.AddMelee(entityID, 26, 0.2f, CollisionLayer::Player);
			melee->collisionBoxSize = { 20, 12 };
			melee->collisionBoxOffset = { 0, 6 };
			melee->positionOffset = 16.0f;
			melee->enemyManaDrain = 1;

			health->max = 2;
			health->current = 2;

			collision->boxSize = { 28, 14 };
			collision->offset = { 2, 9 };

			c_AnimatedSprite* animatedSprite = spriteSystem.AddAnimatedSprite(entityID, 1);
			unsigned int animation[3] = { 33, 34, 35 };
			animatedSprite->AddAnimation(animation, 3, true);
			animatedSprite->isPlaying = true;

			entityDataSystem.AddScoreIncrease(entityID, 8);
			directionSystem.Add2Direction(entityID);
			movementSystem.AddComponent(entityID, 120.0f);
			attackProximity->attackCooldownCounter = 12.0f;
			attackProximity->attackCooldown = 0.4f;
			break;
		}
		}

		entityDataSystem.AddFollow(entityID, 384.0f, CollisionLayer::Player);
	}

	void s_Spawner::SpawnEnemies(
		Vec2 originPosition,
		EntityManager& entityManager,
		s_Position& positionSystem,
		s_Movement& movementSystem,
		s_Sprites& spriteSystem,
		s_Directions& directionSystem,
		s_Collisions& collisionSystem,
		s_Attacks& attackSystem,
		s_EntityData& entityDataSystem,
		EnemyTypes enemyType,
		unsigned int maxCount)
	{
		std::random_device randomDevice;
		std::mt19937 rng(randomDevice());
		std::uniform_int_distribution<int> distributionOffset(-1, 1);
		std::uniform_int_distribution<int> distributionCount(1, maxCount);

		for (int i = 0; i < distributionCount(rng);) {
			int positionOffsetX = distributionOffset(rng);
			int positionOffsetY = distributionOffset(rng);
			
			if (positionOffsetX == 0 && positionOffsetY == 0) continue;
			Vec2 spawnPosition = originPosition + Vec2{ positionOffsetX, positionOffsetY } * (float)Renderer::GRID_SIZE;
			SpawnEnemy(spawnPosition, entityManager, positionSystem, movementSystem, spriteSystem, directionSystem, collisionSystem, attackSystem, entityDataSystem, enemyType);
			i++;
		}
		
	}

	void s_EntityData::UpdateHealth(unsigned int entityID, float deltaTime) {
		c_Health* health = GetHealth(entityID);
		if (health == nullptr) return;
		
		if (health->passiveRegenerationRate > 0) {
			health->passiveRegenerationCounter += deltaTime;
			if (health->passiveRegenerationCounter >= health->passiveRegenerationRate) {
				health->current += 1;
				health->passiveRegenerationCounter -= health->passiveRegenerationRate;
			}
		}

		health->current = (health->current > health->max) ? health->max : health->current;
	}

	void s_EntityData::UpdateMana(unsigned int entityID, float deltaTime) {
		c_Mana* mana = GetMana(entityID);
		if (mana == nullptr) return;

		if (mana->passiveRegenerationRate > 0) {
			mana->passiveRegenerationCounter += deltaTime;
			if (mana->passiveRegenerationCounter >= mana->passiveRegenerationRate) {
				mana->current += 1;
				mana->passiveRegenerationCounter -= mana->passiveRegenerationRate;
			}
		}

		mana->current = (mana->current > mana->max) ? mana->max : mana->current;
	}

	void s_EntityData::UpdateCollectible(unsigned int entityID, Dataset::Array<unsigned int> collidingEntities) {
		c_Collectable* collectable = GetCollectable(entityID);
		if (collectable == nullptr) return;
		if (collectable->isCollected) return;
		if (collidingEntities.GetSize() == 0) return;

		const unsigned int collector = collidingEntities.Get(0);
		switch (collectable->type)
		{
		case CollectableType::Health: {
			c_Health* health = GetHealth(collector);
			if (health == nullptr) return;
			health->current += 20;
			break;
		}
		case CollectableType::Mana: {
			c_Mana* mana = GetMana(collector);
			if (mana == nullptr) return;
			mana->current += 10;
			break;
		}
		case CollectableType::Key: {
			c_KeyCount* keyCount = GetKeyCount(collector);
			if (keyCount == nullptr) return;
			keyCount->keyCount += 1;
			break;
		}
		}
		collectable->isCollected = true;
	}

	void s_EntityData::UpdateFollow(unsigned int entityID, unsigned int datasetSize, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem) {
		c_Follow* follow = GetFollow(entityID);
		if (follow == nullptr) return;
		
		if (follow->hasSearched && followUpdateCounter < runFollowCode) return;
		follow->hasSearched = true;
		
		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		c_CollisionBox* collisionBox = collisionSystem.GetCollisionBox(entityID);
		if (collisionBox == nullptr) return;

		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;

		Vec2 centerPos = position->position + collisionBox->offset + (collisionBox->boxSize / 2);
		follow->hasTarget = GetNearestEntityPosition(datasetSize, follow->lineOfSight, centerPos, follow->mask, positionSystem, collisionSystem, follow->targetPosition);

		if (!follow->hasTarget) return;
		movement->velocity = (centerPos.DistanceTo(follow->targetPosition) > 12.0f) ? (follow->targetPosition - centerPos).Normalize() : Vec2{ 0.0f };
	}

	void s_EntityData::CheckOutsideCamera(unsigned int entityID, Renderer* renderer, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem) {
		c_NoOutsideCamera* noOutsideCamera = GetNoOutsideCamera(entityID);
		if (noOutsideCamera == nullptr) return;

		c_Position* position = positionSystem.GetComponent(entityID);
		if (position == nullptr) return;

		c_CollisionBox* collisionBox = collisionSystem.GetCollisionBox(entityID);
		if (collisionBox == nullptr) return;

		c_Movement* movement = movementSystem.GetComponent(entityID);
		if (movement == nullptr) return;

		if (movement->velocity.Magnitude() <= 0) return;
		Vec2 topLeft = renderer->ConvertWorldToScreen(position->position + collisionBox->offset + movement->velocity);
		Vec2 bottomRight = topLeft + collisionBox->boxSize;

		Vec2 cameraTopLeft = { (float)renderer->cameraBorderWidth };
		Vec2 cameraBottomRight = cameraTopLeft + Vec2{ (float)renderer->cameraWidth, (float)renderer->cameraHeight };

		if ((movement->velocity.GetX() > 0 && collisionSystem.AABB(
			topLeft,
			bottomRight,
			{ cameraBottomRight.GetX(), cameraTopLeft.GetY() },
			cameraBottomRight + Vec2{ 32.0f, 0.0f }
		)) || (movement->velocity.GetX() < 0 && collisionSystem.AABB(
			topLeft,
			bottomRight,
			cameraTopLeft - Vec2{ 32.0f, 0.0f },
			{ cameraTopLeft.GetX(), cameraBottomRight.GetY() }
		))) {
			movement->velocity.SetX(0.0f);
		}

		if ((movement->velocity.GetY() > 0 && collisionSystem.AABB(
			topLeft,
			bottomRight,
			{ cameraTopLeft.GetX(), cameraBottomRight.GetY() },
			cameraBottomRight + Vec2{ 0.0f, 32.0f }
		)) || (movement->velocity.GetY() < 0 && collisionSystem.AABB(
			topLeft,
			bottomRight,
			cameraTopLeft - Vec2{ 0.0f, 32.0f },
			{ cameraBottomRight.GetX(), cameraTopLeft.GetY() }
		))) {
			movement->velocity.SetY(0.0f);
		}
	}

	void s_EntityData::UpdateFollowCounter(float deltaTime) {
		followUpdateCounter += deltaTime;
	}

	void s_EntityData::ResetFollowCounter() {
		if (followUpdateCounter >= runFollowCode) {
			followUpdateCounter -= runFollowCode;
		}
	}

	bool s_EntityData::SaveDeathState(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities, s_Triggers& triggers) {
		// beauty of doing for loop outside function
		// we can just get the last entry of unloaded entity
		if (unloadedEntities.GetSize() <= 0 || unloadedEntities.Get(unloadedEntities.GetSize() - 1) != entityID) return false;

		c_DeathSavable* deathSavable = GetDeathSavable(entityID);
		if (deathSavable == nullptr) return false;
		bool canSave = false;

		c_Health* health = GetHealth(entityID);
		if (health != nullptr && health->current == 0) canSave = true;
		
		c_Collectable* collectable = GetCollectable(entityID);
		if (collectable != nullptr && collectable->isCollected) canSave = true;
		
		c_TriggerKill* kill = triggers.GetTriggerKill(entityID);
		if (kill != nullptr && kill->isDead) canSave = true;
		
		if (!canSave) return false;

		saveSystem.AddDeadTile(deathSavable->roomID, deathSavable->tileIndex);
		return true;
	}
}