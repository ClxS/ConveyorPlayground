function lib(name)
    project(name)
        kind "StaticLib"
    	language "C++"
    	files {
    		"**.h",
    		"**.cpp",
    	}
    	flags { "FatalWarnings" }
    	pchsource("src/" .. name .. "PCH.cpp")
    	pchheader(name .. "PCH.h")
end


