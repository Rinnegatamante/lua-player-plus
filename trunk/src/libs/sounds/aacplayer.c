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
#include <string.h>
#include "id3.h"
#include "player.h"
#include "aacplayer.h"

#define __ALLEGREX__
#include "aacdec.h"

#ifdef AAC_ENABLE_SBR
#define SBR_MUL		2
#else
#define SBR_MUL		1
#endif

#define AAC_FILE_BUFFER_SIZE 2048

/////////////////////////////////////////////////////////////////////////////////////////
//Globals
/////////////////////////////////////////////////////////////////////////////////////////
int AAC_audio_channel;
char AAC_fileName[264];
int AAC_fd = -1;
int AAC_eos = 0;
struct fileInfo AAC_info;
int AAC_isPlaying = 0;
unsigned int AAC_volume_boost = 0.0;
int AAC_playingSpeed = 0; // 0 = normal
long AAC_suspendPosition = -1;
long AAC_suspendIsPlaying = 0;
int AAC_defaultCPUClock = 70;
double AAC_fileSize = 0;
HAACDecoder *hAACDecoder;

unsigned char AACfileBuffer[AAC_FILE_BUFFER_SIZE];
unsigned int samplesAvailable;
unsigned int AAC_filePos;

/////////////////////////////////////////////////////////////////////////////////////////
//Fill file buffer:
/////////////////////////////////////////////////////////////////////////////////////////
int AAC_fillFileBuffer(unsigned char *target, unsigned int bytesRequired){
    int bytesRed = 0;

    while (bytesRequired > 0){
        const unsigned int bytesRead = sceIoRead(AAC_fd, target, bytesRequired);
        // EOF?
        if (bytesRead == 0){
            AAC_eos = 1;
            break;
        }
        bytesRed += bytesRead;
        bytesRequired -= bytesRead;
        AAC_filePos += bytesRead;
    }
    pspDebugScreenSetXY(0, 19);
    pspDebugScreenPrintf("File position: %i / %i\n", AAC_filePos, (int)AAC_fileSize);
    return bytesRed;
}

