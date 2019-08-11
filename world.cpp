#include "world.h"

world::world()
{
    //ctor
}

bool world::init(int window_width,int window_height,int* pPlayer_color_id,bool* pKeys,int texture_tile,sound* pSound)
{
    m_pPlayer_color_id=pPlayer_color_id;
    m_pKeys=pKeys;
    m_pSound=pSound;

    m_texture_tile=texture_tile;

    m_id_counter_city=m_id_counter_trade=100;

    m_window_width=window_width;
    m_window_height=window_height;

    m_mouse_drag_active=m_mouse_drag_ignore=false;
    m_network_mode=0;

    m_time_counter=0;
    m_request_city_expansion=m_request_city_trade=-1;

    m_start_pos_pix[0]=m_start_pos_pix[1]=0;
    m_selected_tile_hex_last[0]=m_selected_tile_hex_last[1]=0;

    //TEMP
    //outs.open("templog.txt");
    //XXX

    return true;
}

bool world::init_menu(int window_width,int window_height,bool* pKeys,int texture_tile)
{
    m_pPlayer_color_id=new int;
    *m_pPlayer_color_id=0;

    m_pKeys=pKeys;

    m_texture_tile=texture_tile;

    m_id_counter_city=m_id_counter_trade=100;
    create_world_menu();

    m_window_width=window_width;
    m_window_height=window_height;

    m_mouse_drag_active=m_mouse_drag_ignore=false;
    m_network_mode=0;

    m_time_counter=0;
    m_request_city_expansion=m_request_city_trade=-1;

    return true;
}

bool world::update(float time,float zoom_level,float& eye_pos_x,float& eye_pos_y)
{
    m_time_counter+=time;
    //update tiles
    for(int x=0;x<_world_size_x;x++)
    for(int y=0;y<_world_size_y;y++)
    {
        m_arr_tiles[x][y].update(m_time_counter);
    }

    //find mouse pos in world
    int real_mouse_pos_x=-eye_pos_x+(float)m_mouse_pos[0];//in pix
    int real_mouse_pos_y=-eye_pos_y+(float)m_mouse_pos[1];//in pix
    //cout<<real_mouse_pos_x/zoom_level<<", "<<real_mouse_pos_y/zoom_level/0.75<<endl;

    //highlight tile
    int selected_tile_hex[2]={-1,-1};
    int selected_tile_type=-1;
    /*int box_num=-1;
    for(int x=0;x<_world_size_x;x++)
    for(int y=0;y<_world_size_y;y++)
    {
        if(box_num!=-1) break;//already found tile
        if(m_mouse_buttons[0]) box_num=m_arr_tiles[x][y].selection_test(zoom_level,real_mouse_pos_x,real_mouse_pos_y);//TEMP
        if( box_num!=-1 )//found tile
        {
            m_arr_tiles[x][y].set_color(1,0,0);
            selected_tile_hex[0]=x;
            selected_tile_hex[1]=y;
            selected_tile_type=m_arr_tiles[x][y].get_tile_type();
            //break;
        }
    }*/

    //mouse pos to hex alternative
    float rel_mouse_pos_x=(float)real_mouse_pos_x/(float)zoom_level;
    float rel_mouse_pos_y=(float)real_mouse_pos_y/(float)zoom_level;
    //convert to hex
    float y_hex_float=rel_mouse_pos_y/0.75;
    float x_hex_float=rel_mouse_pos_x-y_hex_float/2.0;
    //cout<<x_hex_float+0.5<<", "<<y_hex_float+0.5<<endl;
    int x_hex_int=int(x_hex_float+0.5);
    int y_hex_int=int(y_hex_float+0.5);
    //float rest_x=x_hex_float+0.5-int(x_hex_float+0.5);
    //float rest_y=y_hex_float+0.5-int(y_hex_float+0.5);
    //if(rest_x>0.75 && rest_y>0.75) x_hex_int++;
    //if(rest_x<0.25 && rest_y<0.25) x_hex_int--;

    //cout<<"REST: "<<rest_x<<", "<<rest_y<<endl;
    //set color
    if(x_hex_int<0 || x_hex_int>=_world_size_x || y_hex_int<0 || y_hex_int>=_world_size_y)
    {
        //cout<<"ERROR: Marked hex coutide world: "<<x_hex_int<<", "<<y_hex_int<<endl;
        //adjust values
        while(x_hex_int<0) x_hex_int+=_world_size_x;
        while(y_hex_int<0) y_hex_int+=_world_size_y;
        while(x_hex_int>=_world_size_x) x_hex_int-=_world_size_x;
        while(y_hex_int>=_world_size_y) y_hex_int-=_world_size_y;
    }
    //else
    {
        //one more test, check specific tiles, and next to
        bool marked_tile_found=false;
        if(m_arr_tiles[x_hex_int][y_hex_int].selection_test_spe_box(zoom_level,real_mouse_pos_x,real_mouse_pos_y) )
        {
            marked_tile_found=true;
            selected_tile_hex[0]=x_hex_int;
            selected_tile_hex[1]=y_hex_int;
            selected_tile_type=m_arr_tiles[x_hex_int][y_hex_int].get_tile_type();
        }
        else//check next tiles
        {
            for(int x_add=-1;x_add<2;x_add++)
            for(int y_add=-1;y_add<2;y_add++)
            {
                if(marked_tile_found) break;
                if(x_add==y_add) continue;//skip center and 2 extra
                int x_temp=x_hex_int+x_add;
                int y_temp=y_hex_int+y_add;
                if(x_temp<0) x_temp+=_world_size_x;
                if(y_temp<0) y_temp+=_world_size_y;
                if(x_temp>=_world_size_x) x_temp-=_world_size_x;
                if(y_temp>=_world_size_y) y_temp-=_world_size_y;
                //test
                if( m_arr_tiles[x_temp][y_temp].selection_test_spe_box(zoom_level,real_mouse_pos_x,real_mouse_pos_y) )
                {
                    marked_tile_found=true;
                    selected_tile_hex[0]=x_temp;
                    selected_tile_hex[1]=y_temp;
                    selected_tile_type=m_arr_tiles[x_temp][y_temp].get_tile_type();
                    break;
                }
            }
            if(!marked_tile_found)
            {//selected tile not found, pick center
                cout<<"Error: Problem selecting tile, use last selection\n";
                cout<<" Mouse pos: "<<real_mouse_pos_x<<", "<<real_mouse_pos_y<<endl;
                cout<<" hex   pos: "<<x_hex_int<<", "<<y_hex_int<<endl;
                //use last selected tile pos
                selected_tile_hex[0]=m_selected_tile_hex_last[0];
                selected_tile_hex[1]=m_selected_tile_hex_last[1];
                selected_tile_type=m_arr_tiles[ selected_tile_hex[0] ][ selected_tile_hex[1] ].get_tile_type();
            }
        }
    }
    //remember last tile pos
    m_selected_tile_hex_last[0]=selected_tile_hex[0];
    m_selected_tile_hex_last[1]=selected_tile_hex[1];
    /*
    else
    {
        //one more test, check distance
        int x_opt=0;
        int y_opt=0;
        float min_dist=m_arr_tiles[x_hex_int][y_hex_int].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        //check left
        int x_temp=x_hex_int;
        int y_temp=y_hex_int;
        if(x_hex_int-1<0) x_temp+=_world_size_x;
        float new_dist=m_arr_tiles[x_temp-1][y_temp].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=-1; y_opt=0;
        }
        //check right
        x_temp=x_hex_int;
        y_temp=y_hex_int;
        if(x_hex_int+1>=_world_size_x) x_temp-=_world_size_x;
        new_dist=m_arr_tiles[x_temp+1][y_temp].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=1; y_opt=0;
        }
        //check top left
        x_temp=x_hex_int;
        y_temp=y_hex_int;
        if(y_hex_int-1<0) y_temp+=_world_size_y;
        new_dist=m_arr_tiles[x_temp][y_temp-1].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=0; y_opt=-1;
        }
        //check down right
        x_temp=x_hex_int;
        y_temp=y_hex_int;
        if(y_hex_int+1>=_world_size_y) y_temp-=_world_size_y;
        new_dist=m_arr_tiles[x_temp][y_temp+1].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=0; y_opt=1;
        }
        //check top right
        x_temp=x_hex_int;
        y_temp=y_hex_int;
        if(x_hex_int+1>=_world_size_x) x_temp-=_world_size_x;
        if(y_hex_int-1<0) y_temp+=_world_size_y;
        new_dist=m_arr_tiles[x_temp+1][y_temp-1].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=1; y_opt=-1;
        }
        //check down left
        x_temp=x_hex_int;
        y_temp=y_hex_int;
        if(x_hex_int-1<0) x_temp+=_world_size_x;
        if(y_hex_int+1>=_world_size_x) y_temp-=_world_size_y;
        new_dist=m_arr_tiles[x_temp-1][y_temp+1].get_distance(zoom_level,real_mouse_pos_x,real_mouse_pos_y);
        if( new_dist<min_dist )
        {
            min_dist=new_dist;
            x_opt=-1; y_opt=1;
        }
        //adjust for opt pos
        x_temp=x_hex_int+x_opt;
        y_temp=y_hex_int+y_opt;
        if(x_temp<0) x_temp+=_world_size_x;
        if(x_temp>=_world_size_x) x_temp-=_world_size_x;
        if(y_temp<0) y_temp+=_world_size_y;
        if(y_temp>=_world_size_y) y_temp-=_world_size_y;

        //cout<<"Marked hex inside world: "<<x_hex_int<<", "<<y_hex_int<<endl;
        //m_arr_tiles[x_hex_int][y_hex_int].set_color(0,0,1);

        selected_tile_hex[0]=x_temp;
        selected_tile_hex[1]=y_temp;
        selected_tile_type=m_arr_tiles[x_hex_int][y_hex_int].get_tile_type();
    }*/


    //mouse drag test
    if(m_mouse_buttons[0])
    {
        if(m_mouse_drag_active && !m_mouse_drag_ignore)
        {//move cam depending on last mouse pos
            eye_pos_x-=float(m_mouse_pos_last[0]-m_mouse_pos[0]);
            eye_pos_y-=float(m_mouse_pos_last[1]-m_mouse_pos[1]);
            m_mouse_pos_last[0]=m_mouse_pos[0];
            m_mouse_pos_last[1]=m_mouse_pos[1];
        }
        else
        {//activate mouse drag
            m_mouse_pos_last[0]=m_mouse_pos[0];
            m_mouse_pos_last[1]=m_mouse_pos[1];
            m_mouse_drag_active=true;
            //test is city was selected, ignore drag untill LMB is released
            if(selected_tile_type==tt_land_w_city) m_mouse_drag_ignore=true;
        }
    }
    else //reset mouse drag
    {
        m_mouse_drag_active=false;
        m_mouse_drag_ignore=false;
    }

    /*//highlight tile
    int selected_tile_hex2[2]={-1,-1};
    //int selected_tile_type=-1;
    selected_tile_hex2[0]=(real_mouse_pos_x+(real_mouse_pos_y)/2.0)/zoom_level+0.5;
    selected_tile_hex2[1]=(real_mouse_pos_y)/0.75/zoom_level-0.5;

    selected_tile_hex2[0]=(real_mouse_pos_x+zoom_level/2.0+(real_mouse_pos_y+zoom_level/2.0)/2.0)/zoom_level;
    selected_tile_hex2[1]=(real_mouse_pos_y+zoom_level/1.5)/0.75/zoom_level;

    cout<<"X: "<<selected_tile_hex2[0]<<" Y: "<<selected_tile_hex2[1]<<endl;*/

    //abort current city mission (space)
    if(m_pKeys[32])
    {
        //if city selected
        if(selected_tile_type==tt_land_w_city)
        {
            //if your city
            if( m_arr_tiles[ selected_tile_hex[0] ][ selected_tile_hex[1] ].get_city_owner_id()==*m_pPlayer_color_id )
            {
                //get city id
                int city_id=m_arr_tiles[ selected_tile_hex[0] ][ selected_tile_hex[1] ].get_city_id();
                //get city index
                int city_index=-1;
                for(int i=0;i<(int)m_vec_cities.size();i++)
                {
                    if( m_vec_cities[i].get_city_id()==city_id )
                    {
                        city_index=i;
                        break;
                    }
                }
                if(city_index==-1) cout<<"ERROR: Mission abort: could not find selected city\n";
                else//abort city mission
                {
                    cout<<"Mission abort: Aborting current mission in city: "<<city_index<<endl;
                    if( m_vec_cities[city_index].abort_current_mission() )
                    {
                        //play sound
                        m_pSound->playSimpleSound(wav_abort_mission,1.0);
                    }
                }
            }
        }
    }

    //TEMP
    /*if(m_pKeys[78]) //n
    {
        if( selected_tile_hex[0]>=0 && selected_tile_hex[0]<_world_size_x &&
            selected_tile_hex[1]>=0 && selected_tile_hex[1]<_world_size_y )
        if( m_arr_tiles[ selected_tile_hex[0] ][ selected_tile_hex[1] ].is_buildable() )
        {
            int city_id=m_id_counter_city++;
            m_vec_cities.push_back( city(selected_tile_hex[0],selected_tile_hex[1],*m_pPlayer_color_id,city_id,m_vec_trades) );
            m_arr_tiles[ selected_tile_hex[0] ][ selected_tile_hex[1] ].set_owner( *m_pPlayer_color_id,city_id );
        }

    }*/
    //TEMP

    //update cities
    m_vec_city_growth_coord.clear();//clear vector of last cycle's values
    m_vec_city_starve_coord.clear();//clear vector of last cycle's values
    for(int i=0;i<(int)m_vec_cities.size();i++)
    {
        //num of trades test
        int num_of_trades=0;
        int longest_dist=0;
        for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
        {
            if( m_vec_cities[i].get_city_id()==m_vec_trades[trade_i].get_city_a_id() ||
                m_vec_cities[i].get_city_id()==m_vec_trades[trade_i].get_city_b_id() )
            {
                num_of_trades++;
                //test dist
                if( m_vec_trades[trade_i].get_trade_distance() > longest_dist ) longest_dist=m_vec_trades[trade_i].get_trade_distance();
            }
        }

        //ownership test, input possible
        bool owner_of_city=false;
        if(m_vec_cities[i].get_city_owner_id()==*m_pPlayer_color_id) owner_of_city=true;
        //update city
        int status=m_vec_cities[i].update(time,m_mouse_buttons,selected_tile_hex,selected_tile_type,owner_of_city,num_of_trades,longest_dist);
        switch(status)
        {
            case 0://
            {

            }break;

            case 1://city growth
            {
                switch(m_network_mode)
                {
                    case 0://test
                    {
                        //cout<<"City growth: Test mode\n";
                        int pos[2];
                        int city_id=m_vec_cities[i].get_city_id();
                        if( find_city_growth_place( city_id,pos ) )
                        {//growth pos available
                            if( do_city_growth(city_id,pos)==2)//merge of two cities
                            {//test to avoid other city update skipped or same checked twice
                                //test if this city was absorbed
                                bool city_id_found=false;
                                for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
                                {
                                    if( m_vec_cities[city_i].get_city_id()==city_id )
                                    {//this city was not absorbed
                                        city_id_found=true;
                                        //test if this city have the same place in the vec after removal of the other city
                                        if( m_vec_cities[city_i].get_city_id()!=m_vec_cities[i].get_city_id() )
                                        {//if not, i--
                                            i--;
                                        }
                                        break;
                                    }
                                }
                                if(!city_id_found) i--;//otherwise one city will skipped due to city removal in merge
                            }
                        }
                    }break;

                    case 1://server
                    {
                        //cout<<"City growth: Server mode\n";
                        //report to game
                        int pos[2];
                        int city_id=m_vec_cities[i].get_city_id();
                        if( find_city_growth_place( city_id,pos ) )
                        {//growth pos available
                            if( do_city_growth(city_id,pos) )//merge of two cities
                            {
                                //if this city was absorbed, then i will point to the next city instead. Test required
                                //test if this city was absorbed
                                bool city_id_found=false;
                                for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
                                {
                                    if( m_vec_cities[city_i].get_city_id()==city_id )
                                    {//this city was not absorbed
                                        city_id_found=true;
                                        //test if this city have the same place in the vec after removal of the other city
                                        if( m_vec_cities[city_i].get_city_id()!=m_vec_cities[i].get_city_id() )
                                        {//if not, i--
                                            i--;
                                        }
                                        break;
                                    }
                                }
                                if(!city_id_found) i--;//otherwise one city will skipped due to city removal in merge

                                //store event
                                m_vec_city_growth_coord.push_back( st_coord_w_val(pos[0],pos[1],city_id) );
                            }
                            else//no skipping
                            {
                                //store event
                                m_vec_city_growth_coord.push_back( st_coord_w_val(pos[0],pos[1],city_id) );
                            }
                        }
                    }break;

                    case 2://client
                    {
                        //cout<<"City growth: Client mode\n";
                        //do nothing
                    }break;
                }

            }break;

            case 2://new city built
            {
                //get hex of new city and old tile
                int city_old_hex_x,city_old_hex_y,city_new_hex_x,city_new_hex_y;
                m_vec_cities[i].get_new_city_expansion_info(city_old_hex_x,city_old_hex_y,city_new_hex_x,city_new_hex_y);

                //test if new city location is still valid
                if( !m_arr_tiles[ city_new_hex_x ][ city_new_hex_y ].is_buildable() ) break;//not buildable anymore

                //remove old city tile
                m_vec_cities[i].remove_city_tile(city_old_hex_x,city_old_hex_y);

                //trade check for that tile
                for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                {
                    if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[i].get_city_id() ||
                        m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[i].get_city_id() )
                    {//test if that tile is involved
                        vector<st_coord_route> vec_trade_route;
                        m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
                        int trade_id=m_vec_trades[trade_i].get_trade_id();
                        for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
                        {
                            if( vec_trade_route[tile_i].x_hex==city_old_hex_x &&
                                vec_trade_route[tile_i].y_hex==city_old_hex_y )
                            {//this tile is involved in that trade, remove trade
                                //inform tiles
                                for(int tile_i2=1;tile_i2<(int)vec_trade_route.size()-1;tile_i2++)
                                {
                                    int temp_tt=m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].get_tile_type();
                                    if(temp_tt==tt_land_w_road)
                                        m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_land,trade_id);
                                    if(temp_tt==tt_water_w_road)
                                        m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_water,trade_id);
                                }
                                //remove trade
                                m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                                trade_i--;
                                break;
                            }
                        }
                    }
                }



                //tell that tile that there is no city anymore
                m_arr_tiles[city_old_hex_x][city_old_hex_y].set_tile_type(tt_land,-1);

                //build new city
                int new_city_id=m_vec_cities[i].get_new_city_id();
                m_vec_cities[i].reset_new_city_id();
                int city_color=m_vec_cities[i].get_city_color();//get player color
                m_vec_cities.push_back( city(city_new_hex_x,city_new_hex_y,city_color,new_city_id,m_vec_trades,&m_arr_tiles[0][0],m_pSound) );
                cout<<"Expansion: New city built at:  X: "<<city_new_hex_x<<"  Y: "<<city_new_hex_y<<endl;

                //tell that tile that there is a city
                m_arr_tiles[city_new_hex_x][city_new_hex_y].set_owner(city_color,new_city_id);

                /*//temp
                cout<<"\nCity print: "<<(int)m_vec_cities.size()<<endl;
                for(int x=0;x<(int)m_vec_cities.size();x++)
                {
                    cout<<m_vec_cities[x].get_city_id()<<", ";
                }
                cout<<endl;*/
                //xxx

                //test if old city have any tiles left
                if( m_vec_cities[i].get_city_size()<1 )
                {//city have no tiles, remove
                    //test if that city have any trades that needs to be removed to (probably not)
                    for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                    {
                        if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[i].get_city_id() ||
                            m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[i].get_city_id() )
                        {//remove this trade
                            //inform tiles
                            vector<st_coord_route> vec_trade_route;
                            m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
                            int trade_id=m_vec_trades[trade_i].get_trade_id();
                            for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)
                            {
                                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                                if(temp_tt==tt_land_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                                if(temp_tt==tt_water_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
                            }
                            //remove trade
                            m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                            trade_i--;
                        }
                    }
                    //remove city
                    remove_city(i);
                    //i is not correct anymore
                }

                //Test if any trade was affected
                //???


                //play sound
                if(owner_of_city) m_pSound->playSimpleSound(wav_finish_mission,1.0);


            }break;

            case 3://new trade route
            {



            }break;

            case 4://request to test new trade route
            {
                //get trade route
                vector<st_coord_route> vec_trade_route;
                m_vec_cities[i].get_new_trade_route(vec_trade_route);
                //get ids of both cities
                int city_id_a=m_arr_tiles[ vec_trade_route.front().x_hex ][ vec_trade_route.front().y_hex ].get_city_id();
                int city_id_b=m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_city_id();
                //check if trade already exists
                bool abort_trade=false;
                for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                {
                    if( (m_vec_trades[trade_i].get_city_a_id()==city_id_a && m_vec_trades[trade_i].get_city_b_id()==city_id_b) ||
                        (m_vec_trades[trade_i].get_city_a_id()==city_id_b && m_vec_trades[trade_i].get_city_b_id()==city_id_a) )
                    {
                        //highlight trade
                        m_vec_trades[trade_i].set_light_pulse(4.0);
                        //abort new trade
                        abort_trade=true;
                        break;
                    }
                }
                if(abort_trade)
                {
                    m_vec_cities[i].abort_trade_mission();
                }

            }break;

            case 5://starvation
            {
                //randomly pick a city tile to remove from city, that is far from the city center
                switch(m_network_mode)
                {
                    case 0://test
                    {
                        //cout<<"City starve: Test mode\n";
                        //do it
                        int pos[2];
                        m_vec_cities[i].get_starvation_coord(pos[0],pos[1]);
                        do_city_starvation( m_vec_cities[i].get_city_id(),pos );
                    }break;

                    case 1://server
                    {
                        //cout<<"City starve: Server mode\n";
                        //do it and store to send to clients
                        int pos[2];
                        m_vec_cities[i].get_starvation_coord(pos[0],pos[1]);
                        do_city_starvation( m_vec_cities[i].get_city_id(),pos );
                        m_vec_city_starve_coord.push_back( st_coord_w_val( pos[0],pos[1],m_vec_cities[i].get_city_id() ) );
                    }break;

                    case 2://client
                    {
                        //cout<<"City starve: Client mode\n";
                        //do nothing
                    }break;
                }
            }break;

            /*case 6://idle test
            {
                cout<<"Idle test\n";
                //test if city is locked (no land next to tiles)
                vector<st_coord_chararr> vec_city_tiles;
                m_vec_cities[i].get_city_tiles_list(vec_city_tiles);
                bool no_free_tiles=true;
                for(int tile_i=0;tile_i<(int)vec_city_tiles.size();tile_i++)
                {
                    for(int x_add=-1;x_add<2;x_add++)
                    for(int y_add=-1;y_add<2;y_add++)
                    {
                        if(x_add==y_add) continue;//avoid +1,+1 0,0 -1,-1
                        int new_x=vec_city_tiles[tile_i].x_hex+x_add;
                        int new_y=vec_city_tiles[tile_i].y_hex+y_add;
                        //border jump test
                        if(new_x<0 || new_x>=_world_size_x || new_y<0 || new_y>=_world_size_y)
                        {//adjust hex loop value
                            if(new_x<0) new_x+=_world_size_x;
                            if(new_x>=_world_size_x) new_x-=_world_size_x;
                            if(new_y<0) new_y+=_world_size_y;
                            if(new_y>=_world_size_y) new_y-=_world_size_y;
                        }

                        //test if tile next to this one is part of defender's city
                        if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land ||
                            m_arr_tiles[new_x][new_y].get_tile_type()==tt_land_w_road )
                        {
                            no_free_tiles=false;
                            break;
                        }
                    }
                }
                if(no_free_tiles)//force starvation, remove one tile
                {
                    int x_hex,y_hex;
                    m_vec_cities[i].force_starvation(x_hex,y_hex);
                    //tell city
                    m_vec_cities[i].remove_city_tile(x_hex,y_hex);
                    //tell tile
                    m_arr_tiles[x_hex][y_hex].set_tile_type(tt_land,-1);
                    //test trades involved with this city
                    for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                    {
                        if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[i].get_city_id() ||
                            m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[i].get_city_id() )
                        {//this trade is involved
                            cout<<"Forced Starvation: City have a trade that needs testing\n";
                            //test these trades if that tile is involved
                            vector<st_coord_route> vec_trade_route;
                            m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
                            int trade_id=m_vec_trades[trade_i].get_trade_id();
                            for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
                            {
                                if( vec_trade_route[tile_i].x_hex==x_hex &&
                                    vec_trade_route[tile_i].y_hex==y_hex )
                                {//this trade route is affected, must be start or endpoint of route (a city tile)
                                    //trade must be removed
                                    cout<<"Forced Starvation: Removing a trade from this tile\n";
                                    //tell tiles
                                    for(int tile_i2=1;tile_i2<(int)vec_trade_route.size()-1;tile_i2++)//skip first and last, cities
                                    {
                                        int temp_tt=m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].get_tile_type();
                                        if(temp_tt==tt_land_w_road)
                                            m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_land,trade_id);
                                        if(temp_tt==tt_water_w_road)
                                            m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_water,trade_id);
                                    }
                                    //remove trade
                                    m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                                    trade_i--;//go back one step, otherwise one element will be missed
                                    break;
                                }
                            }
                        }
                    }
                }
            }break;*/

            case 7://requesting new expansion
            {
                //if many cities requests this the same world update?
                //impossible, needs user input for initialization, only one per cycle
                m_request_city_expansion=m_vec_cities[i].get_city_id();
            }break;

            case 8://requesting new trade
            {
                //if many cities requests this the same world update?
                //impossible, needs user input for initialization, only one per cycle
                m_request_city_trade=m_vec_cities[i].get_city_id();
            }break;

            case 10://mission started, play sound
            {
                m_pSound->playSimpleSound(wav_start_mission,0.5);
            }break;
        }
    }

    //update trades
    for(int i=0;i<(int)m_vec_trades.size();i++)
    {
        m_vec_trades[i].update(time);
    }

    return true;
}

