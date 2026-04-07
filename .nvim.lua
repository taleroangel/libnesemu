-- Make parameters
vim.opt.makeprg = "cmake --build build"

local function build()
    vim.cmd("make")
end

-- Debug `Emulator`
local dap = require("dap")
dap.configurations.c = {
    {
        name = "Launch Emulator with `nestest.nes` ROM",
        type = "codelldb",
        request = "launch",
        cwd = "build/emulator",
        program = function()
            build()
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
            build()
            return "build/tests/TestNestest"
        end
    }
}
