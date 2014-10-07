Screen.clear()
red = Color.new(255,0,0)
blue = Color.new(0,0,255)
green = Color.new(0,255,0)
base = Color.new(100,100,255)
base_r = 1
base_g = 1
base_b = 1
while true do
red_b = Color.getR(base)
blue_b = Color.getB(base)
green_b = Color.getG(base)
if (red_b < 1) then
base_r = 1
end
if (green_b < 1) then
base_g = 1
end
if (blue_b < 1) then
base_b = 1
end
if (red_b > 254) then
base_r = 0
end
if (green_b > 254) then
base_g = 0
end
if (blue_b > 254) then
base_b = 0
end
if (base_r == 1) then
base = Color.new(red_b+1,green_b,blue_b)
else
base = Color.new(red_b-1,green_b,blue_b)
end
red_b = Color.getR(base)
if (base_g == 1) then
base = Color.new(red_b,green_b+1,blue_b)
else
base = Color.new(red_b,green_b-1,blue_b)
end
green_b = Color.getG(base)
if (base_b == 1) then
base = Color.new(red_b,green_b,blue_b+1)
else
base = Color.new(red_b,green_b,blue_b-1)
end
Screen.debugPrintGradient(0,0,"My first LUA plugin",red,green)
Screen.debugPrint(0,1,"*** Hello World ***",base)
Screen.debugPrint(0,2,"Press Triangle to return to menu",blue)
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