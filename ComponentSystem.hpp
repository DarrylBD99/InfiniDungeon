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
		//c_Position* AddComponent(unsigned int entityID, const Vec2 position) {
		//	c_Position* component = ComponentSystem<c_Position>::AddComponent(entityID);
		//	component->position = position;

		//	return component;
		//}

	};

	class s_Movement : public ComponentSystem<c_Movement> {
	public:
		void Update(unsigned int datasetSize, float deltaTime, s_Position& positionSystem);
	};

	class s_Collisions {
	private:
		ComponentSystem<c_CollisionBox> collisionBox;
		ComponentSystem<c_CollisionCircle> collisionCircle;
	public:
		c_CollisionBox* AddCollisionBox(unsigned int entityID) {
			c_CollisionBox* component = collisionBox.AddComponent(entityID);
			return component;
		}

		c_CollisionCircle* AddCollisionCircle(unsigned int entityID) {
			c_CollisionCircle* component = collisionCircle.AddComponent(entityID);
			return component;
		}

		c_CollisionBox* GetCollisionBox(unsigned int entityID) const {
			return collisionBox.GetComponent(entityID);
		}

		c_CollisionCircle* GetCollisionCircle(unsigned int entityID) const {
			return collisionCircle.GetComponent(entityID);
		}

		void DeleteCollisionComponent(unsigned int entityID) {
			collisionBox.DeleteComponent(entityID);
			collisionCircle.DeleteComponent(entityID);
		}

		void DeleteComponents(unsigned int entityID[], unsigned int size) {
			collisionBox.DeleteComponents(entityID, size);
			collisionCircle.DeleteComponents(entityID, size);
		}
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

	class s_4Direction : public ComponentSystem<c_4Direction> {
	public:
		void UpdateDirection(c_4Direction* direction, c_Movement* movement);
		unsigned int GetAnimationIndex(c_4Direction* direction, c_Movement* movement);

		void Update(unsigned int datasetSize, s_Movement& movementSystem, s_Sprites& spriteSystem);
	};

	class s_Input : public ComponentSystem<c_Input> {
	public:
		void Update(unsigned int datasetSize, GamesEngineeringBase::Window& window, s_Movement& movementSystem);
	};
}