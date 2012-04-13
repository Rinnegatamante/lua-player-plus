/*----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#------  This File is Part Of : ----------------------------------------------------------------------------------------#
#------- _  -------------------  ______   _   --------------------------------------------------------------------------#
#------ | | ------------------- (_____ \ | |  --------------------------------------------------------------------------#
#------ | | ---  _   _   ____    _____) )| |  ____  _   _   ____   ____   ----------------------------------------------#
#------ | | --- | | | | / _  |  |  ____/ | | / _  || | | | / _  ) / ___)  ----------------------------------------------#
#------ | |_____| |_| |( ( | |  | |      | |( ( | || |_| |( (/ / | |  --------------------------------------------------#
#------ |_______)\____| \_||_|  |_|      |_| \_||_| \__  | \____)|_|  --------------------------------------------------#
#------------------------------------------------- (____/  -------------------------------------------------------------#
#------------------------   ______   _   -------------------------------------------------------------------------------#
#------------------------  (_____ \ | |  -------------------------------------------------------------------------------#
#------------------------   _____) )| | _   _   ___   ------------------------------------------------------------------#
#------------------------  |  ____/ | || | | | /___)  ------------------------------------------------------------------#
#------------------------  | |      | || |_| ||___ |  ------------------------------------------------------------------#
#------------------------  |_|      |_| \____|(___/   ------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Licensed under the GPL License --------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Copyright (c) Nanni <cusunanni@hotmail.it> --------------------------------------------------------------------------#
#- Copyright (c) Rinnegatamante <rinnegatamante@eternalongju2.com> -----------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Official Forum : http://rinnegatamante.eu/luaplayerplus/forum.php ---------------------------------------------------#
#- For help using LuaPlayerPlus, coding help, and other please visit : http://rinnegatamante.eu/luaplayerplus/forum.php #
#-----------------------------------------------------------------------------------------------------------------------#
#- Credits : -----------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#- Homemister for LPHM sourcecode --------------------------------------------------------------------------------------#
#- Zack & Shine for LP Euphoria sourcecode -----------------------------------------------------------------------------#
#- ab5000 for support on psp-ita.com -----------------------------------------------------------------------------------#
#- valantin for sceIoMvdir and sceIoCpdir improved functions------------------------------------------------------------#
#- Dark_AleX for usbdevice ---------------------------------------------------------------------------------------------#
#- VirtuosFlame & ColdBird for iso drivers and kuBridge ----------------------------------------------------------------#
#- sakya for Media Engine and OslibMod ---------------------------------------------------------------------------------#
#- Booster & silverspring for EEPROM write/read functions --------------------------------------------------------------#
#- Akind for RemoteJoyLite ---------------------------------------------------------------------------------------------#
#- cooleyes for mpeg4 lib ----------------------------------------------------------------------------------------------#
#- Arshia001 for PSPAALib ----------------------------------------------------------------------------------------------#
#- InsertWittyName & MK2k for PGE sourcecode ---------------------------------------------------------------------------#
#- Youresam for LUA BMPLib ---------------------------------------------------------------------------------------------#
#- Raphael for vram manager code ---------------------------------------------------------------------------------------#
#- Dynodzzo for LSD concepts -------------------------------------------------------------------------------------------#
#- ab_portugal for Image.negative function -----------------------------------------------------------------------------#
#- JiCé for drawCircle function ----------------------------------------------------------------------------------------#
#- Rapper_skull & DarkGiovy for testing LuaPlayer Plus and coming up with some neat ideas for it. ----------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------#
#-----------------------------------------------------------------------------------------------------------------------*/

#ifndef __WLANL_H_
#define __WLANL_H_

#define SOCKET int
#define INVALID_SOCKET (0xFFFFFFFF)

struct sockaddr {
	unsigned char sa_size; // size, not used
	unsigned char sa_family;              /* address family */
	char    sa_data[14];            /* up to 14 bytes of direct address */
};

// Socket address, internet style.
struct sockaddr_in {
	unsigned char sin_size; // size, not used
	unsigned char sin_family; // usually AF_INET
	unsigned short sin_port; // use htons()
	u32 sin_addr;
	char sin_zero[8];
};