bool world::update_menu(float time)
{
    m_time_counter+=time;
    //update tiles
    for(int x=0;x<_world_size_x;x++)
    for(int y=0;y<_world_size_y;y++)
    {
        m_arr_tiles[x][y].update(m_time_counter);
    }

    return true;
}

bool world::draw(float& zoom_level,float& eye_pos_x,float& eye_pos_y)
{
    //find view window
    int numof_hex_x=(float)m_window_width/zoom_level;
    int numof_hex_y=(float)m_window_height/zoom_level/0.75;
    //y
    int hex_y_min=-eye_pos_y/zoom_level/0.75;
    int hex_y_max=hex_y_min+numof_hex_y+4;//draw extra 2 hex coutide
    //x
    int hex_x_min=-eye_pos_x/zoom_level+eye_pos_y*0.5/zoom_level/0.75-(float)numof_hex_y*0.5-2.0;
    int hex_x_max=hex_x_min+numof_hex_x+(float)numof_hex_y*0.5+6;//draw extra 2 hex outside

    //coutide center map test, shift eye pos
    if(hex_x_max<0)
    {//shift right
        float x_screen_shift=_world_size_x*zoom_level;

        eye_pos_x-=x_screen_shift;

        //recalc limits
        //y
        hex_y_min=-eye_pos_y/zoom_level/0.75;
        hex_y_max=hex_y_min+numof_hex_y+2;//draw extra 2 hex coutide
        //x
        hex_x_min=-eye_pos_x/zoom_level+eye_pos_y*0.5/zoom_level/0.75-(float)numof_hex_y*0.5;
        hex_x_max=hex_x_min+numof_hex_x+(float)numof_hex_y*0.5+2;//draw extra 2 hex coutide
    }
    if(hex_x_min>_world_size_x)
    {//shift left
        float x_screen_shift=_world_size_x*zoom_level;

        eye_pos_x+=x_screen_shift;

        //recalc limits
        //y
        hex_y_min=-eye_pos_y/zoom_level/0.75;
        hex_y_max=hex_y_min+numof_hex_y+2;//draw extra 2 hex coutide
        //x
        hex_x_min=-eye_pos_x/zoom_level+eye_pos_y*0.5/zoom_level/0.75-(float)numof_hex_y*0.5;
        hex_x_max=hex_x_min+numof_hex_x+(float)numof_hex_y*0.5+2;//draw extra 2 hex coutide
    }
    if(hex_y_max<0)
    {//shift down
        float x_screen_shift=(0.0+_world_size_y/2.0)*zoom_level;
        float y_screen_shift=(_world_size_y*0.75)*zoom_level;

        eye_pos_x-=x_screen_shift;
        eye_pos_y-=y_screen_shift;

        //recalc limits
        //y
        hex_y_min=-eye_pos_y/zoom_level/0.75;
        hex_y_max=hex_y_min+numof_hex_y+2;//draw extra 2 hex coutide
        //x
        hex_x_min=-eye_pos_x/zoom_level+eye_pos_y*0.5/zoom_level/0.75-(float)numof_hex_y*0.5;
        hex_x_max=hex_x_min+numof_hex_x+(float)numof_hex_y*0.5+2;//draw extra 2 hex coutide
    }
    if(hex_y_min>_world_size_y)
    {//shift up
        float x_screen_shift=(0.0+_world_size_y/2.0)*zoom_level;
        float y_screen_shift=(_world_size_y*0.75)*zoom_level;

        eye_pos_x+=x_screen_shift;
        eye_pos_y+=y_screen_shift;

        //recalc limits
        //y
        hex_y_min=-eye_pos_y/zoom_level/0.75;
        hex_y_max=hex_y_min+numof_hex_y+2;//draw extra 2 hex coutide
        //x
        hex_x_min=-eye_pos_x/zoom_level+eye_pos_y*0.5/zoom_level/0.75-(float)numof_hex_y*0.5;
        hex_x_max=hex_x_min+numof_hex_x+(float)numof_hex_y*0.5+2;//draw extra 2 hex coutide
    }

    //world limits for the 9 boxes
    int draw_limits[9][4];

    //top left
    draw_limits[0][0]=hex_x_min+_world_size_x; if(draw_limits[0][0]<0) draw_limits[0][0]=0;
    draw_limits[0][1]=hex_x_max+_world_size_x; if(draw_limits[0][1]>_world_size_x) draw_limits[0][1]=_world_size_x;
    draw_limits[0][2]=hex_y_min+_world_size_y; if(draw_limits[0][2]<0) draw_limits[0][2]=0;
    draw_limits[0][3]=hex_y_max+_world_size_y; if(draw_limits[0][3]>_world_size_y) draw_limits[0][3]=_world_size_y;
    //left
    draw_limits[1][0]=hex_x_min+_world_size_x; if(draw_limits[1][0]<0) draw_limits[1][0]=0;
    draw_limits[1][1]=hex_x_max+_world_size_x; if(draw_limits[1][1]>_world_size_x) draw_limits[1][1]=_world_size_x;
    draw_limits[1][2]=hex_y_min; if(draw_limits[1][2]<0) draw_limits[1][2]=0;
    draw_limits[1][3]=hex_y_max; if(draw_limits[1][3]>_world_size_y) draw_limits[1][3]=_world_size_y;
    //down left
    draw_limits[2][0]=hex_x_min+_world_size_x; if(draw_limits[2][0]<0) draw_limits[2][0]=0;
    draw_limits[2][1]=hex_x_max+_world_size_x; if(draw_limits[2][1]>_world_size_x) draw_limits[2][1]=_world_size_x;
    draw_limits[2][2]=hex_y_min-_world_size_y; if(draw_limits[2][2]<0) draw_limits[2][2]=0;
    draw_limits[2][3]=hex_y_max-_world_size_y; if(draw_limits[2][3]>_world_size_y) draw_limits[2][3]=_world_size_y;
    //top
    draw_limits[3][0]=hex_x_min; if(draw_limits[3][0]<0) draw_limits[3][0]=0;
    draw_limits[3][1]=hex_x_max; if(draw_limits[3][1]>_world_size_x) draw_limits[3][1]=_world_size_x;
    draw_limits[3][2]=hex_y_min+_world_size_y; if(draw_limits[3][2]<0) draw_limits[3][2]=0;
    draw_limits[3][3]=hex_y_max+_world_size_y; if(draw_limits[3][3]>_world_size_y) draw_limits[3][3]=_world_size_y;
    //center
    draw_limits[4][0]=hex_x_min; if(draw_limits[4][0]<0) draw_limits[4][0]=0;
    draw_limits[4][1]=hex_x_max; if(draw_limits[4][1]>_world_size_x) draw_limits[4][1]=_world_size_x;
    draw_limits[4][2]=hex_y_min; if(draw_limits[4][2]<0) draw_limits[4][2]=0;
    draw_limits[4][3]=hex_y_max; if(draw_limits[4][3]>_world_size_y) draw_limits[4][3]=_world_size_y;
    //down
    draw_limits[5][0]=hex_x_min; if(draw_limits[5][0]<0) draw_limits[5][0]=0;
    draw_limits[5][1]=hex_x_max; if(draw_limits[5][1]>_world_size_x) draw_limits[5][1]=_world_size_x;
    draw_limits[5][2]=hex_y_min-_world_size_y; if(draw_limits[5][2]<0) draw_limits[5][2]=0;
    draw_limits[5][3]=hex_y_max-_world_size_y; if(draw_limits[5][3]>_world_size_y) draw_limits[5][3]=_world_size_y;
    //top right
    draw_limits[6][0]=hex_x_min-_world_size_x; if(draw_limits[6][0]<0) draw_limits[6][0]=0;
    draw_limits[6][1]=hex_x_max-_world_size_x; if(draw_limits[6][1]>_world_size_x) draw_limits[6][1]=_world_size_x;
    draw_limits[6][2]=hex_y_min+_world_size_y; if(draw_limits[6][2]<0) draw_limits[6][2]=0;
    draw_limits[6][3]=hex_y_max+_world_size_y; if(draw_limits[6][3]>_world_size_y) draw_limits[6][3]=_world_size_y;
    //right
    draw_limits[7][0]=hex_x_min-_world_size_x; if(draw_limits[7][0]<0) draw_limits[7][0]=0;
    draw_limits[7][1]=hex_x_max-_world_size_x; if(draw_limits[7][1]>_world_size_x) draw_limits[7][1]=_world_size_x;
    draw_limits[7][2]=hex_y_min; if(draw_limits[7][2]<0) draw_limits[7][2]=0;
    draw_limits[7][3]=hex_y_max; if(draw_limits[7][3]>_world_size_y) draw_limits[7][3]=_world_size_y;
    //down right
    draw_limits[8][0]=hex_x_min-_world_size_x; if(draw_limits[8][0]<0) draw_limits[8][0]=0;
    draw_limits[8][1]=hex_x_max-_world_size_x; if(draw_limits[8][1]>_world_size_x) draw_limits[8][1]=_world_size_x;
    draw_limits[8][2]=hex_y_min-_world_size_y; if(draw_limits[8][2]<0) draw_limits[8][2]=0;
    draw_limits[8][3]=hex_y_max-_world_size_y; if(draw_limits[8][3]>_world_size_y) draw_limits[8][3]=_world_size_y;



    glPushMatrix();
    //go to eye pos
    glTranslatef(eye_pos_x,eye_pos_y,0);

    /*//draw tiles, repeat 9 times TEST
    int tile_counter=0;
    int box_index=4;//from 0 - 8, top left to down right
    for(int x_shift=0;x_shift<1;x_shift++)
    for(int y_shift=0;y_shift<1;y_shift++)
    {
        for(int x=draw_limits[box_index][0];x<draw_limits[box_index][1];x++)
        for(int y=draw_limits[box_index][2];y<draw_limits[box_index][3];y++)
        {
            m_arr_tiles[x][y].draw(zoom_level,x_shift,y_shift);
            tile_counter++;
        }
        box_index++;
    }
    cout<<"Tiles drawn: "<<tile_counter<<endl;*/

    //draw tiles
    //set common settings for all tiles:
    glLineWidth(1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_texture_tile);
    //start drawing
    //int tile_conter=0;//TEMP
    int box_index=0;//from 0 - 8, top left to down right
    for(int x_shift=-1;x_shift<2;x_shift++)
    for(int y_shift=-1;y_shift<2;y_shift++)
    {
        for(int x=draw_limits[box_index][0];x<draw_limits[box_index][1];x++)
        for(int y=draw_limits[box_index][2];y<draw_limits[box_index][3];y++)
        {
            m_arr_tiles[x][y].draw(zoom_level,x_shift,y_shift);
            //tile_conter++;
        }
        box_index++;
    }
    //cout<<"Tiles drawn: "<<tile_conter<<endl;
    //disable settings for all tiles:
    glDisable(GL_TEXTURE_2D);

    /*//TEST only center box
    for(int x=draw_limits[4][0];x<draw_limits[4][1];x++)
    for(int y=draw_limits[4][2];y<draw_limits[4][3];y++)
    {
        m_arr_tiles[x][y].draw(zoom_level,0,0);
    }*/


    //draw cities
    for(int i=0;i<(int)m_vec_cities.size();i++)
    {
        m_vec_cities[i].draw(zoom_level,draw_limits);
    }

    //draw trade routes
    for(int i=0;i<(int)m_vec_trades.size();i++)
    {
        m_vec_trades[i].draw(zoom_level);
    }

    glPopMatrix();

    return true;
}

