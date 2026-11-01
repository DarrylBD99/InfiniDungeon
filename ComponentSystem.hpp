#pragma once
#include "Entity.hpp"
#include "Component.hpp"
#include "Dataset.hpp"
#include "Renderer.hpp"

namespace WM908_Assessment {

	class EntityManager {
	private:
		unsigned int datasetSize;
		Dataset::Array<Entity*> entityDataset;
		void ResetDatasetSize();

	public:
		EntityManager();
		~EntityManager();

		bool EntityExist(unsigned int id) const;

		unsigned int GetDatasetSize() const;

		unsigned int AppendEntity();
		Entity* GetEntityData(unsigned int entityID) const;
		void RemoveEntity(unsigned int id);
		void RemoveEntities(unsigned int id[], unsigned int size);
	};

	template<class C>
	class ComponentSystem {
	protected:
		Dataset::Array<C*> components;
	public:

		C* AddComponent(unsigned int entityID) {
			C* component = new C();
			components.SetValue(component, entityID);
			return component;
		}

		C* GetComponent(unsigned int entityID) const {
			return components.Get(entityID);
		}

		void DeleteComponent(unsigned int entityID) {
			C* component = components.Get(entityID);
			if (components.Get(entityID) == nullptr) return;

			delete component;
			components.SetValue(nullptr, entityID);
		}

		void DeleteComponents(unsigned int entityID[], unsigned int size) {
			for (unsigned int idx = 0; idx < size; idx++) {
				DeleteComponent(entityID[idx]);
			}
		}

		~ComponentSystem() {
			for (C* component : components) {
				if (component == nullptr) continue;

				delete component;
			}
		}
	};

	class s_Position : public ComponentSystem<c_Position> {
	public:
		c_Position* AddComponent(unsigned int entityID, const Vec2& position) {
			c_Position* component = ComponentSystem<c_Position>::AddComponent(entityID);
			component->position = position;

			return component;
		}

	};

	// I wanted to add different shapes of collisions, but it is much cheaper and simpler to stick to one collision shape
	class s_Collisions {
	private:
		ComponentSystem<c_CollisionBox> collisionBox;
		//ComponentSystem<c_CollisionCircle> collisionCircle;
	public:
		bool showCollisions;

		c_CollisionBox* AddCollisionBox(unsigned int entityID, Vec2 boxSize, Vec2 offset = { 0.0f, 0.0f }, unsigned int collisionLayer = 0, unsigned int collisionMask = 0) {
			c_CollisionBox* component = collisionBox.AddComponent(entityID);
			component->boxSize = boxSize;
			component->offset  = offset;
			component->collisionLayer = collisionLayer;
			component->collisionMask = collisionMask;

			return component;
		}

		//c_CollisionCircle* AddCollisionCircle(unsigned int entityID, float radius, bool isSolid = false) {
		//	c_CollisionCircle* component = collisionCircle.AddComponent(entityID);
		//	component->radius = radius;
		//	component->isSolid = isSolid;
		//	return component;
		//}

		c_CollisionBox* GetCollisionBox(unsigned int entityID) const {
			return collisionBox.GetComponent(entityID);
		}

		//c_CollisionCircle* GetCollisionCircle(unsigned int entityID) const {
		//	return collisionCircle.GetComponent(entityID);
		//}

		void DeleteCollisionComponent(unsigned int entityID) {
			collisionBox.DeleteComponent(entityID);
			//collisionCircle.DeleteComponent(entityID);
		}

		void DeleteCollisionComponents(unsigned int entityID[], unsigned int size) {
			collisionBox.DeleteComponents(entityID, size);
			//collisionCircle.DeleteComponents(entityID, size);
		}

		void DrawCollisionBox(unsigned int datasetSize, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) const;
		
