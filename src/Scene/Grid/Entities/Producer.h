#pragma once

#include "Renderer.h"
#include "Entity.h"

namespace cpp_conv
{
	class Item;
	struct SceneContext;
	struct RenderContext;

	class Producer : public Entity
	{
	public:
		Producer(int x, int y, Direction direction, Item* pItem, uint64_t productionRate);

		bool IsReadyToProduce() const;

		Item* ProduceItem();

		Direction GetDirection() const { return m_direction; }

		void Tick(const SceneContext& kContext) override;
		void Draw(RenderContext& kContext) const override;

	private:
		Item* m_pItem;
		Direction m_direction;

		uint64_t m_uiTick;
		uint64_t m_productionRate;
		bool m_bProductionReady;
	};
}