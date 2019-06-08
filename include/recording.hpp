#ifndef RECORDING_HPP
#define RECORDING_HPP


#include <string>
#include <ctime>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stdbool.h>

#include <lsl_cpp.h>

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <devioctl.h>
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif


#define SZ_INPUT (256)
#define SZ_DATA_EMG (64 * 27)
#define SZ_DATA_ACC (192 * 2)
#define SZ_DATA_IM_EMG (64 * 15)
#define SZ_DATA_IM_AUX (576 * 2)


class Recording {
    public:
        Recording(std::string ip, int com_port);
        ~Recording();

        void closeSocket();

        bool getError();
        void setRecord(bool value);
        void setLSLSharing(bool value);

        void checkConnection();
        void startRecording();
        void basicExchange(char *request);

        void getData();

        void sendRequest(int socket, char *request);
        char *getReply(int socket);

        char *readReplyIfReady(int socket, int bytesExpected);

        void setupLSL(lsl::stream_info *s_info);

        lsl::stream_info *info;
        lsl::stream_outlet *outlet;

    private:
        bool error;
        bool record;
        bool lsl_sharing;

        std::string ip_address;
        struct sockaddr_in server_address;

        int comm_sock, EMG_sock, acc_sock, imemg_sock, aux_sock;

        char* reply;

        char temp_request[SZ_INPUT];
        char sensor_type[16];

        /* Network communication and program state */
        #ifdef _WIN32
            WSADATA wsaData;
        #else
            in_addr_t ipAddress;
        #endif

    /* User input */
    // char szIpAddress[SZ_INPUT];
    // char szStartTrigger[SZ_INPUT];
    // char szStopTrigger[SZ_INPUT];
    // char szCollectDuration[SZ_INPUT];
    // char szTmp[SZ_INPUT];
    // char szTmp1[SZ_INPUT];

    /* Recieved data */
    // char emgData[SZ_DATA_EMG];
    // char accData[SZ_DATA_ACC];
    // char imemgData[SZ_DATA_IM_EMG];
    // char auxData[SZ_DATA_IM_AUX];
    // char sensorType[16];
};

#endif