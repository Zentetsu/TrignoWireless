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

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>


#include "../include/signal_type.hpp"


#define SZ_INPUT (256)
#define SZ_DATA_EMG (64 * 27)
#define SZ_DATA_ACC (192 * 2)
#define SZ_DATA_IM_EMG (64 * 15)
#define SZ_DATA_IM_AUX (576 * 2)


class Recording {
    public:
        Recording(std::string ip, std::string new_list_channels, std::string new_list_EMG, int com_port);
        ~Recording();

        void closeSocket();

        bool getError();
        void setRecord(bool value);
        void setLSLSharing(bool value);
        void setupLSLSharing(bool value);

        void checkConnection();
        void startRecording();
        void basicExchange(char *request);

        void getSensorType();
        void getData();

        void sendRequest(int socket, char *request);
        char *getReply(int socket);

        void settingConfig(std::string new_list_channels, std::string new_list_EMG);

        Signal resolveSignal(std::string input);

        lsl::stream_info *info_EMG;
        lsl::stream_info *info_ACC;
        lsl::stream_info *info_IMEMG;
        lsl::stream_info *info_AUX;
        lsl::stream_outlet *outlet_EMG;
        lsl::stream_outlet *outlet_ACC;
        lsl::stream_outlet *outlet_IMEMG;
        lsl::stream_outlet *outlet_AUX;

      private:
        bool error;
        bool record;
        bool lsl_sharing;

        std::vector<std::string> list_channels;
        std::vector<std::string> list_EMG;

        std::vector<std::vector<float>> chanels_EMG;
        std::vector<std::vector<float>> chanels_ACC;
        std::vector<std::vector<float>> chanels_IMEMG;
        std::vector<std::vector<float>> chanels_AUX;

        int nb_channels_EMG, nb_channels_ACC, nb_channels_IMEMG, nb_channels_AUX;

        std::string ip_address;
        struct sockaddr_in server_address;

        int COMM_sock, EMG_sock, ACC_sock, IMEMG_sock, AUX_sock;
        bool sig_EMG, sig_ACC, sig_IMEMG, sig_AUX;

        char EMG_data[SZ_DATA_EMG];
        char ACC_data[SZ_DATA_ACC];
        char IMEMG_data[SZ_DATA_IM_EMG];
        char AUX_data[SZ_DATA_IM_AUX];
        char sensor_type[16];

        char* reply;

        char temp_request[SZ_INPUT];

        #ifdef _WIN32
            WSADATA wsaData;
        #else
            in_addr_t ipAddress;
        #endif
};

#endif