require "ConveyorModule"

workspace "CppConveyor"
	location "build"
	platforms { --[["Console",]] "SDL" }
	configurations { "Debug", "Release" }
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
			["module"] = {
				"lua/**.lua",
			},
			["premake/*"] = {
				"*.lua",
				"src/**.lua",
			},
		},
		["data/*"] = {
			"data/**",
		}
	}

group("Tools")
    include("src/Tools/Core")
    include("src/Tools/AssetBuilder")

group("")
	include("src/Game")

group("ThirdParty")
	include('src/ThirdParty/sdl.lua')
	include('src/ThirdParty/sdlimage.lua')
	include('src/ThirdParty/imgui.lua')
	include('src/ThirdParty/tomlcpp.lua')
