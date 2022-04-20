project "CppConveyor"
	language "C++"
	editandcontinue "On"
	debugdir "$(TargetDir)"
	files {
	    "premake5.lua",
		"**.h",
		"**.cpp",
		"**.natvis",
	}
	filter {"files:Scene/Grid/**"}
	    flags{"ExcludeFromBuild"}
	filter{}
	flags { "FatalWarnings" }
	dependson {
		"AssetBuilder"
	}
	customprops {
		["ToolsDir"] = path.getabsolute('../../bin/tools'),
		["DataDir"] = path.getabsolute('../../data'),
		["CodeDir"] = path.getabsolute('.'),
		["DataNamespace"] = 'cpp_conv::resources::registry',
	}

	if _ACTION == 'vs2022' then
		customtargets {
			[[
			  <Target Name="GenerateAssetSpec" BeforeTargets="InitializeBuildStatus">
			  	<Message Text="Generating Asset Specification" Importance="High" />
			  	<Exec Command="$(ToolsDir)/AssetBuilder.exe -r $(DataDir) --platform $(BuildPlatform) -d --ns $(DataNamespace) -o $(CodeDir)/Generated/AssetRegistry.h" />
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
			"Platform/SDL/**",
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
			"Platform/Console/**",
		}
		links {
			"SDL",
			"SDLImage",
			"imgui",
		}
	filter {}

	links {
		"tomlcpp",
		"eigen",
		"fixed_string",
		"bgfx",
		"RmlUI",

		"AtlasAppHost",
		"AtlasGame",
		"AtlasScene",
		"AtlasResource",
		"AtlasRender",
	}
	includedirs {
		".",
		"**",
	}
