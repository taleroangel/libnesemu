-- Debug `Emulator`
local dap = require("dap")
dap.configurations.c = {
    {
        name = "Launch Emulator with `nestest.nes` ROM",
        type = "codelldb",
        request = "launch",
        cwd = "build/emulator",
        program = function()
            return "build/emulator/nesemu_exe"
        end,
        args = { "nestest.nes" }
    },
    {
        name = "Launch 'nestest'",
        type = "codelldb",
        request = "launch",
        cwd = "tests/resources",
        program = function()
            return "build/tests/TestNestest"
        end
    }
}

require("dalton").add({
    build = "cmake --build build",
    emulator = { "./build/emulator/nesemu_exe", "./tests/resources/nestest.nes" },
})
