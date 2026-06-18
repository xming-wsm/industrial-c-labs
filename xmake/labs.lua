-- Lab 目标与任务（Lab 1~11）
local rootdir = os.projectdir()

local labs = {
    {n = 1,  dir = "lab01_ring_buffer"},
    {n = 2,  dir = "lab02_command_queue"},
    {n = 3,  dir = "lab03_linked_list_pool"},
    {n = 4,  dir = "lab04_hash_table"},
    {n = 5,  dir = "lab05_fsm"},
    {n = 6,  dir = "lab06_producer_consumer", pthread = true},
    {n = 7,  dir = "lab07_thread_pool",       pthread = true},
    {n = 8,  dir = "lab08_scheduler"},
    {n = 9,  dir = "lab09_socket_comm",       pthread = true},
    {n = 10, dir = "lab10_frame_parser"},
    {n = 11, dir = "lab11_modbus_tcp"},
}

for _, info in ipairs(labs) do
    local lab_dir = path.join(rootdir, "labs", info.dir)
    local lib_target = info.dir
    local test_target = "test_" .. info.dir

    target(lib_target)
        set_kind("static")
        set_default(false)
        add_files(path.join(lab_dir, "src/*.c"))
        add_includedirs(path.join(lab_dir, "include"))
        if info.pthread then
            add_syslinks("pthread")
        end

    target(test_target)
        set_kind("binary")
        set_default(false)
        add_files(path.join(lab_dir, "test/*.c"))
        add_deps(lib_target)
        add_includedirs(path.join(rootdir, "common"), path.join(lab_dir, "include"))
        if info.pthread then
            add_syslinks("pthread")
        end
end

for _, info in ipairs(labs) do
    local test_target = "test_" .. info.dir
    local task_name = "lab" .. info.n

    task(task_name)
        set_menu({
            usage = "xmake " .. task_name .. " [test]",
            description = "Build Lab " .. info.n .. " (" .. info.dir .. ")",
            options = {
                {nil, "action", "v", nil, "Action: test (default: build)"},
            },
        })
        on_run(function ()
            import("core.base.option")
            import("core.project.task")

            local action = option.get("action")

            if action == "test" then
                task.run("build", {target = test_target})
                task.run("run", {target = test_target})
            else
                task.run("build", {target = test_target})
            end
        end)
end
