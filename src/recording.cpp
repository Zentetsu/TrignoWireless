#include "../include/recording.hpp"


Recording::Recording(std::string ip, std::string new_list_channels, std::string new_list_EMG, int com_port) {
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

    settingConfig(new_list_channels, new_list_EMG);
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

void Recording::setupLSLSharing(bool value) {
    if(value) {
        if(sig_EMG) outlet_EMG = new lsl::stream_outlet(*info_EMG);
        if(sig_acc) outlet_ACC = new lsl::stream_outlet(*info_ACC);
        if(sig_imemg) outlet_IMEMG = new lsl::stream_outlet(*info_IMEMG);
        if(sig_aux) outlet_AUX = new lsl::stream_outlet(*info_AUX);
    } else {
        if(sig_EMG) delete outlet_EMG;
        if(sig_acc) delete outlet_ACC;
        if(sig_imemg) delete info_IMEMG;
        if(sig_aux) delete info_AUX;
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
    sendRequest(comm_sock, request);
    //std::string reply = getReply(comm_sock);
    std::cout << getReply(comm_sock) << std::endl;
}

void Recording::getSensorType() {
    char* reply;

    for(int i = 0; i < list_EMG.size(); i++) {
        sprintf(temp_request, "SENSOR %d TYPE?\r\n\r\n", i+1);
        sendRequest(comm_sock, temp_request);
        reply = getReply(comm_sock);
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
    float acc_data_flt[SZ_DATA_ACC / sizeof(float)];
    unsigned int samples_IMEMG = SZ_DATA_IM_EMG / sizeof(float) / 16;
    float imemg_data_flt[SZ_DATA_IM_EMG / sizeof(float)];
    unsigned int samples_AUX = SZ_DATA_IM_AUX / sizeof(float) / 144;
    float aux_data_flt[SZ_DATA_IM_AUX / sizeof(float)];

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

        if(sig_acc) {
            chanels_acc.resize(samples_ACC);

            for(int k = 0; k < samples_ACC; k++)
                chanels_acc[k].resize(nb_channels_acc);

            if (recv(acc_sock, acc_data, SZ_DATA_ACC, MSG_PEEK) >= SZ_DATA_ACC) {
                recv(acc_sock, acc_data, SZ_DATA_ACC, 0);
                memcpy(acc_data_flt, acc_data, SZ_DATA_ACC);

                for (int i = 0; i < samples_ACC; i++) {
                    std::cout << "ACC: ";

                    for (int j = 0; j < 3 * list_EMG.size(); j++) {
                        std::cout << acc_data_flt[i * 48 + j] << "    ";
                        chanels_acc[i][j] = acc_data_flt[i * 48 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_ACC->push_chunk(chanels_acc);
        }

        if(sig_imemg) {
            chanels_imemg.resize(samples_IMEMG);

            for(int k = 0; k < samples_IMEMG; k++)
                chanels_imemg[k].resize(nb_channels_imemg);

              if (recv(imemg_sock, imemg_data, SZ_DATA_IM_EMG, MSG_PEEK) >= SZ_DATA_IM_EMG) {
                recv(imemg_sock, imemg_data, SZ_DATA_IM_EMG, 0);
                memcpy(imemg_data_flt, imemg_data, SZ_DATA_IM_EMG);

                for (int i = 0; i < samples_IMEMG; i++) {
                    std::cout << "IM_EMG: ";

                    for (int j = 0; j < list_EMG.size(); j++) {
                        std::cout <<  imemg_data_flt[i * 16 + j] << "    ";
                        chanels_imemg[i][j] = imemg_data_flt[i * 48 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_IMEMG->push_chunk(chanels_imemg);
        }
        
        if(sig_aux) {
            chanels_aux.resize(samples_AUX);

            for(int k = 0; k < samples_AUX; k++)
                chanels_aux[k].resize(nb_channels_aux);

            if (recv(aux_sock, aux_data, SZ_DATA_IM_AUX, MSG_PEEK) >= SZ_DATA_IM_AUX) {
                recv(aux_sock, aux_data, SZ_DATA_IM_AUX, 0);
                memcpy(aux_data_flt, aux_data, SZ_DATA_IM_AUX);

                for (int i = 0; i < samples_AUX; i++) {
                    std::cout << "AUX: ";

                    for (int j = 0; j < 9 * list_EMG.size(); j++) {
                        std::cout <<  aux_data_flt[i * 144 + j] << "    ";
                        chanels_aux[i][j] = aux_data_flt[i * 48 + j];
                    }
                    
                    std::cout << std::endl;
                }
            }

            if (lsl_sharing)
                outlet_AUX->push_chunk(chanels_aux);
        }

        // #ifdef _WIN32
        //     Sleep(100);
        // #else
        //     usleep(500000);
        // #endif
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

void Recording::settingConfig(std::string new_list_channels, std::string new_list_EMG) {
    boost::algorithm::split(list_channels, new_list_channels, boost::algorithm::is_any_of(", "), boost::algorithm::token_compress_on);
    boost::algorithm::split(list_EMG, new_list_EMG, boost::algorithm::is_any_of(", "), boost::algorithm::token_compress_on);

    sig_EMG = false;
    sig_acc = false;
    sig_imemg = false;
    sig_aux = false;

    for (auto it : list_channels) {
        switch(resolveSignal(it)) {
            case _EMG:
                nb_channels_EMG = list_EMG.size();
                sig_EMG = true;
                
                break;
            case _ACC:
                nb_channels_acc = 3 * list_EMG.size();
                sig_acc = true;
                
                break;
            case _IMEMG:
                nb_channels_imemg = list_EMG.size();
                sig_imemg = true;

                break;
            case _AUX:
                nb_channels_aux = 9 * list_EMG.size();
                sig_aux = true;

                break;
            case _NONE:
                std::cout << "ERROR" << std::endl;
                
                break;
        }
    }

    info_EMG = new lsl::stream_info("Trigno Wireless EMG", "EMG",nb_channels_EMG, 2000, lsl::cf_float32, boost::asio::ip::host_name());
    info_ACC = new lsl::stream_info("Trigno Wireless ACC", "ACC", nb_channels_acc, (int)256/2, lsl::cf_float32, boost::asio::ip::host_name());
    info_IMEMG = new lsl::stream_info("Trigno Wireless IMEMG", "IMEMG", nb_channels_imemg, 2000, lsl::cf_float32, boost::asio::ip::host_name());
    info_AUX = new lsl::stream_info("Trigno Wireless AUX", "AUX", nb_channels_aux, 2000, lsl::cf_float32, boost::asio::ip::host_name());
    
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