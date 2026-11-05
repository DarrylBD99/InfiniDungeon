#pragma once
#include "Entity.hpp"
#include "Component.hpp"
#include "Dataset.hpp"
#include "Renderer.hpp"

namespace WM908_Assessment {

	class EntityManager {
	private:
		Dataset::Array<Entity*> entityDataset;

	public:
		~EntityManager() {
			for (Entity* entity : entityDataset) {
				delete entity;
			}
		}

		bool EntityExist(unsigned int id) const;

		unsigned int GetDatasetSize() const;

		unsigned int AppendEntity();
		Entity* GetEntityData(unsigned int entityID) const;
		void RemoveEntity(unsigned int id);
		void RemoveEntities(Dataset::Array<unsigned int>* ids);
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

		void DeleteComponents(Dataset::Array<unsigned int>* entityIDs) {
			for (unsigned int entityID : *entityIDs) {
				DeleteComponent(entityID);
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

		void UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>* unloadedEntities, Renderer* renderer);
	};

	// I wanted to add different shapes of collisions, but it is much cheaper and simpler to stick to one collision shape
	class s_Collisions {
	private:
		ComponentSystem<c_CollisionBox> collisionBox;
		//ComponentSystem<c_CollisionCircle> collisionCircle;
	public:
		bool showCollisions = false;

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

		void DeleteCollisionComponents(Dataset::Array<unsigned int>* entityID) {
			collisionBox.DeleteComponents(entityID);
			//collisionCircle.DeleteComponents(entityID);
		}

		void DrawCollisionBox(unsigned int entityID, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) const;
		
		bool CanCollide(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox) const;
		bool AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const;
		
		bool IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition) const;
		void SubtractVelocity(const unsigned int entityID, const unsigned int targetEntityID, const s_Position& positionSystem, Vec2& velocity) const;
		
		Dataset::Array<unsigned int> GetCollidingEntities(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, const Vec2 velocity = { 0.0f }, const Dataset::Array<unsigned int>* entityToIgnore = nullptr) const;
	};

	class s_Movement : public ComponentSystem<c_Movement> {
	public:
		c_Movement* AddComponent(unsigned int entityID, const float speed) {
			c_Movement* component = ComponentSystem<c_Movement>::AddComponent(entityID);
			component->speed = speed;

			return component;
		}
		void Update(unsigned int entityID, unsigned int datasetSize, float deltaTime, s_Position& positionSystem, s_Collisions& collisionSystem);
	};

	class s_Sprites {
	private:
		ComponentSystem<c_Sprite> sprite;
		//ComponentSystem<c_LayeredSprite> layeredSprite;
		ComponentSystem<c_AnimatedSprite> animatedSprite;
		//ComponentSystem<c_LayeredAnimatedSprite> layeredAnimatedSprite;
	public:
		c_Sprite* AddSprite(unsigned int entityID, unsigned int spritesheetID, unsigned int drawLayer, Vec2 offset = {}) {
			c_Sprite* component = sprite.AddComponent(entityID);
			component->spritesheetID = spritesheetID;
			component->drawLayer = drawLayer;
			component->offset = offset;
			return component;
		}
		//c_LayeredSprite* AddLayeredSprite(unsigned int entityID, unsigned int spritesheetIDs[], unsigned int numberOfLayers, unsigned int drawLayer, Vec2* offset = nullptr) {
		//	c_LayeredSprite* component = layeredSprite.AddComponent(entityID);
		//	for (unsigned int idx = 0; idx < numberOfLayers; idx++) {
		//		c_Sprite* sprite = component->AddLayer(spritesheetIDs[idx]);
		//		
		//		if ((offset + idx) == nullptr) continue;
		//		sprite->offset = offset[idx];
		//	}
		//	
		//	component->drawLayer = drawLayer;
		//	return component;
		//}

		c_AnimatedSprite* AddAnimatedSprite(unsigned int entityID, unsigned int drawLayer, Vec2 offset = {}) {
			c_AnimatedSprite* component = animatedSprite.AddComponent(entityID);
			component->drawLayer = drawLayer;
			component->offset = offset;
			return component;
		}
		
		//c_LayeredAnimatedSprite* AddLayeredAnimatedSprite(unsigned int entityID, unsigned int drawLayer) {
		//	c_LayeredAnimatedSprite* component = layeredAnimatedSprite.AddComponent(entityID);
		//	component->drawLayer = drawLayer;
		//	return component;
		//}

		c_Sprite* GetSprite(unsigned int entityID) const {
			return sprite.GetComponent(entityID);
		}

