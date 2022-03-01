project "tomlcpp"
	kind "StaticLib"
	files {
		"tomlcpp/toml.h",
		"tomlcpp/toml.c",
		"tomlcpp/tomlcpp.hpp",
		"tomlcpp/tomlcpp.cpp",
	}

	exports {
		["includedirs"]	= path.getabsolute("tomlcpp"),
	}