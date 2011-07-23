font = {w = 8, h = 8}

-- Patched print function
local oldprint = screen.print
screen.print = function(image, x, y, text, color) 
    local function tremas(x, y, color)
        screen:pixel(x + 1, y, color)
        screen:pixel(x + 3, y, color)
    end
    local specialChar = {{"é", "e", "'"}, {"è", "e", "`"}, {"ê", "e", "^"}, {"ë", "e", "¨"}, {"à", "a", "`"}, {"â", "a", "^"}, {"ä", "a", "¨"}, {"î", "i", "^"}, {"ï", "i", "¨"}, {"ô", "o", "^"}, {"ö", "o", "¨"}, {"ù", "u", "`"}, {"û", "u", "^"}, {"ü", "u", "¨"}}
    for i, c in ipairs(specialChar) do 
        local s = text:find(c[1])	
        while s do
            if c[3] == "¨" then
                tremas(x + (s * font.w) - font.w, y, color)
            else
                oldprint(image, x + (s * font.w) - font.w, y, c[3], color)
            end
            text = text:sub(0, s - 1)  .. c[2] .. text:sub(s + 1)
            s = text:find(c[1])
        end
    end
    oldprint(image, x, y, text, color)
end

--Underline
function screen.printUnderline(image, x, y, text, text_color, line_color)
    image:print(x, y, text, text_color)
    image:drawLine(x - 2, y + font.h + 1, x + font.w * string.len(text), y + font.h + 1, line_color)
end

--Write vertically
function screen.printVertical(image,x, y, text, distance, color)
    i, taille = 1, string.len(text)
    for i = 0, taille do
        i = i + 1
        image:print(x, y + i * distance, string.sub(text, i, i), color)
    end
end

-- Gradient text
function screen.printGradient(image,x, y, text, color1, color2)
   local t = {}
   local first_color, second_color = color1:colors(), color2:colors()
   local diffR, diffG, diffB = second_color.r -first_color.r, second_color.g - first_color.g, second_color.b - first_color.b
   local stepR, stepG, stepB = diffR / #text, diffG / #text, diffB / #text
   for i = 1, #text do
      table.insert(t, string.sub(text, i, i))
   end
   for i, v in ipairs(t) do
      image:print((x + i * 8) - 8, y, v, Color.new(i * stepR + first_color.r, i * stepG + first_color.g, i * stepB + first_color.b))
   end
end

-- Center
function screen.printCenter(image, y, text, color)
    local x0 = 240 - (string.len(text) * font.w) / 2
    image:print(x0, y0, text, color)
end
-- BMPLib
function b(byte_number, byte_length)
if byte_length == 1 then
    return string.byte(string.sub(filestring, byte_number, byte_number))
elseif byte_length == 2 then
    byte1 = string.byte(string.sub(filestring, byte_number, byte_number))
    byte2 = string.byte(string.sub(filestring, byte_number + 1, byte_number + 1))
    return dec_to_dec2(byte1,byte2)

elseif byte_length == 4 then
    byte1 = string.byte(string.sub(filestring, byte_number, byte_number))
    byte2 = string.byte(string.sub(filestring, byte_number + 1, byte_number + 1))
    byte3 = string.byte(string.sub(filestring, byte_number + 2, byte_number + 2))
    byte4 = string.byte(string.sub(filestring, byte_number + 3, byte_number + 3))
    return dec_to_dec4(byte1,byte2,byte3,byte4)
end
end
function dec_to_dec2(hexvalue1, hexvalue2)
first_byte = hexvalue1
second_byte = hexvalue2
sample = (second_byte * 256) + first_byte
if sample > 32767  then
  sample = 0 - (65536 - sample)
end
return sample
end
function dec_to_dec4(hexvalue1, hexvalue2, hexvalue3,hexvalue4)
sample = (hexvalue4 * 16777216) + (hexvalue3 * 65536) + (hexvalue2 * 256) + hexvalue1
return sample
end

function Image.bmpload(Bmp_filename)
file = io.open(Bmp_filename, "rb")
if file then
filestring = file:read("*all")
file:close()
else
error("BMPLIB: NO SUCH FILE",2)
end
if b(29,2) ~= 24 then error("BMPLIB: "..b(29,2) .. " BIT (MUST BE 24 BIT)",2) end
width = b(19,4) * 4
real_width = b(19,4)
height = b(23,4)
row_length = width - math.floor(b(19,4)*.25)*4
pixel_length = b(19,4) * 3
image = Image.createEmpty(real_width,height)
image_r = {}
image_g = {}
image_b = {}
for columns=55,55+(row_length*height)-row_length,row_length do
for a=columns,columns+pixel_length-1,3 do
table.insert(image_r,b(a+2,1))
table.insert(image_g,b(a+1,1))
table.insert(image_b,b(a,1))
end
end
num_x = 0
num_y = 0
for b=height,1,-1 do
for a=1,real_width do
num = (real_width*b)-real_width+a
image:drawLine(num_x,num_y,num_x,num_y,Color.new(image_r[num],image_g[num],image_b[num]))
num_x = num_x + 1
end
num_y = num_y + 1
num_x = 0
end
return image
end

