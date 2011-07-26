#include <nds.h>
//#include <dswifi9.h>
//#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "protocol.h"

// types

// linked list implementation for parameters in readMessage
struct list_el {
    int val;
    struct list_el * next;
};

typedef struct list_el item;


// methods

// TODO make real connection via wifi to server
/*
int openSocket() {
int my_socket = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in = AF_INET;
sain.sin_port = htons(9005);
sain.sin_addr.s_addr = "192.168.0.12";
}
*/

void sendMessage(int socket, message message) {
    // no parameters to encode: only encode status
    int checksum = 0;
    for (int i = 0; i < 6; i++) {
        checksum ^= message.status[i];
    }

    // TODO determine exact needed size of parambuf
    char parambuf[200];
    parambuf[0] = '\0';
    if (message.parameter_count > 0) {
        // we have parameters: construct parameter part first
        for (uint i = 0; i < message.parameter_count; i++) {
            sprintf(parambuf, "%s,%d", parambuf, message.parameters[i]);
        }

        // calculate checksum of parameter buffer
        for (uint i = 0; i < strlen(parambuf); i++) {
            checksum ^= parambuf[i];
        }
    }

    // TODO calculate (nearly) exact needed size of retbuf
    char retbuf[256];
    sprintf(retbuf, "$%s%s*%d\n", message.status, parambuf, checksum);

    // TODO send it over the socket
    printf("Message to send: %s", retbuf);
}

