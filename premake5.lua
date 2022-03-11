require "ConveyorModule"

workspace "CppConveyor"
	location "build"
	platforms { --[["Console",]] "SDL" }
	configurations { "Debug", "Release" }
	cppdialect "C++latest"
    flags { "MultiProcessorCompile" }
	filter { "platforms:SDL" }
		system "Windows"
		architecture "x86_64"
	filter{}

	filter { "platforms:Tools_Win64" }
		configmap {
			["SDL"] = "Tools_Win64",
		}

    filter { "configurations:Release" }
   		optimize "Full"
		flags "NoRuntimeChecks"
		inlining "Auto"
		intrinsics "on"
	filter{}

	solutionitems {
		["build"] = {
			["module/*"] = {
				"lua/**.lua",
			},
			["premake/*"] = {
				"*.lua",
			},
		},
		["data/*"] = {
			"data/**",
		}
	}

group("Tools")
    include("src/Tools/Core")
    include("src/Tools/AssetBuilder")

group("Engine")
    include("src/Engine/AtlasAppHost")
    include("src/Engine/AtlasCore")
    include("src/Engine/AtlasInput")
    include("src/Engine/AtlasMaths")
    include("src/Engine/AtlasRender")
    include("src/Engine/AtlasResource")
    include("src/Engine/AtlasScene")


group("")
	include("src/Game")


group("ThirdParty")
	include('src/ThirdParty/eigen.lua')
	include('src/ThirdParty/freetype.lua')
	include('src/ThirdParty/imgui.lua')
	include('src/ThirdParty/rmlui.lua')
	include('src/ThirdParty/sdl.lua')
	include('src/ThirdParty/sdlimage.lua')
	include('src/ThirdParty/tomlcpp.lua')

group("ThirdParty/bgfx")
	include('src/ThirdParty/bgfx.lua')
	include('src/ThirdParty/bimg.lua')
	include('src/ThirdParty/bx.lua')
