#include "Game.h"
#include "AppHost.h"
#include "SwapChain.h"
#include "Renderer.h"
#include "Grid.h"
#include "Sequence.h"
#include "SceneContext.h"
#include "RenderContext.h"
#include "Command.h"
#include "Input.h"
#include "Simulator.h"
#include "Renderer.h"
#include "Profiler.h"
#include "FrameLimiter.h"
#include "ResourceManager.h"
#include "Map.h"

#include <tuple>
#include <vector>
#include <queue>
#include <chrono>

using namespace cpp_conv::resources;

void cpp_conv::game::run()
{
	srand((unsigned int)time(NULL));

	int iWidth, iHeight;
	std::tie(iWidth, iHeight) = cpp_conv::apphost::getAppDimensions();

	cpp_conv::renderer::SwapChain swapChain(iWidth, iHeight);
	cpp_conv::renderer::init(swapChain);

	resource_manager::initialize();
	AssetPtr<Map> map = resource_manager::loadAsset<Map>(registry::data::MapSimple);

	std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(map->GetGrid(), map->GetConveyors());
	cpp_conv::SceneContext kSceneContext =
	{ 
		{ 0, 0 }, 
		map->GetGrid(),
		sequences,
		map->GetConveyors(),
		map->GetOtherEntities(),
		{ std::chrono::high_resolution_clock::now() }
	};

	cpp_conv::RenderContext kRenderContext = { 0, 0, swapChain.GetWriteSurface(), map->GetGrid() };

	cpp_conv::FrameLimiter frameLimter(10);
	std::queue<cpp_conv::commands::InputCommand> commands;

	frameLimter.Start();
	while (true)
	{
		PROFILE(Input, cpp_conv::input::receiveInput(commands));
		PROFILE(CommandProcess, cpp_conv::command::processCommands(kSceneContext, commands));
		PROFILE(Simulation, cpp_conv::simulation::simulate(kSceneContext));
		PROFILE(ResizeSwap, [&swapChain]() {
			int iNewWidth;
			int iNewHeight;
			std::tie(iNewWidth, iNewHeight) = cpp_conv::apphost::getAppDimensions();
			if (swapChain.RequiresResize(iNewWidth, iNewHeight))
			{
				swapChain.ResizeBuffers(iNewWidth, iNewHeight);
			}
		}());
		PROFILE(Render, cpp_conv::renderer::render(kSceneContext, kRenderContext));
		PROFILE(Present, swapChain.SwapAndPresent());

		PROFILE(FrameCapSleep, frameLimter.Limit())
		frameLimter.EndFrame();
	}
}
