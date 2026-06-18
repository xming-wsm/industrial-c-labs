set_project("industrial_c_labs")
set_version("0.1.0")
set_description("Industrial C Labs - 注塑机控制系统实验课")
set_languages("c11")
add_rules("mode.debug", "mode.release")
add_cflags("-Wall", "-Wextra", "-Wshadow")

includes("xmake/labs.lua")
