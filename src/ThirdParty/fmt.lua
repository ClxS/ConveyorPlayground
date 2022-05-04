project "fmt"
	kind "StaticLib"
	files {
		"fmt.lua",
		"fmt/include/**",
		"fmt/src/**",
	}
	exports {
		["includedirs"]	= path.getabsolute("fmt/include"),
	}
