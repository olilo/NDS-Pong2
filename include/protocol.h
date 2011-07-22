#include <nds.h>

// Types

typedef struct {
    char* status;

    int * parameters;

    int parameter_count;

    int checksum;

    char * restbuffer;
} message;


// Methods

message readMessage(int socket, char * buffer);
