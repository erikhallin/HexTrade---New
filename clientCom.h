#ifndef CLIENTCOM_H
#define CLIENTCOM_H

#include <iostream> //For Debug
#include <stdlib.h> //For itoa()
#include <string>
#include <vector>
#include <winsock2.h>

using namespace std;

class clientCom
{
    public:
        clientCom();
        //Variables
        bool m_ready;
        //Functions
        bool init(void);
        bool set_IP_and_connect(string sIP,int port);
        bool test_connection(void);
        bool broadcast_my_ip(void);
        bool check_for_broadcast_reply(void);
        bool set_broadcast_net_and_port(string net,int port_sending,int port_replying);
        string get_server_IP(void);
        bool get_server_IP_and_port(string& IP_and_port);
        bool send_data(string data_string);
        bool send_data(char* data_array);
        bool recv_data(string& data_string);
        bool recv_data(char* data_array);
        SOCKET get_server_socket(void);
        bool clean_up(void);

    private:
        //Variables
        bool   m_broadcast_ready,m_found_server;
        string m_server_IP,m_my_IP,m_my_name,m_broadcast_net;
        int    m_broadcast_port,m_broadcast_port_reply,m_server_port;
        SOCKET m_SocServer;
        SOCKET m_SocUDP_for_broadcasting,m_SocUDP_for_broadcast_reply;
        //Functions
        bool init_broadcast_socket(void);
        bool close_broadcast_socket(void);
        bool ip_is_valid(char *str);
};

#endif // CLIENTCOM_H
