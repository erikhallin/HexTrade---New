#ifndef CITY_H
#define CITY_H

#include <gl/gl.h>
#include <vector>
#include <iostream>
#include <stdlib.h>//rand

#include "tile.h"
#include "trade.h"
#include "definitions.h"
#include "sound.h"

using namespace std;

class city
{
    public:
        city();
        city(float pos_x_hex,float pos_y_hex,int player_owner,int id,vector<trade>& vec_trades,tile* arr_tiles,sound* pSound);

        int  update(float time,bool mouse_buttons[4],int selected_tile_hex[2],int selected_tile_type,bool city_owner,
                    int num_of_trades,int longest_trade_distance);
        bool draw(float zoom_level,int draw_limits[9][4]);
        bool get_center_hex_coordinates(int& x_hex,int& y_hex);
        bool add_tile_to_city(int pos_x_hex,int pos_y_hex);
        bool remove_city_tile(int pos_x_hex,int pos_y_hex);
        bool get_city_tiles_list(vector<st_coord_chararr>& city_tiles_list);
        bool get_new_city_expansion_info(int& city_old_hex_x,int& city_old_hex_y,int& city_new_hex_x,int& city_new_hex_y);
        bool get_new_expansion_route(vector<st_coord_route>& vec_expansion_route);
        bool set_new_expansion_route(vector<st_coord_route> vec_expansion_route);
        bool get_new_trade_route(vector<st_coord_route>& vec_trade_route);
        bool set_new_trade_route(vector<st_coord_route> vec_trade_route);
        int  get_city_id(void);
        int  get_city_size(void);
        int  get_city_owner_id(void);
        bool abort_trade_mission(void);
        bool abort_current_mission(void);
        bool get_starvation_coord(int& x_hex,int& y_hex);
        bool set_city_mode(int mode_pop,int mode_work);
        int  get_city_color(void);
        bool set_recalc_border_flag(bool flag);
        bool set_new_city_id(int new_id);
        int  get_new_city_id(void);
        bool reset_new_city_id(void);
        bool force_finish_new_city(void);

        //bool force_starvation(int& x_hex,int& y_hex);
        //bool force_idle_test(void);

    private:

        int   m_player_owner;
        float m_center_pos_x_pix,m_center_pos_y_pix;
        int   m_center_pos_x_hex,m_center_pos_y_hex;
        float m_color[3];
        int   m_city_mode_pop,m_city_mode_work,m_city_id,m_new_city_id;
        bool  m_recalc_border;//flag if border needs to be recalculated
        //bool  m_forced_idle_test;//next update return value will force direct idle test

        //time
        float m_lifetime,m_pop_adjust_timer,m_waiting_for_response_timer;

        float m_growth_progress,m_starve_progress,m_idle_progress,m_next_growth_time,m_next_starve_time;
        float m_mission_expansion_time,m_mission_expansion_progress,m_mission_trade_time,m_mission_trade_progress;

        vector<st_coord_chararr> m_vec_city_tiles;
        vector<st_coord_route>   m_vec_expansion_route;
        vector<st_coord_route>   m_vec_trade_route;
        vector<trade>*           m_pVec_trades_all;
        tile*                    m_p_arr_tiles;

        //info for World for new city expansion
        int m_city_old_hex_x,m_city_old_hex_y,m_city_new_hex_x,m_city_new_hex_y;
        //info for World for new trade route
        vector<st_coord_route> m_vec_new_trade_route;
        //info for World for starvation
        int m_starve_hex_x,m_starve_hex_y;

        sound* m_pSound;

        bool recalc_border(void);
        bool population_count(void);
        bool adjust_pop_count(void);
        int  get_city_max_size(int numof_trades,int longest_trade_distance);
        bool calc_city_center_tile(void);
        bool get_random_city_tile_furthest_from_center(int& x_hex,int& y_hex);
        bool make_path(vector<st_coord_route>& temp_path,int& first_direction_out,
                       int new_x,int new_y,int last_x,int last_y,int test_type,int x_box_shift,int y_box_shift);
};

#endif // CITY_H
