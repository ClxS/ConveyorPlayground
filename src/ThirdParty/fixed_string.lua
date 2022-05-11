project "fixed_string"
	kind "Utility"
	files {
		"fixed_string/include/**",
	}

	exports {
		["includedirs"]	= path.getabsolute("fixed_string/include"),
	}