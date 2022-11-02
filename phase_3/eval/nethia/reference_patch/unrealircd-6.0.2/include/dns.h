/* OMG... OMG! WHAT AN INCLUDE HORROR !!! */
#include <ares.h>
#include <ares_version.h>

typedef enum {
	DNSREQ_CLIENT = 1,
	DNSREQ_LINKCONF = 2,
	DNSREQ_CONNECT = 3
} DNSReqType;

typedef struct DNSReq DNSReq;

/* Depending on the request type, some fields are filled in:
 * cptr: DNSREQ_CLIENT, DNSREQ_CONNECT
 * link: DNSREQ_LINKCONF, DNSREQ_CONNECT
 */

struct DNSReq {
	DNSReq *prev, *next;
	char *name; /**< Name being resolved (only for DNSREQ_LINKCONF and DNSREQ_CONNECT) */
	char ipv6; /**< Resolving for ipv6 or ipv4? */
	DNSReqType type; /**< DNS Request type (DNSREQ_*) */
	Client *client; /**< Client the request is for, NULL if client died OR unavailable */
	ConfigItem_link *linkblock; /**< Linkblock */
};

typedef struct DNSCache DNSCache;

struct DNSCache {
	DNSCache *prev, *next;		/**< Previous and next in linked list */
	DNSCache *hprev, *hnext;	/**< Previous and next in hash list */
	char *name;					/**< The hostname */
	char *ip;					/**< The IP address */
	time_t expires;				/**< When record expires */
};

typedef struct DNSStats DNSStats;

struct DNSStats {
	unsigned int cache_hits;
	unsigned int cache_misses;
	unsigned int cache_adds;
};

/** Time to keep cache records. */
#define DNSCACHE_TTL			600

/** Size of the hash table (prime!).
 * Consumes <this>*4 on ia32 and <this>*4 on 64 bit
 * 241 seems a good bet.. which ~1k on ia32 and ~2k on ia64.
 */
#define DNS_HASH_SIZE	241

/** Max # of entries we want in our cache.
 * This:
 * a) prevents us from using too much memory, and
 * b) prevents us from keeping useless cache records
 *
 * A dnscache item is roughly ~80 bytes in size (slightly more on x86),
 * so 241*80=~20k, which seems reasonable ;).
 */
#define DNS_MAX_ENTRIES	DNS_HASH_SIZE


extern ares_channel resolver_channel;

extern void init_resolver(int);

struct hostent *unrealdns_doclient(Client *cptr);

extern void unreal_gethostbyname(const char *name, int family, ares_host_callback callback, void *arg);

