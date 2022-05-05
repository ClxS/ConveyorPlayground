project "bgfx"
	kind "StaticLib"
	files {
		"bgfx.lua",
	    "bgfx/src/amalgamated.**",
		"bgfx/include/**",
		"bgfx/3rdparty/meshoptimizer/src/**",
		--"bgfx/examples/common/*",
		--"bgfx/examples/common/entry/**.h",
		--"bgfx/examples/common/entry/entry.cpp",
	}
	includedirs {
		"bx/include",
        "bx/include/compat/msvc",
        "bgfx/3rdparty",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/include"),
		    path.getabsolute("bgfx/3rdparty/meshoptimizer/src"),
		    --path.getabsolute("bgfx/examples/common"),
		    path.getabsolute("bimg/include"),
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
	    "__STDC_FORMAT_MACROS",
        "BGFX_CONFIG_RENDERER_OPENGL=0",
        "BGFX_CONFIG_RENDERER_OPENGLES=0",
        "BGFX_CONFIG_RENDERER_WEBGPU=0",
        "BGFX_CONFIG_RENDERER_DIRECT3D11=1",
        "BGFX_CONFIG_RENDERER_DIRECT3D12=0",
        "BGFX_CONFIG_RENDERER_VULKAN=0",
	}
    removefiles {
        "**/example-glue.cpp",
    }

