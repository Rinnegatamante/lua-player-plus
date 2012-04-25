#ifndef PSP_ADHOC_H
#define PSP_ADHOC_H

/** @defgroup Adhoc Adhoc Library
 *  @{
 */

// INCLUDES
#include <psptypes.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet.h>

#define _ADHOC_EVENT_ERROR			0x0001
#define _ADHOC_EVENT_CONNECT			0x0002
#define _ADHOC_EVENT_DISCONNECT		0x0004
#define _ADHOC_EVENT_SCAN			0x0008
#define _ADHOC_EVENT_GAMEMODE		0x0010
#define _ADHOC_EVENT_CANCEL			0x0020

#define _ADHOC_MATCHING_EVENT_NONE			0
#define _ADHOC_MATCHING_EVENT_HELLO			1
#define _ADHOC_MATCHING_EVENT_REQUEST		2
#define _ADHOC_MATCHING_EVENT_LEAVE			3
#define _ADHOC_MATCHING_EVENT_DENY			4
#define _ADHOC_MATCHING_EVENT_CANCEL			5
#define _ADHOC_MATCHING_EVENT_ACCEPT			6
#define _ADHOC_MATCHING_EVENT_ESTABLISHED	7
#define _ADHOC_MATCHING_EVENT_TIMEOUT		8
#define _ADHOC_MATCHING_EVENT_ERROR			9
#define _ADHOC_MATCHING_EVENT_BYE			10

#define _ADHOC_TYPE_NORMAL			0
#define _ADHOC_TYPE_GAMESHARING		2

#define _ADHOC_MATCHING_MODE_GAME	0
#define _ADHOC_MATCHING_MODE_PTP		1

#define _ADHOC_MATCHING_TYPE_HOST	0
#define _ADHOC_MATCHING_TYPE_CLIENT	1

typedef struct
{
	char mac[18];
	char nickname[128];
	char hello[128];
	int event;
	
} AdhocPeerEvent;

// PROTOTYPES

int AdhocInit(int type);

int AdhocShutdown(void);

int AdhocGameHost(const char *name);

int AdhocConnect(const char *name);

int AdhocPeerExists(const char *mac);

int AdhocMatchingInit(int mode, int type);

int AdhocMatchingStart(const char *hello);

int AdhocMatchingAccept(const char *mac);

int AdhocMatchingDecline(const char *mac);

int AdhocMatchingShutdown(void);

AdhocPeerEvent* AdhocMatchingGetEvents(void);

int AdhocMatchingClearEvent(AdhocPeerEvent *event);

int AdhocPtpHostStart(void);

int AdhocPtpClientStart(const char *servermac);

int AdhocPtpReceive(void *data, int *length);

int AdhocPtpSend(void *data, int *length);

int AdhocPtpCheckForData(void);

int AdhocPtpFlush(void);

int AdhocPtpHostShutdown(void);

int AdhocPtpClientShutdown(void);

int AdhocGetState(void);

int AdhocGetError(void);

/** @} */

#endif