bool world::set_mouse_input(int xpos,int ypos,bool mouse_buttons[4])
{
    m_mouse_pos[0]=xpos;
    m_mouse_pos[1]=ypos;

    m_mouse_buttons[0]=mouse_buttons[0];
    m_mouse_buttons[1]=mouse_buttons[1];
    m_mouse_buttons[2]=mouse_buttons[2];
    m_mouse_buttons[3]=mouse_buttons[3];

    return true;
}

bool world::init_game_world(int seed,int numof_players)
{
    m_seed=seed;
    srand(seed);
    create_world();
    if( !place_start_cities(numof_players) )
    {//could not place cities
        //try one more time
        create_world();
        if( !place_start_cities(numof_players) )
        {
            cout<<"Error: Creating World: Could not place cities\n";
            return false;
        }
    }

    return true;
}

bool world::init_game_world_menu(int seed,int numof_players)
{
    cout<<"World Gen menu: Generating world with "<<numof_players<<" cities\n";
    m_seed=seed;
    srand(seed);
    create_world();
    //place cities, 3 owners
    vector<st_coord> vec_city_pos;
    int shortest_side=_world_size_x;
    if(_world_size_x>_world_size_y) shortest_side=_world_size_y;
    int min_distance=shortest_side/(numof_players+2);
    if(min_distance<1) min_distance=1;

    int numof_tries=10000;
    bool retry=false;

    for(int dist_step_i=0;dist_step_i<5;dist_step_i++)
    {
        for(int try_i=0;try_i<numof_tries;try_i++)
        {
            retry=false;
            vec_city_pos.clear();
            for(int player_i=0;player_i<numof_players;player_i++)
            {
                //place city at random pos
                int x_pos=rand()%_world_size_x;
                int y_pos=rand()%_world_size_y;
                st_coord temp_pos(x_pos,y_pos);

                //test if terrain is land
                if( m_arr_tiles[x_pos][y_pos].get_tile_type()!=tt_land )
                {
                    retry=true;
                }

                //test if distance to other cities is far enough
                for(int city_i=0;city_i<(int)vec_city_pos.size();city_i++)
                {
                    if( vec_city_pos[city_i].distance( temp_pos )<min_distance )
                    {
                        retry=true;
                        break;
                    }
                }
                if(retry) break;
                else vec_city_pos.push_back(temp_pos);//store pos
            }
            if(!retry) break;
        }

        if(retry)//decrese min distance
        {
            min_distance/=2;
            if(min_distance<1) min_distance=1;
        }
        else//done, assign cities pos
        {
            cout<<"World Gen menu: Placing start cities at: "<<endl;
            for(int player_i=0;player_i<numof_players;player_i++)
            {
                cout<<vec_city_pos[player_i].x_hex<<", "<<vec_city_pos[player_i].y_hex<<endl;
                int id=m_id_counter_city++;

                int owner=rand()%3;
                m_vec_cities.push_back( city( vec_city_pos[player_i].x_hex,vec_city_pos[player_i].y_hex ,
                                              owner,id,m_vec_trades,&m_arr_tiles[0][0],m_pSound) );

                m_arr_tiles[ vec_city_pos[player_i].x_hex ][ vec_city_pos[player_i].y_hex ].set_owner(player_i,id);
            }
            break;
        }
    }
    cout<<"World Gen menu: Placed cities: "<<(int)m_vec_cities.size()<<endl;
    if( (int)m_vec_cities.size()<numof_players )
    {
        cout<<"Error: World Gen menu: Could not place all cities\n";
        return false;
    }

    //expand cities
    cout<<"World Gen menu: Expanding cities\n";
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        int randval=rand()%20+1;
        for(int growth=0;growth<randval;growth++)
        {
            int pos[2];//growth pos
            int city_id=m_vec_cities[city_i].get_city_id();
            if( find_city_growth_place( city_id,pos ) )
            {//growth pos available
                do_city_growth(city_id,pos);
            }
        }
    }

    /*//create trades by random walk until hit another city, Did not look good!
    cout<<"World Gen menu: Creating trades\n";
    int numof_trades=50;
    for(int trade_try=0;trade_try<numof_trades;trade_try++)
    {
        //cout<<"Trade try: "<<trade_try<<endl;
        vector<st_coord_route> vec_trade_route;

        //pick city
        int city_index=rand()%(int)m_vec_cities.size();
        int start_city_id=m_vec_cities[city_index].get_city_id();
        //cout<<"World Gen menu: Picked city: "<<city_index<<", id: "<<start_city_id<<endl;
        //pick start tile
        vector<st_coord_chararr> vec_city_tiles;
        m_vec_cities[city_index].get_city_tiles_list(vec_city_tiles);
        int tile_index=rand()%(int)vec_city_tiles.size();
        //cout<<"World Gen menu: Picked tile: "<<tile_index<<" "<<vec_city_tiles[tile_index].x_hex<<", "<<vec_city_tiles[tile_index].y_hex<<endl;
        //get start pos
        int hex_x_start=vec_city_tiles[tile_index].x_hex;
        int hex_y_start=vec_city_tiles[tile_index].y_hex;
        int last_hex_x=hex_x_start;
        int last_hex_y=hex_y_start;
        //insert first tile in route, start city tile
        vec_trade_route.push_back( st_coord_route( hex_x_start,hex_y_start,0,0 ) );

        int max_steps=1000;
        bool route_finished=false;
        for(int step=0;step<max_steps;step++)
        {
            //cout<<"Route step: "<<step<<endl;
            //get latest direction
            int latest_dir=-1;
            if( (int)vec_trade_route.size()>1 )
            {
                latest_dir=vec_trade_route.back().road[0];
            }
            //calc allowed directions
            bool allowed_dir[6]={true,true,true,true,true,true};
            switch(latest_dir)
            {
                case 0:
                {
                    allowed_dir[0]=true; allowed_dir[1]=true; allowed_dir[2]=false;
                    allowed_dir[3]=false; allowed_dir[4]=false; allowed_dir[5]=true;
                }break;
                case 1:
                {
                    allowed_dir[0]=true; allowed_dir[1]=true; allowed_dir[2]=true;
                    allowed_dir[3]=false; allowed_dir[4]=false; allowed_dir[5]=false;
                }break;
                case 2:
                {
                    allowed_dir[0]=false; allowed_dir[1]=true; allowed_dir[2]=true;
                    allowed_dir[3]=true; allowed_dir[4]=false; allowed_dir[5]=false;
                }break;
                case 3:
                {
                    allowed_dir[0]=false; allowed_dir[1]=false; allowed_dir[2]=true;
                    allowed_dir[3]=true; allowed_dir[4]=true; allowed_dir[5]=false;
                }break;
                case 4:
                {
                    allowed_dir[0]=false; allowed_dir[1]=false; allowed_dir[2]=false;
                    allowed_dir[3]=true; allowed_dir[4]=true; allowed_dir[5]=true;
                }break;
                case 5:
                {
                    allowed_dir[0]=true; allowed_dir[1]=false; allowed_dir[2]=false;
                    allowed_dir[3]=false; allowed_dir[4]=true; allowed_dir[5]=true;
                }break;
            }

            //get possible directions
            vector<st_coord_route> vec_possible_tiles;
            for(int add_x=-1;add_x<2;add_x++)
            for(int add_y=-1;add_y<2;add_y++)
            {
                if(add_x==add_y) continue;

                int new_x_hex=last_hex_x+add_x;
                int new_y_hex=last_hex_y+add_y;
                int x_shift=0;
                int y_shift=0;
                if(new_x_hex<0)              {x_shift-=1;new_x_hex+=_world_size_x;}
                if(new_x_hex>=_world_size_x) {x_shift+=1;new_x_hex-=_world_size_x;}
                if(new_y_hex<0)              {y_shift-=1;new_y_hex+=_world_size_y;}
                if(new_y_hex>=_world_size_y) {y_shift+=1;new_y_hex-=_world_size_y;}

                //check if direction is allowed
                int new_direction=-1;
                if(add_x==-1 && add_y==0)  new_direction=4;
                if(add_x==-1 && add_y==1)  new_direction=3;
                if(add_x==0  && add_y==-1) new_direction=5;
                if(add_x==0  && add_y==1)  new_direction=2;
                if(add_x==1  && add_y==-1) new_direction=0;
                if(add_x==1  && add_y==0)  new_direction=1;
                if( !allowed_dir[new_direction] ) continue;

                //check if tile already in route
                for( int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++ )
                 if( vec_trade_route[tile_i].x_hex==new_x_hex && vec_trade_route[tile_i].y_hex==new_y_hex )
                {//tile already in planed route
                    continue;
                }
                //check if terrain allows
                if( m_arr_tiles[new_x_hex][new_y_hex].get_tile_type()==tt_land ||
                    m_arr_tiles[new_x_hex][new_y_hex].get_tile_type()==tt_water )
                {
                    //land buildable, add to possibility list
                    vec_possible_tiles.push_back( st_coord_route(new_x_hex,new_y_hex,x_shift,y_shift) );
                    continue;
                }
                *//*else if( m_arr_tiles[new_x_hex][new_y_hex].get_tile_type()==tt_land_w_road ||
                         m_arr_tiles[new_x_hex][new_y_hex].get_tile_type()==tt_water_w_road )
                {
                    //check if roads allow (NOT TESTED RIGHT NOW)
                    //get that (those) trade roads


                    //if yes, add to possibility list
                }*//*
                else if( m_arr_tiles[new_x_hex][new_y_hex].get_tile_type()==tt_land_w_city )
                {
                    //test if same city
                    if( m_arr_tiles[new_x_hex][new_y_hex].get_city_id()!=start_city_id )
                    {//if no, trade complete
                        vec_possible_tiles.push_back( st_coord_route(new_x_hex,new_y_hex,x_shift,y_shift) );
                        continue;
                    }
                }

                //tile not buildable, do not add

            }
            //if no directions allowed, restart
            if( vec_possible_tiles.empty() )
            {
                cout<<"World Gen menu: Route hit dead end\n";
                break;//end route
            }
            //pick one direction
            int step_index=rand()%((int)vec_possible_tiles.size());
            //translate direction
            int new_pos_x=vec_possible_tiles[step_index].x_hex+vec_possible_tiles[step_index].x_box_shift*_world_size_x;
            int new_pos_y=vec_possible_tiles[step_index].y_hex+vec_possible_tiles[step_index].y_box_shift*_world_size_y;
            int x_dif=new_pos_x-last_hex_x;
            int y_dif=new_pos_y-last_hex_y;
            last_hex_x=new_pos_x;
            last_hex_y=new_pos_y;
            int last_tile_road=-1;
            int new_tile_road=-1;
            //get road dir
            switch(x_dif)
            {
                case -1:
                {
                    switch(y_dif)
                    {
                        case  0: last_tile_road=1; new_tile_road=4; break;
                        case  1: last_tile_road=0; new_tile_road=3; break;
                    }
                }break;
                case  0:
                {
                    switch(y_dif)
                    {
                        case -1: last_tile_road=2; new_tile_road=5; break;
                        case  1: last_tile_road=5; new_tile_road=2; break;
                    }
                }break;
                case  1:
                {
                    switch(y_dif)
                    {
                        case -1: last_tile_road=3; new_tile_road=0; break;
                        case  0: last_tile_road=4; new_tile_road=1; break;
                    }
                }break;
            }
            //add tile to route and update road
            vec_trade_route.back().road[1]=last_tile_road;
            vec_trade_route.push_back( vec_possible_tiles[step_index] );
            vec_trade_route.back().road[0]=new_tile_road;

            //end test, if last tile is a new city
            if( m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_tile_type()==tt_land_w_city )
            {
                //test if same city
                if( m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_city_id()!=start_city_id )
                {
                    //trade complete
                    cout<<"World Gen menu: Trade route ready\n";
                    route_finished=true;
                    break;
                }
            }
        }

        //finish trade, if ok
        if( !route_finished ) continue;//end route

        //get ids of both cities
        int city_id_a=m_arr_tiles[ vec_trade_route.front().x_hex ][ vec_trade_route.front().y_hex ].get_city_id();
        int city_id_b=m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_city_id();

        int trade_id=m_id_counter_trade++;
        m_vec_trades.push_back( trade(vec_trade_route,trade_id,city_id_a,city_id_b) );
        //tell tiles on route
        for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, cities
        {//add road to land or water, if already road there, add trade id
            int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
            if(temp_tt==tt_land || temp_tt==tt_land_w_road)
                m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land_w_road,trade_id);
            if(temp_tt==tt_water || temp_tt==tt_water_w_road)
                m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water_w_road,trade_id);
        }
    }*/

    cout<<"World Gen menu: World Complete\n";

    return true;
}

