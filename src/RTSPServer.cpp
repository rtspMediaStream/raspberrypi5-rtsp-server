#include "RTSPServer.h"
#include "Global.h"
#include "ClientSession.h"
#include "TCPHandler.h"
#include "UDPHandler.h"
#include "RequestHandler.h"
#include "MediaStreamHandler.h"

#include <string>
#include <thread>
#include <iostream>
#include <csignal>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>
using namespace std;

RTSPServer::RTSPServer()
{
    
}

RTSPServer::~RTSPServer()
{
    std::cout << "Interrupt signal (" << signal << ") received. Shutting down..." << std::endl;
    TCPHandler::GetInstance().CloseClientConnection();
}

int RTSPServer::startServerThread()
{
    if(isPrivilegedPort(g_serverRtpPort) && !isRunningAsRoot()) {
        std::cerr << "Error: Program must be run as root to bind to privileged ports.\n";
        return 1;
    };

    std::cout << "Start RTSP server " << std::endl;

    std::thread ([]()->void{
        while(true){
            std::string newIp;
            int newClient = TCPHandler::GetInstance().AcceptClientConnection(newIp);
            if(newClient == -1){
                break;
            }else {
                std::cout << "Client Ip:" << newIp << " connected." << std::endl;
            }
            ClientSession* clientSession = new ClientSession(newClient, newIp);

            RequestHandler requestHandler(clientSession);
            requestHandler.StartThread();
        }
        std::cout << "Stop RTSP server" << std::endl;
    }).detach();
 
    return 0;
}

bool RTSPServer::isPrivilegedPort(int port) {
    return port <= 1024;
}
bool RTSPServer::isRunningAsRoot() {
    return getuid() == 0;
}