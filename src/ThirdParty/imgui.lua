project "imgui"
	kind "None"
	filter {"platforms:SDL"}
		kind "StaticLib"
		includedirs {
			"imgui",
			"SDL/include",
		}
		files {
			"imgui/*.cpp",
			"imgui/*.h",
			"imgui/misc/cpp/*.cpp",
			"imgui/misc/cpp/*.h",
			"imgui/misc/debuggers/*.natvis",
			"imgui/backends/*_impl_sdl*",
		}
		flags { "MultiProcessorCompile" }

		exports {
			["includedirs"]	= path.getabsolute("imgui"),
		}
	filter {}
