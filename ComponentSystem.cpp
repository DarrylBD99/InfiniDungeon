#include "ComponentSystem.hpp"

namespace WM908_Assessment {

	// EntityManager Class
	EntityManager::EntityManager() {

	}

	EntityManager::~EntityManager() {
		for (Entity* entity : entityDataset) {
			delete entity;
		}
	}

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

	void s_Collisions::DrawCollisionBox(unsigned int datasetSize, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_CollisionBox* collisionBox = GetCollisionBox(entityID);
			if (collisionBox == nullptr) continue;

			c_Position* position = positionSystem.GetComponent(entityID);
			if (position == nullptr) continue;
			Vec2 screenPosition = renderer->ConvertWorldToScreen(position->position);

			for (unsigned int y = 0; y < collisionBox->boxSize.GetY(); y++) {
				int pixelY = screenPosition.GetY() + y;
				if (!renderer->IsScreenYWithinCamera(pixelY)) continue;

				for (unsigned int x = 0; x < collisionBox->boxSize.GetX(); x++) {
					if (x > 0 && x < (collisionBox->boxSize.GetX() - 1) && y > 0 && y < (collisionBox->boxSize.GetY() - 1)) continue;

					int pixelX = screenPosition.GetX() + x;
					if (!renderer->IsScreenXWithinCamera(pixelX)) continue;

					canvas.draw(pixelX, pixelY, ((collisionBox->isSolid) ? 255 : 0), 0, ((collisionBox->isSolid) ? 0 : 255));
				}
			}
		}
	}

	bool s_Collisions::AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const {
		return (entityTopLeft.GetX() < targetBottomRight.GetX() && entityBottomRight.GetX() > targetTopLeft.GetX()
			&& entityTopLeft.GetY() < targetBottomRight.GetY() && entityBottomRight.GetY() > targetTopLeft.GetY());
	}

	bool s_Collisions::IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition, Vec2& velocity, bool requireSolid) const {
		if (requireSolid && !targetCollisionBox->isSolid) return false;

		bool isColliding = false;

		for (unsigned int i = 0; i < 2; i++) {
			Vec2 entityTargetPosition = position;
			
			// Adds velocity into positions
			if (i % 2 == 0) entityTargetPosition += Vec2{ velocity.GetX(), 0.0f }; // Check X Axis first, then Y axis
			else entityTargetPosition += Vec2{ 0.0f, velocity.GetY() }; // (prevent kick back behaviour when hugging wall)

			Vec2 entityTargetBottomRight = entityTargetPosition + collisionBox->boxSize;

			Vec2 targetBottomRight = targetPosition + targetCollisionBox->boxSize;

			// Use Basic AABB to check collision between boxes
			if (!AABB(entityTargetPosition, entityTargetBottomRight, targetPosition, targetBottomRight)) continue;
			isColliding = true; // Toggles on if collision exists in either direction

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

	bool s_Collisions::CheckCollision(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, Vec2& velocity, bool requireSolid) const {
		c_Position* position1 = positionSystem.GetComponent(entityID);
		
		if (position1 == nullptr) return false;

		bool isColliding = false;

		for (unsigned int otherEntityID = 0; otherEntityID < datasetSize; otherEntityID++) {
			if (otherEntityID == entityID) continue;
			c_Position* position2 = positionSystem.GetComponent(otherEntityID);

			if (position2 == nullptr) continue;
			c_CollisionBox* collisionBox1 = GetCollisionBox(entityID);
			c_CollisionBox* collisionBox2 = GetCollisionBox(otherEntityID);

			if (collisionBox1 != nullptr && collisionBox2 != nullptr) {
				if (IsBoxColliding(collisionBox1, collisionBox2, position1->position, position2->position, velocity, requireSolid))
					isColliding = true;
			}
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
			collisionSystem.CheckCollision(datasetSize, entityID, positionSystem, velocity, true);

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

			c_LayeredAnimatedSprite* layeredAnimatedSprite = spriteSystem.GetLayeredAnimatedSprite(entityID);
			if (layeredAnimatedSprite != nullptr) {
				spriteSystem.PlayAnimationAll(layeredAnimatedSprite, animationIdx);
			}
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

	void s_Sprites::PlayAnimationLayer(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int layerID, unsigned int animationIdx, unsigned int startFrame) {
		c_AnimatedSprite* layer = layeredAnimatedSprite->layers[layerID];

		PlayAnimation(layer, animationIdx, startFrame);
	}

	void s_Sprites::PlayAnimationAll(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int animationIdx, unsigned int startFrame) {
		for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
			PlayAnimationLayer(layeredAnimatedSprite, i, animationIdx, startFrame);
		}
	}

	void s_Sprites::SyncAnimationLayerFrames(c_LayeredAnimatedSprite* layeredAnimatedSprite) {
		unsigned int parentLayerIdx = layeredAnimatedSprite->parentLayerIdx;

		c_AnimatedSprite* parentLayer = layeredAnimatedSprite->layers[parentLayerIdx];

		for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
			if (i == parentLayerIdx) continue;

			PlayAnimationLayer(layeredAnimatedSprite, i, parentLayer->currentAnimation, parentLayer->animationFrameIndex);
		}
	}

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

			c_LayeredAnimatedSprite* layeredAnimatedSprite = GetLayeredAnimatedSprite(entityID);
			if (layeredAnimatedSprite != nullptr) {
				SyncAnimationLayerFrames(layeredAnimatedSprite);
				
				for (unsigned int i = 0; i < layeredAnimatedSprite->numberOfLayers; i++) {
					renderer->UpdateAnimationFrame(deltaTime, layeredAnimatedSprite->layers[i]);
				}

				continue;
			}
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

				c_LayeredSprite* layeredSprite = GetLayeredSprite(entityID);
				if (layeredSprite != nullptr) {
					if (layeredSprite->drawLayer != drawLayer) continue;

					for (unsigned int idx = 0; idx < layeredSprite->numberOfLayers; idx++) {
						DrawSprite(layeredSprite->layers[idx], positionSystem.GetComponent(entityID), renderer);
					}
				}

				c_AnimatedSprite* animatedSprite = GetAnimatedSprite(entityID);
				if (animatedSprite != nullptr) {
					if (animatedSprite->drawLayer != drawLayer) continue;

					DrawAnimatedSprite(
						animatedSprite,
						positionSystem.GetComponent(entityID),
						renderer
					);
				}

				c_LayeredAnimatedSprite* layeredAnimatedSprite = GetLayeredAnimatedSprite(entityID);
				if (layeredAnimatedSprite != nullptr) {
					if (layeredAnimatedSprite->drawLayer != drawLayer) continue;

					for (unsigned int idx = 0; idx < layeredAnimatedSprite->numberOfLayers; idx++) {
						DrawAnimatedSprite(layeredAnimatedSprite->layers[idx], positionSystem.GetComponent(entityID), renderer);
					}
				}
			}
		}
	}

	void s_Input::Update(unsigned int datasetSize, GamesEngineeringBase::Window& window, s_Movement& movementSystem) {
		for (unsigned int entityID = 0; entityID < datasetSize; entityID++) {
			c_Input* input = GetComponent(entityID);
			if (input == nullptr) continue;

			c_Movement* movement = movementSystem.GetComponent(entityID);
			if (movement == nullptr) continue;
			Vec2 velocity = { 0, 0 };

			velocity += {window.keyPressed(input->keyRight), window.keyPressed(input->keyDown)};
			velocity -= {window.keyPressed(input->keyLeft), window.keyPressed(input->keyUp)};
			movement->velocity = velocity.Normalize();
		}
	}
}