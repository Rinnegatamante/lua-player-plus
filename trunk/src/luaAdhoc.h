#ifndef PSP_ADHOC_H
#define PSP_ADHOC_H

/** @defgroup pgeAdhoc Adhoc Library
 *  @{
 */

// INCLUDES
#include <psptypes.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet.h>

#define PGE_ADHOC_EVENT_ERROR            0x0001
#define PGE_ADHOC_EVENT_CONNECT            0x0002
#define PGE_ADHOC_EVENT_DISCONNECT        0x0004
#define PGE_ADHOC_EVENT_SCAN            0x0008
#define PGE_ADHOC_EVENT_GAMEMODE        0x0010
#define PGE_ADHOC_EVENT_CANCEL            0x0020

#define PGE_ADHOC_MATCHING_EVENT_NONE            0
#define PGE_ADHOC_MATCHING_EVENT_HELLO            1
#define PGE_ADHOC_MATCHING_EVENT_REQUEST        2
#define PGE_ADHOC_MATCHING_EVENT_LEAVE            3
#define PGE_ADHOC_MATCHING_EVENT_DENY            4
#define PGE_ADHOC_MATCHING_EVENT_CANCEL            5
#define PGE_ADHOC_MATCHING_EVENT_ACCEPT            6
#define PGE_ADHOC_MATCHING_EVENT_ESTABLISHED    7
#define PGE_ADHOC_MATCHING_EVENT_TIMEOUT        8
#define PGE_ADHOC_MATCHING_EVENT_ERROR            9
#define PGE_ADHOC_MATCHING_EVENT_BYE            10

#define PGE_ADHOC_TYPE_NORMAL            0
#define PGE_ADHOC_TYPE_GAMESHARING        2

#define PGE_ADHOC_MATCHING_MODE_GAME    0
#define PGE_ADHOC_MATCHING_MODE_PTP        1

#define PGE_ADHOC_MATCHING_TYPE_HOST    0
#define PGE_ADHOC_MATCHING_TYPE_CLIENT    1

typedef struct
{
    char mac[18];
    char nickname[128];
    char hello[128];
    int event;
    
} pgeAdhocPeerEvent;

// PROTOTYPES

int pgeAdhocInit(int type);

int pgeAdhocShutdown(void);

int pgeAdhocGameHost(const char *name);

int pgeAdhocConnect(const char *name);

int pgeAdhocPeerExists(const char *mac);

int pgeAdhocMatchingInit(int mode, int type);

int pgeAdhocMatchingStart(const char *hello);

int pgeAdhocMatchingAccept(const char *mac);

int pgeAdhocMatchingDecline(const char *mac);

int pgeAdhocMatchingShutdown(void);

pgeAdhocPeerEvent* pgeAdhocMatchingGetEvents(void);

int pgeAdhocMatchingClearEvent(pgeAdhocPeerEvent *event);

int pgeAdhocPtpHostStart(void);

int pgeAdhocPtpClientStart(const char *servermac);

int pgeAdhocPtpReceive(void *data, int *length);

int pgeAdhocPtpSend(void *data, int *length);

int pgeAdhocPtpCheckForData(void);

int pgeAdhocPtpFlush(void);

int pgeAdhocPtpHostShutdown(void);

int pgeAdhocPtpClientShutdown(void);

int pgeAdhocGetState(void);

int pgeAdhocGetError(void);

/** @} */

#endif

