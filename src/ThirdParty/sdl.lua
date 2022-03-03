project "SDL"
	kind "None"
	filter {"platforms:SDL"}
		kind "StaticLib"
		links {
			"winmm.lib",
			"d3d11.lib",
			"setupapi.lib",
			"imm32.lib",
			"version.lib",
		}
		includedirs {
			"SDL/include",
		}
		files {
			"SDL/include/*",
			"SDL/src/*",
			"SDL/src/*/*",
			"SDL/src/*/windows/*",
			"SDL/src/audio/wasapi/*",
			"SDL/src/audio/directsound/*",
			"SDL/src/audio/winmm/*",
			"SDL/src/audio/disk/*",
			"SDL/src/audio/dummy/*",
			"SDL/src/joystick/virtual/*",
			"SDL/src/joystick/hidapi/*",
			"SDL/src/render/direct3d/*",
			"SDL/src/render/direct3d11/*",
			"SDL/src/render/opengl/*",
			"SDL/src/render/opengles2/*",
			"SDL/src/render/software/*",
			"SDL/src/thread/generic/*",
			"SDL/src/video/yuv2rgb/*",
			"SDL/src/video/dummy/*",
		}		
		removefiles {
			"**winrt.h",
			"**winrt.cpp",
		}
		defines {
			'HAVE_LIBC',
		}
		flags { "MultiProcessorCompile" }
		
		exports {
			["includedirs"]	= path.getabsolute("SDL/include"),
		}
	filter {}
