project "fmt"
	kind "StaticLib"
	files {
		"fmt.lua",
		"fmt/include/**",
		"fmt/src/os.cc",
		"fmt/src/format.cc",
	}
	exports {
		["includedirs"]	= path.getabsolute("fmt/include"),
	}
