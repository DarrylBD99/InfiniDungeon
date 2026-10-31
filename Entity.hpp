#pragma once

namespace WM908_Assessment {
	class Entity {
	private:
		unsigned int id;

	public:
		Entity(unsigned int id_) : id(id_) {}
		~Entity() = default;

		const unsigned int GetEntityID() const { return id; }
		void SetEntityID(int id_) { id = id_; }
	};
}
