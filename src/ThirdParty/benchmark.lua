project "benchmark"
	kind "StaticLib"
	files {
		"benchmark.lua",
		"benchmark/src/**",
		"benchmark/include/**",
		"AtlasShims/benchmark/include/**",
	}
    includedirs {
        "benchmark/include",
    }
    defines {
        "BENCHMARK_STATIC_DEFINE",
    }

	exports {
		["includedirs"]	= {
		    path.getabsolute("benchmark/include"),
		    path.getabsolute("AtlasShims/benchmark/include"),
        }
	}
