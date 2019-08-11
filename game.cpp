#include "game.h"

game::game(int width, int height, networkCom* pNetCom)
{
    m_game_state=gs_init;
    m_window_width=width;
    m_window_height=height;
    m_pNetCom=pNetCom;
    m_check_for_broadcast_flag=m_check_for_broadcast_reply_flag=false;
    init();
}

int game::cycle(void)
{
    int status=update();
    draw();

    return status;
}

bool game::set_mouse_pos(int xpos,int ypos)
{
    m_mouse_pos[0]=xpos;
    m_mouse_pos[1]=ypos+30;

    return true;
}

bool game::set_mouse_button_left(bool status)
{
    m_mouse_button[0]=status;

    if(status) cout<<"Pressed LMB\n";
    else cout<<"Released LMB\n";

    return true;
}

bool game::set_mouse_button_right(bool status)
{
    m_mouse_button[1]=status;

    if(status) cout<<"Pressed RMB\n";
    else cout<<"Released RMB\n";

    //temp
    if(status) cout<<"Mouse at :  X = "<<m_mouse_pos[0]<<" , Y = "<<m_mouse_pos[1]<<" - "<<
                     (float)m_mouse_pos[0]/(float)m_window_width<<", "<<(float)m_mouse_pos[1]/(float)m_window_height<<endl;
    if(status) cout<<"Cam at : "<<m_eye_pos_x<<", "<<m_eye_pos_y<<endl;

    return true;
}

bool game::set_mouse_scroll_up(bool status)
{
    m_mouse_button[2]=status;

    cout<<"Scrolled UP\n";

    return true;
}

bool game::set_mouse_scroll_down(bool status)
{
    m_mouse_button[3]=status;

    cout<<"Scrolled DOWN\n";

    return true;
}

bool game::set_keyboard_key(int key_id,bool status)
{
    m_keys[key_id]=status;

    if(status) cout<<"Pressed key: "<<key_id<<endl;
    else cout<<"Released key: "<<key_id<<endl;

    return true;
}

bool game::set_debug_mode(bool status)
{
    m_debug_mode=status;

    cout<<"Debug mode is ENABLED\n";

    return true;
}

bool game::set_server_ip(string server_ip)
{
    m_server_ip=server_ip;
    m_Menu.set_IP(server_ip);

    return true;
}

string game::get_server_ip(void)
{
    return m_server_ip;
}

bool game::set_check_for_broadcast_flag(bool flag)
{
    m_check_for_broadcast_flag=flag;

    return true;
}

bool game::get_check_for_broadcast_flag(void)
{
    return m_check_for_broadcast_flag;
}

bool game::set_check_for_broadcast_reply_flag(bool flag)
{
    m_broadcast_reply_check_counter=30;
    m_check_for_broadcast_reply_flag=flag;

    return true;
}

bool game::get_check_for_broadcast_reply_flag(void)
{
    if(m_check_for_broadcast_reply_flag)
    {
        if(m_broadcast_reply_check_counter<0) m_check_for_broadcast_reply_flag=false;
        else m_broadcast_reply_check_counter--;
        cout<<"Broadcast Response Timer: "<<m_broadcast_reply_check_counter<<endl;
    }

    return m_check_for_broadcast_reply_flag;
}

bool game::recv_data(SOCKET soc_sender)
{
    char* data_array=new char[1024];

    bool recv_successful=true;
    if( !m_pNetCom->recv_data(data_array,soc_sender) )
    {
        recv_successful=false;
        cout<<"ERROR: Network: Received bad data\n";
    }
    else interpret_data(data_array,soc_sender);

    delete[] data_array;

    return recv_successful;
}

bool game::send_start_package_to_client(SOCKET soc_client)
{
    //send list of other players



    return true;
}

bool game::send_start_package_to_server(void)
{
    //clear earlier names from playerlist, and players from vector
    m_Menu.remove_all_players_from_list();
    m_vec_players.clear();

    cout<<"Network: Sending player name to server\n";
    //send name, size,type,id,game version,name
    string player_name=m_Menu.get_player_name();
    player_name.append(1,'\0');
    char* data_array=new char[ int(player_name.length())+16 ];
    memcpy(data_array+16,player_name.c_str(), player_name.length() );
    int packet_size=int(player_name.length())+16;
    int packet_type=1000;
    int packet_id=m_packet_id_counter++;
    float game_version=_version;
    memcpy(data_array,&packet_size,4);
    memcpy(data_array+4,&packet_type,4);
    memcpy(data_array+8,&packet_id,4);
    memcpy(data_array+12,&game_version,4);

    m_vec_packets_to_send.push_back( net_packet((int)m_pNetCom->get_server_socket(),data_array,packet_id) );

    return true;
}

bool game::send_client_denied_package(SOCKET soc_client)
{
    cout<<"Network: Sending decline new client package to client\n";
    //send size,type
    int packet_size=12;
    int packet_type=1109;
    int packet_id=m_packet_id_counter++;
    char* data_array=new char[packet_size];
    memcpy(data_array,&packet_size,4);
    memcpy(data_array+4,&packet_type,4);
    memcpy(data_array+8,&packet_id,4);

    m_vec_packets_to_send.push_back( net_packet((int)soc_client,data_array,packet_id) );

    return true;
}

bool game::add_server_player(void)
{
    //clear earlier players
    m_Menu.remove_all_players_from_list();
    m_vec_players.clear();
    //add server player to list
    st_player temp_player;
    temp_player.name=m_Menu.get_player_name();
    temp_player.id=m_pNetCom->get_server_socket();
    m_vec_players.push_back(temp_player);
    m_Menu.add_player_to_list(temp_player.name);
    m_player_id=temp_player.id;

    return true;
}

bool game::lost_player(int sock_id)
{
    //find player
    int player_index=-1;
    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
    {
        if( m_vec_players[player_i].id==sock_id )
        {
            player_index=player_i;
            break;
        }
    }
    if(player_index==-1)
    {
        cout<<"Error: Network: Lost connection to player: Could not find player\n";
        cout<<"Network: Remove packets to unknown sockets\n";
        cout<<"Network: Known sockets: ";
        for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
         cout<<m_vec_players[player_i].id<<", ";
        cout<<endl;
        for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
        {
            bool known_player=false;
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
            {
                if(m_vec_players[player_i].id==m_player_id) continue;
                if(m_vec_players[player_i].id==m_vec_packets_to_send[pac_i].soc_client_recv)
                {
                    known_player=true;
                    break;//next packet
                }
            }
            if(!known_player)
            {
                //remove this packet
                cout<<"Network: Removing packet to socket: "<<m_vec_packets_to_send[pac_i].soc_client_recv<<endl;
                delete[] m_vec_packets_to_send[pac_i].data_array;
                m_vec_packets_to_send.erase( m_vec_packets_to_send.begin()+pac_i );
                pac_i--;
            }
        }
        return false;
    }
    cout<<"Network: Lost connection to player: "<<m_vec_players[player_index].name<<endl;
    //remove from lobby list
    m_Menu.remove_player_from_list(m_vec_players[player_index].name);
    //remove player from vec
    int id_removed_player=m_vec_players[player_index].id;

    m_vec_players.erase( m_vec_players.begin()+player_index );
    //OBS! om stadsägandet beror på col id kommer den att ändras?
    //men ens col id sätts vid start och är oberoende av vectorn, så kanske inte gör nått iaf...

    //tell other clients (if this is server)
    if(m_pNetCom->get_status()==net_server)
    {
        for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
        {
            if(m_vec_players[player_i].id==m_player_id) continue;//skip yourself

            char* char_arr=new char[16];
            int packet_id=m_packet_id_counter++;
            int int_arr[4]={16,1103,packet_id,id_removed_player};
            memcpy(char_arr,int_arr,16);

            m_vec_packets_to_send.push_back( net_packet(int(m_vec_players[player_i].id),char_arr,packet_id) );
        }
        //remove unsent packages to that player
        for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
        {
            if(m_vec_packets_to_send[pac_i].soc_client_recv==id_removed_player)
            {
                delete[] m_vec_packets_to_send[pac_i].data_array;
                m_vec_packets_to_send.erase( m_vec_packets_to_send.begin()+pac_i );
                pac_i--;
            }
        }
        for(int pac_i=0;pac_i<(int)m_vec_replies_to_send.size();pac_i++)
        {
            if(m_vec_replies_to_send[pac_i].soc_client_recv==id_removed_player)
            {
                delete[] m_vec_replies_to_send[pac_i].data_array;
                m_vec_replies_to_send.erase( m_vec_replies_to_send.begin()+pac_i );
                pac_i--;
            }
        }
    }

    return true;
}

bool game::lost_server(void)
{
    cout<<"Network: Lost connection to server\n";
    m_vec_players.clear();
    m_pNetCom->clean_up();
    //clear packet vec
    for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
    {
        delete[] m_vec_packets_to_send[pac_i].data_array;
    }
    m_vec_packets_to_send.clear();
    for(int pac_i=0;pac_i<(int)m_vec_replies_to_send.size();pac_i++)
    {
        delete[] m_vec_replies_to_send[pac_i].data_array;
    }
    m_vec_replies_to_send.clear();
    m_vec_players_at_start.clear();
    m_vec_processed_packets.clear();

    //if not in game over state, return to main menu
    if(m_game_state!=gs_game_over)
    {
        m_Menu.set_menu_state(ms_main);
        m_game_state=gs_menu_main;
    }

    return true;
}

bool game::set_multisend(bool flag)
{
    m_server_multisend=flag;
    cout<<"Commands: Multisend is ";
    if(m_server_multisend) cout<<"ON\n";
    else cout<<"OFF\n";

    return true;
}


//---Private-----


