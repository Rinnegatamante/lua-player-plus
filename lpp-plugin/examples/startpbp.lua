Screen.clear()
Screen.debugPrint(0,0,"Press Triangle to start LPP/FBOOT.PBP",0xFFFFFF)
Screen.debugPrint(0,1,"Press Square to start DINO/FBOOT.PBP with POPS",0xFFFFFF)
while true do
pad2 = Controls.read()
if pad2:triangle() then
System.startPBP("ms0:/PSP/GAME/LPP/FBOOT.PBP")
end
if pad2:square() then
System.startPSX("ms0:/PSP/GAME/DINO/FBOOT.PBP")
end
end