project "bgfx"
	kind "StaticLib"
	files {
		"bgfx.lua",
		"bgfx/src/**",
		"bgfx/include/**",
	}
	removefiles {
	    "bgfx/src/renderer*",
	    "bgfx/src/debug_renderdoc*",
	    "bgfx/src/gl*",
	    "bgfx/src/amalgamated.**",
	}
	exports {
		["includedirs"]	= path.getabsolute("bgfx/include"),
	}
	links {
		"bx",
		"bimg",
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

    filter { "system:windows" }
        files {
            "bgfx/src/renderer_d3d*",
            "bgfx/src/renderer_dx*",
        }
        removefiles {
            "bgfx/src/renderer_d3d12*", -- No DX12 yet
        }
	filter {}
