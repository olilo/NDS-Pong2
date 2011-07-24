#include <nds.h>

// Types

typedef struct {
    char status[6];

    int * parameters;

    int parameter_count;

    int checksum;

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
