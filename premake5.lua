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

group("Tools")
	project "ToolsCore"
		platforms { "Tools_Win64" }
		kind "StaticLib"
		targetdir "bin/tools/lib"
		language "C++"
		files {
			"tools/Core/src/**",
			"tools/Core/include/**",
		}
		includedirs {
			"tools/Core/src",
			"tools/Core/include",
		}
		flags { "FatalWarnings" }
		cppdialect "C++latest"

	project "AssetBuilder"
		platforms { "Tools_Win64" }
		kind "ConsoleApp"
		targetdir "bin/tools"
		targetname "AssetBuilder"
		location "build/AssetBuilder"
		language "C++"
		debugdir "$(TargetDir)"	
		files {
			"tools/AssetBuilder/src/**",
			"tools/AssetBuilder/include/**",
		}	
		includedirs {
			"tools/Core/include",
			"tools/AssetBuilder/src/**",	
			"tools/AssetBuilder/include/**",		
		}
		links {
			"ToolsCore",
		}
		flags { "FatalWarnings" }
		cppdialect "C++latest"

group("")
	project "CppConveyor"
		location "build/CppConveyor"
		language "C++"
		editandcontinue "On"
		debugdir "$(TargetDir)"
		files {
			".editorconfig",
			"data/**",
			"src/**"
		}
		flags { "FatalWarnings" }
		dependson {
			"AssetBuilder"
		}
		customprops {
			["ToolsDir"] = path.getabsolute('bin/tools'),
			["DataDir"] = path.getabsolute('data'),
			["CodeDir"] = path.getabsolute('src'),
			["DataNamespace"] = 'cpp_conv::resources::registry',			
		}

		if _ACTION == 'vs2022' then
			customtargets {
				[[
				  <Target Name="GenerateAssetSpec" BeforeTargets="InitializeBuildStatus">
				  	<Message Text="Generating Asset Specification" Importance="High" />
				  	<Exec Command="$(ToolsDir)\AssetBuilder.exe -r $(DataDir) --platform $(BuildPlatform) -d --ns $(DataNamespace) -o $(CodeDir)\Generated\AssetRegistry.h" />
				  </Target>
				  <Target Name="DeployAssets" AfterTargets="Build">
				    <Message Text="Deploying Assets" Importance="High" />
				    <Exec Command="$(ToolsDir)\AssetBuilder.exe cook -r $(DataDir) --platform $(BuildPlatform) -o $(TargetDir)data " />
				  </Target>
				]]
			}
		end

		filter {"platforms:Console"}
			kind "ConsoleApp"
			customprops {
				["BuildPlatform"] = 'Console',
			}
			defines {
				"_CONSOLE"
			}
			removefiles {
				"src/Platform/SDL/**",
			}
		filter {"platforms:SDL"}
			kind "WindowedApp"
			customprops {
				["BuildPlatform"] = 'SDL',
			}
			defines {
				"_SDL"
			}
			removefiles {
				"src/Platform/Console/**",
			}
			includedirs {
				"third_party/SDL/include",
				"third_party/SDL_Image",
				"third_party/imgui",
			}
			links {
				"SDL",
				"SDLImage",
				"imgui",
			}
		filter {}

		links {
			"tomlcpp",
		}
		includedirs {
			"src",
			"src/**",
			"third_party/tomlcpp",
		}

		cppdialect "C++latest"
		vpaths
		{
			["*"] = "src",
			["data/*"] = "data/*",
			["data/*"] = "data/common/**",
			["data/platform/*"] = "data/console/**",
		}
		flags { "MultiProcessorCompile" }

group("ThirdParty")
	dofile('third_party/sdl.lua')
	dofile('third_party/sdlimage.lua')
	dofile('third_party/imgui.lua')
	dofile('third_party/tomlcpp.lua')
