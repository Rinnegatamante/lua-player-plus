//    LightMP3
//    Copyright (C) 2007, 2008 Sakya
//    sakya_tg@yahoo.it
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#include <pspkernel.h>
#include <psppower.h>
#include <pspsdk.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Funzioni gestione BUS & CLOCK
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getCpuClock(){
    return scePowerGetCpuClockFrequency();
}

void setBusClock(int bus){
    if (bus >= 54 && bus <= 111 && sceKernelDevkitVersion() < 0x03070110)
        scePowerSetBusClockFrequency(bus);
}

void setCpuClock(int cpu){
    if (cpu >= 10 && cpu <= 266){
        if (sceKernelDevkitVersion() < 0x03070110){
            scePowerSetCpuClockFrequency(cpu);
            if (scePowerGetCpuClockFrequency() < cpu)
                scePowerSetCpuClockFrequency(++cpu);
        }else{
            scePowerSetClockFrequency(cpu, cpu, cpu/2);
            if (scePowerGetCpuClockFrequency() < cpu){
                cpu++;
                scePowerSetClockFrequency(cpu, cpu, cpu/2);
            }
        }
    }
}
