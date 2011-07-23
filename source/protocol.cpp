#include <nds.h>
//#include <dswifi9.h>
//#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "protocol.h"

// TODO make real connection via wifi to server
/*
my_socket = socket(AF_INET, SOCK_STREAM, 0);

struct sockaddr_in = AF_INET;
sain.sin_port = htons(9005);
sain.sin_addr.s_addr = "192.168.0.12";
*/

// linked list implementation for parameters
struct list_el {
    int val;
    struct list_el * next;
};

typedef struct list_el item;

// parse states
enum parsestate {
    START,
    STATUS_DECODE,
    PARAM_DECODE,
    NEXT_PARAM,
    CHECKSUM,
    ERROR
};

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
    char statuscode[5];
    int statuscode_length = 0;
    char checksum[10];
    int checksum_length = 0;
    char param_str[10];
    int param_str_length = 0;
    int param_size = 0;

    // linked list implementation for parameters
    item * curr, * previous, * head;
    previous = NULL;
    head = NULL;

    // calculated checksum (is calculated on-the-fly during parsing)
    int calculated_checksum = 0;

    // read from the socket while it is still open (0 means closed)
    while (received_length != 0) {
        if (received_length > 0) {
            // data was received, yay :)
            
            // protocol handling
            // TODO handle protocol without finite-state machine to improve performance

            // go through every received character and check it
            for (int i = 0; i < received_length; i++) {

                // this works like a finite-state machine:
                // make an action depending on the current state and the current character
                switch (parsestate) {
                    case START:
                        if (buffer[i] == '$') {
                            parsestate = STATUS_DECODE;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case STATUS_DECODE:
                        calculated_checksum ^= buffer[i];

                        // get status code, must be exactly 5 characters long
                        if (buffer[i] >= 'A' && buffer[i] <= 'Z' && statuscode_length < 5) {
                            statuscode[statuscode_length++] = buffer[i];
                        } else if (buffer[i] == ',' && statuscode_length == 5) {
                            message.status = statuscode;
                            parsestate = PARAM_DECODE;
                        } else if (buffer[i] == '*' && statuscode_length == 5) {
                            message.status = statuscode;
                            parsestate = CHECKSUM;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case PARAM_DECODE:
                        calculated_checksum ^= buffer[i];

                        // get next digit for parameter or change state
                        if (buffer[i] >= '0' && buffer[i] <= '9' && param_str_length < 10) {
                            param_str[param_str_length++] = buffer[i];
                        } else if (buffer[i] == ',') {
                            parsestate = NEXT_PARAM;
                        } else if (buffer[i] == '*') {
                            parsestate = CHECKSUM;
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
                        calculated_checksum ^= buffer[i];

                        // initialize next parameter
                        param_str_length = 0;

                        // state change
                        if (buffer[i] == ',') {
                            // no state change, but empty parameter: add parameter with value 0
                            // we don't need to check head or previous because
                            // both must have been set previously by PARAM_DECODE
                            param_size++;
                            curr = (item *) malloc(sizeof(item));
                            curr->val = 0;
                            curr->next = NULL;
                            previous->next = curr;
                            previous = curr;
                        } else if (buffer[i] >= '0' && buffer[i] <= '9') {
                            param_str[0] = buffer[i];
                            param_str_length = 1;
                            parsestate = PARAM_DECODE;
                        } else if (buffer[i] == '*') {
                            parsestate = CHECKSUM;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case CHECKSUM:
                        if (buffer[i] >= '0' && buffer[i] <= '9' && checksum_length < 10) {
                            checksum[checksum_length++] = buffer[i];
                        } else if (buffer[i] == '\n' && checksum_length > 0) {
                            message.checksum = atoi(checksum);

                            // check message for correctness
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
                    for (int j = 0; j < param_size; j++) {
                        params[j] = curr->val;
                        previous = curr;
                        curr = curr->next;
                        free(previous);
                    }
                    message.parameters = params;
                    message.parameter_count = param_size;

                    // TODO this pointer arithmetic is probably not enough ...
                    message.restbuffer = buffer + i;

                    return message;
                }

                // parse state was set to error and we reached the end of the message:
                // discard the message and restart, parsing the next one
                if (parsestate == ERROR && buffer[i] == '\n') {
                    parsestate = START;

                    // TODO make a full reset on all variables

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
        }

        // TODO add real reading from wifi here
    }

    // TODO what do we do if the socket is closed?
    message.status = "SCLOS";
    message.restbuffer = buffer;
    return message;
}

