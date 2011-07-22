#include <nds.h>

// Types

typedef struct {
    char* status;

    int * parameters;

    int checksum;

    char * restbuffer;
} message;


// Methods

message readMessage(int socket, char * buffer);
