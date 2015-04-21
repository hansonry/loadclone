
config_tool_path     = "config_tool/"
config_tool_settings = NewSettings()
config_tool_source   = Collect(config_tool_path .. "*.c")
config_tool_objects  = Compile(config_tool_settings, config_tool_source)
config_tool_exe      = Link(config_tool_settings, config_tool_path .. "config_tool", config_tool_objects)


settings = NewSettings()


if family == "windows" then
   settings.cc.includes:Add("SDL2-2.0.1/include");
   settings.debug = 0
   settings.cc.flags:Add("/MD");
   settings.link.flags:Add("/SUBSYSTEM:CONSOLE");   
   settings.link.libs:Add("SDL2main");
   settings.link.libpath:Add("SDL2-2.0.1/lib/x86");
else
   settings.cc.flags:Add("-Wall");
end




settings.link.libs:Add("SDL2");
settings.link.libs:Add("SDL2_image");
settings.link.libs:Add("SDL2_ttf");
settings.link.libs:Add("SDL2_mixer");

source = Collect("*.c");

objects = Compile(settings, source)
exe = Link(settings, "loadclone", objects)

