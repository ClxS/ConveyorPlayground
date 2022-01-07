#pragma once
#include "Entity.h"

namespace cpp_conv
{
	class Storage : public Entity
	{
	public:
		Storage(int x, int y);

		void Tick(const SceneContext& kContext) override;

		void Draw(RenderContext& kRenderContext) const override;
	};
}