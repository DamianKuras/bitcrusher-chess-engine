local google_test_dir = path.join(DEPS_DIR,"googletest-1.15.2","googletest")

ensure_dependency(
    "google-test",
    "https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip",
    google_test_dir
)

project "GoogleTest"
    kind "StaticLib"
    files {
        path.join(google_test_dir,"src","*.cc"),
        path.join(google_test_dir, "include/gtest/**")
    }
    removefiles{
        path.join(google_test_dir,"src","gtest-all.cc")
    }
    includedirs {
        path.join(google_test_dir,"include"),
        google_test_dir
    }

project "Tests"
    kind "ConsoleApp"
    dependson {
        "Engine",
        "GoogleTest"
       
    }
    files {
        path.join(TESTS_DIR,"**.cpp"),
        path.join(TESTS_DIR,"**.h")
    }
    links {
        "Engine",
        "GoogleTest"
    }
    includedirs {
        path.join(google_test_dir,"include"),
        ENGINE_INCLUDE_DIR
    }