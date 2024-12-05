workspace "GBMU"
   configurations { "Debug", "Release" } 
    toolset "clang"

project "gbmu"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "obj/%{cfg.buildcfg}"
   libdirs { "libs/**" }

   files { "src/**.c", "src/**.cpp", "src/**.h", "include/**.h" }
   includedirs { "include" }
   links { "SDL3.lib" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      buildoptions { "-g" } -- Add debug symbols for clang

    filter "configurations:Release"
      defines { "RELEASE" }
      symbols "On"
      buildoptions { "-g" } -- Add debug symbols for clang

   filter "system:windows"
      buildoptions { "-I./include" }
      links { "msvcrt" } -- Add any specific libraries for Windows
