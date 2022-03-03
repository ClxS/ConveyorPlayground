project "ToolsCore"
	platforms { "Tools_Win64" }
	kind "StaticLib"
	targetdir "bin/tools/lib"
	language "C++"
	files {
		"**",
	}
	includedirs {
		"**",
	}
	flags { "FatalWarnings" }
	cppdialect "C++latest"

	exports {
		["includedirs"]	= path.getabsolute("include"),	
	}