#include "../include/recording.hpp"


Recording::Recording(std::string ip, int com_port) {
    setbuf(stdout, NULL);

    error = false;
    record = true;
    lsl_sharing = false;

    #ifdef _WIN32
        if(WSAStartup(MAKEWORD(2, 0), &wsaData) !=0)
        exit(1);
    #endif

    if((comm_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    //TODO to pass in the constructor
    ip_address = ip;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_address.sin_port = htons(com_port);

    if(connect(comm_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    if ((EMG_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    server_address.sin_port = htons(50041);

    if (connect(EMG_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    if ((acc_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    server_address.sin_port = htons(50042);

    if (connect(acc_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    if ((imemg_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    server_address.sin_port = htons(50043);

    if (connect(imemg_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    if ((aux_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::exception();
        //TODO write exception
        error = true;
    }

    server_address.sin_port = htons(50044);

    if (connect(aux_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::exception();
        //TODO write exception
        error = true;
    }
}

Recording::~Recording() {
    basicExchange((char *)"QUIT\r\n\r\n");
    this->closeSocket();
}

void Recording::closeSocket() {
    #ifdef _WIN32
        closesocket(comm_sock);
        closesocket(EMG_sock);
        closesocket(aux_sock);
        closesocket(imemg_sock);
        closesocket(aux_sock);
        WSACleanup();
    #else
        close(comm_sock);
        close(EMG_sock);
        close(aux_sock);
        close(imemg_sock);
        close(aux_sock);
    #endif
}

bool Recording::getError() {
    return error;
}

void Recording::setRecord(bool value) {
    record = value;
}

void Recording::setLSLSharing(bool value) {
    lsl_sharing = value;
}

void Recording::checkConnection() {
    //TODO change parameter
    std::cout << "Requesting data..." << std::endl;
    sprintf(temp_request, "TRIGGER START %s\r\nTRIGGER STOP %s\r\nUPSAMPLE OFF\r\n\r\n", false?"ON":"OFF", false?"ON":"OFF");
    basicExchange(temp_request);

    basicExchange((char*)"ENDIAN LITTLE\r\n\r\n");
    basicExchange((char *)"ENDIANNESS?\r\n\r\n");
}

void Recording::startRecording() {
    std::cout << "Start recording..." << std::endl;
    basicExchange((char *)"START\r\n\r\n");

    sendRequest(comm_sock, (char*)"SENSOR 1 TYPE?\r\n\r\n");

    char* reply = getReply(comm_sock);
    sensor_type[0] = *reply;

    std::cout<< reply << std::endl;

    free(reply);

    getData();
}

void Recording::basicExchange(char* request) {
    sendRequest(comm_sock, request);
    //std::string reply = getReply(comm_sock);
    std::cout<< getReply(comm_sock) << std::endl;
}

void Recording::getData() {
    std::cout << "Data..." << std::endl;
    float sample[64];

    while(record) {
        if(lsl_sharing) {
            std::cout << "sharing..." << std::endl;
            
            for (int c=0;c<64;c++)
                sample[c] = (rand()%1500)/500.0-1.5;

            outlet->push_sample(sample);
        } else {
            std::cout << "get..." << std::endl;
        }

        #ifdef _WIN32
            Sleep(100);
        #else
            usleep(10000);
        #endif
    }
}

void Recording::sendRequest(int socket, char *request) {
    std::cout << "Request:" << std::endl << request << std::endl;

    if (send(socket, request, strlen(request), 0) == -1) {
        std::cout << "ERROR!" << std::endl;
        throw std::exception();
        error = true;
        //TODO write exception
    }
}

char* Recording::getReply(int socket) {
    u_long bytesAvail = 0;

    #ifdef _WIN32
        while (!bytesAvail && ioctlsocket(socket, FIONREAD, &bytesAvail) >= 0) {
            Sleep(100 * 1);

            std::cout << ".";
        }
    #else
        while (!bytesAvail && ioctl(socket, FIONREAD, &bytesAvail) >= 0) {
            usleep(1000);

            std::cout << ".";
        }
    #endif

    std::cout << std::endl;

    if (bytesAvail > 0) {
        char* tmp = (char*)malloc(bytesAvail);
        recv(comm_sock, tmp, bytesAvail, 0);
        tmp[bytesAvail] = '\0';

        return tmp;
    }
    char *tmp = (char *)malloc(1);
    tmp[0] = '\0';
    return tmp;
}

char * Recording::readReplyIfReady(int socket, int bytesExpected) {
    u_long bytesAvail = 0;
    #ifdef _WIN32
    if (ioctlsocket(socket, FIONREAD, &bytesAvail) >= 0 && bytesAvail < bytesExpected)
        return NULL;
    #else
    if (ioctl(socket, FIONREAD, &bytesAvail) >= 0 && bytesAvail < bytesExpected)
        return NULL;
    #endif


    char *tmp = (char*)(bytesAvail);
    recv(socket, tmp, bytesAvail, 0);
    tmp[bytesAvail] = '\0';

    return tmp;
}

void Recording::setupLSL(lsl::stream_info *s_info) {
    info = s_info;
    outlet = new lsl::stream_outlet(*info);

    lsl_sharing = true;
}