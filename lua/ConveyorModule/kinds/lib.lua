function lib(name)
    project(name)
        kind "StaticLib"
    	language "C++"
    	files {
    		"*.lua",
    		"include/**",
    		"src/**",
    		"platform/SDL/**",
    	}
        includedirs {
            ".",
            "**",
        }

    	flags { "FatalWarnings" }
    	pchsource("src/" .. name .. "PCH.cpp")
    	pchheader(name .. "PCH.h")
        exports {
            ["includedirs"] = {
                path.getabsolute("include"),
            }
        }

        filter { "platforms:SDL" }
            includedirs {
                "platform/SDL/src",
            }
            exports {
                ["includedirs"] = {
                    path.getabsolute("platform/SDL/include"),
                }
            }
        filter {}
end


