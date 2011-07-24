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

    // make a test of the protocol
    const char * test_message = "$SACCP*66\n$garbage,ignore\n$SLOCA,1,,212,1,32,12,,*97\n";

    printf("Test message is:\n%s", test_message);
    printf("\n");

    strcpy(buffer, test_message);

    message = readMessage(0, buffer);

    printf("Got message: status: %s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);
    printf("Parameters: ");
    for (int i = 0; i < message.parameter_count; i++) printf("%d,", message.parameters[i]);
    printf("\n\n");

    strcpy(buffer, message.restbuffer);

    message = readMessage(0, buffer);

    printf("Got message: status=%s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);
    printf("Parameters: ");
    for (int i = 0; i < message.parameter_count; i++) printf("%d,", message.parameters[i]);
    printf("\n\n");

    strcpy(buffer, message.restbuffer);

    message = readMessage(0, buffer);

    printf("Got message: status=%s, param count: %d, checksum: %d\n", message.status, message.parameter_count, message.checksum);

    // TODO make a connection to server, setup display etc.
}
