#ifndef MENU_H
#define MENU_H

#include "definitions.h"
#include "decal.h"
#include "button.h"
#include "text_input_box.h"
#include "text_list.h"
#include <vector>

using namespace std;

class menu
{
    public:
        //Constructors
        menu();

        //Variables
        bool m_ready;

        //Functions
        bool   init(int window_width,int window_height,int texture_gameover,int texture_buttons,int texture_title,int texture_help[8],
                    int texture_font[3],float time,
                    bool* pKeys,bool* pMouse_buttons,int* pMouse_pos,sound* pSound);
        int    update(float time);
        bool   draw();
        bool   set_IP(string sIP);
        string get_IP(void);
        string get_player_name(void);
        bool   set_player_name(string name);
        int    get_limit_time(void);
        int    get_limit_tile(void);
        //string get_server_name(void);
        bool   info_missing(string type);
        bool   add_player_to_list(string name);
        bool   add_player_to_gameover_list(string name,int score);
        bool   remove_player_from_list(string name);
        bool   remove_all_players_from_list(void);
        bool   set_menu_state(int state);

    private:
        //Variables
        int             m_menu_state,m_help_curr;
        bool*           m_pKeys;
        bool*           m_pMouse_buttons;
        int*            m_pMouse_pos;
        bool            m_show_help;
        float           m_LMB_delay,m_last_time;
        decal           m_deTitle,m_deCred,m_deHelp[8],m_deGameover;
        button          m_buTest,m_buHelp,m_buHost,m_buJoin,m_buEdit,m_buExit,m_buNext,m_buBack,m_buEdit_sound,m_buEdit_music;
        text_input_box  m_tibPlayer_name,m_tibIP_number,m_tibLimit_time,m_tibLimit_tile;
        display         m_diVersion;
        text_list       m_tlLobby_players,m_tlPlayers,m_tlScore;
        sound*          m_pSound;
        vector<string>  m_vServers;//?

        //Functions
};

#endif // MENU_H