int world::get_request_expansion(void)
{
    int ret_val=m_request_city_expansion;
    m_request_city_expansion=-1;//send only once
    return ret_val;
}

int world::get_request_trade(void)
{
    int ret_val=m_request_city_trade;
    m_request_city_trade=-1;//send only once
    return ret_val;
}

bool world::reset_city_request(int city_id)
{
    //find city
    int city_index=-1;
    for(int city_i=0;(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Reset city request: Could not find city\n";
        return false;
    }

    m_vec_cities[city_index].set_city_mode(-1,cm_idle);

    return true;
}

bool world::accept_city_request_expansion(int city_id)
{
    //find city
    int city_index=-1;
    for(int city_i=0;(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Accept city request: Could not find city\n";
        return false;
    }

    m_vec_cities[city_index].set_city_mode(-1,cm_expansion_mission);

    return true;
}

bool world::accept_city_request_trade(int city_id)
{
    //find city
    int city_index=-1;
    for(int city_i=0;(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Accept city request: Could not find city\n";
        return false;
    }

    m_vec_cities[city_index].set_city_mode(-1,cm_idle);//the trade is done in world/game, releave city from waiting for response mode

    return true;
}

bool world::translate_route_to_char(vector<char>& vec_char_route,int city_id,int route_type,int start_pos[2])
{
    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Route translation to char: Could not find city\n";
        return false;
    }
    //get route
    vector<st_coord_route> vec_route;
    switch(route_type)
    {
        case cm_expansion_mission: m_vec_cities[city_index].get_new_expansion_route(vec_route); break;
        case cm_trade_mission    : m_vec_cities[city_index].get_new_trade_route(vec_route); ;break;
    }
    start_pos[0]=vec_route.front().x_hex+vec_route.front().x_box_shift*_world_size_x;
    start_pos[1]=vec_route.front().y_hex+vec_route.front().y_box_shift*_world_size_x;

    //translation
    int last_pos[2]={start_pos[0],start_pos[1]};
    for(int tile_i=1;tile_i<(int)vec_route.size();tile_i++)
    {
        int new_pos[2]={vec_route[tile_i].x_hex+vec_route[tile_i].x_box_shift*_world_size_x,
                        vec_route[tile_i].y_hex+vec_route[tile_i].y_box_shift*_world_size_y};
        int x_dif=new_pos[0]-last_pos[0];
        int y_dif=new_pos[1]-last_pos[1];
        last_pos[0]=new_pos[0];
        last_pos[1]=new_pos[1];
        //check side
        switch(x_dif)
        {
            case -1:
            {
                switch(y_dif)
                {
                    case -1: vec_char_route.push_back('x'); break;
                    case  0: vec_char_route.push_back('b'); break;
                    case  1: vec_char_route.push_back('a'); break;
                    default: vec_char_route.push_back('x'); break;//error
                }
            }break;
            case  0:
            {
                switch(y_dif)
                {
                    case -1: vec_char_route.push_back('c'); break;
                    case  0: vec_char_route.push_back('x'); break;
                    case  1: vec_char_route.push_back('f'); break;
                    default: vec_char_route.push_back('x'); break;//error
                }
            }break;
            case  1:
            {
                switch(y_dif)
                {
                    case -1: vec_char_route.push_back('d'); break;
                    case  0: vec_char_route.push_back('e'); break;
                    case  1: vec_char_route.push_back('x'); break;
                    default: vec_char_route.push_back('x'); break;//error
                }
            }break;

            default: vec_char_route.push_back('x'); break;//error
        }
    }

    return true;
}

bool world::translate_char_to_route_and_add_expansion(vector<char> vec_char_route,int city_id,int new_city_id,int start_pos[2])
{
    //translate route
    vector<st_coord_route> vec_route;
    st_coord_route start_coord(start_pos[0],start_pos[1]);
    vec_route.push_back(start_coord);

    for(int i=0;i<(int)vec_char_route.size();i++)
    {
        int new_x_pos=vec_route.back().x_hex;
        int new_y_pos=vec_route.back().y_hex;
        int box_shift_x=vec_route.back().x_box_shift;
        int box_shift_y=vec_route.back().y_box_shift;
        int new_road_dir=-1;
        switch(vec_char_route[i])
        {
            case 'a':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=0;//last
                new_road_dir=3;//new
            }break;

            case 'b':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=1;//last
                new_road_dir=4;//new
            }break;

            case 'c':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=2;//last
                new_road_dir=5;//new
            }break;

            case 'd':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=3;//last
                new_road_dir=0;//new
            }break;

            case 'e':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=4;//last
                new_road_dir=1;//new
            }break;

            case 'f':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=5;//last
                new_road_dir=2;//new
            }break;
        }
        vec_route.push_back( st_coord_route(new_x_pos,new_y_pos,box_shift_x,box_shift_y) );
        vec_route.back().road[0]=new_road_dir;
    }

    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Route translation to route: Could not find city\n";
        return false;
    }

    //send route to city
    m_vec_cities[city_index].set_new_expansion_route(vec_route);
    m_vec_cities[city_index].set_city_mode(-1,cm_expansion_mission);
    m_vec_cities[city_index].set_new_city_id(new_city_id);

    return true;
}

bool world::translate_char_to_route_and_add_trade(vector<char> vec_char_route,int city_id,int start_pos[2])
{
    //translate route
    vector<st_coord_route> vec_trade_route;
    st_coord_route start_coord(start_pos[0],start_pos[1]);
    vec_trade_route.push_back(start_coord);

    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Route translation to route: Could not find city\n";
        return false;
    }

    for(int i=0;i<(int)vec_char_route.size();i++)
    {
        int new_x_pos=vec_trade_route.back().x_hex;
        int new_y_pos=vec_trade_route.back().y_hex;
        int box_shift_x=vec_trade_route.back().x_box_shift;
        int box_shift_y=vec_trade_route.back().y_box_shift;
        int new_road_dir=-1;
        switch(vec_char_route[i])
        {
            case 'a':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=0;//last
                new_road_dir=3;//new
            }break;

            case 'b':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=1;//last
                new_road_dir=4;//new
            }break;

            case 'c':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=2;//last
                new_road_dir=5;//new
            }break;

            case 'd':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=3;//last
                new_road_dir=0;//new
            }break;

            case 'e':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=4;//last
                new_road_dir=1;//new
            }break;

            case 'f':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=5;//last
                new_road_dir=2;//new
            }break;
        }
        vec_trade_route.push_back( st_coord_route(new_x_pos,new_y_pos,box_shift_x,box_shift_y) );
        vec_trade_route.back().road[0]=new_road_dir;
    }

    //get ids of both cities
    int city_id_a=m_arr_tiles[ vec_trade_route.front().x_hex ][ vec_trade_route.front().y_hex ].get_city_id();
    int city_id_b=m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_city_id();

    int trade_id=m_id_counter_trade++;
    m_vec_trades.push_back( trade(vec_trade_route,trade_id,city_id_a,city_id_b) );
    //tell tiles on route
    for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, cities
    {//add road to land or water, if already road there, add trade id
        int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
        if(temp_tt==tt_land || temp_tt==tt_land_w_road)
            m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land_w_road,trade_id);
        if(temp_tt==tt_water || temp_tt==tt_water_w_road)
            m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water_w_road,trade_id);
    }

    //reset city mode to idle from waiting for response
    m_vec_cities[city_index].set_city_mode(-1,cm_idle);

    return true;
}

bool world::translate_char_to_route_and_test_expansion(vector<char> vec_char_route,int city_id,int start_pos[2])
{
    //translate route
    vector<st_coord_route> vec_route;
    st_coord_route start_coord(start_pos[0],start_pos[1]);
    vec_route.push_back(start_coord);

    for(int i=0;i<(int)vec_char_route.size();i++)
    {
        int new_x_pos=vec_route.back().x_hex;
        int new_y_pos=vec_route.back().y_hex;
        int box_shift_x=vec_route.back().x_box_shift;
        int box_shift_y=vec_route.back().y_box_shift;
        int new_road_dir=-1;
        switch(vec_char_route[i])
        {
            case 'a':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=0;//last
                new_road_dir=3;//new
            }break;

            case 'b':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=1;//last
                new_road_dir=4;//new
            }break;

            case 'c':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=2;//last
                new_road_dir=5;//new
            }break;

            case 'd':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=3;//last
                new_road_dir=0;//new
            }break;

            case 'e':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=4;//last
                new_road_dir=1;//new
            }break;

            case 'f':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_route.back().road[1]=5;//last
                new_road_dir=2;//new
            }break;
        }
        vec_route.push_back( st_coord_route(new_x_pos,new_y_pos,box_shift_x,box_shift_y) );
        vec_route.back().road[0]=new_road_dir;
    }

    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Expansion route translation to route: Could not find city\n";
        return false;
    }

    //is route buildable
    for(int tile_i=1;tile_i<(int)vec_route.size();tile_i++)//skip first, in city
    {
        if( m_arr_tiles[ vec_route[tile_i].x_hex ][ vec_route[tile_i].y_hex ].get_tile_type()!=tt_land &&
            m_arr_tiles[ vec_route[tile_i].x_hex ][ vec_route[tile_i].y_hex ].get_tile_type()!=tt_water )
        {
            cout<<"Error: Expansion route test: Route not buildable\n";
            return false;
        }
    }

    //final tile have to be land
    if( m_arr_tiles[ vec_route.back().x_hex ][ vec_route.back().y_hex ].get_tile_type()!=tt_land )
    {
        cout<<"Error: Expansion route test: Route not buildable\n";
        return false;
    }

    return true;//route ok
}

bool world::translate_char_to_route_and_test_trade(vector<char>& vec_char_route,int city_id,int start_pos[2])
{
    //translate route
    vector<st_coord_route> vec_trade_route;
    st_coord_route start_coord(start_pos[0],start_pos[1]);
    vec_trade_route.push_back(start_coord);

    for(int i=0;i<(int)vec_char_route.size();i++)
    {
        int new_x_pos=vec_trade_route.back().x_hex;
        int new_y_pos=vec_trade_route.back().y_hex;
        int box_shift_x=vec_trade_route.back().x_box_shift;
        int box_shift_y=vec_trade_route.back().y_box_shift;
        int new_road_dir=-1;
        switch(vec_char_route[i])
        {
            case 'a':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=0;//last
                new_road_dir=3;//new
            }break;

            case 'b':
            {
                //update tile pos
                new_x_pos+=-1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=1;//last
                new_road_dir=4;//new
            }break;

            case 'c':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=2;//last
                new_road_dir=5;//new
            }break;

            case 'd':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=-1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=3;//last
                new_road_dir=0;//new
            }break;

            case 'e':
            {
                //update tile pos
                new_x_pos+=+1; new_y_pos+=0;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=4;//last
                new_road_dir=1;//new
            }break;

            case 'f':
            {
                //update tile pos
                new_x_pos+=0; new_y_pos+=1;
                //box shift test
                if(new_x_pos<0)              {box_shift_x-=1;new_x_pos+=_world_size_x;}
                if(new_x_pos>=_world_size_x) {box_shift_x+=1;new_x_pos-=_world_size_x;}
                if(new_y_pos<0)              {box_shift_y-=1;new_y_pos+=_world_size_y;}
                if(new_y_pos>=_world_size_y) {box_shift_y+=1;new_y_pos-=_world_size_y;}
                //road direction
                vec_trade_route.back().road[1]=5;//last
                new_road_dir=2;//new
            }break;
        }
        vec_trade_route.push_back( st_coord_route(new_x_pos,new_y_pos,box_shift_x,box_shift_y) );
        vec_trade_route.back().road[0]=new_road_dir;
    }

    //get ids of both cities
    int city_id_a=m_arr_tiles[ vec_trade_route.front().x_hex ][ vec_trade_route.front().y_hex ].get_city_id();
    int city_id_b=m_arr_tiles[ vec_trade_route.back().x_hex ][ vec_trade_route.back().y_hex ].get_city_id();

    //test if trade is still valid
    bool abort_trade=false;
    //have the two cities merged?
    if(city_id_a==city_id_b || city_id_a==-1 || city_id_b==-1) abort_trade=true;//Yes, skip further testing
    else for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
    {
        if( (m_vec_trades[trade_i].get_city_a_id()==city_id_a && m_vec_trades[trade_i].get_city_b_id()==city_id_b) ||
            (m_vec_trades[trade_i].get_city_a_id()==city_id_b && m_vec_trades[trade_i].get_city_b_id()==city_id_a) )
        {
            cout<<"Trade: New trade aborted: Cities have merged\n";
            abort_trade=true;
            break;
        }
    }
    if(abort_trade) return false;//city does not have to know so far

    //test tiles on route, have to be tt_land or water
    bool simple_road=true;//trade does not cross with other roads
    for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, cities
    {
        //test if placed or rock
        if( m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type()==tt_rock )
        {//unbuildable terrain, abort
            abort_trade=true;
            break;
        }

        //test if placed on city
        if( m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type()==tt_land_w_city )
        {//already built here, could be start or end city
            if( m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_city_id()==city_id_a )
            {//collision with owner city, front
                //cut tiles from start to this tile, not including this tile
                vec_char_route.erase( vec_char_route.begin(), vec_char_route.begin()+tile_i );
                vec_trade_route.erase( vec_trade_route.begin(), vec_trade_route.begin()+tile_i );

                //set new start pos
                start_pos[0]=vec_trade_route.front().x_hex;
                start_pos[1]=vec_trade_route.front().y_hex;

                //restart loop
                tile_i=0;
                continue;
            }

            if( m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_city_id()==city_id_b )
            {//collision with owner city, back
                //cut tiles from this tile to the end, not including this tile
                vec_char_route.erase( vec_char_route.begin()+tile_i, vec_char_route.end() );
                vec_trade_route.erase( vec_trade_route.begin()+tile_i+1, vec_trade_route.end() );

                //set new start pos (same)
                start_pos[0]=vec_trade_route.front().x_hex;
                start_pos[1]=vec_trade_route.front().y_hex;

                //restart loop
                tile_i=0;
                continue;
            }

            //other city, abort
            abort_trade=true;
            break;
        }

        //test if placed on other road
        if( m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type()==tt_land_w_road ||
            m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type()==tt_water_w_road )
        {//crossed road, test more
            simple_road=false;
            break;
        }
    }
    if(!simple_road && !abort_trade)//more tests have to be done
    {//find tiles that cross roads with other trades and test if they are still allowed
        //go through all trades and their route tile to find intersection
        for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
        {
            if(abort_trade) break;
            //get trade route
            vector<st_coord_route> vec_trade_route_old;
            m_vec_trades[trade_i].get_trade_route_list(vec_trade_route_old);
            for(int tile_i_old=0;tile_i_old<(int)vec_trade_route_old.size();tile_i_old++)
            {
                if(abort_trade) break;
                //go through all tile of new trade path
                for(int tile_new_i=1;tile_new_i<(int)vec_trade_route.size()-1;tile_new_i++)//skip first and last, cities
                {
                    if( vec_trade_route[tile_new_i].x_hex==vec_trade_route_old[tile_i_old].x_hex &&
                        vec_trade_route[tile_new_i].y_hex==vec_trade_route_old[tile_i_old].y_hex )
                    {//intersection, test if current road placement is allowed
                        //test if new road enter or exits tile same as other route tile
                        if( vec_trade_route[tile_new_i].road[0]==vec_trade_route_old[tile_i_old].road[0] ||
                            vec_trade_route[tile_new_i].road[0]==vec_trade_route_old[tile_i_old].road[1] ||
                            vec_trade_route[tile_new_i].road[1]==vec_trade_route_old[tile_i_old].road[0] ||
                            vec_trade_route[tile_new_i].road[1]==vec_trade_route_old[tile_i_old].road[1] )
                        {//same enter/exit used, cancel route
                            cout<<"Trade: Road collision at: "<<vec_trade_route[tile_new_i].x_hex<<", "<<vec_trade_route[tile_new_i].y_hex<<endl;
                            abort_trade=true;
                            break;
                        }
                    }
                }
            }
        }
    }
    if(abort_trade)
    {//cancel trade request
        cout<<"Trade: New trade route placement was not ok due to bad road crossing\n";
        return false;
    }

    return true;//route ok
}

