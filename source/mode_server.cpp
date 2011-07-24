#include <nds.h>
#include <maxmod9.h>
#include <stdio.h>

// Includes
#include "audio.h"
#include "video.h"
#include "backgrounds.h"
#include "sprites.h"
#include "ball.h"
#include "player.h"
#include "stats.h"
#include "protocol.h"


// Methods

void mode_server(void) {
    consoleDemoInit();

    // buffer for messaging
    char buffer[256];
    message message;

    // test message for the protocol on read input
    const char * test_message = "$SACCP*66\n$garbage,ignore\n$SLOCA,1,,212,1,32,12,,*97\n";

    strcpy(buffer, test_message);

    printf("Test message is:\n%s", test_message);
    printf("\n");

    // test protocol: send a CREQU
    strcpy(message.status, "CREQU");
    message.parameter_count = 0;
    sendMessage(0, message);

    // read first message: SACCP
    message = readMessage(0, buffer);

    printf("Got message: status: %s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);
    printf("Parameters: ");
    for (uint i = 0; i < message.parameter_count; i++) printf("%d,", message.parameters[i]);
    printf("\n\n");

    // send a CMOVE (skipping some stages in the communication like CPING etc.)
    if (strcmp(message.status, "SACCP") == 0) {
        strcpy(message.status, "CMOVE");
        int params[3] = {1, 2, 19283};
        message.parameters = params;
        message.parameter_count = 3;
        sendMessage(0, message);
    }

    // read second message: SLOCA (skipping some stages in the communication like SPING etc.)
    strcpy(buffer, message.restbuffer);
    
    message = readMessage(0, buffer);

    printf("Got message: status=%s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);
    printf("Parameters: ");
    for (uint i = 0; i < message.parameter_count; i++) printf("%d,", message.parameters[i]);
    printf("\n\n");

    // read again, but the socket should be 'closed' now
    strcpy(buffer, message.restbuffer);

    message = readMessage(0, buffer);

    printf("Got message: status=%s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);

    // TODO make a connection to server, setup display etc.
}
