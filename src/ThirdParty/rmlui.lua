project "RmlUI"
	kind "StaticLib"
	files {
		"rmlui.lua",
		"RmlUi/Include/**",
		"RmlUi/Source/Core/**",
		"RmlUi/Source/Debugger/**",
		--"RmlUi/Source/Lottie/**",
		--"RmlUi/Source/Lua/**",
		--"RmlUi/Source/SVG/**",
	}
	defines {
		"RMLUI_STATIC_LIB",
	}
    links {
        "freetype",
    }
	exports {
		["includedirs"]	= path.getabsolute("RmlUi/Include"),
		["defines"] = {
		    "RMLUI_STATIC_LIB",
		}
	}