bool world::is_tile_in_city(int city_id,int pos[2])
{
    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: Route translation to route: Could not find city\n";
        return false;
    }

    //get city tiles
    vector<st_coord_chararr> vec_city_tiles;
    m_vec_cities[city_index].get_city_tiles_list(vec_city_tiles);

    //test pos
    for(int tile_i=0;tile_i<(int)vec_city_tiles.size();tile_i++)
    {
        if( vec_city_tiles[tile_i].x_hex==pos[0] &&
            vec_city_tiles[tile_i].y_hex==pos[1] )
        {
            return true;
        }
    }

    return false;
}

bool world::set_network_mode(int mode)//0 test, 1 server, 2 client
{
    m_network_mode=mode;
    cout<<"Network: Network mode is now: "<<m_network_mode<<endl;
    return true;
}

bool world::get_city_growth(vector<st_coord_w_val>& vec_growth_coord)
{
    vec_growth_coord=m_vec_city_growth_coord;

    return true;
}

int world::do_city_growth(int city_id,int pos[2])//with road and merge test
{
    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"City growth: Could not find city, try to force build\n";
        //find if a city with this id is on the way of construction (server could already build it before client)
        bool city_force_build=false;
        for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
        {
            if( m_vec_cities[city_i].get_new_city_id()==city_id )
            {
                //force this city to be build now
                m_vec_cities[city_i].force_finish_new_city();

                //get hex of new city and old tile
                int city_old_hex_x,city_old_hex_y,city_new_hex_x,city_new_hex_y;
                m_vec_cities[city_i].get_new_city_expansion_info(city_old_hex_x,city_old_hex_y,city_new_hex_x,city_new_hex_y);

                //test if new city location is still valid, skip
                //if( !m_arr_tiles[ city_new_hex_x ][ city_new_hex_y ].is_buildable() ) break;//not buildable anymore

                //remove old city tile
                m_vec_cities[city_i].remove_city_tile(city_old_hex_x,city_old_hex_y);

                //trade check for that tile
                for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                {
                    if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[city_i].get_city_id() ||
                        m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[city_i].get_city_id() )
                    {//test if that tile is involved
                        vector<st_coord_route> vec_trade_route;
                        m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
                        int trade_id=m_vec_trades[trade_i].get_trade_id();
                        for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
                        {
                            if( vec_trade_route[tile_i].x_hex==city_old_hex_x &&
                                vec_trade_route[tile_i].y_hex==city_old_hex_y )
                            {//this tile is involved in that trade, remove trade
                                //inform tiles
                                for(int tile_i2=1;tile_i2<(int)vec_trade_route.size()-1;tile_i2++)
                                {
                                    int temp_tt=m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].get_tile_type();
                                    if(temp_tt==tt_land_w_road)
                                        m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_land,trade_id);
                                    if(temp_tt==tt_water_w_road)
                                        m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_water,trade_id);
                                }
                                //remove trade
                                m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                                trade_i--;
                                break;
                            }
                        }
                    }
                }

                //tell that tile that there is no city anymore
                m_arr_tiles[city_old_hex_x][city_old_hex_y].set_tile_type(tt_land,-1);

                //build new city
                int new_city_id=m_vec_cities[city_i].get_new_city_id();
                m_vec_cities[city_i].reset_new_city_id();
                int city_color=m_vec_cities[city_i].get_city_color();//get player color
                m_vec_cities.push_back( city(city_new_hex_x,city_new_hex_y,city_color,new_city_id,m_vec_trades,&m_arr_tiles[0][0],m_pSound) );
                cout<<"Expansion: New city built at:  X: "<<city_new_hex_x<<"  Y: "<<city_new_hex_y<<endl;

                //tell that tile that there is a city
                m_arr_tiles[city_new_hex_x][city_new_hex_y].set_owner(city_color,new_city_id);

                //test if old city have any tiles left
                if( m_vec_cities[city_i].get_city_size()<1 )
                {//city have no tiles, remove
                    //test if that city have any trades that needs to be removed to (probably not)
                    for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
                    {
                        if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[city_i].get_city_id() ||
                            m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[city_i].get_city_id() )
                        {//remove this trade
                            //inform tiles
                            vector<st_coord_route> vec_trade_route;
                            m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
                            int trade_id=m_vec_trades[trade_i].get_trade_id();
                            for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)
                            {
                                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                                if(temp_tt==tt_land_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                                if(temp_tt==tt_water_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
                            }
                            //remove trade
                            m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                            trade_i--;
                        }
                    }
                    //remove city
                    remove_city(city_i);
                    //i is not correct anymore
                }

                //Test if any trade was affected
                //??? again, se above

                cout<<"Fix: Force city to finish the new city, continue with growth\n";
                city_force_build=true;
                break;
            }
        }

        if(!city_force_build)
        {
            cout<<"Error: City growth: City with new id could not be force build\n";
            return 0;//could not be fixed
        }

        //find index again after force build
        for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
        {
            if( m_vec_cities[city_i].get_city_id()==city_id )
            {
                city_index=city_i;
                break;
            }
        }
        if(city_index==-1)//still not there
        {
            cout<<"Error: City growth: Could not find force build city\n";
            return 0;
        }
    }

    //GROWTH at x_hex,y_hex
    cout<<"Growth: Growth for city id: "<<city_id<<", index: "<<city_index<<endl;
    int x_hex=pos[0];
    int y_hex=pos[1];
    m_vec_cities[city_index].add_tile_to_city( x_hex,y_hex );

    //Test if any trade was affected
    if( m_arr_tiles[x_hex][y_hex].get_tile_type()==tt_land_w_road )
    {//test if this destroys the trade
        cout<<"City growth: Expansion on a tile with road\n";
        for(int road=0;road<3;road++)//can be 3 roads per tile, test all possible trades
        {//trade can only be saved if the same city grew, absorbing part of the road
            int trade_id=m_arr_tiles[x_hex][y_hex].get_trade_id(road);
            if(trade_id==-1) break;//no more trades on this tile
            //find trade index
            int trade_index=-1;
            for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
            {
                if( m_vec_trades[trade_i].get_trade_id()==trade_id )
                {
                    trade_index=trade_i;
                    break;
                }
            }
            if(trade_index==-1)//bad
            {
                cout<<"ERROR: City growth: Could not find the tiles trade index\n";
            }
            else//continue test
            {
                //test if city that expanded is involded in this trade

                //temp
                cout<<"City growth: Expansion on a tile with road: City id a: "<<m_vec_trades[trade_index].get_city_a_id();
                cout<<" b: "<<m_vec_trades[trade_index].get_city_b_id()<<" this city id: "<<m_vec_cities[city_index].get_city_id()<<endl;
                //xxx

                if( m_vec_trades[trade_index].get_city_a_id()==m_vec_cities[city_index].get_city_id() ||
                    m_vec_trades[trade_index].get_city_b_id()==m_vec_cities[city_index].get_city_id() )
                {//trade can be saved
                    cout<<"City growth: Expansion on a tile with road: Trade can be saved\n";
                    //find which tile to remove from trade route
                    vector<st_coord_route> vec_trade_route;
                    m_vec_trades[trade_index].get_trade_route_list(vec_trade_route);
                    //test if first element is this city
                    if( m_arr_tiles[ vec_trade_route.front().x_hex ][ vec_trade_route.front().y_hex ].get_city_id() == m_vec_cities[city_index].get_city_id() )
                    {//remove tiles from start to new pos
                        //tell tiles about the update
                        for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
                        {
                            if( vec_trade_route[tile_i].x_hex==x_hex && vec_trade_route[tile_i].y_hex==y_hex )
                            {//stop removing
                                break;
                            }
                            else//remove road from this tile
                            {
                                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                                if(temp_tt==tt_land_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                                if(temp_tt==tt_water_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
                            }
                        }

                        //remove tiles from route
                        m_vec_trades[trade_index].city_expansion_update( vec_trade_route.front().x_hex, vec_trade_route.front().y_hex,
                                                                         x_hex, y_hex );
                    }
                    else//last element must be this city
                    {//remove tiles from new pos to end
                        //tell tiles about the update
                        for(int tile_i=(int)vec_trade_route.size()-1;tile_i>=0;tile_i--)//backwards
                        {
                            if( vec_trade_route[tile_i].x_hex==x_hex && vec_trade_route[tile_i].y_hex==y_hex )
                            {//stop removing
                                break;
                            }
                            else//remove road from this tile
                            {
                                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                                if(temp_tt==tt_land_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                                if(temp_tt==tt_water_w_road)
                                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
                            }
                        }

                        //remove tiles from route
                        m_vec_trades[trade_index].city_expansion_update( x_hex, y_hex,
                                                                         vec_trade_route.back().x_hex, vec_trade_route.back().y_hex );
                    }

                }
                else//remove this trade
                {
                    cout<<"City growth: Expansion on a tile with road: Removing trade route\n";
                    vector<st_coord_route> vec_trade_route;
                    m_vec_trades[trade_index].get_trade_route_list(vec_trade_route);
                    //tell tiles on route
                    for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, cities
                    {//remove road from land or water
                        int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                        if(temp_tt==tt_land_w_road)
                            m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                        if(temp_tt==tt_water_w_road)
                            m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
                    }

                    //tell trade
                    m_vec_trades.erase( m_vec_trades.begin()+trade_index );
                }
            }
        }
        //cout<<"City growth: Expansion on a tile with road: Complete\n";
    }

    //tell tile about this
    int owner_id=m_vec_cities[city_index].get_city_owner_id();
    m_arr_tiles[x_hex][y_hex].set_owner( owner_id, m_vec_cities[city_index].get_city_id() );
    //cout<<"Expansion: Expansion complete\n";

    //test if close to another friendly city
    //cout<<"Expansion: Merge test\n";
    int this_city_id=m_vec_cities[city_index].get_city_id();
    for(int x_add=-1;x_add<2;x_add++)
    for(int y_add=-1;y_add<2;y_add++)
    {
        if(x_add==y_add) continue; //skip -1,-1 and +1,+1 and center 0,0
        if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) continue;//coutide map

        if( m_arr_tiles[ x_hex+x_add ][ y_hex+y_add ].get_tile_type() == tt_land_w_city )
        {//other city nearby, could be origin city
            cout<<"Merge test: City is nearby\n";
            if(m_arr_tiles[ x_hex+x_add ][ y_hex+y_add ].get_city_id()!=this_city_id )
            {//other city nearby, could be other owner
                cout<<"Merge test: Other city is nearby\n";
                //find city index
                int other_city_index=-1;
                int other_city_id=m_arr_tiles[ x_hex+x_add ][ y_hex+y_add ].get_city_id();
                for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
                {
                    if(m_vec_cities[city_i].get_city_id()==other_city_id)
                    {
                        other_city_index=city_i;
                        cout<<"Merge test: Other city index: "<<other_city_index<<endl;
                        break;
                    }
                }
                if(other_city_index==-1)
                {//error
                    cout<<"ERROR: Merging cities: Could not find other city id\n";
                    break;
                }

                if( m_vec_cities[city_index].get_city_owner_id()==m_vec_cities[other_city_index].get_city_owner_id() )
                {//same owner, merge both cities, tuching is enough
                    cout<<"Merge test: Same owner, merge now\n";
                    merge_cities(city_index,other_city_index);
                    return 2;
                }
                else//different owners, more merge testing is required
                {
                    if( takeover_city_test(city_index,other_city_index) )
                    {//city taken, merge
                        cout<<"Merge test: Not same owner, merge now\n";
                        merge_cities(city_index,other_city_index);
                        return 2;
                    }
                    else cout<<"Merge test: Not same owner, dont merge\n";
                }

            }
            else cout<<"Merge test: Same city, no merge\n";
        }

    }
    m_vec_cities[city_index].set_recalc_border_flag(true);
    //cout<<"Expansion: Merge test complete\n";

    return 1;//expansion without merge
}

bool world::get_city_starvation(vector<st_coord_w_val>& vec_starve_coord)
{
    vec_starve_coord=m_vec_city_starve_coord;

    return true;
}

bool world::do_city_starvation(int city_id,int pos[2])
{
    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: City growth: Could not find city\n";
        return false;
    }

    int x_hex=pos[0];
    int y_hex=pos[1];

    //tell city
    m_vec_cities[city_index].remove_city_tile(x_hex,y_hex);

    //tell tile
    m_arr_tiles[x_hex][y_hex].set_tile_type(tt_land,-1);

    //test trades involved with this city
    for(int trade_i=0;trade_i<(int)m_vec_trades.size();trade_i++)
    {
        if( m_vec_trades[trade_i].get_city_a_id()==m_vec_cities[city_index].get_city_id() ||
            m_vec_trades[trade_i].get_city_b_id()==m_vec_cities[city_index].get_city_id() )
        {//this trade is involved
            cout<<"Starvation: City have a trade that needs testing\n";
            //test these trades if that tile is involved
            vector<st_coord_route> vec_trade_route;
            m_vec_trades[trade_i].get_trade_route_list(vec_trade_route);
            int trade_id=m_vec_trades[trade_i].get_trade_id();
            for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
            {
                if( vec_trade_route[tile_i].x_hex==x_hex &&
                    vec_trade_route[tile_i].y_hex==y_hex )
                {//this trade route is affected, must be start or endpoint of route (a city tile)
                    //trade must be removed
                    cout<<"Starvation: Removing a trade from this tile\n";
                    //tell tiles
                    for(int tile_i2=1;tile_i2<(int)vec_trade_route.size()-1;tile_i2++)//skip first and last, cities
                    {
                        int temp_tt=m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].get_tile_type();
                        if(temp_tt==tt_land_w_road)
                            m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_land,trade_id);
                        if(temp_tt==tt_water_w_road)
                            m_arr_tiles[ vec_trade_route[tile_i2].x_hex ][ vec_trade_route[tile_i2].y_hex ].set_tile_type(tt_water,trade_id);
                    }
                    //remove trade
                    m_vec_trades.erase( m_vec_trades.begin()+trade_i );
                    trade_i--;//go back one step, otherwise one element will be missed
                    break;
                }
            }
        }
    }

    return true;
}

int world::get_free_id_city(void)
{
    return m_id_counter_city++;
}

bool world::get_start_pos_pix(int pos_pix[2])
{
    pos_pix[0]=m_start_pos_pix[0];
    pos_pix[1]=m_start_pos_pix[1];

    //cout<<"View: "<<m_start_pos_pix[0]<<", "<<m_start_pos_pix[1]<<endl;

    return true;
}

bool world::get_city_tile_count_result(vector<int>& vec_tile_count)
{
    //go through all cities and count tiles, add to counter
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        int city_size=m_vec_cities[city_i].get_city_size();
        int city_owner=m_vec_cities[city_i].get_city_owner_id();//col id
        if( city_owner<0 || city_owner>=(int)vec_tile_count.size() )
        {
            cout<<"Error: Game over test: Tile counting: Bad city owner\n";
            return false;
        }
        vec_tile_count[city_owner]+=city_size;
    }

    return true;
}



//---Private---



