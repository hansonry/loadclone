
config_tool_settings = NewSettings()
settings = NewSettings()


if family == "windows" then
   sep = "\\"
   settings.cc.includes:Add("SDL2-2.0.1/include");
   settings.debug = 0
   settings.cc.flags:Add("/MD");
   settings.link.flags:Add("/SUBSYSTEM:CONSOLE");   
   settings.link.libs:Add("SDL2main");
   settings.link.libpath:Add("SDL2-2.0.1/lib/x86");
else
   sep = "/"
   settings.cc.flags:Add("-Wall");
   config_tool_settings.cc.flags:Add("-Wall");
end

-- Build the config tool
config_tool_path     = "config_tool" .. sep
config_tool_source   = Collect(config_tool_path .. "*.c")
config_tool_objects  = Compile(config_tool_settings, config_tool_source)
config_tool_exe      = Link(config_tool_settings, config_tool_path .. "config_tool", config_tool_objects)

-- Set up jobs and deps for generating config data
config_tool_path = "config_tool" .. sep .. "config_tool"
AddJob("GameConfigData.h",    "Generating Config Data Struct", config_tool_path)
AddJob("GameConfigData.inl",  "Generating Config Data INL Function", config_tool_path)
AddJob("config_template.txt", "Generating Config Data Template", config_tool_path)
AddDependency("GameConfigData.h",    "config_source.txt", config_tool_exe);
AddDependency("GameConfigData.inl",  "config_source.txt", config_tool_exe);
AddDependency("config_template.txt", "config_source.txt", config_tool_exe);



-- Build the game
settings.link.libs:Add("SDL2")
settings.link.libs:Add("SDL2_image")
settings.link.libs:Add("SDL2_ttf")
settings.link.libs:Add("SDL2_mixer")

source = Collect("*.c")

objects = Compile(settings, source)
exe = Link(settings, "loadclone", objects)