bool game::init(void)
{
    srand(time(0));
    m_seed=rand()%100000+1;

    m_player_color_id=0;//temp

    cout<<"Software Startup, version "<<_version<<endl;
    cout<<"Seed: "<<m_seed<<endl;

    //temp log
    //ofs_log.open("errors.txt");

    //sound
    load_sound();

    //reset keys
    for(int i=0;i<256;i++) m_keys[i]=false;
    m_mouse_button[0]=false;
    m_mouse_button[1]=false;
    m_mouse_button[2]=false;
    m_mouse_button[3]=false;

    if( load_textures() ) cout<<"Textures loaded successfully\n";
    else cout<<"ERROR: Could not load textures\n";

    //eye pos
    m_eye_pos_x=m_eye_pos_y=0;
    m_eye_pos_z=_zoom_start;

    //time reset
    m_time_this_cycle=m_time_last_cycle=0;
    m_send_packet_delay=0.0;

    m_World_menu.init_menu(m_window_width,m_window_height,m_keys,m_texture_tile);
    m_World_menu.init_game_world_menu(m_seed,0);

    m_World.init(m_window_width,m_window_height,&m_player_color_id,m_keys,m_texture_tile,m_pSound);

    m_Menu.init(m_window_width,m_window_height,m_texture_gameover,m_texture_buttons,m_texture_title,m_texture_help,
                m_texture_font,(float)clock()/CLOCKS_PER_SEC,
                m_keys,m_mouse_button,m_mouse_pos,m_pSound);

    m_de_quitscreen=decal(m_window_width*0.5-689.0*0.5, m_window_height*0.5-163.0*0.5,
                          689,163,m_texture_gameover,0,689,360,523);
    m_de_quitscreen.masking(false);
    m_de_quitscreen.draw_transparent(true);
    m_show_quitscreen=m_exit_now=false;

    m_game_state=gs_menu_main;

    m_server_multisend=true;
    m_packet_id_counter=1;
    m_error_flag=0;
    m_key_delay=0.0;
    m_fps_frame_counter=0;
    m_fps_time_last_measutment=m_fps=0.0;

    //sound
    m_intro_done=m_music_intro_not_played=false;
    m_music_source=20;
    //m_pSound->enable_sound(false);
    //m_pSound->playSimpleSound( wav_start_game,1.0 );
    m_pSound->playSimpleSound( wav_music_intro,1.0,m_music_source );
    m_pSound->set_music_source(m_music_source);

    return true;
}