bool world::create_world(void)
{
    cout<<"Generating terrain\n";
    //reset old cities and trades
    m_vec_cities.clear();
    m_vec_trades.clear();

    int map_size=(_world_size_x+_world_size_y)/2.0;
    //fill with land
    for(int x=0;x<_world_size_x;x++)
    for(int y=0;y<_world_size_y;y++)
    {
        m_arr_tiles[x][y]=tile(x,y,tt_land,m_texture_tile);
    }

    //mountains
    int numof_mountains=rand()%int(map_size/4.0+1);
    cout<<"Number of Mountains: "<<numof_mountains<<endl;
    vector<st_coord> vec_mountains;
    //placement
    for(int i=0;i<numof_mountains;i++)
    {
        int x_hex_center=rand()%_world_size_x;
        int y_hex_center=rand()%_world_size_y;
        //int z_hex_center=-(x_hex+y_hex);
        int radius=rand()%5+1;

        //store mountain
        vec_mountains.push_back( st_coord(x_hex_center,y_hex_center) );

        vector<st_coord> vec_mountain_tiles;
        for(int x_add=-radius;x_add<=radius;x_add++)
        for(int y_add=-radius;y_add<=radius;y_add++)
        {
            //cout<<"new add\n";
            int x_hex_new=x_hex_center+x_add;
            int y_hex_new=y_hex_center+y_add;
            //int z_hex_new=-(x_hex_new+y_hex_new);

            //test radius
            if(x_add+y_add>radius || x_add+y_add<-radius) continue;

            //test if coutide map
            int x_shift=0; int y_shift=0;
            if(x_hex_new<0 || x_hex_new>=_world_size_x || y_hex_new<0 || y_hex_new>=_world_size_y)
            {//loop map
                while(x_hex_new<0)              {x_hex_new+=_world_size_x; x_shift-=1;}
                while(x_hex_new>=_world_size_x) {x_hex_new-=_world_size_x; x_shift+=1;}
                while(y_hex_new<0)              {y_hex_new+=_world_size_y; y_shift-=1;}
                while(y_hex_new>=_world_size_y) {y_hex_new-=_world_size_y; y_shift+=1;}
            }
            //change tt
            m_arr_tiles[x_hex_new][y_hex_new].set_tile_type(tt_rock,-1);
            //store tile
            vec_mountain_tiles.push_back( st_coord(x_hex_new,y_hex_new,x_shift,y_shift) );
        }
        //let mountain "grow"
        int max_growths_cycles=(rand()%int(radius*5))+radius*10;//expand with this number of tiles
        for(int growths=0;growths<max_growths_cycles;growths++)
        {
            vector<st_coord> vec_possible_tiles;
            for(int tile_i=0;tile_i<(int)vec_mountain_tiles.size();tile_i++)
            {
                int x_hex=vec_mountain_tiles[tile_i].x_hex;
                int y_hex=vec_mountain_tiles[tile_i].y_hex;
                //test tile type around this
                int x_shift=0; int y_shift=0;
                for(int x_add=-1;x_add<=1;x_add++)
                for(int y_add=-1;y_add<=1;y_add++)
                {
                    if(x_add==y_add) continue;

                    int new_x=x_hex+x_add;
                    int new_y=y_hex+y_add;

                    if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                    {//if tile is on the border to another box, special test required
                        if(new_x<0)                     {new_x+=_world_size_x; x_shift=-1;}
                        else if(new_x>=_world_size_x)   {new_x-=_world_size_x; x_shift=1;}
                        if(new_y<0)                     {new_y+=_world_size_y; y_shift=-1;}
                        else if(new_y>=_world_size_y)   {new_y-=_world_size_y; y_shift=1;}
                    }
                    if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land )
                    {
                        vec_possible_tiles.push_back( st_coord(new_x,new_y,x_shift,y_shift) );
                    }
                }
            }
            //pick winner
            if( !vec_possible_tiles.empty() )
            {
                int numof_tiles=(int)vec_possible_tiles.size();
                float chance_part=1.0/(float)numof_tiles;
                float rand_val=float(rand()%100)/100.0;
                int winner=-1;
                for(int tile_i=0;tile_i<numof_tiles;tile_i++)
                {
                    if(rand_val<chance_part*float(tile_i+1))
                    {
                        winner=tile_i; break;
                    }
                }
                if(winner==-1) winner=0;//to be safe

                m_arr_tiles[ vec_possible_tiles[winner].x_hex ][ vec_possible_tiles[winner].y_hex ].set_tile_type(tt_rock,-1);
                vec_mountain_tiles.push_back( st_coord( vec_possible_tiles[winner].x_hex , vec_possible_tiles[winner].y_hex ) );
            }
        }
    }
    //cout<<"Mountains Complete\n";

    //mountain ridges, connect two mountains
    int numof_ridges=rand()%int(numof_mountains/5.0+1);
    cout<<"Number of Mountain ridges: "<<numof_ridges<<endl;
    for(int i=0;i<numof_ridges;i++)
    {
        if(numof_mountains<2) break;
        //pick two mountains
        int mountain_1_index=rand()%numof_mountains;
        int mountain_2_index=rand()%numof_mountains;
        while(mountain_1_index==mountain_2_index) mountain_2_index=rand()%numof_mountains;

        int ridge_radius=rand()%4+1;
        //cout<<"River radius: "<<ridge_radius<<endl;

        //start with one and walk to the next one
        int x_hex_start=vec_mountains[mountain_1_index].x_hex;
        int y_hex_start=vec_mountains[mountain_1_index].y_hex;
        int x_hex_end=vec_mountains[mountain_2_index].x_hex;
        int y_hex_end=vec_mountains[mountain_2_index].y_hex;
        st_coord coord_start(x_hex_start,y_hex_start);
        st_coord coord_end(x_hex_end,y_hex_end);

        st_coord coord_last_pos(x_hex_start,y_hex_start);
        while( true )
        {
            //int distance_total=coord_start.distance( coord_end );
            int distance_to_go=coord_last_pos.distance( coord_end );
            if( distance_to_go<ridge_radius*2 )
            {
                break;//done with this ridge
            }
            //cout<<"Distance to go: "<<distance_to_go<<endl;
            //cout<<"At: "<<coord_last_pos.x_hex<<", "<<coord_last_pos.y_hex<<endl;

            //get new jump pos
            int x_jump,y_jump;
            while( true )
            {
                if(ridge_radius==1)
                {
                    x_jump=rand()%3-1;
                    y_jump=rand()%3-1;
                }
                else
                {
                    x_jump=(rand()%(ridge_radius*2)+1)-ridge_radius;
                    y_jump=(rand()%(ridge_radius*2)+1)-ridge_radius;
                }


                st_coord temp_coord_jump_pos( coord_last_pos.x_hex+x_jump,coord_last_pos.y_hex+y_jump);
                if( temp_coord_jump_pos.distance(coord_end) <= coord_last_pos.distance(coord_end) )//test if new pos i closer to end pos
                {
                    //cout<<temp_coord_jump_pos.distance(coord_end)<<" < "<<coord_last_pos.distance(coord_end)<<endl;
                    break;
                }
            }

            int new_x_hex_center=coord_last_pos.x_hex+x_jump;
            int new_y_hex_center=coord_last_pos.y_hex+y_jump;
            coord_last_pos=st_coord(new_x_hex_center,new_y_hex_center);

            //fill new spot with mountain tiles
            vector<st_coord> vec_mountain_tiles;
            for(int x_add=-ridge_radius;x_add<=ridge_radius;x_add++)
            for(int y_add=-ridge_radius;y_add<=ridge_radius;y_add++)
            {
                int x_hex_new=new_x_hex_center+x_add;
                int y_hex_new=new_y_hex_center+y_add;
                //int z_hex_new=-(x_hex_new+y_hex_new);

                //test radius
                if(x_add+y_add>ridge_radius || x_add+y_add<-ridge_radius) continue;

                //test if coutide map
                int x_shift=0; int y_shift=0;
                if(x_hex_new<0 || x_hex_new>=_world_size_x || y_hex_new<0 || y_hex_new>=_world_size_y)
                {//loop map
                    while(x_hex_new<0)              {x_hex_new+=_world_size_x; x_shift-=1;}
                    while(x_hex_new>=_world_size_x) {x_hex_new-=_world_size_x; x_shift+=1;}
                    while(y_hex_new<0)              {y_hex_new+=_world_size_y; y_shift-=1;}
                    while(y_hex_new>=_world_size_y) {y_hex_new-=_world_size_y; y_shift+=1;}
                }
                //change tt
                m_arr_tiles[x_hex_new][y_hex_new].set_tile_type(tt_rock,-1);
                //store tile
                vec_mountain_tiles.push_back( st_coord(x_hex_new,y_hex_new,x_shift,y_shift) );
            }
            //add randomness
            int max_growths_cycles=(rand()%int(ridge_radius*5))+ridge_radius;//expand with this number of tiles
            for(int growths=0;growths<max_growths_cycles;growths++)
            {
                vector<st_coord> vec_possible_tiles;
                for(int tile_i=0;tile_i<(int)vec_mountain_tiles.size();tile_i++)
                {
                    int x_hex=vec_mountain_tiles[tile_i].x_hex;
                    int y_hex=vec_mountain_tiles[tile_i].y_hex;
                    //test tile type around this
                    int x_shift=0; int y_shift=0;
                    for(int x_add=-1;x_add<=1;x_add++)
                    for(int y_add=-1;y_add<=1;y_add++)
                    {
                        if(x_add==y_add) continue;

                        int new_x=x_hex+x_add;
                        int new_y=y_hex+y_add;

                        if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                        {//if tile is on the border to another box, special test required
                            if(new_x<0)                     {new_x+=_world_size_x; x_shift=-1;}
                            else if(new_x>=_world_size_x)   {new_x-=_world_size_x; x_shift=1;}
                            if(new_y<0)                     {new_y+=_world_size_y; y_shift=-1;}
                            else if(new_y>=_world_size_y)   {new_y-=_world_size_y; y_shift=1;}
                        }
                        if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land )
                        {
                            vec_possible_tiles.push_back( st_coord(new_x,new_y,x_shift,y_shift) );
                        }
                    }
                }
                //pick winner
                if( !vec_possible_tiles.empty() )
                {
                    int numof_tiles=(int)vec_possible_tiles.size();
                    float chance_part=1.0/(float)numof_tiles;
                    float rand_val=float(rand()%100)/100.0;
                    int winner=-1;
                    for(int tile_i=0;tile_i<numof_tiles;tile_i++)
                    {
                        if(rand_val<chance_part*float(tile_i+1))
                        {
                            winner=tile_i; break;
                        }
                    }
                    if(winner==-1) winner=0;//to be safe

                    m_arr_tiles[ vec_possible_tiles[winner].x_hex ][ vec_possible_tiles[winner].y_hex ].set_tile_type(tt_rock,-1);
                    vec_mountain_tiles.push_back( st_coord( vec_possible_tiles[winner].x_hex , vec_possible_tiles[winner].y_hex ) );
                }
            }
        }

    }
    //cout<<"Mountains Ridges Complete\n";

    //Hills, single tile mountains
    int numof_hills=rand()%(map_size+1);
    cout<<"Number of Hills: "<<numof_hills<<endl;
    for(int i=0;i<numof_hills;i++)
    {
        int x_hex=rand()%_world_size_x;
        int y_hex=rand()%_world_size_y;

        m_arr_tiles[x_hex][y_hex].set_tile_type(tt_rock,-1);
    }
    //cout<<"Hills Complete\n";

    //Oceans
    int numof_oceans=rand()%int(map_size/10.0+1);
    cout<<"Number of Oceans: "<<numof_oceans<<endl;
    vector<st_coord> vec_oceans;
    //placement
    for(int i=0;i<numof_oceans;i++)
    {
        int x_hex_center=rand()%_world_size_x;
        int y_hex_center=rand()%_world_size_y;
        //int z_hex_center=-(x_hex+y_hex);
        int radius=rand()%10+1;

        //store ocean
        vec_oceans.push_back( st_coord(x_hex_center,y_hex_center) );

        vector<st_coord> vec_ocean_tiles;
        for(int x_add=-radius;x_add<=radius;x_add++)
        for(int y_add=-radius;y_add<=radius;y_add++)
        {
            int x_hex_new=x_hex_center+x_add;
            int y_hex_new=y_hex_center+y_add;
            //int z_hex_new=-(x_hex_new+y_hex_new);

            //test radius
            if(x_add+y_add>radius || x_add+y_add<-radius) continue;

            //test if coutide map
            int x_shift=0; int y_shift=0;
            if(x_hex_new<0 || x_hex_new>=_world_size_x || y_hex_new<0 || y_hex_new>=_world_size_y)
            {//loop map
                while(x_hex_new<0)              {x_hex_new+=_world_size_x; x_shift-=1;}
                while(x_hex_new>=_world_size_x) {x_hex_new-=_world_size_x; x_shift+=1;}
                while(y_hex_new<0)              {y_hex_new+=_world_size_y; y_shift-=1;}
                while(y_hex_new>=_world_size_y) {y_hex_new-=_world_size_y; y_shift+=1;}
            }
            //change tt
            m_arr_tiles[x_hex_new][y_hex_new].set_tile_type(tt_water,-1);
            //store tile
            vec_ocean_tiles.push_back( st_coord(x_hex_new,y_hex_new,x_shift,y_shift) );
        }
        //let ocean "grow"
        int max_growths_cycles=(rand()%int(radius*5))+radius*10;//expand with this number of tiles
        for(int growths=0;growths<max_growths_cycles;growths++)
        {
            vector<st_coord> vec_possible_tiles;
            for(int tile_i=0;tile_i<(int)vec_ocean_tiles.size();tile_i++)
            {
                int x_hex=vec_ocean_tiles[tile_i].x_hex;
                int y_hex=vec_ocean_tiles[tile_i].y_hex;
                //test tile type around this
                int x_shift=0; int y_shift=0;
                for(int x_add=-1;x_add<=1;x_add++)
                for(int y_add=-1;y_add<=1;y_add++)
                {
                    if(x_add==y_add) continue;

                    int new_x=x_hex+x_add;
                    int new_y=y_hex+y_add;

                    if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                    {//if tile is on the border to another box, special test required
                        if(new_x<0)                     {new_x+=_world_size_x; x_shift=-1;}
                        else if(new_x>=_world_size_x)   {new_x-=_world_size_x; x_shift=1;}
                        if(new_y<0)                     {new_y+=_world_size_y; y_shift=-1;}
                        else if(new_y>=_world_size_y)   {new_y-=_world_size_y; y_shift=1;}
                    }
                    if( m_arr_tiles[new_x][new_y].get_tile_type()!=tt_water )
                    {
                        vec_possible_tiles.push_back( st_coord(new_x,new_y,x_shift,y_shift) );
                    }
                }
            }
            //pick winner
            if( !vec_possible_tiles.empty() )
            {
                int numof_tiles=(int)vec_possible_tiles.size();
                float chance_part=1.0/(float)numof_tiles;
                float rand_val=float(rand()%100)/100.0;
                int winner=-1;
                for(int tile_i=0;tile_i<numof_tiles;tile_i++)
                {
                    if(rand_val<chance_part*float(tile_i+1))
                    {
                        winner=tile_i; break;
                    }
                }
                if(winner==-1) winner=0;//to be safe

                m_arr_tiles[ vec_possible_tiles[winner].x_hex ][ vec_possible_tiles[winner].y_hex ].set_tile_type(tt_water,-1);
                vec_ocean_tiles.push_back( st_coord( vec_possible_tiles[winner].x_hex , vec_possible_tiles[winner].y_hex ) );
            }
        }
    }
    //cout<<"Oceans Complete\n";

    //rivers, ocean to ocean
    int numof_rivers_ocean=rand()%int(numof_oceans/1.0+1);
    cout<<"Number of Ocean Rivers: "<<numof_rivers_ocean<<endl;
    for(int i=0;i<numof_rivers_ocean;i++)
    {
        if(numof_oceans<2) break;
        //pick two oceans
        int ocean_1_index=rand()%numof_oceans;
        int ocean_2_index=rand()%numof_oceans;
        while(ocean_1_index==ocean_2_index) ocean_2_index=rand()%numof_oceans;

        int river_radius=rand()%4;
        //cout<<"River radius: "<<river_radius<<endl;

        //start with one and walk to the next one
        int x_hex_start=vec_oceans[ocean_1_index].x_hex;
        int y_hex_start=vec_oceans[ocean_1_index].y_hex;
        int x_hex_end=vec_oceans[ocean_2_index].x_hex;
        int y_hex_end=vec_oceans[ocean_2_index].y_hex;
        st_coord coord_start(x_hex_start,y_hex_start);
        st_coord coord_end(x_hex_end,y_hex_end);

        st_coord coord_last_pos(x_hex_start,y_hex_start);
        while( true )
        {
            //int distance_total=coord_start.distance( coord_end );
            int distance_to_go=coord_last_pos.distance( coord_end );
            if( distance_to_go<river_radius*2+2 )
            {
                break;//done with this ridge
            }
            //cout<<"Distance to go: "<<distance_to_go<<endl;
            //cout<<"At: "<<coord_last_pos.x_hex<<", "<<coord_last_pos.y_hex<<endl;

            //get new jump pos
            int x_jump,y_jump;
            while( true )
            {
                if(river_radius<2)
                {
                    x_jump=rand()%3-1;
                    y_jump=rand()%3-1;
                }
                else
                {
                    x_jump=(rand()%(river_radius*2)+1)-river_radius;
                    y_jump=(rand()%(river_radius*2)+1)-river_radius;
                }


                st_coord temp_coord_jump_pos( coord_last_pos.x_hex+x_jump,coord_last_pos.y_hex+y_jump);
                if( temp_coord_jump_pos.distance(coord_end) <= coord_last_pos.distance(coord_end) )//test if new pos i closer to end pos
                {
                    //cout<<temp_coord_jump_pos.distance(coord_end)<<" < "<<coord_last_pos.distance(coord_end)<<endl;
                    break;
                }
            }

            int new_x_hex_center=coord_last_pos.x_hex+x_jump;
            int new_y_hex_center=coord_last_pos.y_hex+y_jump;
            coord_last_pos=st_coord(new_x_hex_center,new_y_hex_center);

            //fill new spot with water tiles
            vector<st_coord> vec_ocean_tiles;
            for(int x_add=-river_radius;x_add<=river_radius;x_add++)
            for(int y_add=-river_radius;y_add<=river_radius;y_add++)
            {
                int x_hex_new=new_x_hex_center+x_add;
                int y_hex_new=new_y_hex_center+y_add;
                //int z_hex_new=-(x_hex_new+y_hex_new);

                //test radius
                if(x_add+y_add>river_radius || x_add+y_add<-river_radius) continue;

                //test if coutide map
                int x_shift=0; int y_shift=0;
                if(x_hex_new<0 || x_hex_new>=_world_size_x || y_hex_new<0 || y_hex_new>=_world_size_y)
                {//loop map
                    while(x_hex_new<0)              {x_hex_new+=_world_size_x; x_shift-=1;}
                    while(x_hex_new>=_world_size_x) {x_hex_new-=_world_size_x; x_shift+=1;}
                    while(y_hex_new<0)              {y_hex_new+=_world_size_y; y_shift-=1;}
                    while(y_hex_new>=_world_size_y) {y_hex_new-=_world_size_y; y_shift+=1;}
                }
                //change tt
                m_arr_tiles[x_hex_new][y_hex_new].set_tile_type(tt_water,-1);
                //store tile
                vec_ocean_tiles.push_back( st_coord(x_hex_new,y_hex_new,x_shift,y_shift) );
            }
            //add randomness
            int max_growths_cycles=(rand()%int(river_radius*2+2))+river_radius;//expand with this number of tiles
            for(int growths=0;growths<max_growths_cycles;growths++)
            {
                vector<st_coord> vec_possible_tiles;
                for(int tile_i=0;tile_i<(int)vec_ocean_tiles.size();tile_i++)
                {
                    int x_hex=vec_ocean_tiles[tile_i].x_hex;
                    int y_hex=vec_ocean_tiles[tile_i].y_hex;
                    //test tile type around this
                    int x_shift=0; int y_shift=0;
                    for(int x_add=-1;x_add<=1;x_add++)
                    for(int y_add=-1;y_add<=1;y_add++)
                    {
                        if(x_add==y_add) continue;

                        int new_x=x_hex+x_add;
                        int new_y=y_hex+y_add;

                        if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                        {//if tile is on the border to another box, special test required
                            if(new_x<0)                     {new_x+=_world_size_x; x_shift=-1;}
                            else if(new_x>=_world_size_x)   {new_x-=_world_size_x; x_shift=1;}
                            if(new_y<0)                     {new_y+=_world_size_y; y_shift=-1;}
                            else if(new_y>=_world_size_y)   {new_y-=_world_size_y; y_shift=1;}
                        }
                        if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land )
                        {
                            vec_possible_tiles.push_back( st_coord(new_x,new_y,x_shift,y_shift) );
                        }
                    }
                }
                //pick winner
                if( !vec_possible_tiles.empty() )
                {
                    int numof_tiles=(int)vec_possible_tiles.size();
                    float chance_part=1.0/(float)numof_tiles;
                    float rand_val=float(rand()%100)/100.0;
                    int winner=-1;
                    for(int tile_i=0;tile_i<numof_tiles;tile_i++)
                    {
                        if(rand_val<chance_part*float(tile_i+1))
                        {
                            winner=tile_i; break;
                        }
                    }
                    if(winner==-1) winner=0;//to be safe

                    m_arr_tiles[ vec_possible_tiles[winner].x_hex ][ vec_possible_tiles[winner].y_hex ].set_tile_type(tt_water,-1);
                    vec_ocean_tiles.push_back( st_coord( vec_possible_tiles[winner].x_hex , vec_possible_tiles[winner].y_hex ) );
                }
            }
        }
    }
    //cout<<"Ocean Rivers Complete\n";

    //rivers, mountains to oceans
    int numof_rivers_mountain=rand()%int(numof_oceans/2.0+1);
    cout<<"Number of Mountain Rivers: "<<numof_rivers_mountain<<endl;
    for(int i=0;i<numof_rivers_mountain;i++)
    {
        if(numof_oceans<1 || numof_mountains<1) break;
        //pick two oceans
        int ocean_1_index=rand()%numof_oceans;
        int mountain_2_index=rand()%numof_mountains;

        int river_radius=rand()%2;
        //cout<<"River radius: "<<river_radius<<endl;

        //start with one and walk to the next one
        int x_hex_start=vec_oceans[ocean_1_index].x_hex;
        int y_hex_start=vec_oceans[ocean_1_index].y_hex;
        int x_hex_end=vec_mountains[mountain_2_index].x_hex;
        int y_hex_end=vec_mountains[mountain_2_index].y_hex;
        st_coord coord_start(x_hex_start,y_hex_start);
        st_coord coord_end(x_hex_end,y_hex_end);

        st_coord coord_last_pos(x_hex_start,y_hex_start);
        while( true )
        {
            //int distance_total=coord_start.distance( coord_end );
            int distance_to_go=coord_last_pos.distance( coord_end );
            if( distance_to_go<river_radius*2+2 )
            {
                break;//done with this ridge
            }
            //cout<<"Distance to go: "<<distance_to_go<<endl;
            //cout<<"At: "<<coord_last_pos.x_hex<<", "<<coord_last_pos.y_hex<<endl;

            //get new jump pos
            int x_jump,y_jump;
            while( true )
            {
                if(river_radius<2)
                {
                    x_jump=rand()%3-1;
                    y_jump=rand()%3-1;
                }
                else
                {
                    x_jump=(rand()%(river_radius*2)+1)-river_radius;
                    y_jump=(rand()%(river_radius*2)+1)-river_radius;
                }


                st_coord temp_coord_jump_pos( coord_last_pos.x_hex+x_jump,coord_last_pos.y_hex+y_jump);
                if( temp_coord_jump_pos.distance(coord_end) <= coord_last_pos.distance(coord_end) )//test if new pos i closer to end pos
                {
                    //cout<<temp_coord_jump_pos.distance(coord_end)<<" < "<<coord_last_pos.distance(coord_end)<<endl;
                    break;
                }
            }

            int new_x_hex_center=coord_last_pos.x_hex+x_jump;
            int new_y_hex_center=coord_last_pos.y_hex+y_jump;
            coord_last_pos=st_coord(new_x_hex_center,new_y_hex_center);

            //fill new spot with water tiles
            vector<st_coord> vec_ocean_tiles;
            for(int x_add=-river_radius;x_add<=river_radius;x_add++)
            for(int y_add=-river_radius;y_add<=river_radius;y_add++)
            {
                int x_hex_new=new_x_hex_center+x_add;
                int y_hex_new=new_y_hex_center+y_add;
                //int z_hex_new=-(x_hex_new+y_hex_new);

                //test radius
                if(x_add+y_add>river_radius || x_add+y_add<-river_radius) continue;

                //test if coutide map
                int x_shift=0; int y_shift=0;
                if(x_hex_new<0 || x_hex_new>=_world_size_x || y_hex_new<0 || y_hex_new>=_world_size_y)
                {//loop map
                    while(x_hex_new<0)              {x_hex_new+=_world_size_x; x_shift-=1;}
                    while(x_hex_new>=_world_size_x) {x_hex_new-=_world_size_x; x_shift+=1;}
                    while(y_hex_new<0)              {y_hex_new+=_world_size_y; y_shift-=1;}
                    while(y_hex_new>=_world_size_y) {y_hex_new-=_world_size_y; y_shift+=1;}
                }
                //change tt
                m_arr_tiles[x_hex_new][y_hex_new].set_tile_type(tt_water,-1);
                //store tile
                vec_ocean_tiles.push_back( st_coord(x_hex_new,y_hex_new,x_shift,y_shift) );
            }
            //add randomness
            int max_growths_cycles=(rand()%int(river_radius*2+2))+river_radius;//expand with this number of tiles
            for(int growths=0;growths<max_growths_cycles;growths++)
            {
                vector<st_coord> vec_possible_tiles;
                for(int tile_i=0;tile_i<(int)vec_ocean_tiles.size();tile_i++)
                {
                    int x_hex=vec_ocean_tiles[tile_i].x_hex;
                    int y_hex=vec_ocean_tiles[tile_i].y_hex;
                    //test tile type around this
                    int x_shift=0; int y_shift=0;
                    for(int x_add=-1;x_add<=1;x_add++)
                    for(int y_add=-1;y_add<=1;y_add++)
                    {
                        if(x_add==y_add) continue;

                        int new_x=x_hex+x_add;
                        int new_y=y_hex+y_add;

                        if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                        {//if tile is on the border to another box, special test required
                            if(new_x<0)                     {new_x+=_world_size_x; x_shift=-1;}
                            else if(new_x>=_world_size_x)   {new_x-=_world_size_x; x_shift=1;}
                            if(new_y<0)                     {new_y+=_world_size_y; y_shift=-1;}
                            else if(new_y>=_world_size_y)   {new_y-=_world_size_y; y_shift=1;}
                        }
                        if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land )
                        {
                            vec_possible_tiles.push_back( st_coord(new_x,new_y,x_shift,y_shift) );
                        }
                    }
                }
                //pick winner
                if( !vec_possible_tiles.empty() )
                {
                    int numof_tiles=(int)vec_possible_tiles.size();
                    float chance_part=1.0/(float)numof_tiles;
                    float rand_val=float(rand()%100)/100.0;
                    int winner=-1;
                    for(int tile_i=0;tile_i<numof_tiles;tile_i++)
                    {
                        if(rand_val<chance_part*float(tile_i+1))
                        {
                            winner=tile_i; break;
                        }
                    }
                    if(winner==-1) winner=0;//to be safe

                    m_arr_tiles[ vec_possible_tiles[winner].x_hex ][ vec_possible_tiles[winner].y_hex ].set_tile_type(tt_water,-1);
                    vec_ocean_tiles.push_back( st_coord( vec_possible_tiles[winner].x_hex , vec_possible_tiles[winner].y_hex ) );
                }
            }
        }
    }
    //cout<<"Mountains Rivers Complete\n";

    //lakes
    int numof_lakes=rand()%(map_size+1);
    cout<<"Number of Lakes: "<<numof_lakes<<endl;
    for(int i=0;i<numof_lakes;i++)
    {
        int x_hex=rand()%_world_size_x;
        int y_hex=rand()%_world_size_y;

        m_arr_tiles[x_hex][y_hex].set_tile_type(tt_water,-1);
    }
    //cout<<"Lakes Complete\n";


    /*//add test city TEMP
    int id=m_id_counter_city++;
    m_vec_cities.push_back( city(5,5,*m_pPlayer_color_id,id,m_vec_trades) );
    m_arr_tiles[5][5].set_owner(*m_pPlayer_color_id,id);
    //TEMP*/


    cout<<"World generation Complete\n";

    return true;
}

