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
    char buffer[256];
    message message;

    // make a test of the protocol
    const char * test_message = "$SACCP*0\n$garbage,ignore\n$SLOCA,1,,212,1,32,12,,*97\n";

    strcpy(buffer, test_message);

    message = readMessage(0, buffer);

    printf("Message with: status=%s, param count: %d, checksum: %d, restbuffer: %s", message.status, message.parameter_count, message.checksum, message.restbuffer);

    strcpy(buffer, message.restbuffer);

    message = readMessage(0, buffer);

    printf("Message with: status=%s, param count: %d, checksum: %d, restbuffer: %s", message.status, message.parameter_count, message.checksum, message.restbuffer);

    // TODO make a connection to server, setup display etc.
}
