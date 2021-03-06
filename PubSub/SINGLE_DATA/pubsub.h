/* pubsub.h */

#define MAX_SUBSCRIBER 8
#define MAX_GROUP 256
#define MAX_TOPIC 256

//entry for pubsub function pointer
struct pubsubfp {
	pid32 pid;
	uint32 group_id;
	void (*handler)(topic16, uint32);
	// 1 - active 0 -inactive
	uint32 subscription_state;
};

// topic table entry
struct pubsubent {
	struct pubsubfp psfp_array[MAX_SUBSCRIBER];
	uint32 count;
};

//publishing queue entry
struct publishqueue {
	topic16 topic;
	uint32 data;	
};

//publishing queue
struct pubqueue {
	struct publishqueue *pubq;
	uint32 head;
	uint32 tail;
	uint32 count;	
};
