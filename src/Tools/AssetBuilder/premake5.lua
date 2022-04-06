project "AssetBuilder"
	platforms { "Tools_Win64" }
	kind "ConsoleApp"
	targetdir(path.getabsolute("../../../bin/tools"))
	targetname "AssetBuilder"
	language "C++"
	debugdir "$(TargetDir)"
	files {
		"**",
	}
	includedirs {
	    "**",
	}
	links {
		"tomlcpp_tools",
		"ToolsCore",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"
