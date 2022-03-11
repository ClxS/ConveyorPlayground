project "eigen"
	kind "StaticLib"
	files {
		"eigen.lua",
		"eigen/Eigen/**",
	}

	exports {
		["includedirs"]	= path.getabsolute("eigen"),
	}