/////////////////////////////////////////////////////////////////////////////////////////
//Audio callback
/////////////////////////////////////////////////////////////////////////////////////////
static void AACDecodeThread(void *buffer, unsigned int samplesToWrite, void *pdata){
    short *outBuffer = (short *)buffer;
    static short AAC_mixBuffer[PSP_NUM_AUDIO_SAMPLES * 2 * 2]__attribute__ ((aligned(64)));
    AACFrameInfo aacFrameInfo;
    static int AACSourceBufferSize = 0;
    static unsigned char *inputBuffer = NULL;
    int offset;

	if (AAC_isPlaying) {	// Playing , so mix up a buffer
        if (inputBuffer == NULL){
            AACSourceBufferSize = AAC_fillFileBuffer(AACfileBuffer, AAC_FILE_BUFFER_SIZE);
            inputBuffer = AACfileBuffer;
            offset = AACFindSyncWord(inputBuffer, AACSourceBufferSize);
            if (offset > 0){
                inputBuffer += offset;
                AACSourceBufferSize -= offset;
            }
        }

        while (samplesAvailable < samplesToWrite) {
            if (AACSourceBufferSize < 1024){
                memcpy(AACfileBuffer, inputBuffer, AACSourceBufferSize);
                AACSourceBufferSize += AAC_fillFileBuffer(AACfileBuffer + AACSourceBufferSize, AAC_FILE_BUFFER_SIZE - AACSourceBufferSize);
                inputBuffer = AACfileBuffer;
                offset = AACFindSyncWord(inputBuffer, AACSourceBufferSize);
                if (offset > 0){
                    inputBuffer += offset;
                    AACSourceBufferSize -= offset;
                }
            }

            unsigned long ret = AACDecode(hAACDecoder, &inputBuffer, &AACSourceBufferSize, &AAC_mixBuffer[samplesAvailable * 2]);
            if (ret == ERR_AAC_INDATA_UNDERFLOW) {
                AACSourceBufferSize = AAC_fillFileBuffer(AACfileBuffer, AAC_FILE_BUFFER_SIZE);
                inputBuffer = AACfileBuffer;
                offset = AACFindSyncWord(inputBuffer, AACSourceBufferSize);
                if (offset > 0){
                    inputBuffer += offset;
                    AACSourceBufferSize -= offset;
                }
                continue;
            } else if (ret == ERR_AAC_NONE){
                AACGetLastFrameInfo(hAACDecoder, &aacFrameInfo);
                pspDebugScreenSetXY(0, 20);
                pspDebugScreenPrintf("Frame's info: bitrate=%i nChans=%i sampRateOut=%i profile=%i\n", aacFrameInfo.bitRate, aacFrameInfo.nChans, aacFrameInfo.sampRateOut, aacFrameInfo.profile);
            }else{
                pspDebugScreenSetXY(0, 20);
                pspDebugScreenPrintf("Decoder error %li filePos: %i\n", ret, AAC_filePos);
                AAC_eos = 1;
                return;
            }
            samplesAvailable += aacFrameInfo.outputSamps;
        }

        if (samplesAvailable >= samplesToWrite){
            int count, count2;
            short *_buf2;
            for (count = 0; count < samplesToWrite; count++) {
                count2 = count + count;
                _buf2 = outBuffer + count2;
                // Double up for stereo
                *(_buf2) = AAC_mixBuffer[count2];
                *(_buf2 + 1) = AAC_mixBuffer[count2 + 1];
            }
            //  Move the pointers
            samplesAvailable -= samplesToWrite;
            //  Now shuffle the buffer along
            for (count = 0; count < samplesAvailable * 2; count++)
                AAC_mixBuffer[count] = AAC_mixBuffer[samplesToWrite * 2 + count];
        }
    } else {			//  Not Playing , so clear buffer
        int count;
        for (count = 0; count < samplesToWrite * 2; count++)
            *(outBuffer + count) = 0;
	}
}



void getAACTagInfo(char *filename, struct fileInfo *targetInfo){
    //ID3:
    struct ID3Tag ID3;
    strcpy(AAC_fileName, filename);
    ID3 = ParseID3(filename);
    strcpy(targetInfo->title, ID3.ID3Title);
    strcpy(targetInfo->artist, ID3.ID3Artist);
    strcpy(targetInfo->album, ID3.ID3Album);
    strcpy(targetInfo->year, ID3.ID3Year);
    strcpy(targetInfo->genre, ID3.ID3GenreText);
    strcpy(targetInfo->trackNumber, ID3.ID3TrackText);
    targetInfo->encapsulatedPictureOffset = ID3.ID3EncapsulatedPictureOffset;
}

