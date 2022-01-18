project "SDLImage"
	kind "None"
	filter {"platforms:SDL"}
		kind "StaticLib"
		defines {
			'LOAD_BMP',
		}
		includedirs {
			"SDL_Image",
			"SDL/include",
		}
		files {
			"SDL_Image/*.c",
			"SDL_Image/*.h",
		}
		flags { "MultiProcessorCompile" }
	filter {}
