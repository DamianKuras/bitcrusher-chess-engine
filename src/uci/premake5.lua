project "Uci"
    kind "ConsoleApp"
    dependson {"Engine"}
    files {
        path.join(UCI_DIR,"**.cpp"),
        path.join(UCI_DIR, "**.h")
    }
    links {
        "Engine"
    }
    includedirs {
        ENGINE_INCLUDE_DIR, 
    }