int game::update(void)
{
    //timing
    m_time_last_cycle=m_time_this_cycle;//store time for last cycle
    m_time_this_cycle=(float)clock()/CLOCKS_PER_SEC;//get time now, in sec
    if(m_key_delay>0) m_key_delay-=m_time_this_cycle-m_time_last_cycle;
    //fps
    if(m_fps_time_last_measutment+_fps_interval_delay<m_time_this_cycle)
    {
        m_fps_time_last_measutment=m_time_this_cycle;
        m_fps=(float)m_fps_frame_counter/_fps_interval_delay;
        m_fps_frame_counter=0;
        cout<<"FPS: "<<m_fps<<endl;
    }
    m_fps_frame_counter++;
    //error flag
    if(m_error_flag!=0) cout<<"ERROR: "<<m_error_flag<<endl;

    //music update
    if(!m_intro_done)
    {
        //test if intro done, start loop
        if( m_pSound->get_source_status(m_music_source) )
        {
            m_pSound->playSimpleSound(wav_music_loop,1.0,m_music_source,true);
            m_intro_done=true;
        }
    }

    //update processed packets vec
    for(int pac_i=0;pac_i<(int)m_vec_processed_packets.size();pac_i++)
    {
        if( m_vec_processed_packets[pac_i].time_left<=0 )
        {//remove packet
            m_vec_processed_packets.erase( m_vec_processed_packets.begin()+pac_i );
            pac_i--;
        }
        else
        {
            m_vec_processed_packets[pac_i].time_left-=m_time_this_cycle-m_time_last_cycle;
        }
    }
    //update packet send delay /*and resend delay*/
    for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
    {
        m_vec_packets_to_send[pac_i].delay_sec+=m_time_this_cycle-m_time_last_cycle;
        /*if( m_vec_packets_to_send[pac_i].resend_sec>0.0 )
         m_vec_packets_to_send[pac_i].resend_sec-=m_time_this_cycle-m_time_last_cycle;*/
    }
    //update packet reply send delay /*and resend delay*/
    for(int pac_i=0;pac_i<(int)m_vec_replies_to_send.size();pac_i++)
    {
        m_vec_replies_to_send[pac_i].delay_sec+=m_time_this_cycle-m_time_last_cycle;
        /*if( m_vec_replies_to_send[pac_i].resend_sec>0.0 )
         m_vec_replies_to_send[pac_i].resend_sec-=m_time_this_cycle-m_time_last_cycle;*/
    }
    //update player send delay
    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
    {
        if( m_vec_players[player_i].send_delay>0.0 )
        {
            m_vec_players[player_i].send_delay-=m_time_this_cycle-m_time_last_cycle;
        }
    }
    //send network package
    if(m_send_packet_delay<0.0)
    {
        //reset flag to track which players that a packet was sent to this cycle, only for server
        if(m_pNetCom->get_status()==net_server)
        {
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
            {
                m_vec_players[player_i].sent_pac_this_cycle=false;
            }
        }

        m_send_packet_delay=_network_send_interval;
        int pack_reply_index=0;
        int pack_send_index=0;
        while( !m_vec_replies_to_send.empty() || !m_vec_packets_to_send.empty() )//if several packets needs to be sent
        {
            //send replies first, otherwise neverending loop
            if( !m_vec_replies_to_send.empty() && pack_reply_index<(int)m_vec_replies_to_send.size() )
            {
                cout<<"Network: Send packet: Number of packets to send: "<<(int)m_vec_packets_to_send.size()<<endl;
                cout<<"Network: Send packet: Number of replies to send: "<<(int)m_vec_replies_to_send.size()<<endl;

                //send first packet reply
                cout<<"Network: Sending reply packet with id: "<<m_vec_replies_to_send[pack_reply_index].pac_id<<endl;
                m_pNetCom->send_data( m_vec_replies_to_send[pack_reply_index].data_array,
                                      m_vec_replies_to_send[pack_reply_index].soc_client_recv );

                //delete reply packet, only should be sent once
                if(m_vec_replies_to_send[pack_reply_index].send_only_once)
                {
                    delete[] m_vec_replies_to_send[pack_reply_index].data_array;
                    m_vec_replies_to_send.erase( m_vec_replies_to_send.begin()+pack_reply_index );
                    cout<<"Network: Removed that reply package\n";
                }
                else pack_reply_index++;//go to next packet, if deleted no need to ++

                /*//old
                //find that packet's player index
                int recv_player_index=-1;
                for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                {
                    if( m_vec_players[player_i].id==m_vec_replies_to_send[pack_reply_index].soc_client_recv )
                    {
                        recv_player_index=player_i;
                        break;
                    }
                }
                if(recv_player_index==-1)//player index not found
                {
                    cout<<"Error: Network: Could not find packet's receiver socet among players\n";
                    m_error_flag=6;
                    pack_reply_index++;//next, but still error
                }
                else//found receiver
                {
                    if( m_vec_players[recv_player_index].send_delay<=0.0 )
                    {
                        //send first packet reply
                        cout<<"Network: Sending reply packet with id: "<<m_vec_replies_to_send[pack_reply_index].pac_id<<endl;
                        m_pNetCom->send_data( m_vec_replies_to_send[pack_reply_index].data_array,
                                              m_vec_replies_to_send[pack_reply_index].soc_client_recv );

                        //delete reply packet, only should be sent once
                        if(m_vec_replies_to_send[pack_reply_index].send_only_once)
                        {
                            delete[] m_vec_replies_to_send[pack_reply_index].data_array;
                            m_vec_replies_to_send.erase( m_vec_replies_to_send.begin()+pack_reply_index );
                            cout<<"Network: Removed that reply package\n";
                        }
                        else pack_reply_index++;//go to next packet, if deleted no need to ++
                    }
                    else pack_reply_index++;//have to wait for this player, go to next packet
                }*/
            }
            else if( !m_vec_packets_to_send.empty() && pack_send_index<(int)m_vec_packets_to_send.size() )
            {
                //if server, test if packet have been sent to this player already this cycle
                if(m_pNetCom->get_status()==net_server)
                {
                    bool dont_send_this_packet=false;
                    //find correct soc_id - player index
                    bool player_known=false;
                    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                    {
                        if(m_vec_players[player_i].id==m_vec_packets_to_send[pack_send_index].soc_client_recv)
                        {//player index found
                            player_known=true;
                            if(m_vec_players[player_i].sent_pac_this_cycle)
                            {//packet already sent to this player, skip this packet
                                dont_send_this_packet=true;
                                break;
                            }
                            else//have not sent to this player this cycle
                            {//raise flag and resume sending
                                m_vec_players[player_i].sent_pac_this_cycle=true;
                                break;
                            }
                        }
                    }
                    if(!player_known)//could not find matching player id
                    {
                        cout<<"Error: Network: Could not match packet receiver id with any player id\n";
                        m_error_flag=5;
                    }
                    //if this packet should not be sent
                    if(dont_send_this_packet)
                    {
                        //go to next package
                        pack_send_index++;
                        //if there is no more packets, the next loop will break the sending
                        continue;
                    }
                }

                cout<<"Network: Send packet: Number of packets to send: "<<(int)m_vec_packets_to_send.size()<<endl;
                cout<<"Network: Send packet: Number of replies to send: "<<(int)m_vec_replies_to_send.size()<<endl;
                //send packet
                cout<<"Network: Sending packet with id:"<<m_vec_packets_to_send[pack_send_index].pac_id<<endl;
                m_pNetCom->send_data( m_vec_packets_to_send[pack_send_index].data_array,
                                      m_vec_packets_to_send[pack_send_index].soc_client_recv );

                //test if package is old, connection problems
                if( m_vec_packets_to_send[pack_send_index].delay_sec>_network_timeout_limit )
                {
                    //CONNECTION PROBLEMS...
                    cout<<"ERROR: Network: Packet with high delay due to connection problems\n";
                }

                //delete if packet only should be sent once
                if(m_vec_packets_to_send[pack_send_index].send_only_once)
                {
                    delete[] m_vec_packets_to_send[pack_send_index].data_array;
                    m_vec_packets_to_send.erase( m_vec_packets_to_send.begin()+pack_send_index );
                    cout<<"Network: Removed that package\n";
                }
                else pack_send_index++;//go to next packet, if deleted no need to ++
            }

            //test if done
            //if server and lots of packets needs to be sent, send to other players aswell...
            if( m_pNetCom->get_status()!=net_server || !m_server_multisend || //one packet is enough if client or multisend disabled
                (pack_reply_index>=(int)m_vec_replies_to_send.size() && //all replies and packets have been sent
                 pack_send_index>=(int)m_vec_packets_to_send.size()) )
            {//stop sending
                break;
            }
            //else, send next packet
        }
    }
    else m_send_packet_delay-=m_time_this_cycle-m_time_last_cycle;

    switch(m_game_state)
    {
        case gs_menu_main:
        {
            m_World_menu.update_menu(m_time_this_cycle-m_time_last_cycle);

            //wants to quit?
            if(m_exit_now) return 6;

            //move eye pos
            m_eye_pos_x-=0.01;
            m_eye_pos_y-=0.005*sinf(m_time_this_cycle*0.5);
            m_eye_pos_z=_zoom_start+10.0*cosf(m_time_this_cycle*0.1);

            //TEMP
            /*m_eye_pos_y-=1.0;
            m_eye_pos_z=5;*/

            //shift extrea due to zoom pos shift
            if( cosf(m_time_this_cycle*0.1)<0 ) m_eye_pos_x-=0.05*cosf(m_time_this_cycle*0.1);

            key_check_menu();
            int status=m_Menu.update(m_time_this_cycle);
            if(status!=0)
            {//play sound
                //m_pSound->playSimpleSound( wav_button_push,1.0 );
            }
            switch(status)
            {
                case 1://test
                {
                    m_player_color_id=0;
                    m_World.set_network_mode(0);
                    m_World.init_game_world(m_seed,1);
                    m_game_state=gs_running;
                    //move cam to your city
                    int pos_pix[2];
                    m_World.get_start_pos_pix(pos_pix);
                    m_eye_pos_x=-pos_pix[0];
                    m_eye_pos_y=-pos_pix[1];
                    m_eye_pos_z=_zoom_start;
                    //reset network
                    m_pNetCom->clean_up();
                    //play start sound
                    m_pSound->playSimpleSound(wav_start_match,1.0);
                }break;

                case 2://host
                {
                    //make a fresh seed
                    m_seed=time(0);
                }break;

                case 3://edit
                {

                }break;

                case 4://join
                {
                    m_check_for_broadcast_reply_flag=true;
                    return 4;
                }break;

                case 5://help
                {
                    ;
                }break;

                case 6://exit
                {
                    return 6;
                }break;

                case 7://start hosting (next in host menu)
                {
                    return 7;
                }break;

                case 8://try to join server
                {
                    m_server_ip=m_Menu.get_IP();
                    return 8;
                }break;

                case 9://host start game (next in lobby for server)
                {
                    if( m_pNetCom->get_status()==net_client ) return 0;//clients cant start the game

                    //reset world
                    m_World.init(m_window_width,m_window_height,&m_player_color_id,m_keys,m_texture_tile,m_pSound);
                    //stop new clients from joining
                    m_pNetCom->set_accept_new_clients_flag(false);
                    //remember time
                    m_time_at_start=m_time_this_cycle;
                    //remember players at start, and their col id (city ownership, as index)
                    m_vec_players_at_start.clear();
                    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                    {
                        m_vec_players_at_start.push_back( m_vec_players[player_i].name );
                    }
                    //tell world that you are the host
                    m_World.set_network_mode(1);
                    m_check_for_broadcast_flag=false;//cant join started game
                    //inform other players with: seed,time lim, tile lim,player_color
                    m_game_limit_time=m_Menu.get_limit_time();
                    m_game_limit_tile=m_Menu.get_limit_tile();
                    //send to all
                    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                    {
                        if(m_vec_players[player_i].id==m_player_id)//dont send to yourself
                        {
                            m_player_color_id=player_i;
                            continue;
                        }

                        int pac_id=m_packet_id_counter++;
                        int int_data[7]={28,1104,pac_id,m_seed,m_game_limit_time,m_game_limit_tile,player_i};
                        char* char_arr=new char[28];
                        memcpy(char_arr,int_data,28);
                        m_vec_packets_to_send.push_back( net_packet( m_vec_players[player_i].id,char_arr,pac_id ) );
                    }
                    //gen world
                    if( !m_World.init_game_world(m_seed,(int)m_vec_players.size()) )
                    {//could not create world, reset game
                        m_Menu.set_menu_state(ms_main);
                        m_pNetCom->clean_up();
                        m_vec_players.clear();
                    }
                    else m_game_state=gs_running;//start game
                    //move cam to your city
                    int pos_pix[2];
                    m_World.get_start_pos_pix(pos_pix);
                    m_eye_pos_x=-pos_pix[0];
                    m_eye_pos_y=-pos_pix[1];
                    m_eye_pos_z=_zoom_start;
                    //play start sound
                    m_pSound->playSimpleSound(wav_start_match,1.0);

                    return 9;
                }break;

                case 10://backed out from lobby or join menu, reset network
                {
                    //empty the packet vec
                    for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
                    {
                        delete[] m_vec_packets_to_send[pac_i].data_array;
                    }
                    m_vec_packets_to_send.clear();
                    return 10;
                }break;

                case 12://play sound flag  (ex: backed out from host menu)
                {
                    return 0;
                }break;

                case 13://turn off sound
                {
                    m_pSound->enable_sound(false);
                }break;

                case 14://turn on sound
                {
                    m_pSound->enable_sound(true);
                }break;

                case 15://turn off music
                {
                    m_pSound->enable_music(false);
                    m_pSound->pause_source(m_music_source);
                    if(!m_intro_done)
                    {
                        m_music_intro_not_played=true;
                    }
                    m_intro_done=true;
                }break;

                case 16://turn on music
                {
                    m_pSound->resume_source(m_music_source);
                    if(m_music_intro_not_played)
                    {
                        m_intro_done=false;
                        m_music_intro_not_played=false;
                    }
                }break;
            }

        }break;

        case gs_running:
        {
            key_check_ingame();

            //wants to quit?
            if(m_exit_now) return 6;

            m_World.update(m_time_this_cycle-m_time_last_cycle,m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);
            //check for city requests
            int city_id=m_World.get_request_expansion();
            if( city_id!=-1 )
            {
                //if test mode, accept direct
                if( m_pNetCom->get_status()==net_error )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_expansion_mission,start_pos);

                    //test route
                    if( test_route_expansion(vec_char_route,city_id,start_pos) )
                    {
                        //build new city
                        int new_city_id=m_World.get_free_id_city();
                        m_World.translate_char_to_route_and_add_expansion(vec_char_route,city_id,new_city_id,start_pos);

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }

                //if client, send data to server
                if( m_pNetCom->get_status()==net_client )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_expansion_mission,start_pos);

                    //test route
                    if( test_route_expansion(vec_char_route,city_id,start_pos) )
                    {
                        //ask server
                        int pack_size=28+(int)vec_char_route.size();
                        int pack_type=1001;
                        int pack_id=m_packet_id_counter++;
                        int x_start=start_pos[0];
                        int y_start=start_pos[1];
                        int new_city_id_space=0;
                        char* char_arr=new char[28+(int)vec_char_route.size()];
                        memcpy(char_arr,&pack_size,4);
                        memcpy(char_arr+4,&pack_type,4);
                        memcpy(char_arr+8,&pack_id,4);
                        memcpy(char_arr+12,&city_id,4);
                        memcpy(char_arr+16,&x_start,4);
                        memcpy(char_arr+20,&y_start,4);
                        memcpy(char_arr+24,&new_city_id_space,4);

                        for(int i=0;i<(int)vec_char_route.size();i++)
                        {
                            char_arr[28+i]=vec_char_route[i];
                        }

                        m_vec_packets_to_send.push_back( net_packet(m_pNetCom->get_server_socket(),char_arr,pack_id) );

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }

                //if server, run test and then send confirmation to clients
                if( m_pNetCom->get_status()==net_server )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_expansion_mission,start_pos);

                    //test route
                    if( test_route_expansion(vec_char_route,city_id,start_pos) )
                    {
                        int new_city_id=m_World.get_free_id_city();
                        //tell other clients (city id, route)
                        for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                        {
                            if(m_vec_players[player_i].id==m_player_id) continue;//skip yourself

                            int pack_size=28+(int)vec_char_route.size();
                            int pack_type=1105;
                            int pack_id=m_packet_id_counter++;
                            int x_start=start_pos[0];
                            int y_start=start_pos[1];
                            char* char_arr=new char[28+(int)vec_char_route.size()];
                            memcpy(char_arr,&pack_size,4);
                            memcpy(char_arr+4,&pack_type,4);
                            memcpy(char_arr+8,&pack_id,4);
                            memcpy(char_arr+12,&city_id,4);
                            memcpy(char_arr+16,&x_start,4);
                            memcpy(char_arr+20,&y_start,4);
                            memcpy(char_arr+24,&new_city_id,4);

                            for(int i=0;i<(int)vec_char_route.size();i++)
                            {
                                char_arr[28+i]=vec_char_route[i];
                            }

                            m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                        }

                        //m_World.accept_city_request_expansion(city_id);//let city start to build????
                        //build new city
                        m_World.translate_char_to_route_and_add_expansion(vec_char_route,city_id,new_city_id,start_pos);

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }
            }
            city_id=m_World.get_request_trade();
            if( city_id!=-1 )
            {
                //if test mode, accept direct
                if( m_pNetCom->get_status()==net_error )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_trade_mission,start_pos);

                    //test route
                    if( test_route_trade(vec_char_route,city_id,start_pos) )
                    {
                        //build trade
                        m_World.translate_char_to_route_and_add_trade(vec_char_route,city_id,start_pos);

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }

                //if client, send data to server
                if( m_pNetCom->get_status()==net_client )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_trade_mission,start_pos);

                    //test route
                    if( test_route_trade(vec_char_route,city_id,start_pos) )
                    {
                        //ask server
                        int pack_size=24+(int)vec_char_route.size();
                        int pack_type=1002;
                        int pack_id=m_packet_id_counter++;
                        int x_start=start_pos[0];
                        int y_start=start_pos[1];
                        char* char_arr=new char[24+(int)vec_char_route.size()];
                        memcpy(char_arr,&pack_size,4);
                        memcpy(char_arr+4,&pack_type,4);
                        memcpy(char_arr+8,&pack_id,4);
                        memcpy(char_arr+12,&city_id,4);
                        memcpy(char_arr+16,&x_start,4);
                        memcpy(char_arr+20,&y_start,4);

                        for(int i=0;i<(int)vec_char_route.size();i++)
                        {
                            char_arr[24+i]=vec_char_route[i];
                        }

                        m_vec_packets_to_send.push_back( net_packet(m_pNetCom->get_server_socket(),char_arr,pack_id) );

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }

                //if server, run test and then send confirmation to clients
                if( m_pNetCom->get_status()==net_server )
                {
                    //get route
                    //translate route to char arr
                    vector<char> vec_char_route;
                    int start_pos[2];
                    m_World.translate_route_to_char(vec_char_route,city_id,cm_trade_mission,start_pos);

                    //test route
                    if( test_route_trade(vec_char_route,city_id,start_pos) )
                    {
                        //tell other clients (city id, route)
                        for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                        {
                            if(m_vec_players[player_i].id==m_player_id) continue;//skip yourself

                            int pack_size=24+(int)vec_char_route.size();
                            int pack_type=1108;
                            int pack_id=m_packet_id_counter++;
                            int x_start=start_pos[0];
                            int y_start=start_pos[1];
                            char* char_arr=new char[24+(int)vec_char_route.size()];
                            memcpy(char_arr,&pack_size,4);
                            memcpy(char_arr+4,&pack_type,4);
                            memcpy(char_arr+8,&pack_id,4);
                            memcpy(char_arr+12,&city_id,4);
                            memcpy(char_arr+16,&x_start,4);
                            memcpy(char_arr+20,&y_start,4);

                            for(int i=0;i<(int)vec_char_route.size();i++)
                            {
                                char_arr[24+i]=vec_char_route[i];
                            }

                            m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                        }

                        //build trade
                        //m_World.accept_city_request_trade(city_id);
                        m_World.translate_char_to_route_and_add_trade(vec_char_route,city_id,start_pos);

                        //play sound
                        m_pSound->playSimpleSound(wav_start_mission,0.5);
                    }
                    else//tell city to return to normal
                    {
                        m_World.reset_city_request(city_id);
                    }
                }
            }

            //test if any cities grew or starved this cycle (only by server)
            if( m_pNetCom->get_status()==net_server )
            {
                //growth
                vector<st_coord_w_val> vec_growth_coord;
                m_World.get_city_growth(vec_growth_coord);
                for(int growth_i=0;growth_i<(int)vec_growth_coord.size();growth_i++)
                {
                    //send to clients, x,y,id
                    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                    {
                        if( m_vec_players[player_i].id==m_player_id ) continue;//dont send to yourself

                        int pack_size=24;
                        int pack_type=1106;
                        int pack_id=m_packet_id_counter++;
                        int int_arr[]={pack_size,pack_type,pack_id,vec_growth_coord[growth_i].x_hex,
                                       vec_growth_coord[growth_i].y_hex,vec_growth_coord[growth_i].val};
                        char* char_arr=new char[pack_size];
                        memcpy(char_arr,int_arr,pack_size);

                        m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                    }
                }

                //starvation
                vector<st_coord_w_val> vec_starve_coord;
                m_World.get_city_starvation(vec_starve_coord);
                for(int starve_i=0;starve_i<(int)vec_starve_coord.size();starve_i++)
                {
                    //send to clients, x,y,id
                    for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                    {
                        if( m_vec_players[player_i].id==m_player_id ) continue;//dont send to yourself

                        int pack_size=24;
                        int pack_type=1107;
                        int pack_id=m_packet_id_counter++;
                        int int_arr[]={pack_size,pack_type,pack_id,vec_starve_coord[starve_i].x_hex,
                                       vec_starve_coord[starve_i].y_hex,vec_starve_coord[starve_i].val};
                        char* char_arr=new char[pack_size];
                        memcpy(char_arr,int_arr,pack_size);

                        m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                    }
                }

                //game over test
                game_over_test();
            }


        }break;

        case gs_game_over:
        {
            m_game_over_anim_time+=m_time_this_cycle-m_time_last_cycle;

            //show old world
            if(m_game_over_anim_time<_gameover_anim_dark_time)
            {//world is shown a few sec more
                //move eye pos
                //zoom center, center
                float zoom_diff=-0.01;
                float rel_pos_x=(m_eye_pos_x-m_window_width/2.0)/(_world_size_x*m_eye_pos_z);
                float rel_pos_y=(m_eye_pos_y-m_window_height/2.0)/(_world_size_y*m_eye_pos_z);
                m_eye_pos_z+=zoom_diff;
                m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z)+m_window_width*0.50;
                m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z)+m_window_height*0.50;
                m_World.update(m_time_this_cycle-m_time_last_cycle,m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);
            }
            else//or menu world
            {
                m_World_menu.update_menu(m_time_this_cycle-m_time_last_cycle);

                //wants to quit?
                if(m_exit_now) return 6;

                //move eye pos
                m_eye_pos_x-=0.00002;
                m_eye_pos_y-=0.001*sinf(m_time_this_cycle*0.5);
                m_eye_pos_z=_zoom_start+10.0*cosf(m_time_this_cycle*0.1);
                //shift extrea due to zoom pos shift
                if( cosf(m_time_this_cycle*0.1)<0 ) m_eye_pos_x-=0.00002*cosf(m_time_this_cycle*0.1);

                key_check_menu();
                int status=m_Menu.update(m_time_this_cycle);
                switch(status)
                {
                    case 11://next, reset all
                    {
                        //reset network
                        //empty the packet vec
                        for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
                        {
                            delete[] m_vec_packets_to_send[pac_i].data_array;
                        }
                        m_vec_packets_to_send.clear();
                        for(int pac_i=0;pac_i<(int)m_vec_replies_to_send.size();pac_i++)
                        {
                            delete[] m_vec_replies_to_send[pac_i].data_array;
                        }
                        m_vec_replies_to_send.clear();
                        m_vec_processed_packets.clear();
                        m_pNetCom->clean_up();
                        //reset players
                        m_vec_players.clear();
                        m_vec_players_at_start.clear();

                        m_game_state=gs_menu_main;

                        //m_pSound->playSimpleSound(wav_button_push,1.0);
                    }break;

                    case 12://play sound flag  (ex: backed out from host menu)
                    {
                        //m_pSound->playSimpleSound(wav_button_push,1.0);
                    }break;
                }
            }

        }break;
    }

    return 0;
}

