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
    char statuscode[5];
    int statuscode_length = 0;
    char checksum[10];
    int checksum_length = 0;

    item * curr, * previous, * head;
    previous = NULL;
    head = NULL;

    char param_str[10];
    int param_str_length = 0;

    // read from the socket while it is still open (0 means closed)
    while (received_length != 0) {
        if (received_length > 0) {
            // data was received :)
            
            // protocol handling

            // TODO protocol handling without finite-state machine to improve performance
            /* first draft:
            // check that the message starts with a $
            if (buffer[0] == '$') {
                buffer++;
            } else {
                // wrong start of message
                error = true;
            }

            // get status code
            char statuscode[5];
            for (int i = 0; i < 5; i++) {
                
            strncpy(statuscode, buffer, 5);
            buffer += 5;

            for (int i = 0; i < received_length; i++) {
                char currentChar = buffer[i];

            }
            */

            for (int i = 0; i < received_length; i++) {
                switch (parsestate) {
                    case START:
                        if (buffer[i] == '$') {
                            parsestate = STATUS_DECODE;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case STATUS_DECODE:
                        if (buffer[i] == ',' && statuscode_length == 5) {
                            message.status = statuscode;
                            parsestate = PARAM_DECODE;
                        } else if (buffer[i] == '*' && statuscode_length == 5) {
                            message.status = statuscode;
                            parsestate = CHECKSUM;
                        } else if (buffer[i] >= 'A' && buffer[i] <= 'Z' && statuscode_length < 5) {
                            statuscode[statuscode_length++] = buffer[i];
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case PARAM_DECODE:
                        // TODO
                        if (buffer[i] >= '0' && buffer[i] <= '9') {
                            param_str[param_str_length++] = buffer[i];
                        } else if (buffer[i] == ',') {
                            parsestate = NEXT_PARAM;
                        } else if (buffer[i] == '*') {
                            parsestate = CHECKSUM;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case NEXT_PARAM:
                        // save previously parsed parameter
                        curr = (item *) malloc(sizeof(item));
                        param_str[param_str_length] = '\0';
                        curr->val = atoi(param_str);
                        curr->next = NULL;
                        if (previous != NULL) previous->next = curr;
                        previous = curr;
                        if (head == NULL) head = curr;

                        // initialize next parameter
                        param_str_length = 0;

                        // state change
                        if (buffer[i] == ',') {
                            // no state change necessary
                        } else if (buffer[i] >= '0' && buffer[i] <= '9') {
                            param_str[param_str_length++] = buffer[i];
                            parsestate = PARAM_DECODE;
                        } else if (buffer[i] == '*') {
                            parsestate = CHECKSUM;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    case CHECKSUM:
                        if (buffer[i] >= '0' && buffer[i] <= '9') {
                            checksum[checksum_length++] = buffer[i];
                        } else if (buffer[i] == '\n') {
                            message.checksum = atoi(checksum);
                            // TODO check message for correctness
                            parsestate = START;
                        } else {
                            parsestate = ERROR;
                        }
                        break;
                    default:
                        parsestate = ERROR;
                        break;
                }


                // after state change we arrived at START again: save the parameters, the rest of the buffer and return the message
                if (parsestate == START) {
                    message.restbuffer = buffer + i;
                    
                    // save parameters as array in message
                    int param_size = 0;
                    curr = head;
                    while (curr != NULL) {
                        param_size++;
                        curr = curr->next;
                    }
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

                    return message;
                }

                // parse state was error and we reached the end of the message: discard message, restart and parse the next one
                if (parsestate == ERROR && buffer[i] == '\n') {
                    parsestate = START;
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

