oldpad = Controls.read()
if (System.getModel == "N1000") then
scripts=System.listDirectory("ef0:/seplugins/script")
else
scripts=System.listDirectory("ms0:/seplugins/script")
end
tot=table.getn(scripts)
tot=tot-2
pointer=1
Screen.clear()
while true do
i=1
x = 3
Screen.debugPrint(0,0,"lpp.prx - Select script to load",0x00FF00)
Screen.debugPrint(40,0,"O = Resume Thread",0x00FF00)
while i <= tot do
if (i==pointer) then
Screen.debugPrint(0,i,scripts[x].name,0x0000FF)
else
Screen.debugPrint(0,i,scripts[x].name,0xFFFFFF)
end
i=i+1
x=x+1
end
pad = Controls.read()
if pad:down() and not oldpad:down() then
pointer=pointer+1
i=1
end
if pad:up() and not oldpad:up() then
pointer=pointer-1
end
if (pointer > tot) then
pointer = 1
end
if (pointer < 1) then
pointer = tot
end
if pad:cross() and not oldpad:cross() then
if (System.getModel == "N1000") then
var = "ef0:/seplugins/script/"
else
var = "ms0:/seplugins/script/"
end
real_pointer = pointer+2
var2 = var .. scripts[real_pointer].name
System.protodofile(var2)
end
if pad:circle() then
System.resumeThread()
end
oldpad = pad
Screen.waitVblankStart()
end