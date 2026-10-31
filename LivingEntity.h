#ifndef LIVING_ENTITY_H
#define LIVING_ENTITY_H

#include "Entity.h"

namespace GameBase {
	class LivingEntity : Entity {
	protected:
		unsigned int health;

	public:
		unsigned int GetHealth() const { return health; }
		void Damage(unsigned int dmg) { health -= dmg; }
		void Heal(unsigned int regained_health) { health += regained_health; }
	};
}

#endif // !LIVING_ENTITY_H
