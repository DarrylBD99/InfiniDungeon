#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "LivingEntity.h"

namespace GameBase {
	class PlayerEntity : LivingEntity {
	private:
		unsigned int mana;
	public:
		unsigned int GetMana() const { return mana; }
		void DepleteMana(unsigned int depleated_mana) { mana -= depleated_mana; }
		void ReplenishMana(unsigned int replenished_mana) { mana += replenished_mana; }
	};
}

#endif // !PLAYER_ENTITY_H
