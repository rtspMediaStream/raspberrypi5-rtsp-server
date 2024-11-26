#include "utils.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"
#include "global.h"

#include <string>
#include <thread>
#include <iostream>

void showHelp(const char *thisName) {
    printf("usage : %s [OPTION]\n"
           " -h            help\n"
           " -m [media type] Audio or Video\n",
           thisName);
}

int main(int argc, char* argv[]) {
    int option;
    extern char *optarg;
    
    /// program option process
    while((option = getopt(argc, argv, "hm:")) != -1)
    {
        swtich(option)
        {
        case 'h':
            showHelp(argv[0]);
            exit(0);
        case 'm':
            std::string mode(optarg);
            if(mode == "Audio")
                g_serverStreamType = ServerStreamType::Audio;
            else if(mode == "Video")
                g_serverStreamType = ServerStreamType::Video;
            break;
        }
    }

    TCPHandler::GetInstance().CreateTCPSocket();

    std::cout << "Start RTSP server" << std::endl;

    while (true) {
        std::pair<int, std::string> newClient = TCPHandler::GetInstance().AcceptClientConnection();

        std::cout << "Client connected" << std::endl;

        ClientSession* clientSession = new ClientSession(newClient);
        clientSession->StartRequestHandlerThread();
        std::cout << "Main while loop." << std::endl;
    }

    return 0;
}