bool game::draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

    //glPushMatrix();

    //go to eye pos, inside world draw
    //glTranslatef(m_eye_pos_x,m_eye_pos_y,0);

    switch(m_game_state)
    {
        case gs_menu_main:
        {
            m_World_menu.draw(m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);
            m_Menu.draw();

            //black screen intro
            if(m_time_this_cycle<_intro_shade_delay)
            {
                glEnable(GL_BLEND);
                glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
                float intensity=1.0-m_time_this_cycle/_intro_shade_delay;
                glColor4f(0.0,0.0,0.0,intensity);
                glBegin(GL_QUADS);
                glVertex2f(0.0,0.0);
                glVertex2f(0.0,m_window_height);
                glVertex2f(m_window_width,m_window_height);
                glVertex2f(m_window_width,0.0);
                glEnd();
            }

            /*//black screen for flicker effect
            //float intensity=(sinf(m_time_this_cycle*100.0)+1.0)/2.0;
            static float intensity=0.5+float(rand()%1000)/2000.0;
            static int counter=0;
            if(--counter<0)
            {
                counter=100;
                intensity=0.0+float(rand()%1000)/4000.0;
                //cout<<intensity<<endl;
            }
            glEnable(GL_BLEND);
            glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            glColor4f(0.0,0.0,0.0,intensity);
            glBegin(GL_QUADS);
            glVertex2f(0.0,0.0);
            glVertex2f(0.0,m_window_height);
            glVertex2f(m_window_width,m_window_height);
            glVertex2f(m_window_width,0.0);
            glEnd();
            glDisable(GL_BLEND);*/

        }break;

        case gs_running:
        {
            m_World.draw(m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);

            //draw exit screen
            if(m_show_quitscreen)
            {
                //draw black background
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.0,0.0,0.0,0.7);
                glBegin(GL_QUADS);
                glVertex2f(0,0);
                glVertex2f(0,m_window_height);
                glVertex2f(m_window_width,m_window_height);
                glVertex2f(m_window_width,0);
                glEnd();
                glDisable(GL_BLEND);
                //draw text decal
                m_de_quitscreen.draw();
            }
        }break;

        case gs_game_over:
        {
            //cout<<m_game_over_anim_time<<endl;
            if(m_game_over_anim_time<_gameover_anim_dark_time)
            {
                m_World.draw(m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);
                //draw black screen, world
                float transp=m_game_over_anim_time/_gameover_anim_dark_time;
                glEnable(GL_BLEND);
                glColor4f(0,0,0,transp);
                glBegin(GL_QUADS);
                glVertex2f(0,0);
                glVertex2f(0,m_window_height);
                glVertex2f(m_window_width,m_window_height);
                glVertex2f(m_window_width,0);
                glEnd();
                glDisable(GL_BLEND);
            }
            else
            {
                m_World_menu.draw(m_eye_pos_z,m_eye_pos_x,m_eye_pos_y);
                //draw first black screen, menu world
                //if(m_game_over_anim_time<_gameover_anim_dark_time*2.5)
                {
                    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    float transp=1.0-(m_game_over_anim_time-_gameover_anim_dark_time)/_gameover_anim_dark_time/2.0;
                    if(transp<0.5) transp=0.5;
                    glEnable(GL_BLEND);
                    glColor4f(0,0,0,transp);
                    glBegin(GL_QUADS);
                    glVertex2f(0,0);
                    glVertex2f(0,m_window_height);
                    glVertex2f(m_window_width,m_window_height);
                    glVertex2f(m_window_width,0);
                    glEnd();
                    glDisable(GL_BLEND);
                }
                m_Menu.draw();

                //draw second black screen, menu
                if(m_game_over_anim_time<_gameover_anim_dark_time*2.0)
                {
                    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    float transp=1.0-(m_game_over_anim_time-_gameover_anim_dark_time)/_gameover_anim_dark_time;
                    if(transp<0.0) transp=0.0;
                    glEnable(GL_BLEND);
                    glColor4f(0,0,0,transp);
                    glBegin(GL_QUADS);
                    glVertex2f(0,0);
                    glVertex2f(0,m_window_height);
                    glVertex2f(m_window_width,m_window_height);
                    glVertex2f(m_window_width,0);
                    glEnd();
                    glDisable(GL_BLEND);
                }
                /*//draw third black screen, list
                if(m_game_over_anim_time<_gameover_anim_dark_time*3.0)
                {
                    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
                    float transp=1.0-(m_game_over_anim_time-_gameover_anim_dark_time*2.0)/_gameover_anim_dark_time;
                    if(transp<0.0) transp=0.0;
                    glEnable(GL_BLEND);
                    glBegin(GL_QUADS);
                    glColor4f(0,0,0,0);
                    glVertex2f(0,m_window_height*0.3);
                    glColor4f(0,0,0,transp);
                    glVertex2f(0,m_window_height*0.5);
                    glVertex2f(m_window_width,m_window_height*0.5);
                    glColor4f(0,0,0,0);
                    glVertex2f(m_window_width,m_window_height*0.3);

                    glColor4f(0,0,0,transp);
                    glVertex2f(0,m_window_height*0.5);
                    glColor4f(0,0,0,0);
                    glVertex2f(0,m_window_height*0.7);
                    glVertex2f(m_window_width,m_window_height*0.7);
                    glColor4f(0,0,0,transp);
                    glVertex2f(m_window_width,m_window_height*0.5);
                    glEnd();
                    glDisable(GL_BLEND);
                }*/
            }

        }break;
    }


    /*glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glBegin(GL_QUADS);
    glTexCoord2f(0,1);
    glVertex2f(0,0);
    glTexCoord2f(0,0);
    glVertex2f(0,120);
    glTexCoord2f(1,0);
    glVertex2f(160,120);
    glTexCoord2f(1,1);
    glVertex2f(160,0);
    glEnd();

    glBegin(GL_QUADS);
    glTexCoord2f(0,1);
    glVertex2f(0,0);
    glTexCoord2f(0,0);
    glVertex2f(0,600);
    glTexCoord2f(1,0);
    glVertex2f(800,600);
    glTexCoord2f(1,1);
    glVertex2f(800,0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);*/

    //glPopMatrix();

    //XXXXX



    return true;
}

