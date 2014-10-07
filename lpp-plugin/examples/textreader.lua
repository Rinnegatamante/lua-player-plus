Screen.clear()
file = System.openFile("ms0:/seplugins/test.txt",PSP_READ)
size = System.seekFile(file,0,END_FILE)
System.seekFile(file,0,INIT_FILE)
string = System.readFile(file,size)
System.closeFile(file)
Screen.debugPrint(0,0,string,0xFFFFFF)
while true do
pad = Controls.read()
if pad:triangle() then
if (System.getModel == "N1000") then
System.protodofile("ef0:/seplugins/script/menu.lua")
else
System.protodofile("ms0:/seplugins/script/menu.lua")
end
end
end