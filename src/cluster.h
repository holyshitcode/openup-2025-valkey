#ifndef __CLUSTER_H
#define __CLUSTER_H

/*-----------------------------------------------------------------------------
 * Cluster exported API.
 *----------------------------------------------------------------------------*/

#define CLUSTER_SLOT_MASK_BITS 14                   /* Number of bits used for slot id. */
#define CLUSTER_SLOTS (1 << CLUSTER_SLOT_MASK_BITS) /* Total number of slots in cluster mode, which is 16384. */
#define CLUSTER_OK 0                                /* Everything looks ok */
#define CLUSTER_FAIL 1                              /* The cluster can't work */
#define CLUSTER_NAMELEN 40                          /* sha1 hex length */

/* Reason why the cluster state changes to fail. When adding new reasons,
 * make sure to update clusterLogFailReason. */
#define CLUSTER_FAIL_NONE 0
#define CLUSTER_FAIL_NOT_FULL_COVERAGE 1
#define CLUSTER_FAIL_MINORITY_PARTITION 2

/* Redirection errors returned by getNodeByQuery(). */
#define CLUSTER_REDIR_NONE 0          /* Node can serve the request. */
#define CLUSTER_REDIR_CROSS_SLOT 1    /* -CROSSSLOT request. */
#define CLUSTER_REDIR_UNSTABLE 2      /* -TRYAGAIN redirection required */
#define CLUSTER_REDIR_ASK 3           /* -ASK redirection required. */
#define CLUSTER_REDIR_MOVED 4         /* -MOVED redirection required. */
#define CLUSTER_REDIR_DOWN_STATE 5    /* -CLUSTERDOWN, global state. */
#define CLUSTER_REDIR_DOWN_UNBOUND 6  /* -CLUSTERDOWN, unbound slot. */
#define CLUSTER_REDIR_DOWN_RO_STATE 7 /* -CLUSTERDOWN, allow reads. */

typedef struct _clusterNode clusterNode;
struct clusterState;

/* Flags that a module can set in order to prevent certain Cluster
 * features to be enabled. Useful when implementing a different distributed
 * system on top of Cluster message bus, using modules. */
#define CLUSTER_MODULE_FLAG_NONE 0
#define CLUSTER_MODULE_FLAG_NO_FAILOVER (1 << 1)
#define CLUSTER_MODULE_FLAG_NO_REDIRECTION (1 << 2)

/* ---------------------- API exported outside cluster.c -------------------- */
/* functions requiring mechanism specific implementations */
void clusterInit(void);
void clusterInitLast(void);
void clusterCron(void);
void clusterBeforeSleep(void);
int verifyClusterConfigWithData(void);
void clusterHandleServerShutdown(void);

int clusterSendModuleMessageToTarget(const char *target,
                                     uint64_t module_id,
                                     uint8_t type,
                                     const char *payload,
                                     uint32_t len);

void clusterUpdateMyselfFlags(void);
void clusterUpdateMyselfIp(void);
void clusterUpdateMyselfClientIpV4(void);
void clusterUpdateMyselfClientIpV6(void);
void clusterUpdateMyselfHostname(void);
void clusterUpdateMyselfAnnouncedPorts(void);
void clusterUpdateMyselfHumanNodename(void);

void clusterPropagatePublish(robj *channel, robj *message, int sharded);

unsigned long getClusterConnectionsCount(void);
int isClusterHealthy(void);

sds clusterGenNodesDescription(client *c, int filter, int tls_primary);
sds genClusterInfoString(void);
/* handle implementation specific debug cluster commands. Return 1 if handled, 0 otherwise. */
int handleDebugClusterCommand(client *c);
const char **clusterDebugCommandExtendedHelp(void);
/* handle implementation specific cluster commands. Return 1 if handled, 0 otherwise. */
int clusterCommandSpecial(client *c);
const char **clusterCommandExtendedHelp(void);

int clusterAllowFailoverCmd(client *c);
void clusterPromoteSelfToPrimary(void);
mstime_t clusterManualFailoverTimeLimit(void);

void clusterCommandSlots(client *c);
void clusterCommandMyId(client *c);
void clusterCommandMyShardId(client *c);
void clusterCommandShards(client *c);
sds clusterGenNodeDescription(client *c, clusterNode *node, int tls_primary);

int clusterNodeCoversSlot(clusterNode *n, int slot);
int getNodeDefaultClientPort(clusterNode *n);
clusterNode *getMyClusterNode(void);
int getClusterSize(void);
int getMyShardSlotCount(void);
int clusterNodePending(clusterNode *node);
int clusterNodeIsPrimary(clusterNode *n);
char **getClusterNodesList(size_t *numnodes);
char *clusterNodeIp(clusterNode *node, client *c);
int clusterNodeIsReplica(clusterNode *node);
clusterNode *clusterNodeGetPrimary(clusterNode *node);
char *clusterNodeGetName(clusterNode *node);
int clusterNodeTimedOut(clusterNode *node);
int clusterNodeIsFailing(clusterNode *node);
int clusterNodeIsNoFailover(clusterNode *node);
char *clusterNodeGetShardId(clusterNode *node);
int clusterNodeNumReplicas(clusterNode *node);
clusterNode *clusterNodeGetReplica(clusterNode *node, int replica_idx);
clusterNode *getMigratingSlotDest(int slot);
clusterNode *getImportingSlotSource(int slot);
clusterNode *getNodeBySlot(int slot);
int clusterNodeClientPort(clusterNode *n, int use_tls);
char *clusterNodeHostname(clusterNode *node);
const char *clusterNodePreferredEndpoint(clusterNode *n, client *c);
clusterNode *clusterLookupNode(const char *name, int length);
int detectAndUpdateCachedNodeHealth(void);
client *createCachedResponseClient(int resp);
void deleteCachedResponseClient(client *recording_client);
void clearCachedClusterSlotsResponse(void);
unsigned int countKeysInSlot(unsigned int hashslot);
int getSlotOrReply(client *c, robj *o);

/* functions with shared implementations */
int clusterNodeIsMyself(clusterNode *n);
clusterNode *getNodeByQuery(client *c, struct serverCommand *cmd, robj **argv, int argc, int *hashslot, int *ask);
int clusterRedirectBlockedClientIfNeeded(client *c);
void clusterRedirectClient(client *c, clusterNode *n, int hashslot, int error_code);
void migrateCloseTimedoutSockets(void);
unsigned int keyHashSlot(char *key, int keylen);
int patternHashSlot(char *pattern, int length);
int isValidAuxString(char *s, unsigned int length);
void migrateCommand(client *c);
void clusterCommand(client *c);
ConnectionType *connTypeOfCluster(void);
int isNodeAvailable(clusterNode *node);
long long getNodeReplicationOffset(clusterNode *node);
sds aggregateClientOutputBuffer(client *c);
void resetClusterStats(void);
#endif /* __CLUSTER_H */