bool game::key_check_ingame(void)
{
    //exit if esc
    if(m_keys[27] && m_key_delay<=0)
    {
        m_show_quitscreen=!m_show_quitscreen;
        m_key_delay=_key_esc_delay;
    }
    //then enter
    if(m_keys[13] && m_key_delay<=0 && m_show_quitscreen)
    {//return to main menu
        m_key_delay=_key_esc_delay;
        m_show_quitscreen=false;
        //empty the packet vec
        for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
        {
            delete[] m_vec_packets_to_send[pac_i].data_array;
        }
        m_vec_packets_to_send.clear();
        //reset network, menu and world
        m_pNetCom->clean_up();
        m_game_state=gs_menu_main;
        m_Menu.set_menu_state(ms_main);
        m_World.init(m_window_width,m_window_height,&m_player_color_id,m_keys,m_texture_tile,m_pSound);

        return true;
    }

    //enable/disable music
    if(m_keys[77] && m_key_delay<=0)
    {
        m_key_delay=_key_esc_delay;
        if( m_pSound->get_source_status(m_music_source) )
        {//music is off, enable
            m_pSound->enable_music(true);
            m_pSound->resume_source(m_music_source);
            if(m_music_intro_not_played)
            {
                m_music_intro_not_played=false;
                m_intro_done=false;
            }
        }
        else//music is on, disable
        {
            m_pSound->enable_music(false);
            m_pSound->pause_source(m_music_source);
            if(!m_intro_done)
            {
                m_music_intro_not_played=true;
                m_intro_done=true;
            }
        }
    }

    //move eye
    if(m_keys[39]) m_eye_pos_x-=1000.0*(m_time_this_cycle-m_time_last_cycle);//right
    if(m_keys[37]) m_eye_pos_x+=1000.0*(m_time_this_cycle-m_time_last_cycle);//left
    if(m_keys[38]) m_eye_pos_y+=1000.0*(m_time_this_cycle-m_time_last_cycle);//up
    if(m_keys[40]) m_eye_pos_y-=1000.0*(m_time_this_cycle-m_time_last_cycle);/*cout<<m_eye_pos_y<<endl; }*///down
    //limits in world

    //reset view [backspace]
    if(m_keys[8])
    {
        int pos_pix[2];
        m_World.get_start_pos_pix(pos_pix);
        m_eye_pos_x=-pos_pix[0];
        m_eye_pos_y=-pos_pix[1];
        m_eye_pos_z=_zoom_start;
    }

    /*//player id swap, TEMP
    if(m_keys[48]) m_player_color_id=0;
    if(m_keys[49]) m_player_color_id=1;
    if(m_keys[50]) m_player_color_id=2;
    if(m_keys[51]) m_player_color_id=3;
    if(m_keys[52]) m_player_color_id=4;
    if(m_keys[53]) m_player_color_id=5;
    if(m_keys[54]) m_player_color_id=6;
    if(m_keys[55]) m_player_color_id=7;
    if(m_keys[56]) m_player_color_id=8;
    if(m_keys[57]) m_player_color_id=9;
    //TEMP*/

    //zooming
    if(m_mouse_button[2])
    {
        float last_z=m_eye_pos_z;
        m_eye_pos_z+=5;//zoom in
        if(m_eye_pos_z>60) m_eye_pos_z=60;
        else
        {
            /*//zoom center up left
            float rel_pos_x=m_eye_pos_x/(_world_size_x*last_z);
            float rel_pos_y=m_eye_pos_y/(_world_size_y*last_z);
            m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z);
            m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z);
            */

            /*//zoom center, center
            float rel_pos_x=(m_eye_pos_x-m_window_width/2.0)/(_world_size_x*last_z);
            float rel_pos_y=(m_eye_pos_y-m_window_height/2.0)/(_world_size_y*last_z);
            m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z)+m_window_width/2.0;
            m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z)+m_window_height/2.0;*/

            //zoom center, mouse pos
            float rel_pos_x=(m_eye_pos_x-m_mouse_pos[0])/(_world_size_x*last_z);
            float rel_pos_y=(m_eye_pos_y-m_mouse_pos[1])/(_world_size_y*last_z);
            m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z)+m_mouse_pos[0];
            m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z)+m_mouse_pos[1];

            //update music vol
            float volume=m_eye_pos_z/50.0;
            if(volume<0.4) volume=0.4;
            else if(volume>1.0) volume=1.0;
            m_pSound->set_volume(m_music_source,volume);
        }
    }
    if(m_mouse_button[3])
    {
        float last_z=m_eye_pos_z;
        m_eye_pos_z-=5;//zoom out
        if(m_eye_pos_z<10) m_eye_pos_z=10;
        else
        {
            /*//zoom center, center
            float rel_pos_x=(m_eye_pos_x-m_window_width/2.0)/(_world_size_x*last_z);
            float rel_pos_y=(m_eye_pos_y-m_window_height/2.0)/(_world_size_y*last_z);
            m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z)+m_window_width/2.0;
            m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z)+m_window_height/2.0;
            */

            //zoom center, mouse pos
            float rel_pos_x=(m_eye_pos_x-m_mouse_pos[0])/(_world_size_x*last_z);
            float rel_pos_y=(m_eye_pos_y-m_mouse_pos[1])/(_world_size_y*last_z);
            m_eye_pos_x=rel_pos_x*(_world_size_x*m_eye_pos_z)+m_mouse_pos[0];
            m_eye_pos_y=rel_pos_y*(_world_size_y*m_eye_pos_z)+m_mouse_pos[1];

            //update music vol
            float volume=m_eye_pos_z/50.0;
            if(volume<0.4) volume=0.4;
            else if(volume>1.0) volume=1.0;
            m_pSound->set_volume(m_music_source,volume);
        }
    }

    m_World.set_mouse_input(m_mouse_pos[0],m_mouse_pos[1],m_mouse_button);

    //reset mouse scroll
    m_mouse_button[2]=m_mouse_button[3]=false;

    return true;
}

bool game::key_check_menu(void)
{
    //quit if ESC
    if(m_keys[27]) m_exit_now=true;

    m_World.set_mouse_input(m_mouse_pos[0],m_mouse_pos[1],m_mouse_button);

    //reset mouse scroll
    m_mouse_button[2]=m_mouse_button[3]=false;

    return true;
}