		bool CanCollide(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox) const;
		bool AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const;
		bool IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition, Vec2& velocity) const;
		bool CheckCollision(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, Vec2& velocity) const;
	};

	class s_Movement : public ComponentSystem<c_Movement> {
	public:
		void Update(unsigned int datasetSize, float deltaTime, s_Position& positionSystem, s_Collisions& collisionSystem);
	};

	class s_Sprites {
	private:
		ComponentSystem<c_Sprite> sprite;
		ComponentSystem<c_LayeredSprite> layeredSprite;
		ComponentSystem<c_AnimatedSprite> animatedSprite;
		ComponentSystem<c_LayeredAnimatedSprite> layeredAnimatedSprite;
	public:
		c_Sprite* AddSprite(unsigned int entityID, unsigned int spritesheetID, unsigned int drawLayer) {
			c_Sprite* component = sprite.AddComponent(entityID);
			component->spritesheetID = spritesheetID;
			component->drawLayer = drawLayer;
			return component;
		}
		c_LayeredSprite* AddLayeredSprite(unsigned int entityID, unsigned int spritesheetIDs[], unsigned int numberOfLayers, unsigned int drawLayer) {
			c_LayeredSprite* component = layeredSprite.AddComponent(entityID);
			for (unsigned int idx = 0; idx < numberOfLayers; idx++) {
				component->AddLayer(spritesheetIDs[idx]);
			}
			
			component->drawLayer = drawLayer;
			return component;
		}

		c_AnimatedSprite* AddAnimatedSprite(unsigned int entityID, unsigned int drawLayer) {
			c_AnimatedSprite* component = animatedSprite.AddComponent(entityID);
			component->drawLayer = drawLayer;
			return component;
		}
		
		c_LayeredAnimatedSprite* AddLayeredAnimatedSprite(unsigned int entityID, unsigned int drawLayer) {
			c_LayeredAnimatedSprite* component = layeredAnimatedSprite.AddComponent(entityID);
			component->drawLayer = drawLayer;
			return component;
		}

		c_Sprite* GetSprite(unsigned int entityID) const {
			return sprite.GetComponent(entityID);
		}

		c_LayeredSprite* GetLayeredSprite(unsigned int entityID) const {
			return layeredSprite.GetComponent(entityID);
		}

		c_AnimatedSprite* GetAnimatedSprite(unsigned int entityID) const {
			return animatedSprite.GetComponent(entityID);
		}

		c_LayeredAnimatedSprite* GetLayeredAnimatedSprite(unsigned int entityID) const {
			return layeredAnimatedSprite.GetComponent(entityID);
		}

		void DeleteSpriteComponent(unsigned int entityID) {
			sprite.DeleteComponent(entityID);
			layeredSprite.DeleteComponent(entityID);
			animatedSprite.DeleteComponent(entityID);
			layeredAnimatedSprite.DeleteComponent(entityID);
		}

		void DeleteSpriteComponents(unsigned int entityID[], unsigned int size) {
			sprite.DeleteComponents(entityID, size);
			layeredSprite.DeleteComponents(entityID, size);
			animatedSprite.DeleteComponents(entityID, size);
			layeredAnimatedSprite.DeleteComponents(entityID, size);
		}

		void PlayAnimation(c_AnimatedSprite* animatedSprite, unsigned int animationIdx, unsigned int startFrame = 0);
		void PlayAnimationLayer(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int layerID, unsigned int animationIdx, unsigned int startFrame = 0);
		void PlayAnimationAll(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int animationIdx, unsigned int startFrame = 0);
		void SyncAnimationLayerFrames(c_LayeredAnimatedSprite* layeredAnimatedSprite);

		void Update(unsigned int datasetSize, float deltaTime, Renderer* renderer);

		void DrawSprite(const c_Sprite* spriteComponent, const c_Position* positionComponent, Renderer* renderer);
		void DrawAnimatedSprite(const c_AnimatedSprite* animatedSpriteComponent, const c_Position* positionComponent, Renderer* renderer);

		void Draw(unsigned int datasetSize, const s_Position& positionSystem, Renderer* renderer);
	};

	class s_EntityData {
	private:
		ComponentSystem<c_Health> health;
		ComponentSystem<c_Mana> mana;
		ComponentSystem<c_Savable> savable;
	public:

		c_Health* AddHealth(unsigned int entityID, unsigned int maxHealth, unsigned int currentHealth = 0) {
			c_Health* component = health.AddComponent(entityID);
			component->max = maxHealth;
			component->current = (currentHealth > 0) ? currentHealth : maxHealth;
			return component;
		}

		c_Mana* AddMana(unsigned int entityID, unsigned int maxMana, unsigned int currentMana = 0) {
			c_Mana* component = mana.AddComponent(entityID);
			component->max = maxMana;
			component->current = (currentMana > 0) ? currentMana : maxMana;
			return component;
		}

		c_Savable* AddSavable(unsigned int entityID) {
			c_Savable* component = savable.AddComponent(entityID);
			return component;
		}

		c_Health* GetHealth(unsigned int entityID) {
			return health.GetComponent(entityID);
		}

		c_Mana* GetMana(unsigned int entityID) const {
			return mana.GetComponent(entityID);
		}

		bool IsSavable(unsigned int entityID) {
			return (savable.GetComponent(entityID) != nullptr);
		}

		void DeleteDataComponent(unsigned int entityID) {
			health.DeleteComponent(entityID);
			mana.DeleteComponent(entityID);
			savable.DeleteComponent(entityID);

		}

		void DeleteDataComponents(unsigned int entityID[], unsigned int size) {
			health.DeleteComponents(entityID, size);
			mana.DeleteComponents(entityID, size);
			savable.DeleteComponents(entityID, size);
		}

	};

	class s_Triggers {
	private:
		ComponentSystem<c_ActiveState> activeState;
		ComponentSystem<c_PressTrigger> pressTrigger;
		ComponentSystem<c_InteractTrigger> interactTrigger;
		ComponentSystem<c_DeathTrigger> deathTrigger;
		ComponentSystem<c_LinkTrigger> linkTrigger;

	public:
		c_ActiveState* AddActiveState(unsigned int entityID, unsigned int deactiveState, bool isActive = true) {
			c_ActiveState* component = activeState.AddComponent(entityID);
			component->deactiveState = deactiveState;
			return component;
		}

		c_PressTrigger* AddPressTrigger(unsigned int entityID) {
			return pressTrigger.AddComponent(entityID);
		}

		c_InteractTrigger* AddInteractTrigger(unsigned int entityID) {
			return interactTrigger.AddComponent(entityID);
		}

		c_DeathTrigger* AddDeathTrigger(unsigned int entityID) {
			return deathTrigger.AddComponent(entityID);
		}

		c_LinkTrigger* AddLinkTrigger(unsigned int entityID) {
			return linkTrigger.AddComponent(entityID);
		}

		c_ActiveState* GetActiveState(unsigned int entityID) const {
			return activeState.GetComponent(entityID);
		}

		c_PressTrigger* GetPressTrigger(unsigned int entityID) const {
			return pressTrigger.GetComponent(entityID);
		}

		c_InteractTrigger* GetInteractTrigger(unsigned int entityID) const {
			return interactTrigger.GetComponent(entityID);
		}

		c_DeathTrigger* GetDeathTrigger(unsigned int entityID) const {
			return deathTrigger.GetComponent(entityID);
		}

		c_LinkTrigger* GetLinkTrigger(unsigned int entityID) const {
			return linkTrigger.GetComponent(entityID);
		}

		void DeleteDataComponent(unsigned int entityID) {
			activeState.DeleteComponent(entityID);
			pressTrigger.DeleteComponent(entityID);
			interactTrigger.DeleteComponent(entityID);
			deathTrigger.DeleteComponent(entityID);
			linkTrigger.DeleteComponent(entityID);

		}

		void DeleteDataComponents(unsigned int entityID[], unsigned int size) {
			activeState.DeleteComponents(entityID, size);
			pressTrigger.DeleteComponents(entityID, size);
			interactTrigger.DeleteComponents(entityID, size);
			deathTrigger.DeleteComponents(entityID, size);
			linkTrigger.DeleteComponents(entityID, size);
		}

		void DeactivateState(const unsigned int entityID, s_Sprites& spriteSystem);
	};

	class s_4Direction : public ComponentSystem<c_4Direction> {
	public:
		void UpdateDirection(c_4Direction* direction, c_Movement* movement);
		unsigned int GetAnimationIndex(c_4Direction* direction, c_Movement* movement);

		void Update(unsigned int datasetSize, s_Movement& movementSystem, s_Sprites& spriteSystem);
	};

	class s_PlayerInput : public ComponentSystem<c_PlayerInput > {
	public:
		void Update(unsigned int datasetSize, GamesEngineeringBase::Window& window, s_Movement& movementSystem, s_Sprites& spriteSystem);
	};
}