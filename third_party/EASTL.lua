project "EASTL"
	kind "StaticLib"
	files {
		"EASTL/include/**",
		"EASTL/source/**",
	}
	includedirs {
		"EASTL/include",
		"EASTL/test/packages/EABase/include/Common",
		"EASTL/test/packages/EAAssert/include/Common",
		"EASTL/test/packages/EAMain/include/Common",
		"EASTL/test/packages/EAThread/include/Common",
	}
	vpaths
	{
		["src/*"] = "EASTL/source",
		["include/*"] = "EASTL/include",
	}