project "glsl-optimizer"
    kind "StaticLib"
    files {
		"bgfx.lua",
		"bgfx/3rdparty/glsl-optimizer/**",
    }
    includedirs {
		"bgfx/3rdparty/glsl-optimizer/**",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/3rdparty/glsl-optimizer/src/glsl"),
        }
	}
	removefiles {
        "bgfx/3rdparty/glsl-optimizer/src/glsl/glcpp/glcpp.c",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/glcpp/tests/**",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/glcpp/**.l",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/glcpp/**.y",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/ir_set_program_inouts.cpp",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/main.cpp",
        "bgfx/3rdparty/glsl-optimizer/src/glsl/builtin_stubs.cpp",
        "bgfx/3rdparty/glsl-optimizer/src/node/**",
    }
	filter { "system:windows" }
        defines {
            "__STDC__",
            "__STDC_VERSION__=199901L",
            "strdup=_strdup",
            "alloca=_alloca",
            "isascii=__isascii",
        }
    filter{}

project "glslang"
    kind "StaticLib"
    files {
		"bgfx.lua",
		"bgfx/3rdparty/glslang/glslang/**",
		"bgfx/3rdparty/glslang/hlsl/**",
		"bgfx/3rdparty/glslang/SPIRV/**",
		"bgfx/3rdparty/glslang/OGLCompilersDLL/**",
    }
    includedirs {
		"bgfx/3rdparty",
		"bgfx/3rdparty/glslang",
		"bgfx/3rdparty/glslang/**",
		"bgfx/3rdparty/spirv-tools/include",
    }
    defines {
        "ENABLE_OPT=1",
        "ENABLE_HLSL=1",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/3rdparty/glslang/glslang/Public"),
		    path.getabsolute("bgfx/3rdparty/glslang/glslang/include"),
		    path.getabsolute("bgfx/3rdparty/glslang"),
        }
	}
	removefiles {
		"bgfx/3rdparty/glslang/glslang/OSDependent/Unix/main.cpp",
		"bgfx/3rdparty/glslang/glslang/OSDependent/Windows/main.cpp",

		"gfx/3rdparty/glslang/**spirv_c_interface.cpp",
	}
	filter { "system:windows" }
		removefiles {
			"bgfx/3rdparty/glslang/glslang/OSDependent/Unix/**.cpp",
			"bgfx/3rdparty/glslang/glslang/OSDependent/Unix/**.h",
		}
	filter { "system:not windows" }
		removefiles {
			"bgfx/3rdparty/glslang/glslang/OSDependent/Windows/**.cpp",
			"bgfx/3rdparty/glslang/glslang/OSDependent/Windows/**.h",
		}
    filter{}

project "spirv-opt"
	kind "StaticLib"

	includedirs {
		"bgfx/3rdparty/spirv-headers/include",
		"bgfx/3rdparty/spirv-tools",

		"bgfx/3rdparty/spirv-tools/include",
		"bgfx/3rdparty/spirv-tools/include/generated",
		"bgfx/3rdparty/spirv-tools/source",
		"bgfx/3rdparty/spirv-tools/include",
	}
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/3rdparty/spirv-tools/include"),
        }
	}

	files {
		"bgfx/3rdparty/spirv-tools/source/**"
	}

project "spirv-cross"
	kind "StaticLib"
	defines {
		"SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS",
	}
	includedirs {
		"bgfx/3rdparty/spirv-cross/include",
	}
	files {
		"bgfx/3rdparty/spirv-cross/**",
	}
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/3rdparty/spirv-cross/include"),
		    path.getabsolute("bgfx/3rdparty/spirv-cross"),
        }
	}

project "fcpp"
    kind "StaticLib"
    files {
		"bgfx.lua",
		"bgfx/3rdparty/fcpp/**.h",
		"bgfx/3rdparty/fcpp/cpp1.c",
		"bgfx/3rdparty/fcpp/cpp2.c",
		"bgfx/3rdparty/fcpp/cpp3.c",
		"bgfx/3rdparty/fcpp/cpp4.c",
		"bgfx/3rdparty/fcpp/cpp5.c",
		"bgfx/3rdparty/fcpp/cpp6.c",
    }
    defines {
        "NINCLUDE=64",
        "NWORK=65536",
        "NBUFF=65536",
        "OLD_PREPROCESSOR=0",
    }
	exports {
		["includedirs"]	= {
		    path.getabsolute("bgfx/3rdparty/fcpp"),
        }
	}

project "shaderc"
	kind "ConsoleApp"
	targetdir(path.getabsolute("../../bin/tools"))
    files {
		"bgfx.lua",
		"bgfx/tools/shaderc/**",
		"bgfx/src/vertexlayout.**",
		"bgfx/src/shader**",
    }
    includedirs {
        "bimg/include",
        "bx/include",
        "bgfx/include",
        "bgfx/3rdparty/webgpu/include",
        "bgfx/3rdparty/dxsdk/include",
    }
    links {
        "bx",
        "spirv-opt",
        "spirv-cross",
        "fcpp",
        "glsl-optimizer",
        "glslang",
    }

project "shaderc"
	kind "ConsoleApp"
	targetdir(path.getabsolute("../../bin/tools"))
    files {
		"bgfx.lua",
		"bgfx/tools/shaderc/**",
		"bgfx/src/vertexlayout.**",
		"bgfx/src/shader**",
    }
    includedirs {
        "bimg/include",
        "bx/include",
        "bgfx/include",
        "bgfx/3rdparty/webgpu/include",
        "bgfx/3rdparty/dxsdk/include",
    }
    links {
        "bx",
        "spirv-opt",
        "spirv-cross",
        "fcpp",
        "glsl-optimizer",
        "glslang",
    }

project "geometryc"
	kind "ConsoleApp"
	targetdir(path.getabsolute("../../bin/tools"))
    files {
		"bgfx.lua",
		"bgfx/tools/geometryc/**",
		"bgfx/src/vertexlayout.**",
		"bgfx/examples/common/bounds.**",
		"bgfx/3rdparty/meshoptimizer/src/**",
    }
    includedirs {
        "bimg/include",
        "bx/include",
        "bgfx/include",
        "bgfx/3rdparty",
    }
    links {
        "bx",
        "bgfx",
        "spirv-opt",
        "spirv-cross",
        "fcpp",
        "glsl-optimizer",
        "glslang",
    }

project "bimg_encode"
	kind "StaticLib"

	includedirs {
		"bimg/include",
		"bimg/3rdparty",
		"bimg/3rdparty/nvtt",
		"bimg/3rdparty/iqa/include",
		"bimg/3rdparty/tinyexr/deps/miniz",
	}

	files {
		"bimg/include/**",
		"bimg/src/image_encode.*",
		"bimg/src/image_cubemap_filter.*",
		"bimg/3rdparty/libsquish/**.cpp",
		"bimg/3rdparty/libsquish/**.h",
		"bimg/3rdparty/edtaa3/**.cpp",
		"bimg/3rdparty/edtaa3/**.h",
		"bimg/3rdparty/etc1/**.cpp",
		"bimg/3rdparty/etc1/**.h",
		"bimg/3rdparty/etc2/**.cpp",
		"bimg/3rdparty/etc2/**.hpp",
		"bimg/3rdparty/nvtt/**.cpp",
		"bimg/3rdparty/nvtt/**.h",
		"bimg/3rdparty/pvrtc/**.cpp",
		"bimg/3rdparty/pvrtc/**.h",
		"bimg/3rdparty/astc/**.cpp",
		"bimg/3rdparty/astc/**.h",
		"bimg/3rdparty/tinyexr/**.h",
		"bimg/3rdparty/iqa/include/**.h",
		"bimg/3rdparty/iqa/source/**.c",
	}
	links {
	    "bx"
    }

project "texturec"
	kind "ConsoleApp"
	targetdir(path.getabsolute("../../bin/tools"))
	files {
		"bimg/tools/texturec/**",
	}
	links {
		"etc1",
		"etc2",
		"nvtt",
		"bimg",
		"bimg_encode",
		"bgfx",
		"bx",
	}

