project("Engine")
    kind "Utility"
    files {
        path.join(ENGINE_DIR, "**/*.hpp"),
        path.join(ENGINE_DIR, "**/*.cpp")
    }

    includedirs {
        ENGINE_INCLUDE_DIR,
        path.join(ENGINE_INCLUDE_DIR, "attack_generators"),
        path.join(ENGINE_INCLUDE_DIR,"legal_move_generators")
    }

