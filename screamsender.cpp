#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string>
#include <fcntl.h>

#define PACKET_DATA_SIZE 1152
#define PACKET_HEADER_SIZE 5
#define PACKET_SIZE (PACKET_DATA_SIZE + PACKET_HEADER_SIZE)

int zeroes = 0;

int main(int argc, char *argv[])  {

    int opt; 
    int iport, isamplerate;
    uint8_t ibitdepth, ichannels, samplerate_multiplier, samplerate_mask;
    bool samplerate_441_base;
    std::string ip = "";
    std::string port = "";
    std::string sample_rate = "48000";
    std::string bit_depth = "32";
    std::string channels = "2";
    uint8_t channel_map[] = {0x00, 0x00};
    uint8_t header[5] = {0};
    sockaddr_in destination;
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    char buffer_pending[(PACKET_DATA_SIZE * 2) + PACKET_HEADER_SIZE] = {0};
    char *buffer_pending_data = buffer_pending + PACKET_HEADER_SIZE;
    int buffer_pending_bytes = 0;
    int retval = 0;

    if(socket_fd<0){
        printf("cannot open socket\n");
        return false;
    }

    // Parse arguments
    while((opt = getopt(argc, argv, "i:p:s:b:c:m:h")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
                ip = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 's':
                sample_rate = optarg;
                break;
            case 'b':
                bit_depth = optarg;
                break;
            case 'c':
                printf("NOT IMPLEMENTED %c\n", optopt);
                break;
            case 'm':
                printf("NOT IMPLEMENTED %c\n", optopt);
                break;
            case 'h':
                printf("ScreamSender Help\n");
                printf("Usage: %s -i <IP> -p <port> -s <sample rate=48000> -b <bit depth=32>\n", argv[0]);
                return 3;
            case ':':
                printf("Option %c needs value\n", optopt);
                break;
            case '?':
                printf("Unknown option %c\n", optopt);
        }  
    }

    for(; optind < argc; optind++){      
        printf("extra arguments: %s\n", argv[optind]);  
    } 

    // Configure Socket Destination
    iport = atoi(port.c_str());
    bzero(&destination,sizeof(destination));
    destination.sin_family = AF_INET;
    destination.sin_addr.s_addr = inet_addr(ip.c_str());
    destination.sin_port = htons(iport);
    printf("Sending to %s:%i\n", ip.c_str(), iport);

    // Build Scream Header
    isamplerate = atoi(sample_rate.c_str());
    ibitdepth = atoi(bit_depth.c_str());
    ichannels = atoi(channels.c_str());

    samplerate_441_base = isamplerate % 44100 == 0;
    samplerate_multiplier = samplerate_441_base ? isamplerate / 44100 : isamplerate / 48000;
    samplerate_mask = (samplerate_441_base << 7) + samplerate_multiplier;

    header[0] = samplerate_mask;
    header[1] = ibitdepth;
    header[2] = ichannels;
    header[3] = channel_map[0];
    header[4] = channel_map[1];
    
    memcpy(buffer_pending, header, PACKET_HEADER_SIZE);
    
    // Send Data
    while (retval == 0) {
        // Read bytes in, append to the end of buffer_pending
        int bytes_read = read(STDIN_FILENO, buffer_pending_data + buffer_pending_bytes, PACKET_DATA_SIZE);
        if (bytes_read <= 0) {
            printf("Lost stdin: %i\n", bytes_read);
            retval = 4;
            continue;
        }
        bool continueWhile = false;
        for (int i=PACKET_HEADER_SIZE;i<PACKET_DATA_SIZE;i++) {
            if (buffer_pending[buffer_pending_bytes + i] == 0) {
                zeroes++;
                if (zeroes >= 1152*4) {
                    continueWhile = true;
                }
            }
            else {
                zeroes = 0;
                continueWhile = false;
                break;
            }
        }
        if (continueWhile)
            continue;
        buffer_pending_bytes = buffer_pending_bytes + bytes_read;
        // If there's >= PACKET_DATA_SIZE bytes then send PACKET_DATA_SIZE bytes
        if (buffer_pending_bytes >= PACKET_DATA_SIZE)
        {
            int result = sendto(socket_fd, buffer_pending, PACKET_SIZE, 0,
                    (sockaddr*)&destination, sizeof(destination));
            if (result >= 0)
            {
                // Shift the buffer up PACKET_DATA_SIZE bytes to remove the data that was just sent
                memcpy(buffer_pending_data,
                    buffer_pending_data + PACKET_DATA_SIZE,
                    PACKET_DATA_SIZE);
                buffer_pending_bytes -= PACKET_DATA_SIZE;
            }
            else
            {
                printf("Error sending message: %i\n", result);
                retval = result;
            }
        }
    }
    close(socket_fd);
    return retval;
}