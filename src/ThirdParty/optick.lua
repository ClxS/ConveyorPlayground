project "optick"
	kind "StaticLib"
	files {
		"optick.lua",
		"optick/src/**",
		"optick/include/**",
	}
	removefiles {
	    "**.d3d12.cpp",
	    "**.vulkan.cpp",
	}
	exports {
		["includedirs"]	= path.getabsolute("optick/include"),
	}
