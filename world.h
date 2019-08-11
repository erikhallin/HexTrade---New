#ifndef WORLD_H
#define WORLD_H

#include <iostream>
#include <vector>
#include <stdlib.h>//rand
#include <ctime>
#include <fstream>//debug

#include "tile.h"
#include "city.h"
#include "trade.h"
#include "sound.h"

using namespace std;

class world
{
    public:
        world();

        bool init(int window_width,int window_height,int* pPlayer_id,bool* pKeys,int texture_tile,sound* pSound);
        bool init_menu(int window_width,int window_height,bool* pKeys,int texture_tile);
        bool update(float time,float zoom_level,float& eye_pos_x,float& eye_pos_y);
        bool update_menu(float time);
        bool draw(float& zoom_level,float& eye_pos_x,float& eye_pos_y);
        bool set_mouse_input(int xpos,int ypos,bool mouse_buttons[4]);
        bool init_game_world(int seed,int numof_players);
        bool init_game_world_menu(int seed,int numof_players);
        int  get_request_expansion(void);
        int  get_request_trade(void);
        bool reset_city_request(int city_id);
        bool accept_city_request_expansion(int city_id);
        bool accept_city_request_trade(int city_id);
        bool translate_route_to_char(vector<char>& vec_char_route,int city_id,int route_type,int start_pos[2]);
        bool translate_char_to_route_and_add_expansion(vector<char> vec_char_route,int city_id,int new_city_id,int start_pos[2]);
        bool translate_char_to_route_and_add_trade(vector<char> vec_char_route,int city_id,int start_pos[2]);
        bool translate_char_to_route_and_test_expansion(vector<char> vec_char_route,int city_id,int start_pos[2]);
        bool translate_char_to_route_and_test_trade(vector<char>& vec_char_route,int city_id,int start_pos[2]);
        bool is_tile_in_city(int city_id,int pos[2]);
        bool set_network_mode(int mode);
        bool get_city_growth(vector<st_coord_w_val>& vec_growth_coord);
        int  do_city_growth(int city_id,int pos[2]);
        bool get_city_starvation(vector<st_coord_w_val>& vec_starve_coord);
        bool do_city_starvation(int city_id,int pos[2]);
        int  get_free_id_city(void);
        bool get_start_pos_pix(int pos_pix[2]);
        bool get_city_tile_count_result(vector<int>& vec_tile_count);

    private:

        ofstream outs;//temp

        //Window
        int   m_window_width,m_window_height;

        int   m_network_mode;
        int   m_seed;
        int   m_request_city_expansion,m_request_city_trade;
        int   m_start_pos_pix[2];

        int*  m_pPlayer_color_id;
        bool* m_pKeys;
        int   m_mouse_pos[2];
        bool  m_mouse_buttons[4];

        bool  m_mouse_drag_active,m_mouse_drag_ignore;
        int   m_mouse_pos_last[2],m_selected_tile_hex_last[2];

        int   m_texture_tile;

        sound* m_pSound;

        //id counters
        int m_id_counter_city,m_id_counter_trade;

        float m_time_counter;

        tile m_arr_tiles[_world_size_x][_world_size_y];
        vector<city>           m_vec_cities;
        vector<trade>          m_vec_trades;
        vector<st_coord_w_val> m_vec_city_growth_coord;
        vector<st_coord_w_val> m_vec_city_starve_coord;

        bool create_world(void);
        bool create_world_menu(void);
        bool merge_cities(int city_index_a,int city_index_b);
        bool takeover_city_test(int city_index_a,int city_index_b);
        bool remove_city(int city_index);
        bool place_start_cities(int numof_players);
        bool find_city_growth_place(int city_id,int pos[2]);


};

#endif // WORLD_H
