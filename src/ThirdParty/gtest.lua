project "gtest"
	kind "StaticLib"
	files {
		"gtest.lua",
		"googletest/googletest/src/**",
	}
    includedirs {
        "googletest/googletest",
    }

	exports {
		["includedirs"]	= path.getabsolute("googletest/googletest/include"),
	}
