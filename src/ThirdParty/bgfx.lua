project "bgfx"
	kind "StaticLib"
	files {
		"bgfx.lua",
	    "bgfx/src/amalgamated.**",
		"bgfx/include/**",
	}
	includedirs {
		"bx/include",
        "bx/include/compat/msvc",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/include"),
        },
        ["links"] = {
            "bx",
        }
	}
	links {
        "bimg",
        "bx",
	}
	defines {
        "BGFX_CONFIG_RENDERER_OPENGL=0",
        "BGFX_CONFIG_RENDERER_OPENGLES=0",
        "BGFX_CONFIG_RENDERER_WEBGPU=0",
	}
    filter { "configurations:Debug" }
        defines {
            "BX_CONFIG_DEBUG=0",
        }
	filter { "configurations:Release" }
        defines {
            "BX_CONFIG_DEBUG=0",
        }
    filter {}
