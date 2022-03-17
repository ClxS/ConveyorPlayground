project "eigen"
	kind "Utility"
	files {
		"eigen.lua",
		"eigen/Eigen/**",
	}

	exports {
		["includedirs"]	= path.getabsolute("eigen"),
	}
