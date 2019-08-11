#ifndef GAME_H
#define GAME_H

#include <SOIL/SOIL.h>
#include <gl/gl.h>
#include <iostream>
#include <time.h>
#include <string>
#include <sstream>
#include <fstream>

#include "world.h"
#include "menu.h"
#include "networkCom.h"
#include "sound.h"
#include "decal.h"
#include "files_in_text.h"
#include "base64.h"

#define _key_esc_delay 0.2
#define _intro_shade_delay 20.0

using namespace std;

class game
{
    public:
        game(int width,int height,networkCom* pNetCom);

        int cycle(void);

        //Input Functions
        bool set_mouse_pos(int xpos,int ypos);
        bool set_mouse_button_left(bool status);
        bool set_mouse_button_right(bool status);
        bool set_mouse_scroll_up(bool status);
        bool set_mouse_scroll_down(bool status);
        bool set_keyboard_key(int key_id,bool status);
        bool set_debug_mode(bool status);
        bool set_server_ip(string server_ip);
        string get_server_ip(void);
        bool set_check_for_broadcast_flag(bool flag);
        bool get_check_for_broadcast_flag(void);
        bool set_check_for_broadcast_reply_flag(bool flag);
        bool get_check_for_broadcast_reply_flag(void);
        bool recv_data(SOCKET soc_sender);
        bool send_start_package_to_client(SOCKET soc_client);
        bool send_start_package_to_server(void);
        bool send_client_denied_package(SOCKET soc_client);
        bool add_server_player(void);
        bool lost_player(int socket);
        bool lost_server(void);
        bool set_multisend(bool flag);

    private:

        //Window
        int m_window_width,m_window_height;

        //Time
        float m_time_this_cycle,m_time_last_cycle,m_time_at_start,m_game_over_anim_time;

        //Eye
        float m_eye_pos_x,m_eye_pos_y,m_eye_pos_z;//z = zoom

        //Map
        int   m_seed;
        world m_World;
        world m_World_menu;

        menu  m_Menu;

        //Basic Functions
        bool init(void);
        int  update(void);
        bool draw(void);
        bool key_check_ingame(void);
        bool key_check_menu(void);
        bool game_over_test(void);
        bool set_game_over(vector<int> vec_tile_count);

        //Input Variables
        float m_key_delay;
        bool  m_debug_mode;
        bool  m_keys[256];
        int   m_mouse_pos[2];//x,y
        bool  m_mouse_button[4];//left,right,scroll up,scroll down

        //Texture
        int  m_texture_title,m_texture_gameover,m_texture_buttons,m_texture_help[8],m_texture_tile;
        int  m_texture_font[3];
        bool load_textures(void);

        //Sound
        bool   m_intro_done,m_music_intro_not_played;
        int    m_music_source;
        sound* m_pSound;
        bool load_sound(void);

        //Network
        networkCom* m_pNetCom;
        string m_server_ip;
        bool   m_check_for_broadcast_flag,m_check_for_broadcast_reply_flag,m_server_multisend;
        int    m_broadcast_reply_check_counter,m_packet_id_counter;
        float  m_send_packet_delay;
        vector<net_packet>     m_vec_packets_to_send;
        vector<net_packet>     m_vec_replies_to_send;
        vector<st_player>      m_vec_players;
        vector<string>         m_vec_players_at_start;//index is that players col id (used for city ownership)
        vector<st_id_and_time> m_vec_processed_packets;
        bool   interpret_data(char* data_array,SOCKET soc_sender);

        //Misc
        bool  m_show_quitscreen,m_exit_now;
        decal m_de_quitscreen;
        int   m_player_id,m_player_color_id;
        int   m_game_state,m_game_limit_time,m_game_limit_tile;
        bool  test_route_expansion(vector<char> vec_char_route,int city_id,int start_pos[2]);
        bool  test_route_trade(vector<char>& vec_char_route,int city_id,int start_pos[2]);
        int   m_error_flag;
        int   m_fps_frame_counter;
        float m_fps_time_last_measutment,m_fps;

        //temp
        //ofstream ofs_log;
};

#endif // GAME_H