int AACgetInfo(){
    getAACTagInfo(AAC_fileName, &AAC_info);

    AAC_info.length = 0;
    strcpy(AAC_info.strLength, "00:00:00");
    AAC_info.hz = 44100;
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//Init:
/////////////////////////////////////////////////////////////////////////////////////////
void AAC_Init(int channel){
    initAudioLib();
    MIN_PLAYING_SPEED=-10;
    MAX_PLAYING_SPEED=9;
    AAC_audio_channel = channel;
    pspAudioSetChannelCallback(AAC_audio_channel, AACDecodeThread, NULL);
    hAACDecoder = (HAACDecoder *) AACInitDecoder();

}


/////////////////////////////////////////////////////////////////////////////////////////
//Load a file:
/////////////////////////////////////////////////////////////////////////////////////////
int AAC_Load(char *filename){
	AAC_isPlaying = 0;
	AAC_eos = 0;
    AAC_playingSpeed = 0;
    samplesAvailable = 0;
    AAC_filePos = 0;
    AAC_fileSize = 0;

    initFileInfo(&AAC_info);
    AAC_fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if (AAC_fd < 0)
        return ERROR_OPENING;
    AAC_fileSize = sceIoLseek32(AAC_fd, 0, PSP_SEEK_END);
	AAC_filePos  = ID3v2TagSize(filename);

	sceIoLseek32(AAC_fd, AAC_filePos, PSP_SEEK_SET);

	strcpy(AAC_fileName, filename);

    if (AACgetInfo() != 0){
        strcpy(AAC_fileName, "");
        sceIoClose(AAC_fd);
        AAC_fd = -1;
        return ERROR_OPENING;
    }

    //Controllo il sample rate:
    if (pspAudioSetFrequency(AAC_info.hz) < 0)
        return ERROR_INVALID_SAMPLE_RATE;

    /*AACFrameInfo aacFrameInfo;
    aacFrameInfo.nChans = 2;
    aacFrameInfo.sampRateCore = AAC_info.hz;
    aacFrameInfo.profile = 1;
    AACSetRawBlockParams(hAACDecoder, 0, &aacFrameInfo);*/
    return OPENING_OK;
}

int AAC_Play(){
	AAC_isPlaying = 1;
	return 0;
}

void AAC_Pause(){
	AAC_isPlaying = !AAC_isPlaying;
}

int AAC_Stop(){
	AAC_isPlaying = 0;
	return 0;
}

void AAC_FreeTune(){
    if (hAACDecoder) {
        AACFreeDecoder(hAACDecoder);
        hAACDecoder = NULL;
    }
    sceIoClose(AAC_fd);
    AAC_fd = -1;
}

void AAC_GetTimeString(char *dest){
    strcpy(dest, "00:00:00");
}


int AAC_EndOfStream(){
	return AAC_eos;
}

struct fileInfo AAC_GetInfo(){
	return AAC_info;
}


struct fileInfo AAC_GetTagInfoOnly(char *filename){
    struct fileInfo tempInfo;
    initFileInfo(&tempInfo);
    getAACTagInfo(filename, &tempInfo);
    return tempInfo;
}


int AAC_GetPercentage(){
	float perc;

    if (AAC_fileSize > 0){
        perc = (float)AAC_filePos / (float)AAC_fileSize * 100.0;
    }else{
        perc = 0;
    }
    return((int)perc);
}

void AAC_End(){
    AAC_Stop();
	pspAudioSetChannelCallback(AAC_audio_channel, 0, 0);
	AAC_FreeTune();
	endAudioLib();
}

int AAC_setMute(int onOff){
    return setMute(AAC_audio_channel, onOff);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fade out:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void AAC_fadeOut(float seconds){
    fadeOut(AAC_audio_channel, seconds);
}


void AAC_setVolumeBoost(int boost){
    AAC_volume_boost = boost;
}

int AAC_getVolumeBoost(){
	return AAC_volume_boost;
}


int AAC_setPlayingSpeed(int playingSpeed){
	if (playingSpeed >= MIN_PLAYING_SPEED && playingSpeed <= MAX_PLAYING_SPEED){
		AAC_playingSpeed = playingSpeed;
		if (playingSpeed == 0)
			setVolume(AAC_audio_channel, 0x8000);
		else
			setVolume(AAC_audio_channel, FASTFORWARD_VOLUME);
		return 0;
	}else{
		return -1;
	}
}

int AAC_getPlayingSpeed(){
	return AAC_playingSpeed;
}

int AAC_GetStatus(){
	return 0;
}

void AAC_setVolumeBoostType(char *boostType){
    //Only old method supported
    MAX_VOLUME_BOOST = 4;
    MIN_VOLUME_BOOST = 0;
}


//Functions for filter (equalizer):
int AAC_setFilter(double tFilter[32], int copyFilter){
	return 0;
}

void AAC_enableFilter(){}

void AAC_disableFilter(){}

int AAC_isFilterSupported(){
	return 0;
}

int AAC_isFilterEnabled(){
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage suspend:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int AAC_suspend(){
    return 0;
}

int AAC_resume(){
    return 0;
}