// override method defined in protocol.h
message readMessage(int socket, char * buffer) {
    // set received length to characters in buffer, or initialize at -1 if buffer is empty
    int received_length = strlen(buffer);
    if (received_length == 0) {
        received_length = -1;
    }

    // setup necessary variables for later
    message message;
    parsestate parsestate = START;

    // string buffers
    uint statuscode_length = 0;
    char checksum[10];
    uint checksum_length = 0;
    char param_str[10];
    uint param_str_length = 0;
    uint param_size = 0;

    // linked list implementation for parameters
    item * curr, * previous, * head;
    previous = NULL;
    head = NULL;

    // calculated checksum (is calculated on-the-fly during parsing)
    int calculated_checksum = 0;

    // read from the socket while it is still open (0 means closed)
    while (received_length != 0) {
        if (received_length < 0) {
            // TODO add real reading from wifi here
            received_length = 0;
        }

        // check if we have received any data from wifi ...
        if (received_length <= 0) {
            continue;
        }

        // data was received, yay :)
        printf("received data: %d chars...\n", received_length);

        // protocol handling
        // TODO handle protocol without finite-state machine to improve performance

        // go through every received character and check it
        for (int i = 0; i < received_length; i++) {

            // this works like a finite-state machine:
            // make an action depending on the current state and the current character of the message
            switch (parsestate) {
                case START:
                    // every message must start with a $, then we decode the status
                    if (buffer[i] == '$') {
                        parsestate = STATUS_DECODE;
                        //printf("Changed to STATUS_DECODE on %d\n", i);
                    } else {
                        parsestate = ERROR;
                    }
                    break;
                case STATUS_DECODE:
                    // get status code, must be exactly 5 characters long
                    if (buffer[i] >= 'A' && buffer[i] <= 'Z' && statuscode_length < 5) {
                        message.status[statuscode_length++] = buffer[i];
                        calculated_checksum ^= buffer[i];
                    } else if (buffer[i] == ',' && statuscode_length == 5) {
                        message.status[statuscode_length] = '\0';
                        parsestate = PARAM_DECODE;
                        calculated_checksum ^= buffer[i];
                        //printf("Changed to PARAM_DECODE on %d\n", i);
                    } else if (buffer[i] == '*' && statuscode_length == 5) {
                        message.status[statuscode_length] = '\0';
                        parsestate = CHECKSUM;
                        //printf("Changed to CHECKSUM on %d\n", i);
                    } else {
                        parsestate = ERROR;
                    }
                    break;
                case PARAM_DECODE:
                    // get next digit for parameter or change state
                    if (buffer[i] >= '0' && buffer[i] <= '9' && param_str_length < 10) {
                        param_str[param_str_length++] = buffer[i];
                        calculated_checksum ^= buffer[i];
                    } else if (buffer[i] == ',') {
                        parsestate = NEXT_PARAM;
                        calculated_checksum ^= buffer[i];
                        //printf("Changed to NEXT_PARAM on %d\n", i);
                    } else if (buffer[i] == '*') {
                        parsestate = CHECKSUM;
                        //printf("Changed to CHECKSUM on %d\n", i);
                    } else {
                        parsestate = ERROR;
                    }

                    // if state has changed: save parsed parameter
                    if (parsestate != PARAM_DECODE) {
                        // convert parameter to int
                        param_str[param_str_length] = '\0';
                        int param = atoi(param_str);
                        param_size++;

                        // linked list logic: add element
                        curr = (item *) malloc(sizeof(item));
                        curr->val = param;
                        curr->next = NULL;
                        if (head == NULL) head = curr;
                        if (previous != NULL) previous->next = curr;
                        previous = curr;
                    }
                    break;
                case NEXT_PARAM:
                    // add new parameter with value 0 if we encounter ',' or '*'
                    if (buffer[i] == ',' || buffer[i] == '*') {
                        param_size++;
                        curr = (item *) malloc(sizeof(item));
                        curr->val = 0;
                        curr->next = NULL;
                        previous->next = curr;
                        previous = curr;
                    }

                    // state change
                    if (buffer[i] == ',') {
                        // no state change, next time we analyze the next parameter
                        calculated_checksum ^= buffer[i];
                    } else if (buffer[i] >= '0' && buffer[i] <= '9') {
                        // next parameter: save encountered first digit and go to PARAM_DECODE
                        param_str[0] = buffer[i];
                        param_str_length = 1;
                        parsestate = PARAM_DECODE;
                        calculated_checksum ^= buffer[i];
                        //printf("Changed to PARAM_DECODE on %d\n", i);
                    } else if (buffer[i] == '*') {
                        parsestate = CHECKSUM;
                        //printf("Changed to CHECKSUM on %d\n", i);
                    } else {
                        parsestate = ERROR;
                    }
                    break;
                case CHECKSUM:
                    if (buffer[i] >= '0' && buffer[i] <= '9' && checksum_length < 10) {
                        checksum[checksum_length++] = buffer[i];
                    } else if (buffer[i] == '\n' && checksum_length > 0) {
                        checksum[checksum_length] = '\0';
                        message.checksum = atoi(checksum);

                        // check message for correctness
                        //printf("Checking checksum; calculated: %d, read: %d\n", calculated_checksum, message.checksum);
                        if (message.checksum == calculated_checksum) {
                            parsestate = START;
                        } else {
                            parsestate = ERROR;
                        }
                    } else {
                        parsestate = ERROR;
                    }
                    break;
                default:
                    parsestate = ERROR;
                    break;
            }


            // the state has changed and we arrived at START again:
            // save the parameters, the rest of the buffer and return the message
            if (parsestate == START) {
                // save parameters as array in message
                int params[param_size];
                curr = head;
                for (uint j = 0; j < param_size; j++) {
                    params[j] = curr->val;
                    previous = curr;
                    curr = curr->next;
                    free(previous);
                }
                message.parameters = params;
                message.parameter_count = param_size;

                // TODO this pointer arithmetic is probably not enough ...
                message.restbuffer = buffer + i + 1;

                return message;
            }

            // parse state was set to error and we reached the end of the message:
            // discard the message and restart, parsing the next one
            if (parsestate == ERROR && buffer[i] == '\n') {
                //printf("Message had an error, i is %d\n", i);
                parsestate = START;

                // we don't need to reset the contents of the message because they will be overwritten

                // make a full reset on all variables
                // string buffer indices; the buffers themselves don't need to be reset
                statuscode_length = 0;
                checksum_length = 0;
                param_str_length = 0;
                param_size = 0;

                // reset the linked list with the parameters and free memory
                curr = head;
                while (curr != NULL) {
                    previous = curr;
                    curr = curr->next;
                    free(previous);
                }
                previous = NULL;
                head = NULL;
            }

        }

        // if we arrive here then the for loop finished without a finished message
        // this means that we have to reset received_length
        // and in the next loop read some more data from the socket into our buffer
        received_length = -1;
    }

    // TODO what status do we return if the socket is closed?
    strcpy(message.status, "SCLOS");
    int params[0];
    message.parameters = params;
    message.parameter_count = 0;
    message.restbuffer = buffer;
    message.checksum = 0;
    return message;
}

