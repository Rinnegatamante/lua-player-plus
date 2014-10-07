Screen.clear()
Screen.debugPrint(0,0,"PSP Indent",0x00FF00)
Screen.debugPrint(40,0,"Triangle = Return back",0x00FF00)
if (System.isPSP() == 1) then
model = "Model: " .. System.getModel()
else
model = "Model: PSVITA"
end
Screen.debugPrint(0,1,model,0xFFFFFF)
version = "Version: " .. System.getVersion()
Screen.debugPrint(0,2,version,0xFFFFFF)
battery = "Battery life percentage: " .. System.powerGetBatteryLifePercent()
Screen.debugPrint(0,3,battery,0xFFFFFF)
while true do
pad = Controls.read()
if pad:triangle() then
if (System.getModel == "N1000") then
System.protodofile("ef0:/seplugins/script/menu.lua")
else
System.protodofile("ms0:/seplugins/script/menu.lua")
end
end
Screen.waitVblankStart()
end