bool game::game_over_test(void)//only for server
{
    bool game_over=false;

    //count tiles
    vector<int> vec_tile_count( (int)m_vec_players_at_start.size(),0 );
    m_World.get_city_tile_count_result(vec_tile_count);

    //check time
    if(m_game_limit_time!=0)
    {
        if(m_time_this_cycle-m_time_at_start>m_game_limit_time*60.0)//time out
        {
            //get winner and tell clients
            game_over=true;
        }
    }

    //check tiles
    if(m_game_limit_tile!=0)
    {
        for(int player_i=0;player_i<(int)vec_tile_count.size();player_i++)
        {
            if(vec_tile_count[player_i]>=m_game_limit_tile)
            {
                game_over=true;
            }
        }
    }

    //game limit reached, end game, tell clients
    if(game_over)
    {
        //send info to clients, score in col id order in m_vec_players_at_start
        for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
        {
            if(m_vec_players[player_i].id==m_player_id) continue;//dont send to yourself

            int pac_size=12+(int)m_vec_players_at_start.size()*4;
            int pac_type=1110;
            int pac_id=m_packet_id_counter++;
            int score_data[ (int)m_vec_players_at_start.size() ];
            for(int i=0;i<(int)m_vec_players_at_start.size();i++)
            {
                score_data[i]=vec_tile_count[i];
            }
            char* char_arr=new char[pac_size];
            memcpy(char_arr,&pac_size,4);
            memcpy(char_arr+4,&pac_type,4);
            memcpy(char_arr+8,&pac_id,4);
            memcpy(char_arr+12,score_data,(int)m_vec_players_at_start.size()*4);

            m_vec_packets_to_send.push_back( net_packet( m_vec_players[player_i].id,char_arr,pac_id ) );
        }

        //set game over state local (server)
        set_game_over(vec_tile_count);
    }

    return true;
}

bool game::set_game_over(vector<int> vec_tile_count)
{
    cout<<"Game over: Score:\n";
    for(int i=0;i<(int)m_vec_players_at_start.size();i++)
    {
        cout<<m_vec_players_at_start[i]<<"\t\t"<<vec_tile_count[i]<<endl;
    }
    cout<<endl;
    //set game state
    m_game_state=gs_game_over;
    //set menu state
    m_Menu.set_menu_state(ms_game_over);
    //reset gameover animation timer
    m_game_over_anim_time=0;

    //sort players score
    cout<<"Game over: Sorting players\n";
    vector<int> vec_sortlist;//given index represent index of player, output value is that players position
    for(int i=0;i<(int)vec_tile_count.size();i++) vec_sortlist.push_back(i);
    while(true)
    {
        bool updated=false;
        for(int i=0;i<(int)vec_tile_count.size()-1;i++)
        {
            if( vec_tile_count[ vec_sortlist[i] ] < vec_tile_count[ vec_sortlist[i+1] ] )
            {//next element have larger value, swap pos
                int temp_val=vec_sortlist[i];
                vec_sortlist[i]=vec_sortlist[i+1];
                vec_sortlist[i+1]=temp_val;
                updated=true;
            }
        }
        if(!updated) break;//sorted
    }

    //update score table in menu
    cout<<"Game over: Updating player score table\n";
    int position=0;
    while(true)
    {
        bool position_found=false;
        for(int player_i=0;player_i<(int)vec_tile_count.size();player_i++)
        {
            if( vec_sortlist[player_i]==position )
            {
                m_Menu.add_player_to_gameover_list( m_vec_players_at_start[ player_i ],vec_tile_count[ player_i ] );
                position++;
                position_found=true;
                break;
            }
        }
        if(!position_found) break;//done
    }

    //play sound
    m_pSound->playSimpleSound(wav_game_over,1.0);

    return true;
}

bool game::load_textures(void)
{
    //load files and decode

    string s_decode=base64_decode( load_base64_file(file_texture_menu) );
    unsigned char* texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_title = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_gameover) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_gameover = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_buttons) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_buttons = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_tile) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_tile = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_MIPMAPS
		);

    s_decode=base64_decode( load_base64_file(file_texture_font_light) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_font[0] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_font_dark) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_font[1] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_font_mask) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_font[2] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help1) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[0] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help2) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[1] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help3) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[2] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help4) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[3] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help5) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[4] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help6) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[5] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help7) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[6] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);

    s_decode=base64_decode( load_base64_file(file_texture_help8) );
    texture_data=(unsigned char*)s_decode.c_str();
    //load texture from memory
    m_texture_help[7] = SOIL_load_OGL_texture_from_memory
		(
		texture_data,//buffer
		(int)s_decode.length(),//Buffer length
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
		);



    /*//old
	m_texture_title = SOIL_load_OGL_texture
	(
		"texture\\menu.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_title == 0) return false;

	m_texture_buttons = SOIL_load_OGL_texture
	(
		"texture\\buttons.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_buttons == 0) return false;


	m_texture_help[0] = SOIL_load_OGL_texture
	(
		"texture\\help1.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[0] == 0) return false;

	m_texture_help[1] = SOIL_load_OGL_texture
	(
		"texture\\help2.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[1] == 0) return false;

	m_texture_help[2] = SOIL_load_OGL_texture
	(
		"texture\\help3.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[2] == 0) return false;

	m_texture_help[3] = SOIL_load_OGL_texture
	(
		"texture\\help4.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[3] == 0) return false;

	m_texture_help[4] = SOIL_load_OGL_texture
	(
		"texture\\help5.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[4] == 0) return false;

	m_texture_help[5] = SOIL_load_OGL_texture
	(
		"texture\\help6.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[5] == 0) return false;

	m_texture_help[6] = SOIL_load_OGL_texture
	(
		"texture\\help7.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[6] == 0) return false;

	m_texture_help[7] = SOIL_load_OGL_texture
	(
		"texture\\help8.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_help[7] == 0) return false;


	m_texture_tile = SOIL_load_OGL_texture
	(
		"texture\\tile.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_tile == 0) return false;

	m_texture_gameover = SOIL_load_OGL_texture
	(
		"texture\\gameover.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_gameover == 0) return false;

	m_texture_font[0] = SOIL_load_OGL_texture
	(
		"texture\\fonts\\default_light.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_font[0] == 0) return false;

	m_texture_font[1] = SOIL_load_OGL_texture
	(
		"texture\\fonts\\default_dark.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_font[1] == 0) return false;

	m_texture_font[2] = SOIL_load_OGL_texture
	(
		"texture\\fonts\\default_mask.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_LOAD_RGBA | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(m_texture_font[2] == 0) return false;*/

	/*//texture settings
	glBindTexture(GL_TEXTURE_2D,m_texture_tile);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    //GL_NEAREST GL_LINEAR*/

    return true;
}

