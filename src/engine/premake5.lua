project("Engine")
    kind "StaticLib"
    files {
        path.join(ENGINE_DIR,"**.h"),
        path.join(ENGINE_DIR,"**.cpp")
    }
    includedirs {
        ENGINE_INCLUDE_DIR
    }