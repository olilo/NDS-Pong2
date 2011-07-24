#include <nds.h>

// Types

typedef struct {
    char status[6];

    int * parameters;

    int parameter_count;

    int checksum;

    char * restbuffer;
} message;


// Methods

message readMessage(int socket, char * buffer);
