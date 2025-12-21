-- Make parameters
vim.opt.makeprg = "cmake --build build"

-- Debug `Emulator`
local dap = require("dap")
dap.configurations.c = {
    {
        name = "Launch Emulator (linux) with `nestest.nes` ROM",
        type = "codelldb",
        request = "launch",
        cwd = "build/emulator",
        program = "build/emulator/nesemu_linux",
        args= { "nestest.nes" }
    },
    {
        name = "Launch nestest",
        type = "codelldb",
        request = "launch",
        cwd = "tests/resources",
        program = "build/tests/TestNestest",
    }
}