bool game::interpret_data(char* data_array,SOCKET soc_sender)
{
    int packet_type=0;
    int packet_size=0;
    int packet_id=0;
    memcpy(&packet_type,data_array+4,4);
    memcpy(&packet_size,data_array,4);
    memcpy(&packet_id,data_array+8,4);
    cout<<"Network: Packet Type: "<<packet_type<<", id: "<<packet_id<<endl;

    //send confirmation reply, size,type,pack_id,reply_id
    if(packet_type!=2000)//do not send reply on replies
    {
        cout<<"Network: Sending confirmation reply for packet: "<<packet_id<<endl;
        int pac_id=m_packet_id_counter++;
        int int_arr[4]={16,2000,pac_id,packet_id};
        char* char_arr0=new char[16];
        memcpy( char_arr0,int_arr,16 );

        m_vec_replies_to_send.push_back( net_packet((int)soc_sender,char_arr0,pac_id,0.0,true) );
    }

    //test if this data already is processed
    for(int pac_i=0;pac_i<(int)m_vec_processed_packets.size();pac_i++)
    {
        if(m_vec_processed_packets[pac_i].pac_id==packet_id &&
           m_vec_processed_packets[pac_i].sock_id==(int)soc_sender)
        {//ignore packet
            cout<<"Network: This packet already processed\n";
            return true;
        }
    }
    //packet was new, add to processed packet vec
    m_vec_processed_packets.push_back( st_id_and_time((int)soc_sender,packet_id,_network_packet_store_time) );

    switch (packet_type)
    {
        case 2000://confirmation reply
        {
            //find packet and remove it
            int confirmed_packet_id=0;
            memcpy( &confirmed_packet_id,data_array+12,4 );
            int pac_index=-1;
            for(int pac_i=0;pac_i<(int)m_vec_packets_to_send.size();pac_i++)
            {
                if( m_vec_packets_to_send[pac_i].pac_id==confirmed_packet_id )
                {
                    pac_index=pac_i;
                    break;
                }
            }
            if(pac_index==-1)
            {
                //this confirmation packet may already been registerd, therefore no match to packets in send vector
                cout<<"Error: Network: Confirmation packet id could not be found, maybe recently deleted\n";
                //ofs_log<<"Error: Network: Confirmation packet id could not be found\n";
            }
            else
            {
                cout<<"Network: Confirmation reply for packet: "<<confirmed_packet_id<<endl;
                delete[] m_vec_packets_to_send[pac_index].data_array;
                m_vec_packets_to_send.erase( m_vec_packets_to_send.begin()+pac_index );
            }
        }break;

        //--To Server---

        case 1000://startpackage from client with name, in string...
        {
            if(!m_pNetCom->get_accept_new_clients_flag())
            {
                cout<<"Network: Received startpackage from declined client, ignored\n";
                return false;
            }
            //test game version
            float game_version_client;
            memcpy(&game_version_client,data_array+12,4);
            if( (float)game_version_client!=(float)_version )
            {//wrong version
                //that client cant join
                cout<<"Network: Client with other game version tried to join, ignored\n";
                cout<<" Server version: "<<_version<<", Client version: "<<game_version_client<<endl;
                return false;
            }

            //convert float arr to char arr
            char* char_arr=new char[packet_size-16];
            memcpy( char_arr,data_array+16,packet_size-16 );
            string name(char_arr);
            cout<<"Network: New player name: "<<name<<endl;
            m_Menu.add_player_to_list(name);
            delete[] char_arr;

            //respond with that players id (socket), size,type,pac_id,sock_id
            int sock_id=(int)soc_sender;
            char* char_arr2=new char[sizeof(int)*4];
            int p_size=sizeof(int)*4;
            int p_type=1100;
            int p_id=m_packet_id_counter++;
            memcpy(char_arr2,&p_size,4);
            memcpy(char_arr2+4,&p_type,4);
            memcpy(char_arr2+8,&p_id,4);
            memcpy(char_arr2+12,&sock_id,4);

            m_vec_packets_to_send.push_back( net_packet(sock_id,char_arr2,p_id) );

            //send list of other player name and id, size,type,pac_id,names...
            string player_list("000011112222");
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
            {
                player_list.append(m_vec_players[player_i].name);
                player_list.append(1,' ');
                char num_arr[256];
                player_list.append( itoa(m_vec_players[player_i].id,num_arr,10) );
                player_list.append(1,' ');
            }
            player_list.append(1,'\0');
            char* char_arr3=new char[player_list.length()];
            p_size=player_list.length();
            p_type=1101;
            p_id=m_packet_id_counter++;
            memcpy(char_arr3,player_list.c_str(),player_list.length());
            memcpy(char_arr3,&p_size,4);
            memcpy(char_arr3+4,&p_type,4);
            memcpy(char_arr3+8,&p_id,4);

            m_vec_packets_to_send.push_back( net_packet(sock_id,char_arr3,p_id) );

            //send info about new player to other players, name and id
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
            {
                if(m_vec_players[player_i].id==m_player_id) continue;//dont send to yourself
                //size,type,pac_id,soc_id,name
                char* char_arr4=new char[packet_size];//slot for id instead of game version
                memcpy( char_arr4+16,data_array+16,packet_size-16 );//insert name from packet 1000
                p_type=1102;
                p_size=packet_size;
                p_id=m_packet_id_counter++;
                int player_id=int(sock_id);
                memcpy(char_arr4,&p_size,4);
                memcpy(char_arr4+4,&p_type,4);
                memcpy(char_arr4+8,&p_id,4);
                memcpy(char_arr4+12,&player_id,4);

                cout<<"\nSENDING:\n";
                cout<<char_arr4+16<<endl<<endl;

                m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr4,p_id) );
            }

            //add the new player to player vec
            st_player temp_player;
            temp_player.name=name;
            temp_player.id=sock_id;
            m_vec_players.push_back(temp_player);

        }break;

        case 1001://city exp request
        {
            int city_id=-1;
            int x_start=-1;
            int y_start=-1;
            memcpy(&city_id,data_array+12,4);
            memcpy(&x_start,data_array+16,4);
            memcpy(&y_start,data_array+20,4);
            cout<<"Network: New city expansion request: "<<city_id<<endl;
            vector<char> vec_char_route;
            for(int i=0;i<packet_size-28;i++)
            {
                vec_char_route.push_back(data_array[i+28]);
            }
            int start_pos[2]={x_start,y_start};
            if( !test_route_expansion(vec_char_route,city_id,start_pos) )
            {
                cout<<"Error: Server can not tolerate new expansion route\n";
                //ofs_log<<"Error: Server can not tolerate new expansion route\n";
                //m_error_flag=2;
            }
            else//route ok
            {
                //tell other clients
                //change packet type
                int pack_type=1105;
                memcpy(data_array+4,&pack_type,4);
                //get new city id and add
                int new_city_id=m_World.get_free_id_city();
                memcpy(data_array+24,&new_city_id,4);

                for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                {
                    if( m_vec_players[player_i].id==m_player_id ) continue;//dont send to yourself

                    char* char_arr=new char[packet_size];
                    for(int i=0;i<packet_size;i++) char_arr[i]=data_array[i];//copy data
                    //update pack id
                    int pack_id=m_packet_id_counter++;
                    memcpy(char_arr+8,&pack_id,4);
                    //send
                    m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                }
                //add route local
                m_World.translate_char_to_route_and_add_expansion(vec_char_route,city_id,new_city_id,start_pos);
            }
        }break;

        case 1002://trade request
        {
            int city_id=-1;
            int x_start=-1;
            int y_start=-1;
            memcpy(&city_id,data_array+12,4);
            memcpy(&x_start,data_array+16,4);
            memcpy(&y_start,data_array+20,4);
            cout<<"Network: New trade request\n";
            vector<char> vec_char_route;
            for(int i=0;i<packet_size-24;i++)
            {
                vec_char_route.push_back(data_array[i+24]);
            }
            int start_pos[2]={x_start,y_start};
            if( !test_route_trade(vec_char_route,city_id,start_pos) )
            {
                cout<<"Error: Network: Server can not tolerate new trade route\n";
                //ofs_log<<"Error: Network: Server can not tolerate new trade route\n";
                //m_error_flag=4;
            }
            else//route ok
            {//tell other clients
                //update packet with new route and new start pos (might not be changed, but updated anyway)
                for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                {
                    if(m_vec_players[player_i].id==m_player_id) continue;//skip yourself

                    int pack_size=24+(int)vec_char_route.size();
                    int pack_type=1108;
                    int pack_id=m_packet_id_counter++;
                    x_start=start_pos[0];
                    y_start=start_pos[1];
                    char* char_arr=new char[24+(int)vec_char_route.size()];
                    memcpy(char_arr,&pack_size,4);
                    memcpy(char_arr+4,&pack_type,4);
                    memcpy(char_arr+8,&pack_id,4);
                    memcpy(char_arr+12,&city_id,4);
                    memcpy(char_arr+16,&x_start,4);
                    memcpy(char_arr+20,&y_start,4);

                    for(int i=0;i<(int)vec_char_route.size();i++)
                    {
                        char_arr[24+i]=vec_char_route[i];
                    }

                    m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                }

                /*//change packet type, old with only resend of requested trade, not with updated path
                int pack_type=1108;
                memcpy(data_array+4,&pack_type,4);
                for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                {
                    if( m_vec_players[player_i].id==m_player_id ) continue;//dont send to yourself

                    char* char_arr=new char[packet_size];
                    for(int i=0;i<packet_size;i++) char_arr[i]=data_array[i];//copy data
                    //update pack id
                    int pack_id=m_packet_id_counter++;
                    memcpy(char_arr+8,&pack_id,4);
                    //send
                    m_vec_packets_to_send.push_back( net_packet(m_vec_players[player_i].id,char_arr,pack_id) );
                }*/

                //add route local
                m_World.translate_char_to_route_and_add_trade(vec_char_route,city_id,start_pos);
            }
        }break;


        //--To Client---

        case 1100://confirmed startpackage from client to server
        {
            //contains this players ID (socket)
            int player_id=0;
            memcpy(&player_id,data_array+8,4);
            m_player_id=player_id;
            cout<<"Network: My ID: "<<player_id<<endl;
        }break;

        case 1101://player list
        {
            cout<<"Network: Player list:\n";
            //contains player name ' ' and id ' '
            string player_list(data_array+12);
            stringstream ss(player_list);
            string word;
            while(ss>>word)
            {
                st_player temp_player;
                temp_player.name=word;
                ss>>word;//get id
                int id=atoi(word.c_str());
                temp_player.id=id;
                cout<<"-"<<temp_player.name<<" "<<temp_player.id<<endl;
                m_vec_players.push_back(temp_player);
            }
            //add yourself
            st_player temp_player;
            temp_player.name=m_Menu.get_player_name();
            temp_player.id=m_player_id;
            m_vec_players.push_back(temp_player);
            //enter lobby
            m_Menu.set_menu_state(ms_lobby);
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
                m_Menu.add_player_to_list(m_vec_players[player_i].name);

            cout<<"done with 1101\n";
        }break;

        case 1102://new player joined lobby 0123-4567-891011-12131415 size,type,pac_id,sock_id,name
        {
            char* char_arr_name=new char[packet_size-16];
            memcpy( char_arr_name,data_array+16,packet_size-16 );
            string name(char_arr_name);
            int player_id=0;
            memcpy(&player_id,data_array+12,4);
            //add to player vec
            st_player temp_player;
            temp_player.name=name;
            temp_player.id=player_id;
            m_vec_players.push_back(temp_player);
            cout<<"Network: New player name: "<<name<<", "<<player_id<<endl;
            m_Menu.add_player_to_list(name);
            delete[] char_arr_name;
        }break;

        case 1103://lost player, id in packet
        {
            int lost_player_id=0;
            memcpy(&lost_player_id,data_array+12,4);
            //remove player
            lost_player(lost_player_id);
        }break;

        case 1104://game started, size,type,id, seed,time lim,tile lim
        {
            cout<<"Network: Server started the game\n";
            //reset world
            m_World.init(m_window_width,m_window_height,&m_player_color_id,m_keys,m_texture_tile,m_pSound);
            //remember players at start, and their col id (city ownership, as index)
            m_vec_players_at_start.clear();
            for(int player_i=0;player_i<(int)m_vec_players.size();player_i++)
            {
                m_vec_players_at_start.push_back( m_vec_players[player_i].name );
            }
            //translate data
            int int_data[7];
            memcpy(int_data,data_array,28);
            m_seed=int_data[3];
            m_game_limit_time=int_data[4];
            m_game_limit_tile=int_data[5];
            m_player_color_id=int_data[6];
            if( !m_World.init_game_world(m_seed,(int)m_vec_players.size()) )
            {//could not create world, reset game
                m_Menu.set_menu_state(ms_main);
                m_pNetCom->clean_up();
                m_vec_players.clear();
            }
            else m_game_state=gs_running;//start game
            //move cam to your city
            int pos_pix[2];
            m_World.get_start_pos_pix(pos_pix);
            m_eye_pos_x=-pos_pix[0];
            m_eye_pos_y=-pos_pix[1];
            m_eye_pos_z=_zoom_start;
            //tell world about network mode
            m_World.set_network_mode(2);
            //play start sound
            m_pSound->playSimpleSound(wav_start_match,1.0);
        }break;

        case 1105://new city expansion started
        {
            int city_id=-1;
            int x_start=-1;
            int y_start=-1;
            int new_city_id=-1;
            memcpy(&city_id,data_array+12,4);
            memcpy(&x_start,data_array+16,4);
            memcpy(&y_start,data_array+20,4);
            memcpy(&new_city_id,data_array+24,4);
            cout<<"Network: New city expansion confirmed: "<<city_id<<endl;
            vector<char> vec_char_route;
            for(int i=0;i<packet_size-28;i++)
            {
                vec_char_route.push_back(data_array[i+28]);
            }
            int start_pos[2]={x_start,y_start};
            if( !test_route_expansion(vec_char_route,city_id,start_pos) )
            {
                cout<<"Error: Client can not tolerate new expansion route\n";
                //ofs_log<<"Error: Client can not tolerate new expansion route\n";
                //m_error_flag=1;
                //remove packet reply to force resend
                delete[] m_vec_replies_to_send.back().data_array;
                m_vec_replies_to_send.pop_back();
                //and remove from processed packets vector
                m_vec_processed_packets.pop_back();

                //or try anyway...
                //m_World.translate_char_to_route_and_add_expansion(vec_char_route,city_id,new_city_id,start_pos);
            }
            else m_World.translate_char_to_route_and_add_expansion(vec_char_route,city_id,new_city_id,start_pos);
        }break;

        case 1106://city growth, x_hex, y_hex, city_id
        {
            int x_hex=-1;
            int y_hex=-1;
            int city_id=-1;
            memcpy(&x_hex,data_array+12,4);
            memcpy(&y_hex,data_array+16,4);
            memcpy(&city_id,data_array+20,4);
            cout<<"Network: City growth at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;
            int pos[2]={x_hex,y_hex};
            if( m_World.do_city_growth(city_id,pos)==0 )
            {//could not find city
                cout<<"Error: Network: City Growth: Can not place growth at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;
                //ofs_log<<"Error: Network: City Growth: Can not place growth at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;

                //remove packet reply to force resend
                delete[] m_vec_replies_to_send.back().data_array;
                m_vec_replies_to_send.pop_back();
                //and remove from processed packets vector
                m_vec_processed_packets.pop_back();
            }
        }break;

        case 1107://city starvation, x_hex, y_hex, city_id
        {
            int x_hex=-1;
            int y_hex=-1;
            int city_id=-1;
            memcpy(&x_hex,data_array+12,4);
            memcpy(&y_hex,data_array+16,4);
            memcpy(&city_id,data_array+20,4);
            cout<<"Network: City starvation at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;
            int pos[2]={x_hex,y_hex};
            if( !m_World.do_city_starvation(city_id,pos) )
            {//could not find city
                cout<<"Error: Network: City Growth: Can not place starvation at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;
                //ofs_log<<"Error: Network: City Growth: Can not place starvation at: "<<x_hex<<", "<<y_hex<<" id: "<<city_id<<endl;

                //remove packet reply to force resend
                delete[] m_vec_replies_to_send.back().data_array;
                m_vec_replies_to_send.pop_back();
                //and remove from processed packets vector
                m_vec_processed_packets.pop_back();
            }
        }break;

        case 1108://new trade started
        {
            int city_id=-1;
            int x_start=-1;
            int y_start=-1;
            memcpy(&city_id,data_array+12,4);
            memcpy(&x_start,data_array+16,4);
            memcpy(&y_start,data_array+20,4);
            cout<<"Network: New trade confirmed\n";
            vector<char> vec_char_route;
            for(int i=0;i<packet_size-24;i++)
            {
                vec_char_route.push_back(data_array[i+24]);
            }
            int start_pos[2]={x_start,y_start};
            if( !test_route_trade(vec_char_route,city_id,start_pos) )
            {
                cout<<"Error: Client can not tolerate new trade route\n";
                //ofs_log<<"Error: Client can not tolerate new trade route\n";
                //m_error_flag=3;
                //remove packet reply to force resend
                delete[] m_vec_replies_to_send.back().data_array;
                m_vec_replies_to_send.pop_back();
                //and remove from processed packets vector
                m_vec_processed_packets.pop_back();

                //but try anyway...
                //m_World.translate_char_to_route_and_add_trade(vec_char_route,city_id,start_pos);
            }
            else m_World.translate_char_to_route_and_add_trade(vec_char_route,city_id,start_pos);
        }break;

        case 1109://client denied to join package
        {
            //reset network and menu
            m_Menu.set_menu_state(ms_main);
            m_pNetCom->clean_up();
            m_vec_packets_to_send.clear();
            m_vec_players.clear();
            m_vec_processed_packets.clear();
        }break;

        case 1110://game over, score table included
        {
            vector<int> vec_tile_count;
            for(int i=0;i<(int)m_vec_players_at_start.size();i++)
            {
                int score=0;
                memcpy(&score,data_array+12+i*4,4);
                vec_tile_count.push_back(score);
            }
            set_game_over(vec_tile_count);
        }break;
    }

    //cout<<"done interp\n";

    return true;
}

