#include "../include/recording.hpp"


Recording::Recording(std::string ip, std::string new_list_channels, std::string new_list_EMG, int com_port) {
    setbuf(stdout, NULL);

    error = false;
    record = true;
    lsl_sharing = false;

    #ifdef _WIN32
        if(WSAStartup(MAKEWORD(2, 0), &wsaData) !=0)
            throw std::runtime_error("\nERROR: Windows failed (again).");
    #endif

    if((COMM_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("\nERROR: socket initialization failed.");

        error = true;
    }

    //TODO to pass in the constructor
    ip_address = ip;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip_address.c_str());
    server_address.sin_port = htons(com_port);

    if(connect(COMM_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("\nERROR: socket connection failed.");
        
        error = true;
    }

    if ((EMG_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("\nERROR: EMG initialization failed.");

        error = true;
    }

    server_address.sin_port = htons(50041);

    if (connect(EMG_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("\nERROR: EMG connection failed.");

        error = true;
    }

    if ((ACC_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("\nERROR: ACC initialization failed.");

        error = true;
    }

    server_address.sin_port = htons(50042);

    if (connect(ACC_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("\nERROR: ACC connection failed.");

        error = true;
    }

    if ((IMEMG_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("\nERROR: IMEMG_sock initialization failed.");

        error = true;
    }

    server_address.sin_port = htons(50043);

    if (connect(IMEMG_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("\nERROR: IMEMG_sock connection failed.");

        error = true;
    }

    if ((AUX_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        throw std::runtime_error("\nERROR: AUX_sock initialization failed.");

        error = true;
    }

    server_address.sin_port = htons(50044);

    if (connect(AUX_sock, (struct sockaddr*)&server_address, sizeof(struct sockaddr_in)) == -1) {
        throw std::runtime_error("\nERROR: AUX_sock connection failed.");

        error = true;
    }

    settingConfig(new_list_channels, new_list_EMG);
}

Recording::~Recording() {
    basicExchange((char *)"QUIT\r\n\r\n");
    this->closeSocket();
}

void Recording::closeSocket() {
    #ifdef _WIN32
        closesocket(COMM_sock);
        closesocket(EMG_sock);
        closesocket(ACC_sock);
        closesocket(IMEMG_sock);
        closesocket(AUX_sock);
        WSACleanup();
    #else
        close(COMM_sock);
        close(EMG_sock);
        close(ACC_sock);
        close(IMEMG_sock);
        close(AUX_sock);
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

void Recording::setupLSLSharing(bool value) {
    if(value) {
        if(sig_EMG) outlet_EMG = new lsl::stream_outlet(*info_EMG);
        if(sig_ACC) outlet_ACC = new lsl::stream_outlet(*info_ACC);
        if(sig_IMEMG) outlet_IMEMG = new lsl::stream_outlet(*info_IMEMG);
        if(sig_AUX) outlet_AUX = new lsl::stream_outlet(*info_AUX);
    } else {
        if(sig_EMG) delete outlet_EMG;
        if(sig_ACC) delete outlet_ACC;
        if(sig_IMEMG) delete outlet_IMEMG;
        if(sig_AUX) delete outlet_AUX;
    }
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

    getSensorType();

    getData();
}

void Recording::basicExchange(char* request) {
    sendRequest(COMM_sock, request);
    //std::string reply = getReply(COMM_sock);
    std::cout << getReply(COMM_sock) << std::endl;
}

void Recording::getSensorType() {
    char* reply;

    for(int i = 0; i < list_EMG.size(); i++) {
        sprintf(temp_request, "SENSOR %d TYPE?\r\n\r\n", i+1);
        sendRequest(COMM_sock, temp_request);
        reply = getReply(COMM_sock);
        sensor_type[i] = *reply;
        
        std::cout << reply << std::endl;

        free(reply);
    }
}

void Recording::getData() {
    std::cout << "Data..." << std::endl;

    unsigned int samples_EMG = SZ_DATA_EMG / sizeof(float) / 16;
    float EMG_data_flt[SZ_DATA_EMG / sizeof(float)];
    unsigned int samples_ACC = SZ_DATA_ACC / sizeof(float) / 48;
    float ACC_data_flt[SZ_DATA_ACC / sizeof(float)];
    unsigned int samples_IMEMG = SZ_DATA_IM_EMG / sizeof(float) / 16;
    float IMEMG_data_flt[SZ_DATA_IM_EMG / sizeof(float)];
    unsigned int samples_AUX = SZ_DATA_IM_AUX / sizeof(float) / 144;
    float AUX_data_flt[SZ_DATA_IM_AUX / sizeof(float)];

    std::cout << samples_EMG << "   " << samples_ACC << "   " << samples_IMEMG << "   " << samples_AUX << std::endl;

    while(record) {
        if(sig_EMG) {
            chanels_EMG.resize(samples_EMG);

            for(int k = 0; k < samples_EMG; k++)
                chanels_EMG[k].resize(nb_channels_EMG);

            if (recv(EMG_sock, EMG_data, SZ_DATA_EMG, MSG_PEEK) >= SZ_DATA_EMG) {
                recv(EMG_sock, EMG_data, SZ_DATA_EMG, 0);
                memcpy(EMG_data_flt, EMG_data, SZ_DATA_EMG);

                for (int i = 0; i < samples_EMG; i++) {
                    for (int j = 0; j < list_EMG.size(); j++) {
                        std::cout << "EMG" << ": " << EMG_data_flt[i * 16 + j] << std::endl;
                        chanels_EMG[i][j] = EMG_data_flt[i * 16 + j];
                    }
                }
            }

            if (lsl_sharing)
                outlet_EMG->push_chunk(chanels_EMG);
        }

        if(sig_ACC) {
            chanels_ACC.resize(samples_ACC);

            for(int k = 0; k < samples_ACC; k++)
                chanels_ACC[k].resize(nb_channels_ACC);

            if (recv(ACC_sock, ACC_data, SZ_DATA_ACC, MSG_PEEK) >= SZ_DATA_ACC) {
                recv(ACC_sock, ACC_data, SZ_DATA_ACC, 0);
                memcpy(ACC_data_flt, ACC_data, SZ_DATA_ACC);

                for (int i = 0; i < samples_ACC; i++) {
                    std::cout << "ACC: ";

                    for (int j = 0; j < 3 * list_EMG.size(); j++) {
                        std::cout << ACC_data_flt[i * 48 + j] << "    ";
                        chanels_ACC[i][j] = ACC_data_flt[i * 48 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_ACC->push_chunk(chanels_ACC);
        }

        if(sig_IMEMG) {
            chanels_IMEMG.resize(samples_IMEMG);

            for(int k = 0; k < samples_IMEMG; k++)
                chanels_IMEMG[k].resize(nb_channels_IMEMG);

            if (recv(IMEMG_sock, IMEMG_data, SZ_DATA_IM_EMG, MSG_PEEK) >= SZ_DATA_IM_EMG) {
                recv(IMEMG_sock, IMEMG_data, SZ_DATA_IM_EMG, 0);
                memcpy(IMEMG_data_flt, IMEMG_data, SZ_DATA_IM_EMG);

                for (int i = 0; i < samples_IMEMG; i++) {
                    std::cout << "IM_EMG: ";

                    for (int j = 0; j < list_EMG.size(); j++) {
                        std::cout <<  IMEMG_data_flt[i * 16 + j] << "    ";
                        chanels_IMEMG[i][j] = IMEMG_data_flt[i * 16 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_IMEMG->push_chunk(chanels_IMEMG);
        }
        
        if(sig_AUX) {
            chanels_AUX.resize(samples_AUX);

            for(int k = 0; k < samples_AUX; k++)
                chanels_AUX[k].resize(nb_channels_AUX);

            if (recv(AUX_sock, AUX_data, SZ_DATA_IM_AUX, MSG_PEEK) >= SZ_DATA_IM_AUX) {
                recv(AUX_sock, AUX_data, SZ_DATA_IM_AUX, 0);
                memcpy(AUX_data_flt, AUX_data, SZ_DATA_IM_AUX);

                for (int i = 0; i < samples_AUX; i++) {
                    std::cout << "AUX: ";

                    for (int j = 0; j < 9 * list_EMG.size(); j++) {
                        std::cout <<  AUX_data_flt[i * 144 + j] << "    ";
                        chanels_AUX[i][j] = AUX_data_flt[i * 144 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_AUX->push_chunk(chanels_AUX);
        }
    }
}

void Recording::sendRequest(int socket, char *request) {
    std::cout << "Request:" << std::endl << request << std::endl;

    if (send(socket, request, strlen(request), 0) == -1) {
        std::cout << "ERROR!" << std::endl;
        throw std::runtime_error("\nERROR: enable to send request.");
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
        recv(COMM_sock, tmp, bytesAvail, 0);
        tmp[bytesAvail] = '\0';

        if(strcmp(tmp, "OK") == 0 || strcmp(tmp, "INVALID COMMAND") == 0) {
            //TODO not working
            throw std::runtime_error("\nERROR: socket exchange failed.");
        }

        return tmp;
    }

    char *tmp = (char *)malloc(1);
    tmp[0] = '\0';

    return tmp;
}

void Recording::settingConfig(std::string new_list_channels, std::string new_list_EMG) {
    boost::algorithm::split(list_channels, new_list_channels, boost::algorithm::is_any_of(", "), boost::algorithm::token_compress_on);
    boost::algorithm::split(list_EMG, new_list_EMG, boost::algorithm::is_any_of(", "), boost::algorithm::token_compress_on);

    sig_EMG = false;
    sig_ACC = false;
    sig_IMEMG = false;
    sig_AUX = false;

    for (auto it : list_channels) {
        switch(resolveSignal(it)) {
            case _EMG:
                nb_channels_EMG = list_EMG.size();
                sig_EMG = true;
                info_EMG = new lsl::stream_info("Trigno Wireless EMG", "EMG",nb_channels_EMG, 2000, lsl::cf_float32, boost::asio::ip::host_name());
                
                break;
            case _ACC:
                nb_channels_ACC = 3 * list_EMG.size();
                sig_ACC = true;
                info_ACC = new lsl::stream_info("Trigno Wireless ACC", "ACC", nb_channels_ACC, 148, lsl::cf_float32, boost::asio::ip::host_name());
                
                break;
            case _IMEMG:
                nb_channels_IMEMG = list_EMG.size();
                sig_IMEMG = true;
                info_IMEMG = new lsl::stream_info("Trigno Wireless IMEMG", "IMEMG", nb_channels_IMEMG, 2000, lsl::cf_float32, boost::asio::ip::host_name());

                break;
            case _AUX:
                nb_channels_AUX = 9 * list_EMG.size();
                sig_AUX = true;
                info_AUX = new lsl::stream_info("Trigno Wireless AUX", "AUX", nb_channels_AUX, 2000, lsl::cf_float32, boost::asio::ip::host_name());

                break;
            case _NONE:
                std::cout << "ERROR" << std::endl;
                
                break;
        }
    }

    
    // lsl::xml_element l_channels_EMG = info_EMG->desc().append_child("list_channels");
    // lsl::xml_element l_channels_ACC = info_ACC->desc().append_child("list_channels");
    // lsl::xml_element l_EMG_EMG = info_EMG->desc().append_child("list_EMG");
    // lsl::xml_element l_EMG_ACC = info_ACC->desc().append_child("list_EMG");

    // for(int i = 0; i < list_channels.size(); i++){
    //     l_channels_EMG.append_child_value("channels", list_channels[i].c_str());
    //     l_channels_ACC.append_child_value("channels", list_channels[i].c_str());
    // }
        
    // for(int i = 0; i < list_EMG.size(); i++){
    //     l_EMG_EMG.append_child_value("EMG", list_EMG[i].c_str());
    //     l_EMG_ACC.append_child_value("EMG", list_EMG[i].c_str());
    // }
}

Signal Recording::resolveSignal(std::string input) {
    if(input == "EMG") return _EMG;
    if(input == "ACC") return _ACC;
    if(input == "IMEMG") return _IMEMG;
    if(input == "AUX") return _AUX;
    
    return _NONE;
}