-- System.getFileSize
function System.getFileSize(filename)
file = io.open(filename,"rb")
size = file:seek("end")
file:close()
return size
end

-- Utilities
function s2i(s)
   return 0x1000000*s:byte(4)+0x10000*s:byte(3)+0x100*s:byte(2)+s:byte(1)
end

-- System.getEboot
function System.getEboot(filename,tipo)
eboot = io.open(filename,'rb')
if tipo == 1 then -- ICON0.PNG
eboot:seek('set',0xC)
icon0png = s2i(eboot:read(4))
icon1pmf = s2i(eboot:read(4))
eboot:seek('set',icon0png)
icon = io.open("ms0:/ICON0.PNG",'w')
icon:write(eboot:read(icon1pmf-icon0png))
icon:close()
image = Image.load("ms0:/ICON0.PNG")
System.removeFile("ms0:/ICON0.PNG")
eboot:close()
return image
end
if tipo == 2 then -- PIC0.PNG
eboot:seek('set',0x14)
icon0png = s2i(eboot:read(4))
icon1pmf = s2i(eboot:read(4))
eboot:seek('set',icon0png)
icon = io.open("ms0:/PIC0.PNG",'w')
icon:write(eboot:read(icon1pmf-icon0png))
icon:close()
image = Image.load("ms0:/PIC0.PNG")
System.removeFile("ms0:/PIC0.PNG")
eboot:close()
return image
end
if tipo == 3 then -- PIC1.PNG
eboot:seek('set',0x18)
icon0png = s2i(eboot:read(4))
icon1pmf = s2i(eboot:read(4))
eboot:seek('set',icon0png)
icon = io.open("ms0:/PIC1.PNG",'w')
icon:write(eboot:read(icon1pmf-icon0png))
icon:close()
image = Image.load("ms0:/PIC1.PNG")
System.removeFile("ms0:/PIC1.PNG")
eboot:close()
return image
end
end

-- System.extractPBP
function System.extractPBP(filename,dir)
final = System.getFileSize(filename)
eboot = io.open(filename,'rb')
eboot:seek('set',0x08)
param = s2i(eboot:read(4))
icon0 = s2i(eboot:read(4))
icon1 = s2i(eboot:read(4))
pic0 = s2i(eboot:read(4))
pic1 = s2i(eboot:read(4))
snd0 = s2i(eboot:read(4))
datapsp = s2i(eboot:read(4))
datapsar = s2i(eboot:read(4))
icon = io.open(dir.."/PARAM.SFO",'w')
icon:write(eboot:read(icon0-param))
icon:close()
if System.getFileSize(dir.."/PARAM.SFO") <= 0 then
System.removeFile(dir.."/PARAM.SFO")
end
icon = io.open(dir.."/ICON0.PNG",'w')
icon:write(eboot:read(icon1-icon0))
icon:close()
if System.getFileSize(dir.."/ICON0.PNG") <= 0 then
System.removeFile(dir.."/ICON0.PNG")
end
icon = io.open(dir.."/ICON1.PMF",'w')
icon:write(eboot:read(pic0-icon1))
icon:close()
if System.getFileSize(dir.."/ICON1.PMF") <= 0 then
System.removeFile(dir.."/ICON1.PMF")
end
icon = io.open(dir.."/PIC0.PNG",'w')
icon:write(eboot:read(pic1-pic0))
icon:close()
if System.getFileSize(dir.."/PIC0.PNG") <= 0 then
System.removeFile(dir.."/PIC0.PNG")
end
icon = io.open(dir.."/PIC1.PNG",'w')
icon:write(eboot:read(snd0-pic1))
icon:close()
if System.getFileSize(dir.."/PIC1.PNG") <= 0 then
System.removeFile(dir.."/PIC1.PNG")
end
icon = io.open(dir.."/SND0.AT3",'w')
icon:write(eboot:read(datapsp-snd0))
icon:close()
if System.getFileSize(dir.."/SND0.AT3") <= 0 then
System.removeFile(dir.."/SND0.AT3")
end
icon = io.open(dir.."/DATA.PSP",'w')
icon:write(eboot:read(datapsar-datapsp))
icon:close()
if System.getFileSize(dir.."/DATA.PSP") <= 0 then
System.removeFile(dir.."/DATA.PSP")
end
if (final-datapsar) > 0 then
icon = io.open(dir.."/DATA.PSAR",'w')
icon:write(eboot:read(final-datapsar))
icon:close()
if System.getFileSize(dir.."/DATA.PSAR") <= 0 then
System.removeFile(dir.."/DATA.PSAR")
end
end
eboot:close()
end

