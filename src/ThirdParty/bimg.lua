project "astc"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/astc/**",
    }
	exports {
		["includedirs"]	= path.getabsolute("bimg/3rdparty/astc")
	}

project "astc-codec"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/astc-codec/include/**",
		"bimg/3rdparty/astc-codec/src/**",
		"bimg/3rdparty/astc-codec/src/**.cc",
    }
    removefiles {
		"bimg/3rdparty/astc-codec/src/**/test/**",
    }
    includedirs {
        "bimg/3rdparty/astc-codec"
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bimg/3rdparty/astc-codec/include"),
        }
	}

project "edtaa3"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/edtaa3/**",
    }
	exports {
		["includedirs"]	= path.getabsolute("bimg/3rdparty/edtaa3")
	}

project "etc1"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/etc1/**",
    }
	exports {
		["includedirs"]	= path.getabsolute("bimg/3rdparty/etc1")
	}

project "etc2"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/etc2/**",
    }
    links {
        "bx",
    }

project "iqa"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/iqa/**",
    }
	exports {
		["includedirs"]	= path.getabsolute("bimg/3rdparty/iqa/include")
	}

project "lodepng"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/lodepng/**",
    }

-- TODO[CJones] We need to fix this. bx has changed nvtt to depend on bx which is not okay
project "nvtt"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/nvtt/**",
    }
    includedirs {
        "bimg/3rdparty/nvtt"
    }
    links {
        "bx"
    }

project "libsquish"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/libsquish/**",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bimg/3rdparty/libsquish"),
        }
	}

project "stb"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/stb/**",
    }

project "tinyexr"
    kind "StaticLib"
    files {
		"bimg.lua",
		"bimg/3rdparty/tinyexr/**",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bimg/3rdparty/tinyexr"),
		    path.getabsolute("bimg/3rdparty/tinyexr/deps/miniz"),
        }
	}

project "bimg"
	kind "StaticLib"
	files {
		"bimg.lua",
		"bimg/src/**",
		"bimg/include/**",
	}
	exports {
		["includedirs"]	= {
		    path.getabsolute("bimg/include"),
        }
	}
	includedirs {
        path.getabsolute("bimg/include"),
        path.getabsolute("bimg/3rdparty"),
	}
    links {
        "astc-codec",
        "libsquish",
        "tinyexr",
        "bx",
        "iqa",
        "tinyexr",
    }