SOCKET sceNetInetSocket(int af, int type, int protocol);
int sceNetInetBind(SOCKET s, void* addr, int namelen);
int sceNetInetListen(SOCKET s, int backlog);
int sceNetInetAccept(SOCKET s, void* sockaddr, int* addrlen);

int sceNetInetConnect(SOCKET s, const void* name, int namelen);

int sceNetInetSend(SOCKET s, const void* buf, int len, int flags);
int sceNetInetSendto(SOCKET s, const void* buf, int len, int flags,
	const void* sockaddr_to, int tolen);
int sceNetInetRecv(SOCKET s, u8* buf, int len, int flags);
int sceNetInetRecvfrom(SOCKET s, u8* buf, int len, int flags,
	void* sockaddr_from, int* fromlen);

int sceNetInetGetErrno();
//REVIEW: need trial-and-error for some of these codes

int sceNetInetGetsockopt(SOCKET s, int level, int optname,
	char* optval, int* optlen);
int sceNetInetSetsockopt(SOCKET s, int level, int optname,
	const char* optval, int optlen);

// different ways of closing -- REVIEW: explore these more
int sceNetInetShutdown(SOCKET s, int how);
// standard shutdown
int sceNetInetClose(SOCKET s);
// standard close
int sceNetInetCloseWithRST(SOCKET s);
int sceNetInetSocketAbort(SOCKET s);
// REVIEW: also a "sceNetThreadAbort" in pspnet.prx

/////////////////////////////////////////////////////////////
// constants

// for 'socket()'
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */

// for Get/Set sockopt
//  NOTE: many do not work as you would expect (ie. setting RCVTIMEO)
//    experiment before relying on anything.
#define SOL_SOCKET      0xffff          /* options for socket level */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
// timeouts in microseconds
// NOTE: Getsockopt may not return the same value
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

#define SO_EXTRA1009    0x1009          /* u32 value - FIONBIO ? */

// experimental defaults: (interesting non-zero ones)
// getsockopt[SO_LINGER] = size=00000008a ??
// getsockopt[SO_ACCEPTCONN] = ERR=FFFFFFFF - at least for new socket()
// getsockopt[SO_SNDBUF] : data32=00004000
// getsockopt[SO_RCVBUF] : data32=00004000 - 16384 bytes TCP/IP
// getsockopt[SO_RCVBUF] : data32=0000A280 - 41600 bytes UDP/IP
// NOTE: recommend you keep it under ~30000 bytes
// getsockopt[SO_SNDLOWAT] : data32=00000800
// getsockopt[SO_RCVLOWAT] : data32=00000001
// getsockopt[SO_SNDTIMEO] : data32=00000000
// getsockopt[SO_RCVTIMEO] : data32=00000000 // ie. wait forever
// getsockopt[1009] : data32=00000000
//   can be set (returns data32=00000080)

////////////////////////////////////////////////////

// minimal or not tested, but *should* work
int sceNetInetGetpeername(SOCKET s, struct sockaddr* name, int* namelen);
int sceNetInetGetsockname(SOCKET s, struct sockaddr* name, int* namelen);
//REVIEW: need a "gethostbyname" workalike using Resolver API

// Other utilities
unsigned short htons(unsigned short wIn);
u32 sceNetInetInetAddr(const char* szIpAddr); // parse a ?.?.?.? string

///////////////////////////////////////////////////////////////////////
#if 0
// not yet tested

int sceNetInetPoll(...) ???

	int sceNetInetInetAton(...);
int sceNetInetInetNtop(...);
int sceNetInetInetPton(...);

// msghdr variants - need msghdr structure
int sceNetInetRecvmsg(SOCKET s, struct msghdr* msg, int flags);
int sceNetInetSendmsg(SOCKET s, const struct msghdr* msg, int flags);
int sceNetInetSelect(int nfds, fd_set* readfds, fd_set* writefds,
	fd_set* exceptfds, const struct timeval FAR *timeout);
#endif

void luaWlan_init(lua_State *L);

#endif /* __WLANL_H_ */