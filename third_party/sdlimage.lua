project "SDLImage"
	kind "None"
	filter {"platforms:SDL"}
		kind "StaticLib"
		links {
		}
		defines {
			'LOAD_BMP',
		}
		includedirs {
			"SDLImage",
			"SDL/include",
		}
		files {
			"SDLImage/*.c",
			"SDLImage/*.h",
		}
		flags { "MultiProcessorCompile" }
	filter {}
