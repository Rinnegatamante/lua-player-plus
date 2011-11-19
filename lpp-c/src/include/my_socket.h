/*
 * LuaPlayer Euphoria
 * ------------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE for details.
 *
 * Copyright (c) 2005 Frank Buss <fb@frank-buss.de> (aka Shine)
 * Copyright (c) 2009 Danny Glover <danny86@live.ie> (aka Zack) 
 *
 * Official Forum : http://www.retroemu.com/forum/forumdisplay.php?f=148
 * For help using LuaPlayer, code help, tutorials etc please visit the official site : http://www.retroemu.com/forum/forumdisplay.php?f=148
 *
 * Credits:
 * 
 * (from Shine/Zack) 
 *
 *   many thanks to the authors of the PSPSDK from http://forums.ps2dev.org
 *   and to the hints and discussions from #pspdev on freenode.net
 *
 * (from Zack Only)
 *
 * Thanks to Brunni for the Swizzle/UnSwizzle code (taken from oslib). 
 * Thanks to Arshia001 for AALIB. It is the sound engine used in LuaPlayer Euphoria. 
 * Thanks to HardHat for being a supportive friend and advisor.
 * Thanks to Benhur for IntraFont.
 * Thanks to Jono for the moveToVram code.
 * Thanks to Raphael for the Vram manager code.
 * Thanks to Osgeld, Dan369 & Cmbeke for testing LuaPlayer Euphoria for me and coming up with some neat ideas for it.
 * Thanks to the entire LuaPlayer Euphoria userbase, for using it and for supporting it's development. You guys rock!
 *
 *
 */

// socket layer - patterned after Berkeley/WinSock sockets 
// functions in this header can be used directly after init
// if you prefer,
//     you can #define them to the regular "socket()", "send()", "recv()" names

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

///////////////////////////////////////////////////////////////////////
