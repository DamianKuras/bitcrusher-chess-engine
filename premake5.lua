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
    configurations {"Debug", "Release", "Profiling"}
    platforms { "x32", "x64" }  
    defaultplatform ("x64")
    location "build"
    language "C++"
    cppdialect "C++23"
    targetdir(path.join(BIN_DIR,"%{cfg.buildcfg}"))
    objdir(path.join(OBJ_DIR,"%{cfg.buildcfg}", "%{prj.name}"))
    filter {"configurations:Debug"}
        defines {"DEBUG"}
        symbols "On"
        optimize "Debug"
    
    filter { "configurations:Debug", "action:gmake* or action:gcc*" }
        symbols "On"
        sanitize { 
            "Thread",
            "UndefinedBehavior",
            "Fuzzer"
        } 
        buildoptions { 
            "-fno-omit-frame-pointer",
            "-g3"
        }
    filter { "action:gmake* or action:gcc*"}
        buildoptions { "-mbmi2" }
        
    filter {"configurations:Release"}
        defines {"NDEBUG"}
        symbols "Off"
        optimize "Speed"

    filter {"configurations:Profiling"}
        defines {"NDEBUG"}
        symbols "On"
        optimize "Speed"

    filter {"platforms:x32"}
        architecture "x32"

    filter {"platforms:x64"}
        architecture "x64"

    filter "system:linux"
        links { "atomic" }

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


