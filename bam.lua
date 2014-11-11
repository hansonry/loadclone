settings = NewSettings()

if family == "windows" then
   settings.cc.includes:Add("SDL2-2.0.1/include");
   settings.debug = 0
   settings.cc.flags:Add("/MD");
   settings.link.flags:Add("/SUBSYSTEM:CONSOLE");   
   settings.link.libs:Add("SDL2main");
   settings.link.libpath:Add("SDL2-2.0.1/lib/x86");
   settings.link.libs:Add("opengl32");
   settings.link.libs:Add("glu32");
   settings.cc.defines:Add("__WIN32__")
else
   settings.link.libs:Add("GL");
   settings.link.libs:Add("GLU");
end



settings.link.libs:Add("SDL2");


source = Collect("*.c");

objects = Compile(settings, source)
exe = Link(settings, "helloworld", objects)