bool world::create_world_menu(void)
{
    //fill with land
    for(int x=0;x<_world_size_x;x++)
    for(int y=0;y<_world_size_y;y++)
    {
        m_arr_tiles[x][y]=tile(x,y,tt_land,m_texture_tile);
    }

    /*//add citys
    int id=m_id_counter_city++;
    int color=0;
    m_vec_cities.push_back( city(5,5,color,id,m_vec_trades) );
    m_arr_tiles[5][5].set_owner(color,id);
    m_vec_cities[0].add_tile_to_city(6,5);
    m_arr_tiles[6][5].set_owner(color,id);

    color=1;
    m_vec_cities.push_back( city(8,6,color,id,m_vec_trades) );
    m_arr_tiles[8][6].set_owner(color,id);*/

    //add trades

    return true;
}

bool world::merge_cities(int city_index_a,int city_index_b)
{
    cout<<"index: "<<city_index_a<<" w "<<city_index_b<<endl;//temp


    cout<<"Merge cities: "<<city_index_a<<" with "<<city_index_b<<endl;
    //limit test
    if( city_index_a<0 || city_index_a>=(int)m_vec_cities.size() ||
        city_index_b<0 || city_index_b>=(int)m_vec_cities.size() )
    {
        cout<<"ERROR: Merging cities: Bad city index, size: "<<m_vec_cities.size()<<endl;
        return false;
    }
    //find out size
    int city_size_a=m_vec_cities[city_index_a].get_city_size();
    int city_size_b=m_vec_cities[city_index_b].get_city_size();

    cout<<"size: "<<city_size_a<<" & "<<city_size_b<<endl;

    //city id from biggest city
    int city_id_losing=m_vec_cities[city_index_b].get_city_id();
    int city_id_winner=m_vec_cities[city_index_a].get_city_id();
    int city_index_losing=city_index_b;
    int city_index_winner=city_index_a;
    if(city_size_b>city_size_a)
    {
        city_id_losing=m_vec_cities[city_index_a].get_city_id();
        city_id_winner=m_vec_cities[city_index_b].get_city_id();
        city_index_losing=city_index_a;
        city_index_winner=city_index_b;
    }
    cout<<"Merge cities: City: "<<city_id_winner<<" is bigger and will consume "<<city_id_losing<<endl;

    cout<<"win_id: "<<city_id_winner<<" lost: "<<city_id_losing<<endl<<endl;

    //get tiles list from losing city
    vector<st_coord_chararr> vec_city_tiles_old;
    m_vec_cities[city_index_losing].get_city_tiles_list(vec_city_tiles_old);

    for(int i=0;i<(int)vec_city_tiles_old.size();i++)
    {
        //update tiles owner and city id
        m_arr_tiles[ vec_city_tiles_old[i].x_hex ][ vec_city_tiles_old[i].y_hex ].set_owner( m_vec_cities[city_index_winner].get_city_color(),
                                                                                             city_id_winner );
        //add tiles to other city
        m_vec_cities[ city_index_winner ].add_tile_to_city( vec_city_tiles_old[i].x_hex,vec_city_tiles_old[i].y_hex );
    }

    //remove old city's trades
    for(int i=0;i<(int)m_vec_trades.size();i++)
    {
        if( m_vec_trades[i].get_city_a_id()==city_id_losing || m_vec_trades[i].get_city_b_id()==city_id_losing )
        {//trade will be removed
            //tell tiles about removal of road
            cout<<"Merge cities: Removing trade from smallest city\n";
            vector<st_coord_route> vec_trade_route;
            m_vec_trades[i].get_trade_route_list(vec_trade_route);
            int trade_id=m_vec_trades[i].get_trade_id();
            for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, part of city
            {
                //reset to land or water
                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                if(temp_tt==tt_land_w_road)
                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                if(temp_tt==tt_water_w_road)
                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
            }
            m_vec_trades.erase( m_vec_trades.begin()+i );
            i--;
        }
    }

    //remove old city from vec
    m_vec_cities.erase( m_vec_cities.begin()+city_index_losing );

    return true;
}