		//c_LayeredSprite* GetLayeredSprite(unsigned int entityID) const {
		//	return layeredSprite.GetComponent(entityID);
		//}

		c_AnimatedSprite* GetAnimatedSprite(unsigned int entityID) const {
			return animatedSprite.GetComponent(entityID);
		}

		//c_LayeredAnimatedSprite* GetLayeredAnimatedSprite(unsigned int entityID) const {
		//	return layeredAnimatedSprite.GetComponent(entityID);
		//}

		void DeleteSpriteComponent(unsigned int entityID) {
			sprite.DeleteComponent(entityID);
			//layeredSprite.DeleteComponent(entityID);
			animatedSprite.DeleteComponent(entityID);
			//layeredAnimatedSprite.DeleteComponent(entityID);
		}

		void DeleteSpriteComponents(Dataset::Array<unsigned int>* entityID) {
			sprite.DeleteComponents(entityID);
			//layeredSprite.DeleteComponents(entityID);
			animatedSprite.DeleteComponents(entityID);
			//layeredAnimatedSprite.DeleteComponents(entityID);
		}

		void PlayAnimation(c_AnimatedSprite* animatedSprite, unsigned int animationIdx, unsigned int startFrame = 0);

		// Wanted to make it so that way the player can move while attacking at the same time
		// Didn't like how it felt and couldn't find a different use case, so I decided to scrap it
		// I'm keeping the code here just so you know it happened
		// 
		//void PlayAnimationLayer(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int layerID, unsigned int animationIdx, unsigned int startFrame = 0);
		//void PlayAnimationAll(c_LayeredAnimatedSprite* layeredAnimatedSprite, unsigned int animationIdx, unsigned int startFrame = 0);
		//void SyncAnimationLayerFrames(c_LayeredAnimatedSprite* layeredAnimatedSprite);

		void UpdateAnimationFrame(float deltaTime, c_AnimatedSprite* animatedSprite);
		void Update(unsigned int entityID, float deltaTime, Renderer* renderer);

		void DrawSprite(const c_Sprite* spriteComponent, const c_Position* positionComponent, Renderer* renderer);
		void DrawAnimatedSprite(const c_AnimatedSprite* animatedSpriteComponent, const c_Position* positionComponent, Renderer* renderer);

		void Draw(unsigned int entityID, unsigned int drawLayer, const c_Position* position, Renderer* renderer);
	};

	class s_EntityData {
	private:
		ComponentSystem<c_Health> health;
		ComponentSystem<c_Mana> mana;
		ComponentSystem<c_Collectable> collectable;
		ComponentSystem<c_Follow> follow;
		ComponentSystem<c_Spawner> spawner;
		ComponentSystem<c_Savable> savable;

		float followUpdateCounter = 0.0f;
		float runFollowCode = 0.1f;
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

		c_Collectable* AddCollectable(unsigned int entityID, CollectableType collectableType) {
			c_Collectable* component = collectable.AddComponent(entityID);
			component->type = collectableType;
			return component;
		}

		c_Spawner* AddSpawner(unsigned int entityID, EnemyTypes enemyType) {
			c_Spawner* component = spawner.AddComponent(entityID);
			component->type = enemyType;
			return component;
		}

		c_Follow* AddFollow(unsigned int entityID, float lineOfSight, unsigned int collisionMask) {
			c_Follow* component = follow.AddComponent(entityID);
			component->lineOfSight = lineOfSight;
			component->mask = collisionMask;
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

		c_Collectable* GetCollectable(unsigned int entityID) {
			return collectable.GetComponent(entityID);
		}

		c_Spawner* GetSpawner(unsigned int entityID) {
			return spawner.GetComponent(entityID);
		}

		c_Follow* GetFollow(unsigned int entityID) {
			return follow.GetComponent(entityID);
		}

		bool IsSavable(unsigned int entityID) {
			return (savable.GetComponent(entityID) != nullptr);
		}

		void DeleteDataComponent(unsigned int entityID) {
			health.DeleteComponent(entityID);
			mana.DeleteComponent(entityID);
			spawner.DeleteComponent(entityID);
			follow.DeleteComponent(entityID);
			savable.DeleteComponent(entityID);
			collectable.DeleteComponent(entityID);

		}

		void DeleteDataComponents(Dataset::Array<unsigned int>* entityID) {
			health.DeleteComponents(entityID);
			mana.DeleteComponents(entityID);
			spawner.DeleteComponents(entityID);
			follow.DeleteComponents(entityID);
			savable.DeleteComponents(entityID);
			collectable.DeleteComponents(entityID);
		}

		bool DamageHealth(unsigned int entityID, unsigned int damage);
		bool DrainMana(unsigned int entityID, unsigned int manaDrain, bool forceDrain = false);
		
		Vec2 FollowGetNearestEntityPosition(unsigned int datasetSize, float lineOfSight, const Vec2& position, c_CollisionBox* collisionBox, s_Position& positionSystem, s_Collisions& collisionSystem);
		//void SpawnEnemy(Vec2 originPosition, EntityManager& entityManager, s_Position& positionSystem, )
		
		void UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>* unloadedEntities);
		void Update(unsigned int entityID, unsigned int datasetSize, float deltaTime, Dataset::Array<unsigned int>* playerIDs, unsigned int numberOfPlayers, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem);
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

