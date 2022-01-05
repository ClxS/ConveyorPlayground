#include "Game.h"
#include "AppHost.h"
#include "SwapChain.h"
#include "Renderer.h"
#include "Grid.h"
#include "Sequence.h"
#include "SceneContext.h"
#include "RenderContext.h"
#include "FileReader.h"
#include "Command.h"
#include "Input.h"
#include "Simulator.h"
#include "Renderer.h"
#include "Profiler.h"
#include "FrameLimiter.h"

#include <tuple>
#include <vector>
#include <queue>
#include <chrono>

void cpp_conv::game::run()
{
	srand((unsigned int)time(NULL));

	int iWidth, iHeight;
	std::tie(iWidth, iHeight) = cpp_conv::apphost::getAppDimensions();

	cpp_conv::renderer::SwapChain swapChain(iWidth, iHeight);
	cpp_conv::renderer::init(swapChain);

	cpp_conv::grid::EntityGrid grid;
	memset(&grid, 0, sizeof(grid));

	std::vector<cpp_conv::Conveyor*> conveyors;
	std::vector<cpp_conv::Entity*> vOtherEntities;
	cpp_conv::file_reader::readFile("data.txt", grid, conveyors, vOtherEntities);

	std::vector<cpp_conv::Sequence> sequences = cpp_conv::InitializeSequences(grid, conveyors);
	cpp_conv::SceneContext kSceneContext =
	{ 
		{ 0, 0 }, 
		grid, 
		sequences,
		conveyors, 
		vOtherEntities, 
		{ std::chrono::high_resolution_clock::now() }
	};

	cpp_conv::RenderContext kRenderContext = { 0, 0, swapChain.GetWriteSurface(), grid };

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
