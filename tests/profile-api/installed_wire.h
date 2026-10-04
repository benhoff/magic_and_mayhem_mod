/* Test-only, versioned little-endian records; no host pointers on disk. */
#define PROFILE_MAGIC 0x504e4d4dU
#define PROFILE_VERSION 1U
#define PROFILE_CAPACITY 16384U
#define PROFILE_QUERY_LIMIT 4096U
struct ProfileQuery {unsigned int op,capacity,fallback;char section[128],key[128];};
struct ProfileRecord {unsigned int returned;char bytes[PROFILE_CAPACITY];};
