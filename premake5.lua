workspace "MyRevoke"
    architecture "x64"
    startproject "RevokeCraft"

    configurations { "Debug", "Release" }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    IncludeDir = {};
    IncludeDir ["GLFW"] = "MyRevoke/vendor/GLFW/include"
    IncludeDir ["ImGui"] = "MyRevoke/vendor/imgui"
    IncludeDir ["GLAD"] = "MyRevoke/vendor/GLAD/include"
    IncludeDir ["GLM"] = "MyRevoke/vendor/glm"
    IncludeDir ["STB_IMAGE"] = "MyRevoke/vendor/stb_image"
    IncludeDir ["ENTT"] = "MyRevoke/vendor/entt"
    IncludeDir ["yaml_cpp"] = "MyRevoke/vendor/yaml-cpp/include"
    IncludeDir ["ImGuizmo"] = "MyRevoke/vendor/ImGuizmo"
    IncludeDir ["Box2D"] = "MyRevoke/vendor/Box2D/include"
    IncludeDir ["OpenAL"] = "MyRevoke/vendor/OpenALBuild/include"
    IncludeDir ["sndfile"] = "MyRevoke/vendor/libsndfile/include"
    IncludeDir ["mono"] = "MyRevoke/vendor/mono/include"

    LibraryDir = {};

    LibraryDir ["mono"] = "%{prj.name}/vendor/mono/Lib/%{cfg.buildcfg}"

    group "Dependencies"
        include "MyRevoke/vendor/GLFW"
        include "MyRevoke/vendor/GLAD"
        include "MyRevoke/vendor/imgui"
        include "MyRevoke/vendor/yaml-cpp"
        include "MyRevoke/vendor/Box2D"
    group ""

    project "MyRevoke"
        location "MyRevoke"
        kind "StaticLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        pchheader "rvpch.h"
        pchsource "MyRevoke/src/rvpch.cpp"

        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp",
            "%{prj.name}/vendor/glm/glm/**.hpp",
            "%{prj.name}/vendor/glm/glm/**.inl",
            "%{prj.name}/vendor/stb_image/stb_image.h",
            "%{prj.name}/vendor/stb_image/stb_image.cpp",
            "%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",
            "%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",

        }
        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        includedirs
        {
            "%{prj.name}/src",
            "%{prj.name}/vendor/spdlog/include",
            "%{IncludeDir.GLFW}",
            "%{IncludeDir.GLAD}",
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.GLM}",
            "%{IncludeDir.STB_IMAGE}",
            "%{IncludeDir.ENTT}",
            "%{IncludeDir.yaml_cpp}",
            "%{IncludeDir.ImGuizmo}",
            "%{IncludeDir.Box2D}",
            "%{IncludeDir.OpenAL}",
            "%{IncludeDir.sndfile}",
            "%{IncludeDir.mono}",
        }

        links
        {
            "GLAD",
            "GLFW",
            "ImGui",
            "yaml-cpp",
            "opengl32.lib",
            "Box2D",
            "OpenAL32.lib",
            "sndfile.lib",
            "libmono-static-sgen.lib",
            "Ws2_32.lib",
            "Winmm.lib",
            "Version.lib",
            "Bcrypt.lib",

        }
        libdirs 
        {
            
        }
        filter "files:vendor/ImGuizmo/**.cpp"
            flags {"NoPCH"}
        filter "system:windows"
            systemversion "latest"

            defines
            {
                "RV_PLATFORM_WINDOWS",
                "RV_BUILD_DLL",
                "GLFW_INCLUDE_NONE",
                "IMGUI_DEFINE_MATH_OPERATORS",
                "RV_ASSERTS_ENABLE",
                "_CRT_SECURE_NO_WARNINGS",
                "YAML_CPP_STATIC_DEFINE",
            }          

        filter "configurations:Debug"
            defines "RV_DEBUG"
            runtime "Debug"
            symbols "On"

            libdirs 
            {
                "%{prj.name}/vendor/OpenALBuild/lib/Debug",
                "%{prj.name}/vendor/libsndfile/lib/Debug",
                "%{prj.name}/vendor/mono/Lib/Debug",
            }
            

        filter "configurations:Release"
            
            defines "RV_RELEASE"
            runtime "Release"
            optimize "On"

            libdirs 
            {
                "%{prj.name}/vendor/OpenALBuild/lib/Release",
                "%{prj.name}/vendor/libsndfile/lib/Release",
                "%{prj.name}/vendor/mono/Lib/Release",
            }


    project "RevokeCraft"
        location "RevokeCraft"
        kind "ConsoleApp"
        cppdialect "C++20"
        language "C++"
        staticruntime "off"
    
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp",
          
        }
    
        includedirs
        {
            "MyRevoke/vendor/spdlog/include",
            "MyRevoke/src",
            "%{IncludeDir.GLM}",
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.ENTT}",
            "%{IncludeDir.ImGuizmo}",
            "%{IncludeDir.OpenAL}",
            "%{IncludeDir.sndfile}",
        }
    
        links
        {
            "MyRevoke",
        }
    
        filter "system:windows"
            
            systemversion "latest"
    
            defines
            {
                "RV_PLATFORM_WINDOWS",
                "_CRT_SECURE_NO_WARNINGS"
            }
    
        filter "configurations:Debug"
            defines "RV_DEBUG"
            runtime "Debug"
            symbols "On"

    

        filter "configurations:Release"
            defines "RV_RELEASE"
            runtime "Release"
            optimize "On"

   

    project "SandBox"
        location "SandBox"
        kind "ConsoleApp"
        cppdialect "C++20"
        language "C++"
        staticruntime "off"
    
        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
        files
        {
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp",
          
        }
    
        includedirs
        {
            "MyRevoke/vendor/spdlog/include",
            "MyRevoke/src",
            "%{IncludeDir.GLM}",
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.ENTT}",
            "%{IncludeDir.OpenAL}",
            "%{IncludeDir.sndfile}",
        }
    
        links
        {
            "MyRevoke"
        }
    
        filter "system:windows"
            
            systemversion "latest"
    
            defines
            {
                "RV_PLATFORM_WINDOWS",
                "_CRT_SECURE_NO_WARNINGS"
            }
    
        filter "configurations:Debug"
            defines "RV_DEBUG"
            runtime "Debug"
            symbols "On"
    
        filter "configurations:Release"
            defines "RV_RELEASE"
            runtime "Release"
            optimize "On"

    
     project "MyRevoke-ScriptCore"
        location "MyRevoke-ScriptCore"
        kind "SharedLib"
        language "C#"
        dotnetframework "4.7.2"
        staticruntime "off"

        targetdir ("RevokeCraft/resourses/scripts/C#")
        objdir ("RevokeCraft/resourses/scripts/C#/intermediates")


        files
        {
            
            "%{prj.name}/Source/**.cs",
            "%{prj.name}/Properties/**.cs",
        }
           

        filter "configurations:Debug"
            optimize "Off"
            symbols "Default"
            

        filter "configurations:Release"
            optimize "Full"
            symbols "Off"
            

     project "MyRevoke-NativeScriptCore"
        location "MyRevoke-NativeScriptCore"
        kind "SharedLib"
        language "C++"
        cppdialect "C++20"
        staticruntime "off"

        targetdir ("RevokeCraft/resourses/scripts/Native")
        objdir ("RevokeCraft/resourses/scripts/Native/intermediates")


        files
        {
            
            "%{prj.name}/src/**.h",
            "%{prj.name}/src/**.cpp",
            "RevokeCraft/assets/Scripts/**.h",
            "RevokeCraft/assets/Scripts/**.cpp",
        }
        includedirs
        {
            "MyRevoke/vendor/spdlog/include",
            "MyRevoke/src",
            "%{IncludeDir.GLM}", 
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.ENTT}",
            "%{IncludeDir.ImGuizmo}",
            "%{IncludeDir.OpenAL}",
            "%{IncludeDir.sndfile}",
            "%{IncludeDir.Box2D}",
        }
        
        links
        {
            "MyRevoke",
        }
           
        defines
        {
            "RV_PLATFORM_WINDOWS",
            "_CRT_SECURE_NO_WARNINGS"
        }

        filter "configurations:Debug"
            optimize "Off"
            symbols "On"

            libdirs 
            {
                "MyRevoke/vendor/OpenALBuild/lib/Debug",
                "MyRevoke/vendor/libsndfile/lib/Debug",
            }
            

        filter "configurations:Release"
            optimize "Full"
            symbols "Off"
            
            libdirs 
            {
                "MyRevoke/vendor/OpenALBuild/lib/Release",
                "MyRevoke/vendor/libsndfile/lib/Release",
                
            }
            