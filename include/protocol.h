#include <nds.h>

// Types

typedef struct {
    char status[6];

    int * parameters;

    uint parameter_count;

    int checksum;

    /**
     * The restbuffer is the remaining rest of the buffer on a readMessage
     * that did not belong to the message.
     * Can be copied to the main buffer with this method call:
     * strcpy(buffer, message.restbuffer);
     */
    char * restbuffer;
} message;

// parse states
enum parsestate {
    START,
    STATUS_DECODE,
    PARAM_DECODE,
    NEXT_PARAM,
    CHECKSUM,
    ERROR
};

// Methods

void sendMessage(int socket, message);

message readMessage(int socket, char * buffer);
