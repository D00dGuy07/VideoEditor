TARGET_DIR = "%{wks.location}/bin/" .. "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}" .. "/%{prj.name}"
OBJ_DIR = "%{wks.location}/bin-int/" .. "%{cfg.system}-%{cfg.architecture}-%{cfg.buildcfg}" .. "/%{prj.name}"

PACKAGE_DIRS = {}

workspace "VideoEditor"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

group "Dependencies"
include "vendor"
group ""

project "VideoEditor"
    language "C++"
    cppdialect "C++17"

    kind "ConsoleApp"
    staticruntime "on"
    systemversion "latest"

    targetdir (TARGET_DIR)
	objdir (OBJ_DIR)

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "src",
        "vendor/libav/include",
        path.join(PACKAGE_DIRS["oglfv2"], "include"),
        path.join(PACKAGE_DIRS["glfw"], "include"),
        path.join(PACKAGE_DIRS["glad"], "include"),
        PACKAGE_DIRS["glm"],
        PACKAGE_DIRS["stb"]
    }

    links
    {
        "glad",
        "glfw",
        "oglfv2",
        "avcodec",
        "avformat",
        "avutil",
        "swscale"
    }

    defines
    {
        "GLFW_INCLUDE_NONE"
    }

    filter "system:windows"
        libdirs
        {
            "%{wks.location}/vendor/libav/lib"
        }

        postbuildcommands
        {
            "{COPYFILE} %[vendor/libav/bin/avformat-61.dll] %[%{cfg.buildtarget.directory}/avformat-61.dll]",
            "{COPYFILE} %[vendor/libav/bin/avcodec-61.dll] %[%{cfg.buildtarget.directory}/avcodec-61.dll]",
            "{COPYFILE} %[vendor/libav/bin/avutil-59.dll] %[%{cfg.buildtarget.directory}/avutil-59.dll]",
            "{COPYFILE} %[vendor/libav/bin/swresample-5.dll] %[%{cfg.buildtarget.directory}/swresample-5.dll]",
            "{COPYFILE} %[vendor/libav/bin/swscale-8.dll] %[%{cfg.buildtarget.directory}/swscale-8.dll]"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "Speed"

    filter "configurations:Dist"
        runtime "Release"
        optimize "Speed"
        symbols "off"