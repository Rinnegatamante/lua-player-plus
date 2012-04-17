cls


set path=%path%; "C:\pspsdk\bin"
set PSPSDK= C:\pspsdk\psp\sdk"

make
del *.o *.sfo *.elf
pause
exit
