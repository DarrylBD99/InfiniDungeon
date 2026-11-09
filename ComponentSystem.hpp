#pragma once
#include "Entity.hpp"
#include "Component.hpp"
#include "Dataset.hpp"
#include "Renderer.hpp"

#include <random>

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
		void RemoveEntities(Dataset::Array<unsigned int>& ids);
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

		void DeleteComponents(Dataset::Array<unsigned int>& entityIDs) {
			for (unsigned int entityID : entityIDs) {
				DeleteComponent(entityID);
			}
		}

		~ComponentSystem() {
			for (C* component : components) {
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

		Vec2 GetCenterPosition(unsigned int entityID);
		Vec2 GetCenterPosition(Vec2 position);
		Vec2 GetCenterPosition(Vec2 position, c_CollisionBox* collisionBox);
		void UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities, Renderer* renderer);
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

		void DeleteCollisionComponents(Dataset::Array<unsigned int>& entityID) {
			collisionBox.DeleteComponents(entityID);
			//collisionCircle.DeleteComponents(entityID);
		}

		void DrawCollisionBox(unsigned int entityID, s_Position& positionSystem, Renderer* renderer, GamesEngineeringBase::Window& canvas) const;
		
		bool CanCollide(const unsigned int collisionMask, const unsigned int targetCollisionLayer) const;
		bool AABB(const Vec2& entityTopLeft, const Vec2& entityBottomRight, const Vec2& targetTopLeft, const Vec2& targetBottomRight) const;
		
		bool IsBoxColliding(const c_CollisionBox* collisionBox, const c_CollisionBox* targetCollisionBox, const Vec2& position, const Vec2& targetPosition) const;
		void SubtractVelocity(const unsigned int entityID, const unsigned int targetEntityID, const s_Position& positionSystem, Vec2& velocity) const;
		
		Dataset::Array<unsigned int> GetCollidingEntities(unsigned int datasetSize, unsigned int entityID, const s_Position& positionSystem, const c_Movement* movement) const;
	};

	class s_Movement : public ComponentSystem<c_Movement> {
	public:
		c_Movement* AddComponent(unsigned int entityID, const float speed) {
			c_Movement* component = ComponentSystem<c_Movement>::AddComponent(entityID);
			component->speed = speed;

			return component;
		}
		void Update(unsigned int entityID, float deltaTime, Dataset::Array<unsigned int> collidingEntities, s_Position& positionSystem, s_Collisions& collisionSystem);
	};

	class s_Triggers {
	private:
		ComponentSystem<c_ActiveState> activeState;
		ComponentSystem<c_PressTrigger> pressTrigger;
		ComponentSystem<c_InteractTrigger> interactTrigger;
		ComponentSystem<c_TriggerKill> triggerKill;
		ComponentSystem<c_EndGoal> endGoal;

	public:
		c_ActiveState* AddActiveState(unsigned int entityID, unsigned int roomID, unsigned int triggerID, unsigned int activeStateSprite, bool isActive = true) {
			c_ActiveState* component = activeState.AddComponent(entityID);
			component->roomID = roomID;
			component->triggerID = triggerID;
			component->activeStateSprite = activeStateSprite;
			component->isActive = isActive;
			return component;
		}

		c_PressTrigger* AddPressTrigger(unsigned int entityID, unsigned int roomID, unsigned int triggerID) {
			c_PressTrigger* component = pressTrigger.AddComponent(entityID);
			component->roomID = roomID;
			component->triggerID = triggerID;
			return component;
		}

		c_InteractTrigger* AddInteractTrigger(unsigned int entityID, unsigned int roomID, unsigned int triggerID) {
			c_InteractTrigger* component = interactTrigger.AddComponent(entityID);
			component->roomID = roomID;
			component->triggerID = triggerID;
			return component;
		}

		c_EndGoal* AddEndGoal(unsigned int entityID) {
			return endGoal.AddComponent(entityID);
		}

		c_TriggerKill* AddTriggerKill(unsigned int entityID) {
			return triggerKill.AddComponent(entityID);
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

		c_TriggerKill* GetTriggerKill(unsigned int entityID) const {
			return triggerKill.GetComponent(entityID);
		}

		c_EndGoal* GetEndGoal(unsigned int entityID) {
			return endGoal.GetComponent(entityID);
		}

		void DeleteTriggerComponent(unsigned int entityID) {
			activeState.DeleteComponent(entityID);
			pressTrigger.DeleteComponent(entityID);
			interactTrigger.DeleteComponent(entityID);
			triggerKill.DeleteComponent(entityID);
			endGoal.DeleteComponent(entityID);

		}

		void DeleteTriggerComponents(Dataset::Array<unsigned int>& entityID) {
			activeState.DeleteComponents(entityID);
			pressTrigger.DeleteComponents(entityID);
			interactTrigger.DeleteComponents(entityID);
			triggerKill.DeleteComponents(entityID);
			endGoal.DeleteComponents(entityID);
		}

		bool TriggerAction(const unsigned int entityID);
		bool UpdatePressTrigger(const unsigned int entityID, Dataset::Array<unsigned int> collidingEntities);
		void UnloadKillTrigger(const unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities);
		bool IsEndGoalReached(const unsigned int entityID);
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

		void DeleteSpriteComponents(Dataset::Array<unsigned int>& entityID) {
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

		void DrawSprite(const c_Sprite* spriteComponent, const c_Position* positionComponent, c_ActiveState* activeState, Renderer* renderer);
		void DrawAnimatedSprite(const c_AnimatedSprite* animatedSpriteComponent, const c_Position* positionComponent, Renderer* renderer);

		void Draw(unsigned int entityID, unsigned int drawLayer, const c_Position* position, c_ActiveState* activeState, Renderer* renderer);
	};

	class s_EntityData {
	private:
		ComponentSystem<c_Health> health;
		ComponentSystem<c_Mana> mana;
		ComponentSystem<c_KeyCount> keyCount;
		ComponentSystem<c_Collectable> collectable;
		ComponentSystem<c_Follow> follow;
		ComponentSystem<c_DeathSavable> deathSavable;
		ComponentSystem<c_NoOutsideCamera> noOutsideCamera;
		ComponentSystem<c_ScoreIncrease> scoreIncrease;
		
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

		c_KeyCount* AddKeyCount(unsigned int entityID) {
			return keyCount.AddComponent(entityID);
		}

		c_Collectable* AddCollectable(unsigned int entityID, CollectableType collectableType) {
			c_Collectable* component = collectable.AddComponent(entityID);
			component->type = collectableType;
			return component;
		}

		c_Follow* AddFollow(unsigned int entityID, float lineOfSight, unsigned int collisionMask) {
			c_Follow* component = follow.AddComponent(entityID);
			component->lineOfSight = lineOfSight;
			component->mask = collisionMask;
			return component;
		}

		c_DeathSavable* AddDeathSavable(unsigned int entityID, unsigned int roomID, unsigned int index) {
			c_DeathSavable* component = deathSavable.AddComponent(entityID);
			component->roomID = roomID;
			component->tileIndex = index;
			return component;
		}

		c_NoOutsideCamera* AddNoOutsideCamera(unsigned int entityID) {
			c_NoOutsideCamera* component = noOutsideCamera.AddComponent(entityID);
			return component;
		}

		c_ScoreIncrease* AddScoreIncrease(unsigned int entityID, unsigned int scoreCount) {
			c_ScoreIncrease* component = scoreIncrease.AddComponent(entityID);
			component->scoreCount = scoreCount;
			return component;
		}

		c_Health* GetHealth(unsigned int entityID) {
			return health.GetComponent(entityID);
		}

		c_Mana* GetMana(unsigned int entityID) const {
			return mana.GetComponent(entityID);
		}

		c_KeyCount* GetKeyCount(unsigned int entityID) const {
			return keyCount.GetComponent(entityID);
		}

		c_Collectable* GetCollectable(unsigned int entityID) {
			return collectable.GetComponent(entityID);
		}

		c_Follow* GetFollow(unsigned int entityID) {
			return follow.GetComponent(entityID);
		}

		c_ScoreIncrease* GetScoreIncrease(unsigned int entityID) {
			return scoreIncrease.GetComponent(entityID);
		}

		c_NoOutsideCamera* GetNoOutsideCamera(unsigned int entityID) {
			return noOutsideCamera.GetComponent(entityID);
		}

		c_DeathSavable* GetDeathSavable(unsigned int entityID) {
			return deathSavable.GetComponent(entityID);
		}

		void DeleteDataComponent(unsigned int entityID) {
			health.DeleteComponent(entityID);
			mana.DeleteComponent(entityID);
			keyCount.DeleteComponent(entityID);
			follow.DeleteComponent(entityID);
			deathSavable.DeleteComponent(entityID);
			collectable.DeleteComponent(entityID);
			noOutsideCamera.DeleteComponent(entityID);
			scoreIncrease.DeleteComponent(entityID);
		}

		void DeleteDataComponents(Dataset::Array<unsigned int>& entityID) {
			health.DeleteComponents(entityID);
			mana.DeleteComponents(entityID);
			keyCount.DeleteComponents(entityID);
			follow.DeleteComponents(entityID);
			deathSavable.DeleteComponents(entityID);
			collectable.DeleteComponents(entityID);
			noOutsideCamera.DeleteComponents(entityID);
			scoreIncrease.DeleteComponents(entityID);
		}

		bool DamageHealth(unsigned int entityID, unsigned int damage);
		bool DrainMana(unsigned int entityID, unsigned int manaDrain, bool forceDrain = false);
		
		bool GetNearestEntity(
			unsigned int datasetSize,
			float lineOfSight,
			const Vec2& centerPosition,
			const unsigned int collisionLayer,
			s_Position& positionSystem,
			s_Collisions& collisionSystem,
			unsigned int& closestEntityID);

		bool GetNearestEntityPosition(
			unsigned int datasetSize,
			float lineOfSight,
			const Vec2& position,
			const unsigned int collisionLayer,
			s_Position& positionSystem,
			s_Collisions& collisionSystem,
			Vec2& targetCenterPosition);

		void UnloadEntities(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities);
		
		void CheckOutsideCamera(unsigned int entityID, Renderer* renderer, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem);
		void UpdateHealth(unsigned int entityID, float deltaTime);
		void UpdateMana(unsigned int entityID, float deltaTime);
		void UpdateCollectible(unsigned int entityID, Dataset::Array<unsigned int> collidingEntities);
		void UpdateFollow(unsigned int entityID, unsigned int datasetSize, s_Position& positionSystem, s_Movement& movementSystem, s_Collisions& collisionSystem);

		void UpdateFollowCounter(float deltaTime);
		void ResetFollowCounter();

		bool SaveDeathState(unsigned int entityID, Dataset::Array<unsigned int>& unloadedEntities, s_Triggers& triggers);
	};

	class s_Attacks {
	private:
		ComponentSystem<c_Melee> melee;
		ComponentSystem<c_Projectile> projectile;
		ComponentSystem<c_AttackProximity> attackProximity;
		ComponentSystem<c_DamageBox> damageBox;
		ComponentSystem<c_Dash> dash;
		ComponentSystem<c_Immunity> immunity;

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

		c_AttackProximity* AddAttackProximity(unsigned int entityID, float attackRange, float attackCooldown = 0.8f) {
			c_AttackProximity* component = attackProximity.AddComponent(entityID);
			component->attackRange = attackRange;
			component->attackCooldown = attackCooldown;
			component->attackCooldownCounter = attackCooldown;
			return component;
		}

		c_DamageBox* AddDamageBox(unsigned int entityID, unsigned int damage, bool isOneTime = false) {
			c_DamageBox* component = damageBox.AddComponent(entityID);
			component->damageCount = damage;
			component->isOneTime = isOneTime;
			return component;
		}

		c_Dash* AddDash(unsigned int entityID, float speed, float duration, unsigned int manaConsume) {
			c_Dash* component = dash.AddComponent(entityID);
			component->speed = speed;
			component->duration = duration;
			component->manaConsume = manaConsume;
			return component;
		}

		c_Immunity* AddImmunity(unsigned int entityID, int maxHits, unsigned int manaConsume) {
			c_Immunity* component = immunity.AddComponent(entityID);
			component->maxHits = maxHits;
			component->manaConsume = manaConsume;
			return component;
		}

		c_Melee* GetMelee(unsigned int entityID) const {
			return melee.GetComponent(entityID);
		}

		c_Projectile* GetProjectile(unsigned int entityID) const {
			return projectile.GetComponent(entityID);
		}

		c_AttackProximity* GetAttackProximity(unsigned int entityID) {
			return attackProximity.GetComponent(entityID);
		}

		c_DamageBox* GetDamageBox(unsigned int entityID) {
			return damageBox.GetComponent(entityID);
		}

		c_Dash* GetDash(unsigned int entityID) {
			return dash.GetComponent(entityID);
		}

		c_Immunity* GetImmunity(unsigned int entityID) const {
			return immunity.GetComponent(entityID);
		}

		void DeleteAttackComponent(unsigned int entityID) {
			melee.DeleteComponent(entityID);
			projectile.DeleteComponent(entityID);
			attackProximity.DeleteComponent(entityID);
			damageBox.DeleteComponent(entityID);
			immunity.DeleteComponent(entityID);
		}

		void DeleteAttackComponents(Dataset::Array<unsigned int>& entityID) {
			melee.DeleteComponents(entityID);
			projectile.DeleteComponents(entityID);
			attackProximity.DeleteComponents(entityID);
			damageBox.DeleteComponents(entityID);
			immunity.DeleteComponents(entityID);
		}

		void DrawImmunityVFX(unsigned int entityID, s_Position& positionSystem, Renderer* renderer);

		void InstanceMeleeAttack(
			const unsigned int entityID,
			c_Melee* meleeAttack,
			const Vec2 entityPosition,
			const Direction facing,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem);

		void InstanceProjectile(
			const unsigned int entityID,
			c_Projectile* projectile,
			const Vec2 entityPosition,
			Vec2 attackDirection,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem);

		void UpdateAttackProximity(
			unsigned int entityID,
			float deltaTime,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Collisions& collisionSystem,
			s_Movement& movementSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem
		);

		void UpdateDamageBox(
			unsigned int entityID,
			float deltaTime,
			Dataset::Array<unsigned int> collidingEntities,
			s_EntityData& entityDataSystem,
			s_Triggers& triggerSystem);

		void UpdateAutoProjectiles(
			const unsigned int entityID,
			float deltaTime,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_Triggers& triggerSystem);

		void UpdateDash(unsigned int entityID, float deltaTime, s_Movement& movementSystem);

		void UnloadDamageBox(
			unsigned int entityID,
			Dataset::Array<unsigned int>& unloadedEntities);
	};

	class s_Directions {
	private:
		ComponentSystem<c_2Direction> direction_2;
		ComponentSystem<c_4Direction> direction_4;
	public:
		c_2Direction* Add2Direction(unsigned int entityID) {
			c_2Direction* component = direction_2.AddComponent(entityID);
			return component;
		}

		c_4Direction* Add4Direction(unsigned int entityID) {
			c_4Direction* component = direction_4.AddComponent(entityID);
			return component;
		}

		c_2Direction* Get2Direction(unsigned int entityID) const {
			return direction_2.GetComponent(entityID);
		}

		c_4Direction* Get4Direction(unsigned int entityID) const {
			return direction_4.GetComponent(entityID);
		}

		void DeleteDirectionComponent(unsigned int entityID) {
			direction_2.DeleteComponent(entityID);
			direction_4.DeleteComponent(entityID);
		}

		void DeleteDirectionComponents(Dataset::Array<unsigned int>& entityID) {
			direction_2.DeleteComponents(entityID);
			direction_4.DeleteComponents(entityID);
		}

		void UpdateDirection(unsigned int entityID, c_Movement* movement);
		unsigned int GetAnimationIndex(c_4Direction* direction, c_Movement* movement);

		Vec2 DirectionToVec2(Direction dir);
		void Update(unsigned int entityID, s_Movement& movementSystem, s_Sprites& spriteSystem);
	};

	class s_Spawner : public ComponentSystem<c_Spawner> {
	public:
		c_Spawner* AddComponent(unsigned int entityID, EnemyTypes enemyType, float spawnCooldown = 8.0f) {
			c_Spawner* component = ComponentSystem<c_Spawner>::AddComponent(entityID);
			component->type = enemyType;
			component->spawnCooldown = spawnCooldown;

			std::random_device randomDevice;
			std::mt19937 rng(randomDevice());
			std::uniform_real_distribution<float> distribution(0.0f, spawnCooldown);

			component->spawnCooldownCounter = distribution(rng);
			return component;
		}

		void Update(
			unsigned int entityID,
			float deltaTime,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_Sprites& spriteSystem,
			s_Directions& directionSystem,
			s_Collisions& collisionSystem,
			s_Attacks& attackSystem,
			s_EntityData& entityDataSystem);

		void SpawnEnemy(
			Vec2 spawnPosition,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_Sprites& spriteSystem,
			s_Directions& directionSystem,
			s_Collisions& collisionSystem,
			s_Attacks& attackSystem,
			s_EntityData& entityDataSystem,
			EnemyTypes enemyType);

		void SpawnEnemies(
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
			unsigned int maxCount);
	};

	class s_PlayerInput : public ComponentSystem<c_PlayerInput > {
	private:
		void InstancePrimaryAttack(
			unsigned int entityID,
			s_Directions& directionSystem,
			EntityManager& entityManager,
			s_Position& positionSystem,
			s_Movement& movementSystem,
			s_EntityData& entityDataSystem,
			s_Sprites& spriteSystem,
			s_Collisions& collisionSystem,
			s_Attacks& attackSystem);

		void InstanceSecondaryAction(
			unsigned int entityID,
			s_Movement& movementSystem,
			s_Directions& directionSystem,
			s_EntityData& entityDataSystem,
			s_Attacks& attackSystem);

	public:
		bool CanInteract(
			unsigned int entityID,
			unsigned int datasetSize,
			s_Position& positionSystem,
			s_EntityData& entityDataSystem,
			s_Triggers& triggerSystem,
			s_Collisions& collisionSystem,
			unsigned int& closestEntityID);

		void Update(
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
			bool canInteract = false,
			unsigned int interactEntityID = 0);

		void DrawInteractIndicator(
			unsigned entityID,
			Renderer* renderer,
			s_Position& positionSystem);
	};
}