bool world::takeover_city_test(int city_index_attacker,int city_index_defender)
{
    //count number of city tiles, attacker have to have more
    int city_size_attacker=m_vec_cities[city_index_attacker].get_city_size();
    int city_size_defender=m_vec_cities[city_index_defender].get_city_size();
    if(city_size_defender>=city_size_attacker)
    {//no takeover
        cout<<"City takeover: No takeover due to few city tiles\n";
        return false;
    }

    //count number of city tiles that the defender have towards the attacker
    int numof_contest_tiles=0;
    vector<st_coord_chararr> vec_city_tiles_attacker;
    vector<st_coord_chararr> vec_city_tiles_defender;
    m_vec_cities[city_index_attacker].get_city_tiles_list(vec_city_tiles_attacker);
    m_vec_cities[city_index_defender].get_city_tiles_list(vec_city_tiles_defender);

    for(int tile_i=0;tile_i<(int)vec_city_tiles_attacker.size();tile_i++)
    {//test if tile next to this one is part of defender's city
        for(int x_add=-1;x_add<2;x_add++)
        for(int y_add=-1;y_add<2;y_add++)
        {
            if(x_add==y_add) continue;//avoid +1,+1 0,0 -1,-1
            int new_x=vec_city_tiles_attacker[tile_i].x_hex+x_add;
            int new_y=vec_city_tiles_attacker[tile_i].y_hex+y_add;
            //border jump test
            if(new_x<0 || new_x>=_world_size_x || new_y<0 || new_y>=_world_size_y)
            {//adjust hex loop value
                if(new_x<0) new_x+=_world_size_x;
                if(new_x>=_world_size_x) new_x-=_world_size_x;
                if(new_y<0) new_y+=_world_size_y;
                if(new_y>=_world_size_y) new_y-=_world_size_y;
            }


            for(int tile_i_def=0;tile_i_def<(int)vec_city_tiles_defender.size();tile_i_def++)
            {
                if( new_x==vec_city_tiles_defender[tile_i_def].x_hex &&
                    new_y==vec_city_tiles_defender[tile_i_def].y_hex )
                {
                    numof_contest_tiles++;
                }
            }
        }
    }

    //count the number of free land tiles that the defender have
    int numof_free_tiles=0;
    for(int tile_i=0;tile_i<(int)vec_city_tiles_defender.size();tile_i++)
    {//test if tile next to this one is land without city
        for(int x_add=-1;x_add<2;x_add++)
        for(int y_add=-1;y_add<2;y_add++)
        {
            if(x_add==y_add) continue;//avoid +1,+1 0,0 -1,-1
            int new_x=vec_city_tiles_defender[tile_i].x_hex+x_add;
            int new_y=vec_city_tiles_defender[tile_i].y_hex+y_add;
            //border jump test
            if(new_x<0 || new_x>=_world_size_x || new_y<0 || new_y>=_world_size_y)
            {//adjust hex loop value
                if(new_x<0) new_x+=_world_size_x;
                if(new_x>=_world_size_x) new_x-=_world_size_x;
                if(new_y<0) new_y+=_world_size_y;
                if(new_y>=_world_size_y) new_y-=_world_size_y;
            }

            //test if tile next to this one is part of defender's city
            if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_land ||
                m_arr_tiles[new_x][new_y].get_tile_type()==tt_land_w_road )
            {
                numof_free_tiles++;
            }

        }
    }

    if( numof_contest_tiles>numof_free_tiles )
    {//merge
        return true;
    }

    return false;
}

bool world::remove_city(int city_index)
{
    cout<<"City Removal: City index: "<<city_index<<endl;
    //remove trades
    int city_id=m_vec_cities[city_index].get_city_id();
    for(int i=0;i<(int)m_vec_trades.size();i++)
    {
        if( m_vec_trades[i].get_city_a_id()==city_id || m_vec_trades[i].get_city_b_id()==city_id )
        {//trade will be removed
            //tell tiles about removal of road
            vector<st_coord_route> vec_trade_route;
            m_vec_trades[i].get_trade_route_list(vec_trade_route);
            int trade_id=m_vec_trades[i].get_trade_id();
            for(int tile_i=1;tile_i<(int)vec_trade_route.size()-1;tile_i++)//skip first and last, part of city
            {
                //reset to land or water
                int temp_tt=m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].get_tile_type();
                if(temp_tt==tt_land_w_road)
                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_land,trade_id);
                if(temp_tt==tt_water_w_road)
                    m_arr_tiles[ vec_trade_route[tile_i].x_hex ][ vec_trade_route[tile_i].y_hex ].set_tile_type(tt_water,trade_id);
            }
            m_vec_trades.erase( m_vec_trades.begin()+i );
            i--;
        }
    }

    //remove city
    m_vec_cities.erase( m_vec_cities.begin()+city_index );

    return true;
}

bool world::place_start_cities(int numof_players)
{
    vector<st_coord> vec_city_pos;
    int shortest_side=_world_size_x;
    if(_world_size_x>_world_size_y) shortest_side=_world_size_y;
    int min_distance=shortest_side/(numof_players+2);
    if(min_distance<1) min_distance=1;

    int numof_tries=1000;
    bool retry=false;

    for(int dist_step_i=0;dist_step_i<5;dist_step_i++)
    {
        for(int try_i=0;try_i<numof_tries;try_i++)
        {
            retry=false;
            vec_city_pos.clear();
            for(int player_i=0;player_i<numof_players;player_i++)
            {
                //place city at random pos
                int x_pos=rand()%_world_size_x;
                int y_pos=rand()%_world_size_y;
                st_coord temp_pos(x_pos,y_pos);

                //test if terrain is land
                if( m_arr_tiles[x_pos][y_pos].get_tile_type()!=tt_land )
                {
                    retry=true;
                }

                //test if distance to other cities is far enough
                for(int city_i=0;city_i<(int)vec_city_pos.size();city_i++)
                {
                    if( vec_city_pos[city_i].distance( temp_pos )<min_distance )
                    {
                        retry=true;
                        break;
                    }
                }
                if(retry) break;
                else vec_city_pos.push_back(temp_pos);//store pos
            }
            if(!retry) break;
        }

        if(retry)//decrese min distance
        {
            min_distance/=2;
            if(min_distance<1) min_distance=1;
        }
        else//test if connected
        {
            bool connected=true;
            bool tile_breaker=false;//allows tiles to be modified, only if cant make connection
            //start at city i pos
            for(int city_index_curr=0;city_index_curr<(int)vec_city_pos.size()-1;city_index_curr++)
            {
                cout<<"World Gen: Trying to connect city "<<city_index_curr+1<<" with city "<<city_index_curr+2<<endl;
                vector<st_coord_route> vec_path;
                vec_path.push_back( st_coord_route(vec_city_pos[city_index_curr].x_hex,
                                                   vec_city_pos[city_index_curr].y_hex) );
                //take steps to reach city i+1
                st_coord_route target_coord(vec_city_pos[city_index_curr+1].x_hex,
                                            vec_city_pos[city_index_curr+1].y_hex);
                int x_pos_curr=vec_path.front().x_hex;
                int y_pos_curr=vec_path.front().y_hex;
                vector<st_coord_route> vec_banned_pos;
                //start stepping
                while(x_pos_curr!=target_coord.x_hex && y_pos_curr!=target_coord.y_hex)
                {
                    //get all step possibilities
                    vector<st_coord_route> vec_possible_tiles;
                    for(int x_step=-1;x_step<2;x_step++)
                    for(int y_step=-1;y_step<2;y_step++)
                    {
                        if(x_step==y_step) continue;
                        int x_pos_new=x_pos_curr+x_step;
                        int y_pos_new=y_pos_curr+y_step;
                        if(x_pos_new<0 || x_pos_new>=_world_size_x ||
                           y_pos_new<0 || y_pos_new>=_world_size_y) continue;
                        //test if banned
                        bool bad_tile=false;
                        for(int tile_i=0;tile_i<(int)vec_banned_pos.size();tile_i++)
                        {
                            if( vec_banned_pos[tile_i].x_hex==x_pos_new &&
                                vec_banned_pos[tile_i].y_hex==y_pos_new )
                            {
                                bad_tile=true;
                                break;
                            }
                        }
                        if(bad_tile) continue;//skip this tile
                        //test if tile already part of path
                        for(int tile_i=0;tile_i<(int)vec_path.size();tile_i++)
                        {
                            if( vec_path[tile_i].x_hex==x_pos_new &&
                                vec_path[tile_i].y_hex==y_pos_new )
                            {
                                bad_tile=true;
                                break;
                            }
                        }
                        if(bad_tile) continue;//skip this tile

                        if(m_arr_tiles[x_pos_new][y_pos_new].get_tile_type()!=tt_rock)
                        {//tile ok
                            vec_possible_tiles.push_back( st_coord_route(x_pos_new,y_pos_new) );
                        }
                        else if(tile_breaker)
                        {
                            //change tt to land and add
                            m_arr_tiles[x_pos_new][y_pos_new].set_tile_type(tt_land,-1);
                            vec_possible_tiles.push_back( st_coord_route(x_pos_new,y_pos_new) );
                        }
                    }
                    if(vec_possible_tiles.empty())
                    {
                        cout<<"World Gen: City connection: No possible tiles to step on\n";
                        //if cant step back, cities not connected
                        if( (int)vec_path.size()<2 )//only start coord left, cant step back
                        {
                            connected=false;
                            break;
                        }

                        //put current pos in banned pos vector and take step back
                        vec_banned_pos.push_back( vec_path.back() );
                        vec_path.pop_back();
                        x_pos_curr=vec_path.back().x_hex;
                        y_pos_curr=vec_path.back().y_hex;
                        continue; //back to while start
                    }
                    //pick one alternative, one with shortest distance to target
                    int shortest_dist=target_coord.distance( vec_possible_tiles.front() );
                    int shortest_dist_index=0;
                    for(int i=0;i<(int)vec_possible_tiles.size();i++)
                    {
                        if( target_coord.distance( vec_possible_tiles[i] ) < shortest_dist )
                        {
                            shortest_dist_index=i;
                            shortest_dist=target_coord.distance( vec_possible_tiles[i] );
                        }
                    }
                    //save step
                    vec_path.push_back(vec_possible_tiles[shortest_dist_index]);
                    x_pos_curr=vec_path.back().x_hex;
                    y_pos_curr=vec_path.back().y_hex;
                }

                if(!connected)//could not connect cities
                {
                    //repeat connection step and remove unbuildable terrain
                    cout<<"World Gen: City connection: Could not connect cities, enable tile breaker\n";
                    tile_breaker=true;
                    city_index_curr--;
                    continue;
                }
                else tile_breaker=false;//reset flag to not modify tiles anymore

                //go to next city and connect that to the city after that
                cout<<"World Gen: City "<<city_index_curr+1<<" connected with "<<city_index_curr+2<<endl;
            }
            if(!connected)//could not connect cities
            {
                //retry city placement
                cout<<"World Gen: Cities could not be connected with current placement, new positions required\n";
                dist_step_i--;
                continue;
            }

            //cities are placed with distance and are connected, assign cities pos
            cout<<"World Gen: Placing start cities at: "<<endl;
            for(int player_i=0;player_i<numof_players;player_i++)
            {
                cout<<vec_city_pos[player_i].x_hex<<", "<<vec_city_pos[player_i].y_hex<<endl;
                int id=m_id_counter_city++;

                m_vec_cities.push_back( city( vec_city_pos[player_i].x_hex,vec_city_pos[player_i].y_hex ,
                                              player_i,id,m_vec_trades,&m_arr_tiles[0][0],m_pSound) );

                m_arr_tiles[ vec_city_pos[player_i].x_hex ][ vec_city_pos[player_i].y_hex ].set_owner(player_i,id);
            }
            break;
        }
    }

    cout<<"World Gen: Placing start cities done\n";

    //could not place cities
    if( (int)vec_city_pos.size()!=numof_players ) return false;

    //set start pos for cam
    if( *m_pPlayer_color_id<0 || *m_pPlayer_color_id>=numof_players ) return false;
    int start_pos_x=vec_city_pos[*m_pPlayer_color_id].x_hex;
    int start_pos_y=vec_city_pos[*m_pPlayer_color_id].y_hex;
    m_start_pos_pix[0]=(start_pos_x+start_pos_y*0.5)*_zoom_start-m_window_width*0.5;
    m_start_pos_pix[1]=start_pos_y*0.75*_zoom_start-m_window_height*0.5;

    return true;
}

bool world::find_city_growth_place(int city_id,int pos[2])
{
    //find city
    int city_index=-1;
    for(int city_i=0;city_i<(int)m_vec_cities.size();city_i++)
    {
        if( m_vec_cities[city_i].get_city_id()==city_id )
        {
            city_index=city_i;
            break;
        }
    }
    if(city_index==-1)
    {
        cout<<"Error: City growth: Could not find city\n";
        return false;
    }

    //find empty tiles
    int x_hex,y_hex;
    m_vec_cities[city_index].get_center_hex_coordinates(x_hex,y_hex);//get hex coordinates
    //cout<<"Expansion: Request for X: "<<x_hex<<" Y: "<<y_hex<<endl;
    //find alternatives around city center
    vector<st_coord_chararr> possible_tiles;
    for(int x_add=-1;x_add<2;x_add++)
    for(int y_add=-1;y_add<2;y_add++)
    {
        if(x_add==y_add) continue; //skip -1,-1 and +1,+1 and center 0,0
        if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
        {//if tile is on the border to another box, special test required
            int new_x=x_hex+x_add;
            int new_y=y_hex+y_add;

            if(new_x<0) new_x+=_world_size_x;
            else if(new_x>=_world_size_x) new_x-=_world_size_x;
            if(new_y<0) new_y+=_world_size_y;
            else if(new_y>=_world_size_y) new_y-=_world_size_y;

            if( m_arr_tiles[new_x][new_y].is_buildable() ) possible_tiles.push_back( st_coord_chararr(new_x,new_y) );
        }
        else if( m_arr_tiles[x_hex+x_add][y_hex+y_add].is_buildable() ) possible_tiles.push_back( st_coord_chararr(x_hex+x_add,y_hex+y_add) );
    }

    //if no tile avaliable, try to expand to outer layer of city
    if( possible_tiles.empty() )
    {
        //get list of all city tiles
        vector<st_coord_chararr> city_tiles;
        m_vec_cities[city_index].get_city_tiles_list(city_tiles);
        //go through all city tiles
        for(int city_tile_id=0;city_tile_id<(int)city_tiles.size();city_tile_id++)
        {
            x_hex=city_tiles[city_tile_id].x_hex;
            y_hex=city_tiles[city_tile_id].y_hex;
            //find alternatives
            for(int x_add=-1;x_add<2;x_add++)
            for(int y_add=-1;y_add<2;y_add++)
            {
                if(x_add==y_add) continue; //skip -1,-1 and +1,+1 and center 0,0
                //if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) continue;//coutide map

                if(x_hex+x_add<0 || y_hex+y_add<0 || x_hex+x_add>=_world_size_x || y_hex+y_add>=_world_size_y) //coutide map
                {//if tile is on the border to another box, special test required
                    int new_x=x_hex+x_add;
                    int new_y=y_hex+y_add;

                    if(new_x<0) new_x+=_world_size_x;
                    else if(new_x>=_world_size_x) new_x-=_world_size_x;
                    if(new_y<0) new_y+=_world_size_y;
                    else if(new_y>=_world_size_y) new_y-=_world_size_y;

                    if( m_arr_tiles[new_x][new_y].is_buildable() ) possible_tiles.push_back( st_coord_chararr(new_x,new_y) );
                    else if( m_arr_tiles[new_x][new_y].get_tile_type()==tt_rock )
                    {//chance of removing rock
                        possible_tiles.push_back( st_coord_chararr(new_x,new_y) );
                    }
                }
                else//not on border
                {
                    if( m_arr_tiles[x_hex+x_add][y_hex+y_add].is_buildable() )
                    {
                        possible_tiles.push_back( st_coord_chararr(x_hex+x_add,y_hex+y_add) );
                    }
                    else if( m_arr_tiles[x_hex+x_add][y_hex+y_add].get_tile_type()==tt_rock )
                    {//chance of removing rock
                        possible_tiles.push_back( st_coord_chararr(x_hex+x_add,y_hex+y_add) );
                    }
                }
            }
        }

    }
    if( !possible_tiles.empty() )//pick random tile
    {
        //cout<<"Expansion: Number of possible tiles: "<<(int)possible_tiles.size()<<endl;
        int numof_tiles=(int)possible_tiles.size();
        float chance_part=1.0/(float)numof_tiles;
        float rand_val=float(rand()%100)/100.0;
        int winner=-1;
        for(int tile_id=0;tile_id<numof_tiles;tile_id++)
        {
            if(rand_val<chance_part*float(tile_id+1))
            {
                winner=tile_id; break;
            }
        }
        if(winner==-1) winner=0;//to be safe
        //cout<<"Expansion: Winning tile index: "<<winner<<endl;

        //Normal expansion
        pos[0]=possible_tiles[winner].x_hex;
        pos[1]=possible_tiles[winner].y_hex;
    }
    else return false;//no room

    return true;
}
