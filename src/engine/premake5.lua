project("Engine")
    kind "StaticLib"
    files {
        path.join(ENGINE_DIR,"**.hpp"),
        path.join(ENGINE_DIR,"**.cpp")
    }
    includedirs {
        ENGINE_INCLUDE_DIR,
        -- path.join(ENGINE_INCLUDE_DIR, "attack_generators"),
        -- path.join(ENGINE_DIR,"legal_moves")
    }