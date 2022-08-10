include("src/Atlas")

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

setEngineRoot("src/Atlas")
includeEngine()
includeEngineTests()
includeEngineTools()
includeEngineThirdParty()

group("")
	include("src/Game")