bool game::test_route_expansion(vector<char> vec_char_route,int city_id,int start_pos[2])
{
    //tiles have to be connected
    for(int tile_i=0;tile_i<(int)vec_char_route.size();tile_i++)
    {
        if( vec_char_route[tile_i]!='a'&&vec_char_route[tile_i]!='b'&&
            vec_char_route[tile_i]!='c'&&vec_char_route[tile_i]!='d'&&
            vec_char_route[tile_i]!='e'&&vec_char_route[tile_i]!='f' )
        {
            cout<<"Error: Testing expansion route: Connection error\n";
            for(int i=0;i<(int)vec_char_route.size();i++)
             cout<<vec_char_route[i];
            cout<<endl;
            return false;
        }
    }

    //have to start in a city
    if( !m_World.is_tile_in_city(city_id,start_pos) )
    {
        cout<<"Error: Testing expansion route: Start pos not in city\n";
        return false;
    }

    //tiles have to be buildable
    //translate char to route
    if( !m_World.translate_char_to_route_and_test_expansion(vec_char_route,city_id,start_pos) )
    {
        cout<<"Error: Testing expansion route: Route is not buildable\n";
        return false;
    }

    return true;
}

bool game::test_route_trade(vector<char>& vec_char_route,int city_id,int start_pos[2])
{
    //tiles have to be connected
    for(int tile_i=0;tile_i<(int)vec_char_route.size();tile_i++)
    {
        if( vec_char_route[tile_i]!='a'&&vec_char_route[tile_i]!='b'&&
            vec_char_route[tile_i]!='c'&&vec_char_route[tile_i]!='d'&&
            vec_char_route[tile_i]!='e'&&vec_char_route[tile_i]!='f' )
        {
            cout<<"Error: Testing trade route: Connection error\n";
            for(int i=0;i<(int)vec_char_route.size();i++)
             cout<<vec_char_route[i];
            cout<<endl;
            return false;
        }
    }

    //have to start in a city
    if( !m_World.is_tile_in_city(city_id,start_pos) )
    {
        cout<<"Error: Testing trade route: Start pos not in city\n";
        return false;
    }

    //tiles have to be buildable
    //translate char to route
    if( !m_World.translate_char_to_route_and_test_trade(vec_char_route,city_id,start_pos) )
    {
        cout<<"Error: Testing trade route: Route is not buildable\n";
        return false;
    }

    return true;
}

bool game::load_sound(void)
{
    cout<<"Loading Sounds\n";
    m_pSound=new sound();

    /*//load sound files, old
    //m_pSound->load_WAVE_from_file(wav_beep1,"sound\\beep1.wav");
    m_pSound->load_WAVE_from_file(wav_abort_mission,"sound\\abort_mission.wav");
    //m_pSound->load_WAVE_from_file(wav_add_path,"sound\\add_path.wav");
    //m_pSound->load_WAVE_from_file(wav_button_mark,"sound\\button_mark.wav");
    //m_pSound->load_WAVE_from_file(wav_button_push,"sound\\button_push.wav");
    m_pSound->load_WAVE_from_file(wav_finish_mission,"sound\\finish_mission.wav");
    m_pSound->load_WAVE_from_file(wav_game_over,"sound\\game_over.wav");
    m_pSound->load_WAVE_from_file(wav_key_input,"sound\\key_input.wav");
    //m_pSound->load_WAVE_from_file(wav_start_game,"sound\\start_game.wav");
    m_pSound->load_WAVE_from_file(wav_start_match,"sound\\start_match.wav");
    m_pSound->load_WAVE_from_file(wav_start_mission,"sound\\start_mission.wav");
    m_pSound->load_WAVE_from_file(wav_music_intro,"sound\\music_intro.wav");
    m_pSound->load_WAVE_from_file(wav_music_loop,"sound\\music_loop.wav");
    */

    //load files from text and decode
    m_pSound->load_WAVE_from_string(wav_abort_mission, base64_decode( load_base64_file(file_sound_abort_mission) ) );
    m_pSound->load_WAVE_from_string(wav_finish_mission, base64_decode( load_base64_file(file_sound_finish_mission) ) );
    m_pSound->load_WAVE_from_string(wav_game_over, base64_decode( load_base64_file(file_sound_game_over) ) );
    m_pSound->load_WAVE_from_string(wav_key_input, base64_decode( load_base64_file(file_sound_key_input) ) );
    m_pSound->load_WAVE_from_string(wav_start_match, base64_decode( load_base64_file(file_sound_start_match) ) );
    m_pSound->load_WAVE_from_string(wav_start_mission, base64_decode( load_base64_file(file_sound_start_mission) ) );
    m_pSound->load_WAVE_from_string(wav_music_intro, base64_decode( load_base64_file(file_sound_music_intro) ) );
    m_pSound->load_WAVE_from_string(wav_music_loop, base64_decode( load_base64_file(file_sound_music_loop) ) );

    int error_flag=m_pSound->get_error();
    if( error_flag!=0 )
    {
        cout<<"ERROR: Could not load sound: "<<error_flag<<endl;
        return false;
    }

    return true;
}
