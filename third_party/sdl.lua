project "SDL"
	kind "None"
	filter {"platforms:SDL"}
		kind "StaticLib"
	filter {}

	includedirs {
		"SDL/include",
	}
	files {
		"SDL/include/*",
		"SDL/src/*",
		"SDL/src/*/*",
		"SDL/src/*/windows/*",
		"SDL/src/*/directsound/*",
	}
	flags { "MultiProcessorCompile" }