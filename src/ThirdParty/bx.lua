project "bx"
	kind "StaticLib"
	files {
		"bx.lua",
		"bx/src/amalgamated.cpp",
		"bx/include/**",
	}
	removefiles {
	    "bgfx/src/amalgamated.cpp",
	}
	defines {
	    "__STDC_LIMIT_MACROS",
        "__STDC_FORMAT_MACROS",
        "__STDC_CONSTANT_MACROS",
	}
	includedirs {
	    "bx/3rdparty",
	}
	exports {
		["includedirs"]	= path.getabsolute("bx/include"),
	}
	filter { "configurations:Debug" }
	    exports {
    		["defines"]	= "BX_CONFIG_DEBUG=1",
    	}
    filter { "configurations:Release" }
	    exports {
    		["defines"]	= "BX_CONFIG_DEBUG=0",
    	}
    filter {}

    filter { "system:windows" }
        exports {
            ["includedirs"]	= {
                path.getabsolute("bx/include"),
                path.getabsolute("bx/include/compat/msvc"),
            }
        }
    filter {}
