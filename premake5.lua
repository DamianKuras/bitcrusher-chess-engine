include "premake5_helper_functions.lua"

workspace "BitcrusherChessEngine"

    -- Directory setup
    ROOT_DIR = path.getabsolute(".")
    DEPS_DIR = path.join(ROOT_DIR, "deps")
    ENGINE_DIR = path.join(ROOT_DIR, "src","engine")
    UCI_DIR = path.join(ROOT_DIR, "src","uci")
    BENCHMARKS_DIR =  path.join(ROOT_DIR, "benchmarks")
    TESTS_DIR = path.join(ROOT_DIR,"tests")
    BIN_DIR = path.join(ROOT_DIR, "bin")
    OBJ_DIR = path.join(ROOT_DIR, "obj")
    ENGINE_INCLUDE_DIR = path.join(ENGINE_DIR, "include")

    -- Workspace configuration
    configurations {"Debug", "Release"}
    location "build"
    language "C++"
    cppdialect "C++17"
    targetdir(path.join(BIN_DIR,"%{cfg.buildcfg}"))
    objdir(path.join(OBJ_DIR,"%{cfg.buildcfg}", "%{prj.name}"))
    warnings "Extra"

    filter {"configurations:Debug"}
        defines {"DEBUG"}
        symbols "On"
        optimize "Off"
        
    filter {"configurations:Release"}
        defines {"NDEBUG"}
        symbols "Off"
        optimize "Speed"

    newoption {
        trigger = "with-tests",
        description = "Enable building tests."
    }
    newoption {
        trigger = "with-benchmarks",
        description = "Enable building benchmarks."
    }
    newoption{
        trigger = "with-uci",
        description = "Enable building uci console application."
    }


-- Core Engine Library
include (ENGINE_DIR)

-- Optional components
if _OPTIONS["with-tests"] then
    include(TESTS_DIR) 
end

if _OPTIONS["with-benchmarks"] then
    include(BENCHMARKS_DIR)
end

if _OPTIONS["with-uci"] then
    include(UCI_DIR)
end

