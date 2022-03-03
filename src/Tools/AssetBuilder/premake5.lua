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
	links {
		"ToolsCore",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"