		void DeleteTriggerComponent(unsigned int entityID) {
			activeState.DeleteComponent(entityID);
			pressTrigger.DeleteComponent(entityID);
			interactTrigger.DeleteComponent(entityID);
			deathTrigger.DeleteComponent(entityID);
			linkTrigger.DeleteComponent(entityID);

		}

		void DeleteTriggerComponents(Dataset::Array<unsigned int>* entityID) {
			activeState.DeleteComponents(entityID);
			pressTrigger.DeleteComponents(entityID);
			interactTrigger.DeleteComponents(entityID);
			deathTrigger.DeleteComponents(entityID);
			linkTrigger.DeleteComponents(entityID);
		}

		void DeactivateState(const unsigned int entityID, s_Sprites& spriteSystem) const;
	};

	class s_Attacks {
	private:
		ComponentSystem<c_Melee> melee;
		ComponentSystem<c_Projectile> projectile;

		Dataset::Array<Entity*> attackInstanceEntityID = Dataset::Array<Entity*>();

	public:
		c_Melee* AddMelee(unsigned int entityID, unsigned int spritesheetID, float instanceDuration, unsigned int collisionMask = 0) {
			c_Melee* component = melee.AddComponent(entityID);
			component->spritesheetID = spritesheetID;
			component->instanceDuration = instanceDuration;
			component->collisionMask = collisionMask;
			return component;
		}

		c_Projectile* AddProjectile(unsigned int entityID, unsigned int spritesheetID, float speed, bool followNearestEnemy = false, unsigned int collisionMask = 0) {
			c_Projectile* component = projectile.AddComponent(entityID);
			component->spritesheetID = spritesheetID;
			component->speed = speed;
			component->followNearestEnemy = followNearestEnemy;
			component->collisionMask = collisionMask;
			
			return component;
		}

		c_Melee* GetMelee(unsigned int entityID) const {
			return melee.GetComponent(entityID);
		}

		c_Projectile* GetProjectile(unsigned int entityID) const {
			return projectile.GetComponent(entityID);
		}

		void DeleteAttackComponent(unsigned int entityID) {
			melee.DeleteComponent(entityID);
			projectile.DeleteComponent(entityID);
		}

		void DeleteAttackComponents(Dataset::Array<unsigned int>* entityID) {
			melee.DeleteComponents(entityID);
			projectile.DeleteComponents(entityID);
		}

		void InstanceMeleeAttack(
			const unsigned int entityID,
			const unsigned int datasetSize,
			c_Melee* meleeAttack,
			const Vec2 entityPosition,
			const Direction facing,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem);

		void InstanceProjectile(
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
			s_Collisions& collisionSystem);
		
		void Update(
			unsigned int entityID,
			unsigned int datasetSize,
			float deltaTime,
			const s_Position& positionSystem,
			const s_Collisions& collisionSystem,
			s_EntityData& entityDataSystem
		);
		
		void DeleteAttackInstance(
			unsigned int entityID,
			unsigned int datasetSize,
			Dataset::Array<unsigned int>* unloadedEntities,
			const s_Position& positionSystem,
			const s_Movement& movementSystem,
			const s_Collisions& collisionSystem,
			s_EntityData& entityDataSystem
		);
	};

	class s_4Direction : public ComponentSystem<c_4Direction> {
	public:
		void UpdateDirection(c_4Direction* direction, c_Movement* movement);
		unsigned int GetAnimationIndex(c_4Direction* direction, c_Movement* movement);

		void Update(unsigned int entityID, s_Movement& movementSystem, s_Sprites& spriteSystem);
	};

	class s_PlayerInput : public ComponentSystem<c_PlayerInput > {
	private:
		void InstancePrimaryAttack(
			unsigned int entityID,
			unsigned int datasetSize,
			s_4Direction& directionSystem,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_Attacks& attackSystem);
	public:
		void Update(
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
			s_Attacks& attackSystem);
	};
}