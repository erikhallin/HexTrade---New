#ifndef SERVERCOM_H
#define SERVERCOM_H

#include <iostream> //For Debug
#include <stdlib.h> //For itoa()
#include <winsock2.h>
#include <string>
#include <vector>

using namespace std;

class serverCom
{
    public:
        //Constructors
        serverCom();
        //Variables
        bool m_ready;
        //Functions
        bool init(void);
        bool start_to_listen(int backLog);
        bool set_port_and_bind(int port);
        bool known_socket(SOCKET new_socket);
        bool check_for_broadcast(void);
        bool set_broadcast_port(int port_sending,int port_replying);
        bool send_data(string data_string);
        bool send_data(char* data_array);
        bool send_data(string data_string,SOCKET SocReceiver);
        bool send_data(char* data_array,SOCKET SocReceiver);
        bool recv_data(string& data_string,SOCKET SocSender);
        bool recv_data(char* data_array,SOCKET SocSender);
        bool add_client(SOCKET& new_client);
        bool remove_client(SOCKET client_to_remove);
        SOCKET get_server_socket(void);
        bool clean_up(void);

    private:
        //Variables
        bool           m_broadcast_ready;
        string         m_host_name,m_host_IP;
        int            m_host_port,m_broadcast_port,m_broadcast_port_reply;
        SOCKET         m_SocServer;
        SOCKET         m_SocUDP_for_broadcast_recv;
        vector<SOCKET> m_vSocClients;
        //Functions
        bool init_broadcast_socket(void);
        bool reply_to_broadcaster(string sIP);
        bool close_broadcast_socket(void);
        bool ip_is_valid(char *str);
};



#endif // SERVERCOM_H