-- System.checkPBP
function System.checkPBP(pbp,file)
if file == "PARAM.SFO" then
offset = 0x08
elseif file == "ICON0.PNG" then
offset = 0xC
elseif file == "ICON1.PMF" then
offset = 0x10
elseif file == "PIC0.PNG" then
offset = 0x14
elseif file == "PIC1.PNG" then
offset = 0x18
elseif file == "SND0.AT3" then
offset = 0x1C
elseif file == "DATA.PSP" then
offset = 0x20
elseif file == "DATA.PSAR" then
base = System.getFileSize(pbp)
end
if file == "DATA.PSAR" then
eboot = io.open(pbp,'rb')
eboot:seek('set',0x24)
pointer = s2i(eboot:read(4))
icon = io.open("DATA.PSAR",'w')
icon:write(eboot:read(base-pointer))
icon:close()
eboot:close()
if System.getFileSize("DATA.PSAR") > 0 then
System.removeFile("DATA.PSAR")
return true
else
System.removeFile("DATA.PSAR")
return false
end
else
eboot = io.open(pbp,'rb')
eboot:seek('set',offset)
pointer = s2i(eboot:read(4))
base = s2i(eboot:read(4))
icon = io.open("CHECK.TMP",'w')
icon:write(eboot:read(base-pointer))
icon:close()
eboot:close()
if System.getFileSize("CHECK.TMP") > 0 then
System.removeFile("CHECK.TMP")
return true
else
System.removeFile("CHECK.TMP")
return false
end
end
end

-- System.getEbootTitle
function System.getEbootTitle(filename)
    file = io.open(filename,'rb')
    file:seek('set',0x08) -- Punto all'offset del pointer di PARAM.SFO
    pointer = s2i(file:read(4)) -- Estraggo il pointer di PARAM.SFO
    offset = pointer + 12
    file:seek('set',offset) -- Punto all'offset del pointer di inizio scrittura dati di PARAM.SFO
    baseread = s2i(file:read(4)) -- Estraggo il suddetto pointer
    file:seek('set',pointer+baseread+48) -- Punto al Titolo dell'EBOOT.PBP
    base = "r"
    testo = ""
    while string.byte(base) ~= 0 do -- Estraggo fino a trovare un byte nullo
        base = file:read(1)
        testo = testo .. base
    end
    file:close()
    return testo -- Ritorno il titolo
end

-- System.getParamTitle
function System.getParamTitle(filename)
    file = io.open(filename,'rb')
    offset = 12
    file:seek('set',offset) -- Punto all'offset del pointer di inizio scrittura dati di PARAM.SFO
    baseread = s2i(file:read(4)) -- Estraggo il suddetto pointer
    file:seek('set',baseread+48) -- Punto al Titolo del PARAM.SFO
    base = "r"
    testo = ""
    while string.byte(base) ~= 0 do -- Estraggo fino a trovare un byte nullo
        base = file:read(1)
        testo = testo .. base
    end
    file:close()
    return testo -- Ritorno il titolo
end

-- System.getSaveInfo
function System.getSaveInfo(filename,info)
    file = io.open(filename,'rb')
    offset = 12
    file:seek('set',offset)
    baseread = s2i(file:read(4))
    if info == 1 then -- Nome Salvataggio
    extra = 8
    end
    if info == 2 then -- Codice Gioco
    extra = 1032
    end
    if info == 3 then -- Descrizione Salvataggio
    extra = 4392
    end
    if info == 4 then -- Titolo Gioco
    extra = 4520
    end
    file:seek('set',baseread+extra)
    base = "r"
    testo = ""
    while string.byte(base) ~= 0 do
        base = file:read(1)
        testo = testo .. base
    end
    return testo
    file:close()
end

-- System.doesFileExist
function System.doesFileExist(filename)
file = io.open(filename,"r")
if file then
file:close()
return true
else
return false
end
end

-- Opening main script
if System.doesFileExist("SYSTEM.LUA") then
    dofile("SYSTEM.LUA")
    else
    if System.doesFileExist("index.lua") then
        dofile("index.lua")
        else
        if System.doesFileExist("script.lua") then
            dofile("script.lua")
            else
            if System.doesFileExist("SYSTEM/SYSTEM.LUA") then
                dofile("SYSTEM/SYSTEM.LUA")
                else
                error("Fatal Error: Main script doesn't exist")
            end
        end
    end
end