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

void showHelp(const char *thisName);
bool isPrivilegedPort(int port);
bool isRunningAsRoot();

int main(int argc, char* argv[]) {
    int option;
    extern char *optarg;

    if(argc < 2){
        showHelp(argv[0]);
        return 1;
    }
    while((option = getopt(argc, argv, "hm:")) != -1)
    {
        switch(option)
        {
        case 'h':
            showHelp(argv[0]);
            return 1;
        case 'm':
            std::string mode(optarg);
            if(mode == "Audio") {
                ServerStream::getInstance().type = Audio;
            }else if(mode == "Video") {
                ServerStream::getInstance().type = Video;
            }
            break;
        }
    }

    if(isPrivilegedPort(g_serverRtpPort) && !isRunningAsRoot()) {
        std::cerr << "Error: Program must be run as root to bind to privileged ports.\n";
        return 1;
    };

    std::cout << "Start RTSP server (" << ServerStream::getInstance().type << ")" << std::endl;

    while (true) {
        std::pair<int, std::string> newClient = TCPHandler::GetInstance().AcceptClientConnection();

        std::cout << "Client connected" << std::endl;

        ClientSession* clientSession = new ClientSession(newClient);
        clientSession->StartRequestHandlerThread();
    }

    return 0;
}

void showHelp(const char *thisName) {
    printf("usage : %s [OPTION]\n"
           " -h            help\n"
           " -m [media type] Audio or Video\n",
           thisName);
}

bool isPrivilegedPort(int port) {
    return port <= 1024;
}
bool isRunningAsRoot() {
    return getuid() == 0;
}