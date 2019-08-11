#include "city.h"

city::city()
{
    m_center_pos_x_pix=m_center_pos_y_pix=0;
    m_center_pos_x_hex=m_center_pos_y_hex=0;
}

city::city(float pos_x_hex,float pos_y_hex,int player_owner,int id,vector<trade>& vec_trades,tile* arr_tiles,sound* pSound)
{
    m_city_id=id;
    m_new_city_id=-1;
    m_waiting_for_response_timer=0.0;

    m_pSound=pSound;

    m_pVec_trades_all=&vec_trades;
    m_p_arr_tiles=arr_tiles;

    m_center_pos_x_pix=pos_x_hex+pos_y_hex/2;
    m_center_pos_y_pix=pos_y_hex*0.75;
    m_center_pos_x_hex=pos_x_hex;
    m_center_pos_y_hex=pos_y_hex;
    m_player_owner=player_owner;
    m_starve_progress=m_growth_progress=m_idle_progress=m_lifetime=m_mission_expansion_time=m_mission_trade_time=0;
    m_city_mode_pop=cm_growth;
    m_city_mode_work=cm_idle;
    m_vec_city_tiles.push_back( st_coord_chararr(pos_x_hex,pos_y_hex) );
    m_recalc_border=true;
    m_pop_adjust_timer=_pop_adjust_time_check;
    m_next_growth_time=float(rand()%1001)/1000.0*_city_growth_time+_city_growth_time;
    m_next_starve_time=float(rand()%1001)/1000.0*_city_starve_time+_city_starve_time;
    //m_forced_idle_test=false;
}

int city::update(float time,bool mouse_buttons[4],int selected_tile_hex[2],int selected_tile_type,bool city_owner,
                 int num_of_trades,int longest_trade_distance)
{
    m_lifetime+=time;

    //population time update
    if(m_pop_adjust_timer>0) m_pop_adjust_timer-=time*(float)m_vec_city_tiles.size();
    else//pop check
    {
        m_pop_adjust_timer=_pop_adjust_time_check;
        adjust_pop_count();
    }

    if(city_owner)//update is called by this city's owner, input possible
    {
        //test if city is clicked LMB (set to listen for expansion mission)
        if(mouse_buttons[0] && m_city_mode_work==cm_idle)//can only expanding if growing or starving (not while other mission)
        {
            //test if this cities tiles are involved
            for(int i=0;i<(int)m_vec_city_tiles.size();i++)
            {
                if(m_vec_city_tiles[i].x_hex==selected_tile_hex[0] && m_vec_city_tiles[i].y_hex==selected_tile_hex[1])
                {//this city is involved, listen for expansion route
                    m_city_mode_work=cm_listen_expansion;
                    //add hex to route vector
                    m_vec_expansion_route.clear();
                    m_vec_expansion_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],0,0) );
                }
            }
        }

        //test if city is clicked RMB (set to listen for trade mission)
        if(mouse_buttons[1] && m_city_mode_work==cm_idle)//can only expanding if growing or starving (not while other mission)
        {
            //test if this cities tiles are involved
            for(int i=0;i<(int)m_vec_city_tiles.size();i++)
            {
                if(m_vec_city_tiles[i].x_hex==selected_tile_hex[0] && m_vec_city_tiles[i].y_hex==selected_tile_hex[1])
                {//this city is involved, listen for trade route
                    m_city_mode_work=cm_listen_trade;
                    //add hex to route vector
                    m_vec_trade_route.clear();
                    m_vec_trade_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],0,0) );
                }
            }
        }
    }

    if(m_recalc_border)
    {
        recalc_border();
    }

    if( m_city_mode_pop!=-1 )//growth/starve test, (-1 == off)
    {
        int city_size=(int)m_vec_city_tiles.size();
        int max_size=get_city_max_size(num_of_trades,longest_trade_distance);
        if(max_size==0)//no limit
        {
            m_city_mode_pop=cm_growth;
        }
        else//follow limit
        {
            if( city_size>max_size ) m_city_mode_pop=cm_starve;
            if( city_size<max_size ) m_city_mode_pop=cm_growth;
            if( city_size==max_size ) m_city_mode_pop=cm_idle;
        }
    }
    /*if( m_forced_idle_test )
    {
        m_forced_idle_test=false;
        m_city_mode=cm_forced_idle;
    }*/

    //update pop dots
    for(int tile_i=0;tile_i<(int)m_vec_city_tiles.size();tile_i++)
     for(int pop=0;pop<(int)m_vec_city_tiles[tile_i].pop_pos.size();pop++)
    {
        //adjust brightness
        if( m_vec_city_tiles[tile_i].pop_pos[pop].flag_remove )
        {//lower brightness to 0.1, then remove
            if( m_vec_city_tiles[tile_i].pop_pos[pop].brightness<0.1 )
            {
                m_vec_city_tiles[tile_i].pop_pos.erase( m_vec_city_tiles[tile_i].pop_pos.begin()+pop );
                pop--;
            }
            else m_vec_city_tiles[tile_i].pop_pos[pop].brightness-=time/2.0;
        }
        else if (m_vec_city_tiles[tile_i].pop_pos[pop].flag_new)
        {//increase brightness untill 0.9, then not new
            if( m_vec_city_tiles[tile_i].pop_pos[pop].brightness>0.9 )
            {
                m_vec_city_tiles[tile_i].pop_pos[pop].flag_new=false;
            }
            else m_vec_city_tiles[tile_i].pop_pos[pop].brightness+=time/2.0;
        }
        else//change brightness up/down
        {
            m_vec_city_tiles[tile_i].pop_pos[pop].brightness=0.5+(cosf( m_vec_city_tiles[tile_i].pop_pos[pop].lifetime )+1.0)/4.0;
            m_vec_city_tiles[tile_i].pop_pos[pop].lifetime+=time;
        }
    }

    switch(m_city_mode_pop)
    {
        case cm_growth://
        {
            //growth test
            m_growth_progress+=time;
            if(m_growth_progress>m_next_growth_time)
            {
                //reset timers
                m_next_growth_time=float(rand()%1001)/1000.0*_city_growth_time+_city_growth_time;
                m_growth_progress=0;
                m_starve_progress=0;
                m_idle_progress=0;
                m_recalc_border=true;
                //add new tile to city
                return 1;
            }
        }break;

        case cm_starve://
        {
            //starve test
            m_starve_progress+=time;
            if(m_starve_progress>m_next_starve_time)
            {
                //reset timers
                m_next_starve_time=float(rand()%1001)/1000.0*_city_starve_time+_city_starve_time;
                m_growth_progress=0;
                m_starve_progress=0;
                m_idle_progress=0;
                m_recalc_border=true;
                //remove tile from city

                //select tile to remove, furthest from city center
                int x_hex,y_hex;
                get_random_city_tile_furthest_from_center(x_hex,y_hex);

                //tell World
                m_starve_hex_x=x_hex;
                m_starve_hex_y=y_hex;

                return 5;
            }
        }break;

        case cm_idle://no growth or starvation
        {
            //idle test
            m_idle_progress+=time;
            if(m_idle_progress>1.0)
            {
                //reset timers
                m_growth_progress=0;
                m_starve_progress=0;
                m_idle_progress=0;

                return 6;
            }
        }break;

        /*case cm_forced_idle://called when city could not grow during growth test
        {
            //reset timers
            m_growth_progress=-1.0;//longer delay
            m_starve_progress=0;
            m_idle_progress=0;
            //force idle test
            return 6;
        }break;*/

    }

    switch(m_city_mode_work)
    {
        case cm_idle://do nothing
        {
            ;
        }break;

        case cm_listen_expansion://
        {
            //test if route is finished (LMB not down)
            if(!mouse_buttons[0])
            {//done
                //test so that last tile buildable, otherwise cancel route and return to other state
                if(selected_tile_type==tt_land)//land without city
                {
                    m_city_mode_work=cm_waiting_for_response;
                    m_waiting_for_response_timer=0;
                    return 7;
                }
                else//cancel route
                {
                    m_city_mode_work=cm_idle;
                    m_vec_expansion_route.clear();
                }
            }
            else//continue route
            {
                //test length
                if( (int)m_vec_expansion_route.size()>_max_route_length )
                {
                    m_city_mode_work=cm_idle;
                    m_vec_expansion_route.clear();
                    break;
                }

                //test if valid tile is selected
                if(selected_tile_hex[0]==-1 || selected_tile_hex[1]==-1) break;

                //test if invalid tile type, unless this is the first step (located in start city)
                if( selected_tile_type==tt_land_w_city || selected_tile_type==tt_rock )
                {//on rock or city
                    //exception
                    if( (int)m_vec_expansion_route.size()==1 && selected_tile_type==tt_land_w_city );//start city
                    else
                    {//dont add tile to path, wait for next tile instead (old: remove route)
                        //m_city_mode_work=cm_idle;
                        //m_vec_expansion_route.clear();
                        break;
                    }
                }

                //test if new tile selected
                if(m_vec_expansion_route.back().x_hex==selected_tile_hex[0] && m_vec_expansion_route.back().y_hex==selected_tile_hex[1])
                {//same tile, break
                    break;//nothing new
                }
                //else cout<<"Expansion: New tile found:  X: "<<selected_tile_hex[0]<<"  Y: "<<selected_tile_hex[1]<<endl;

                //test if new tile is OK, next to previous tile
                int box_cross_x=0; int box_cross_y=0;
                int x_hex_dif=selected_tile_hex[0]-m_vec_expansion_route.back().x_hex;
                int y_hex_dif=selected_tile_hex[1]-m_vec_expansion_route.back().y_hex;
                if(x_hex_dif<-1 || x_hex_dif>1 || y_hex_dif<-1 || y_hex_dif>1 || x_hex_dif==y_hex_dif)//pos not OK
                {//outside borders of last tile, or crossed to another box
                    //test if last place was close to a border
                    int last_x=m_vec_expansion_route.back().x_hex;
                    int last_y=m_vec_expansion_route.back().y_hex;
                    int new_x=selected_tile_hex[0];
                    int new_y=selected_tile_hex[1];
                    if( last_x==_world_size_x-1 && new_x==0 ) box_cross_x=1;
                    else if( last_x==0 && new_x==_world_size_x-1 ) box_cross_x=-1;
                    if( last_y==_world_size_y-1 && new_y==0 ) box_cross_y=1;
                    else if( last_y==0 && new_y==_world_size_y-1 ) box_cross_y=-1;
                    //if none above is true then new tile is to far away
                    if( box_cross_x==0 && box_cross_y==0 )
                    {//new tile within same box
                        //try to make path
                        vector<st_coord_route> temp_path;
                        int first_direction_out=0;
                        if( make_path(temp_path,first_direction_out,new_x,new_y,last_x,last_y,1,
                                      m_vec_expansion_route.back().x_box_shift,m_vec_expansion_route.back().y_box_shift) )
                        {
                            //have path, have been tested, just to add to exp route
                            m_vec_expansion_route.back().road[1]=first_direction_out;
                            m_vec_expansion_route.insert( m_vec_expansion_route.end(), temp_path.begin(), temp_path.end() );
                            //play sound
                            //m_pSound->playSimpleSound(wav_add_path,1.0);
                            break;
                        }
                        else//no path could be made
                        {
                            //cancel route
                            cout<<"Expansion: New tile is too far away...\n";
                            m_city_mode_work=cm_idle;
                            m_vec_expansion_route.clear();
                            break;
                        }
                    }
                    //border was crossed, test if new tile is close enough
                    //adjust hex values according to box shift
                    if(box_cross_x==1) new_x+=_world_size_x;
                    else if(box_cross_x==-1) new_x-=_world_size_x;
                    if(box_cross_y==1) new_y+=_world_size_y;
                    else if(box_cross_y==-1) new_y-=_world_size_y;
                    //test distance again
                    x_hex_dif=new_x-last_x;
                    y_hex_dif=new_y-last_y;
                    if( x_hex_dif<-1 || x_hex_dif>1 || y_hex_dif<-1 || y_hex_dif>1 || x_hex_dif==y_hex_dif )//pos not OK
                    {//still too far away
                        //try to make path
                        vector<st_coord_route> temp_path;
                        int first_direction_out=0;
                        if( make_path(temp_path,first_direction_out,new_x,new_y,last_x,last_y,1,
                                      m_vec_expansion_route.back().x_box_shift,m_vec_expansion_route.back().y_box_shift) )
                        {
                            //have path, have been tested, just to add to exp route
                            m_vec_expansion_route.back().road[1]=first_direction_out;
                            m_vec_expansion_route.insert( m_vec_expansion_route.end(), temp_path.begin(), temp_path.end() );
                            //play sound
                            //m_pSound->playSimpleSound(wav_add_path,1.0);
                            break;
                        }
                        else//no path could be made
                        {
                            //cancel route
                            cout<<"Expansion: New tile is too far away...\n";
                            m_city_mode_work=cm_idle;
                            m_vec_expansion_route.clear();
                            break;
                        }
                    }
                    //else border crossed safely, continue
                }

                //test if new tile is already part of expansion route
                bool tile_found_in_path=false;
                for(int tile_i=0;tile_i<(int)m_vec_expansion_route.size();tile_i++)
                {
                    if(m_vec_expansion_route[tile_i].x_hex==selected_tile_hex[0] && m_vec_expansion_route[tile_i].y_hex==selected_tile_hex[1])
                    {//remove route tile beyond this tile
                        m_vec_expansion_route.erase( m_vec_expansion_route.begin()+tile_i+1,m_vec_expansion_route.end() );
                        tile_found_in_path=true;
                        break;
                    }
                }
                if(tile_found_in_path) break;

                //not a city, test if tile is buildable
                if(selected_tile_type==tt_land || selected_tile_type==tt_water)
                {//tile type ok, add to route
                    int last_box_shift_x=m_vec_expansion_route.back().x_box_shift;
                    int last_box_shift_y=m_vec_expansion_route.back().y_box_shift;
                    int direction_this_in=-1;
                    int direction_last_out=-1;
                    int dif_x=(m_vec_expansion_route.back().x_hex) - (selected_tile_hex[0]+box_cross_x*_world_size_x);
                    int dif_y=(m_vec_expansion_route.back().y_hex) - (selected_tile_hex[1]+box_cross_y*_world_size_y);

                    if(dif_x<-1 || dif_x>1 || dif_y<-1 || dif_y>1 || dif_x==dif_y)//bad
                    {//cancel route
                        cout<<"ERROR: Expansion route: Bad tile shift: "<<dif_x<<", "<<dif_y<<endl;
                        m_city_mode_work=cm_idle;
                        m_vec_expansion_route.clear();
                        break;
                    }

                    switch(dif_x)
                    {
                        case -1:
                        {
                            switch(dif_y)
                            {
                                case 0: direction_this_in=1; direction_last_out=4; break;
                                case 1: direction_this_in=0; direction_last_out=3; break;
                            }
                        }break;

                        case 0:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=2; direction_last_out=5; break;
                                case 1: direction_this_in=5; direction_last_out=2; break;
                            }
                        }break;

                        case 1:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=3; direction_last_out=0; break;
                                case 0: direction_this_in=4; direction_last_out=1; break;
                            }
                        }break;
                    }

                    //assign last out dir
                    m_vec_expansion_route.back().road[1]=direction_last_out;
                    //store tile
                    m_vec_expansion_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],
                                                                    last_box_shift_x+box_cross_x, last_box_shift_y+box_cross_y) );
                    //assign this in dir
                    m_vec_expansion_route.back().road[0]=direction_this_in;

                    //play sound
                    //m_pSound->playSimpleSound(wav_add_path,1.0);

                    break;//done
                }
            }

        }break;

        case cm_listen_trade://
        {
            //test if route is finished (RMB not down)
            if(!mouse_buttons[1])
            {//done, if last tile was another city, route should have been build, therefore cancel route if RMB released
                m_city_mode_work=cm_idle;
                m_vec_trade_route.clear();
            }
            else//continue route
            {
                //test length
                if( (int)m_vec_trade_route.size()>_max_route_length )
                {
                    m_city_mode_work=cm_idle;
                    m_vec_trade_route.clear();
                    break;
                }

                //test if valid tile is selected
                if(selected_tile_hex[0]==-1 || selected_tile_hex[1]==-1) break;

                //test if invalid tile type, unless this is the first step (located in start city)
                if( selected_tile_type==tt_rock )
                {//dont add tile to path, wait for next pos (old: remove route)
                    //m_city_mode_work=cm_idle;
                    //m_vec_trade_route.clear();
                    break;
                }

                //test if new tile selected
                if(m_vec_trade_route.back().x_hex==selected_tile_hex[0] && m_vec_trade_route.back().y_hex==selected_tile_hex[1])
                {//same tile, break
                    break;//nothing new
                }

                int box_cross_x=0; int box_cross_y=0;
                int x_hex_dif=selected_tile_hex[0]-m_vec_trade_route.back().x_hex;
                int y_hex_dif=selected_tile_hex[1]-m_vec_trade_route.back().y_hex;
                if(x_hex_dif<-1 || x_hex_dif>1 || y_hex_dif<-1 || y_hex_dif>1 || x_hex_dif==y_hex_dif)//pos not OK
                {//outside borders of last tile, or crossed to another box
                    //test if last place was close to a border
                    int last_x=m_vec_trade_route.back().x_hex;
                    int last_y=m_vec_trade_route.back().y_hex;
                    int new_x=selected_tile_hex[0];
                    int new_y=selected_tile_hex[1];
                    if( last_x==_world_size_x-1 && new_x==0 ) box_cross_x=1;
                    else if( last_x==0 && new_x==_world_size_x-1 ) box_cross_x=-1;
                    if( last_y==_world_size_y-1 && new_y==0 ) box_cross_y=1;
                    else if( last_y==0 && new_y==_world_size_y-1 ) box_cross_y=-1;
                    //if none above is true then new tile is to far away
                    if( box_cross_x==0 && box_cross_y==0 )
                    {//new tile within same box
                        //try to make path
                        vector<st_coord_route> temp_path;
                        int first_direction_out=0;
                        if( make_path(temp_path,first_direction_out,new_x,new_y,last_x,last_y,2,
                                      m_vec_trade_route.back().x_box_shift,m_vec_trade_route.back().y_box_shift) )
                        {
                            //have path, have been tested, just to add to exp route
                            m_vec_trade_route.back().road[1]=first_direction_out;
                            m_vec_trade_route.insert( m_vec_trade_route.end(), temp_path.begin(), temp_path.end() );
                            //play sound
                            //m_pSound->playSimpleSound(wav_add_path,1.0);
                            break;
                        }
                        else//no path could be made
                        {
                            //cancel route
                            cout<<"Trade route: New tile is too far away...\n";
                            m_city_mode_work=cm_idle;
                            m_vec_trade_route.clear();
                            break;
                        }
                    }
                    //border was crossed, test if new tile is close enough
                    //adjust hex values according to box shift
                    if(box_cross_x==1) new_x+=_world_size_x;
                    else if(box_cross_x==-1) new_x-=_world_size_x;
                    if(box_cross_y==1) new_y+=_world_size_y;
                    else if(box_cross_y==-1) new_y-=_world_size_y;
                    //test distance again
                    x_hex_dif=new_x-last_x;
                    y_hex_dif=new_y-last_y;
                    if( x_hex_dif<-1 || x_hex_dif>1 || y_hex_dif<-1 || y_hex_dif>1 || x_hex_dif==y_hex_dif )//pos not OK
                    {//still too far away
                        //try to make path
                        vector<st_coord_route> temp_path;
                        int first_direction_out=0;
                        if( make_path(temp_path,first_direction_out,new_x,new_y,last_x,last_y,2,
                                      m_vec_trade_route.back().x_box_shift,m_vec_trade_route.back().y_box_shift) )
                        {
                            //have path, have been tested, just to add to exp route
                            m_vec_trade_route.back().road[1]=first_direction_out;
                            m_vec_trade_route.insert( m_vec_trade_route.end(), temp_path.begin(), temp_path.end() );
                            //play sound
                            //m_pSound->playSimpleSound(wav_add_path,1.0);
                            break;
                        }
                        else//no path could be made
                        {
                            //cancel route
                            cout<<"Trade route: New tile is too far away...\n";
                            m_city_mode_work=cm_idle;
                            m_vec_trade_route.clear();
                            break;
                        }
                    }
                    //else border crossed safely, continue
                }

                //test if new tile is already part of the trade route
                bool tile_found_in_path=false;
                for(int tile_i=0;tile_i<(int)m_vec_trade_route.size();tile_i++)
                {
                    if(m_vec_trade_route[tile_i].x_hex==selected_tile_hex[0] && m_vec_trade_route[tile_i].y_hex==selected_tile_hex[1])
                    {//remove route tile beyond this tile
                        m_vec_trade_route.erase( m_vec_trade_route.begin()+tile_i+1,m_vec_trade_route.end() );
                        tile_found_in_path=true;
                        break;
                    }
                }
                if(tile_found_in_path) break;

                //test if tile is another city
                if(selected_tile_type==tt_land_w_city)
                {//is part of a city
                    //test if part of this city
                    bool same_city=false;
                    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
                    {
                        if(m_vec_city_tiles[i].x_hex==selected_tile_hex[0] && m_vec_city_tiles[i].y_hex==selected_tile_hex[1])
                        {//route to same city, cancel route
                            same_city=true;
                            m_city_mode_work=cm_idle;
                            m_vec_trade_route.clear();
                            break;
                        }
                    }
                    if(!same_city)//not the same city, maybe already trading with this city?
                    {
                        //test last direction out
                        int last_box_shift_x=m_vec_trade_route.back().x_box_shift;
                        int last_box_shift_y=m_vec_trade_route.back().y_box_shift;
                        int direction_this_in=-1;
                        int direction_last_out=-1;
                        int dif_x=(m_vec_trade_route.back().x_hex) - (selected_tile_hex[0]+box_cross_x*_world_size_x);
                        int dif_y=(m_vec_trade_route.back().y_hex) - (selected_tile_hex[1]+box_cross_y*_world_size_y);

                        if(dif_x<-1 || dif_x>1 || dif_y<-1 || dif_y>1 || dif_x==dif_y)//bad
                        {//cancel route
                            cout<<"ERROR: Trade route: Bad tile shift finishing route\n";
                            m_city_mode_work=cm_idle;
                            m_vec_trade_route.clear();
                            break;
                        }
                        //get directions
                        switch(dif_x)
                        {
                            case -1:
                            {
                                switch(dif_y)
                                {
                                    case 0: direction_this_in=1; direction_last_out=4; break;
                                    case 1: direction_this_in=0; direction_last_out=3; break;
                                }
                            }break;

                            case 0:
                            {
                                switch(dif_y)
                                {
                                    case -1: direction_this_in=2; direction_last_out=5; break;
                                    case 1: direction_this_in=5; direction_last_out=2; break;
                                }
                            }break;

                            case 1:
                            {
                                switch(dif_y)
                                {
                                    case -1: direction_this_in=3; direction_last_out=0; break;
                                    case 0: direction_this_in=4; direction_last_out=1; break;
                                }
                            }break;
                        }
                        //test if last direction out was allowed
                        if( !m_vec_trade_route.back().road_allowed_exits[direction_last_out] )
                        {//that direction not allowed
                            //cancel route
                            cout<<"Trade route listen: That direction not allowed\n";
                            m_city_mode_work=cm_idle;
                            m_vec_trade_route.clear();
                            break;
                        }

                        //trade route finished

                        //assign last exit to road
                        m_vec_trade_route.back().road[1]=direction_last_out;

                        m_vec_trade_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],
                                                                    last_box_shift_x+box_cross_x, last_box_shift_y+box_cross_y) );
                        //assign this in dir
                        m_vec_trade_route.back().road[0]=direction_this_in;

                        m_city_mode_work=cm_trade_mission;
                        cout<<"Trade route listen: Trade route finished\n";

                        //copy route to new vector
                        m_vec_new_trade_route.clear();
                        m_vec_new_trade_route.insert(m_vec_new_trade_route.begin(),m_vec_trade_route.begin(),m_vec_trade_route.end());
                        m_vec_trade_route.clear();

                        return 4;//world will test if this city already is trading with that city
                        //break;
                    }
                    else break;//same city, break
                }

                //not a city, test if tile is buildable
                if(selected_tile_type==tt_land || selected_tile_type==tt_water)
                {//tile type ok, add to route
                    //add road direction for previous and present tile
                    int last_box_shift_x=m_vec_trade_route.back().x_box_shift;
                    int last_box_shift_y=m_vec_trade_route.back().y_box_shift;
                    int direction_this_in=-1;
                    int direction_last_out=-1;
                    int dif_x=(m_vec_trade_route.back().x_hex) - (selected_tile_hex[0]+box_cross_x*_world_size_x);
                    int dif_y=(m_vec_trade_route.back().y_hex) - (selected_tile_hex[1]+box_cross_y*_world_size_y);

                    if(dif_x<-1 || dif_x>1 || dif_y<-1 || dif_y>1 || dif_x==dif_y)//bad
                    {//cancel route
                        cout<<"ERROR: Trade route: Bad tile shift: "<<dif_x<<", "<<dif_y<<endl;
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }

                    switch(dif_x)
                    {
                        case -1:
                        {
                            switch(dif_y)
                            {
                                case 0: direction_this_in=1; direction_last_out=4; break;
                                case 1: direction_this_in=0; direction_last_out=3; break;
                            }
                        }break;

                        case 0:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=2; direction_last_out=5; break;
                                case 1: direction_this_in=5; direction_last_out=2; break;
                            }
                        }break;

                        case 1:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=3; direction_last_out=0; break;
                                case 0: direction_this_in=4; direction_last_out=1; break;
                            }
                        }break;
                    }

                    //test if last direction out was allowed
                    if( !m_vec_trade_route.back().road_allowed_exits[direction_last_out] )
                    {//that direction not allowed
                        //cancel route
                        cout<<"Trade route listen: That direction not allowed\n";
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }

                    //assign last out dir
                    m_vec_trade_route.back().road[1]=direction_last_out;

                    m_vec_trade_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],
                                                                last_box_shift_x+box_cross_x, last_box_shift_y+box_cross_y) );
                    //assign this in dir
                    m_vec_trade_route.back().road[0]=direction_this_in;

                    //play sound
                    //m_pSound->playSimpleSound(wav_add_path,1.0);

                    break;
                }

                //more tests if multiple roads on same tile
                if(selected_tile_type==tt_land_w_road || selected_tile_type==tt_water_w_road)
                {
                    cout<<"Trade route: Trying to build on old road\n";
                    //test if 3 roads already
                    bool roads_full=false;
                    //find trade(s) that is on this tile
                    int roads[3][2]={-1,-1,-1,-1,-1,-1};//info about older roads
                    int road_counter=0;
                    for(int trade_i=0;trade_i<(int)m_pVec_trades_all->size();trade_i++)
                    {
                        if(roads_full) break;
                        vector<st_coord_route> vec_trade_route;
                        (*m_pVec_trades_all)[trade_i].get_trade_route_list(vec_trade_route);
                        for(int tile_i=0;tile_i<(int)vec_trade_route.size();tile_i++)
                        {
                            if(roads_full) break;
                            if( vec_trade_route[tile_i].x_hex==selected_tile_hex[0] &&
                                vec_trade_route[tile_i].y_hex==selected_tile_hex[1] )
                            {//road present, store in/out positions
                                roads[road_counter][0]=vec_trade_route[tile_i].road[0];//in
                                roads[road_counter][1]=vec_trade_route[tile_i].road[1];//out
                                cout<<"Tile found with road on: Used exits: "<<roads[road_counter][0]<<", "<<roads[road_counter][1]<<endl;
                                road_counter++;
                                if(road_counter>=3) roads_full=true;
                            }
                        }
                    }
                    if(roads_full)
                    {//cancel route
                        cout<<"Trade route listen: Already 3 roads on that tile\n";
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }

                    //one more road could be placed here

                    //get direction
                    int last_box_shift_x=m_vec_trade_route.back().x_box_shift;
                    int last_box_shift_y=m_vec_trade_route.back().y_box_shift;
                    int direction_this_in=-1;
                    int direction_last_out=-1;
                    int dif_x=(m_vec_trade_route.back().x_hex) - (selected_tile_hex[0]+box_cross_x*_world_size_x);
                    int dif_y=(m_vec_trade_route.back().y_hex) - (selected_tile_hex[1]+box_cross_y*_world_size_y);

                    if(dif_x<-1 || dif_x>1 || dif_y<-1 || dif_y>1 || dif_x==dif_y)//bad
                    {//cancel road
                        cout<<"ERROR: Trade route: Bad tile shift on road\n";
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }

                    switch(dif_x)
                    {
                        case -1:
                        {
                            switch(dif_y)
                            {
                                case 0: direction_this_in=1; direction_last_out=4; break;
                                case 1: direction_this_in=0; direction_last_out=3; break;
                            }
                        }break;

                        case 0:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=2; direction_last_out=5; break;
                                case 1: direction_this_in=5; direction_last_out=2; break;
                            }
                        }break;

                        case 1:
                        {
                            switch(dif_y)
                            {
                                case -1: direction_this_in=3; direction_last_out=0; break;
                                case 0: direction_this_in=4; direction_last_out=1; break;
                            }
                        }break;
                    }
                    //test if last out direction was allowed
                    if( !m_vec_trade_route.back().road_allowed_exits[direction_last_out] )
                    {//that direction not allowed
                        //cancel route
                        cout<<"Trade route listen: That exit direction not allowed\n";
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }
                    //test if new in direction for this tile is allowed
                    if( roads[0][0]==direction_this_in || roads[0][1]==direction_this_in ||
                        roads[1][0]==direction_this_in || roads[1][1]==direction_this_in )
                    {//this tile can not be entered from that direction
                        cout<<"Trade route listen: That entering direction is not allowed\n";
                        m_city_mode_work=cm_idle;
                        m_vec_trade_route.clear();
                        break;
                    }

                    //road can be placed

                    //assign last exit to road
                    m_vec_trade_route.back().road[1]=direction_last_out;

                    m_vec_trade_route.push_back( st_coord_route(selected_tile_hex[0],selected_tile_hex[1],
                                                                last_box_shift_x+box_cross_x, last_box_shift_y+box_cross_y) );
                    //assign this in dir
                    m_vec_trade_route.back().road[0]=direction_this_in;
                    //lock used roads in/out
                    if( roads[0][0]!=-1 && roads[0][1]!=-1 )//first used road
                    {
                        m_vec_trade_route.back().road_allowed_exits[ roads[0][0] ]=false;
                        m_vec_trade_route.back().road_allowed_exits[ roads[0][1] ]=false;
                        cout<<"This tile can not be exited from: "<<roads[0][0]<<" and "<<roads[0][1]<<endl;
                    }
                    if( roads[1][0]!=-1 && roads[1][1]!=-1 )//second used road, if any third road , this road could not been placed
                    {
                        m_vec_trade_route.back().road_allowed_exits[ roads[1][0] ]=false;
                        m_vec_trade_route.back().road_allowed_exits[ roads[1][1] ]=false;
                        cout<<"This tile can not be exited from: "<<roads[1][0]<<" and "<<roads[1][1]<<endl;
                    }

                    //play sound
                    //m_pSound->playSimpleSound(wav_add_path,1.0);

                    break;
                }
            }

        }break;

        case cm_expansion_mission://
        {
            //calc mission time
            if(m_mission_expansion_time==0)
            {//first time
                m_recalc_border=true;//just lost one tile
                int path_length=(int)m_vec_expansion_route.size();
                m_mission_expansion_time=(float)path_length*_mission_expansion_time;
                m_mission_expansion_progress=0;
                cout<<"Expansion: Route time "<<m_mission_expansion_time<<" seconds\n";
            }

            //update mission progress
            m_mission_expansion_progress+=time;

            //test if done
            if(m_mission_expansion_progress>m_mission_expansion_time)
            {//done
                m_mission_expansion_time=0;
                m_city_mode_work=cm_idle;
                int x_hex_old=m_vec_expansion_route.front().x_hex;
                int y_hex_old=m_vec_expansion_route.front().y_hex;
                int x_hex_new=m_vec_expansion_route.back().x_hex;
                int y_hex_new=m_vec_expansion_route.back().y_hex;
                cout<<"Expansion: Remove city tile at:  X: "<<x_hex_old<<"  Y: "<<y_hex_old<<endl;
                //remove tile from city, is done in world, if ok
                /*bool city_tile_not_found=true;
                for(int i=0;i<(int)m_vec_city_tiles.size();i++)
                {
                    if(m_vec_city_tiles[i].x_hex==x_hex_old && m_vec_city_tiles[i].y_hex==y_hex_old)
                    {
                        m_vec_city_tiles.erase( m_vec_city_tiles.begin()+i );
                        city_tile_not_found=false;
                        break;
                    }
                }
                if(city_tile_not_found) cout<<"ERROR: City expansion, old tile now found!\n";
                */
                m_vec_expansion_route.clear();

                //tell world about this, to fix old tile, and build new city
                m_city_old_hex_x=x_hex_old;
                m_city_old_hex_y=y_hex_old;
                m_city_new_hex_x=x_hex_new;
                m_city_new_hex_y=y_hex_new;

                return 2;
            }

        }break;

        case cm_trade_mission://
        {
            //calc mission time
            if(m_mission_trade_time==0)
            {//first time
                int path_length=(int)m_vec_new_trade_route.size();
                m_mission_trade_time=(float)path_length*_mission_trade_time;
                m_mission_trade_progress=0;
                cout<<"Trade: Route time "<<m_mission_trade_time<<" seconds\n";
                m_mission_trade_progress+=time;
                return 10;//for sound
            }

            //update mission progress
            m_mission_trade_progress+=time;

            //test if done
            if(m_mission_trade_progress>m_mission_trade_time)
            {//done
                m_mission_trade_time=0;
                m_city_mode_work=cm_waiting_for_response;
                m_waiting_for_response_timer=0;
                //tell world about this, to create new trade route
                return 8;
            }
        }break;

        case cm_waiting_for_response://
        {
            m_waiting_for_response_timer+=time;
            if(m_waiting_for_response_timer>_network_city_waiting_for_response_limit)
            {
                //waited too long, stop waiting, reset state
                m_city_mode_work=cm_idle;
                m_vec_expansion_route.clear();
                m_vec_trade_route.clear();
                m_vec_new_trade_route.clear();
                m_waiting_for_response_timer=0;
            }
        }break;
    }

    return 0;
}

bool city::draw(float zoom_level,int draw_limits[9][4])
{
    //test if city is inside view window
    bool draw_city=false;
    bool draw_city_in_box[9]={false};
    for(int box_index=0;box_index<9;box_index++)
    {
        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            if(draw_limits[box_index][0]<=m_vec_city_tiles[i].x_hex && draw_limits[box_index][1]>m_vec_city_tiles[i].x_hex &&
               draw_limits[box_index][2]<=m_vec_city_tiles[i].y_hex && draw_limits[box_index][3]>m_vec_city_tiles[i].y_hex)
            {
                draw_city_in_box[box_index]=true;
                break;//go to next box
            }
        }
    }

    float scale=zoom_level;

    //draw city tiles
    /*//set player color
    switch(m_player_owner)
    {
        case 0: glColor3f(0.8,0.2,0.2);break;
        case 1: glColor3f(0.2,0.8,0.2);break;
        case 2: glColor3f(0.2,0.2,0.8);break;
        case 3: glColor3f(0.8,0.8,0.2);break;
        default: glColor3f(1.0,0.0,1.0);break;
    }*/

    for(int box_index=0;box_index<9;box_index++)
     if(draw_city_in_box[box_index])
    {
        //get box shift values
        int x_shift,y_shift;
        switch(box_index)
        {
            case 0: x_shift=-1; y_shift=-1; break;
            case 1: x_shift=-1; y_shift=0; break;
            case 2: x_shift=-1; y_shift=1; break;
            case 3: x_shift=0; y_shift=-1; break;
            case 4: x_shift=0; y_shift=0; break;
            case 5: x_shift=0; y_shift=1; break;
            case 6: x_shift=1; y_shift=-1; break;
            case 7: x_shift=1; y_shift=0; break;
            case 8: x_shift=1; y_shift=1; break;
        }

        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            //set player color
            /*switch(m_player_owner)
            {
                case 0: glColor3f(0.8,0.2,0.2);break;
                case 1: glColor3f(0.2,0.8,0.2);break;
                case 2: glColor3f(0.2,0.2,0.8);break;
                case 3: glColor3f(0.8,0.8,0.2);break;
                case 4: glColor3f(0.8,0.8,0.8);break;
                case 5: glColor3f(0.2,0.8,0.8);break;
                case 6: glColor3f(0.8,0.2,0.8);break;
                case 7: glColor3f(0.2,0.2,0.2);break;
                case 8: glColor3f(0.4,0.8,0.2);break;
                case 9: glColor3f(0.2,0.4,0.8);break;

                default: glColor3f(1.0,0.0,1.0);break;
            }*/

            //calc tile pos

            float x_pos=(float)m_vec_city_tiles[i].x_hex+_world_size_x*x_shift+((float)m_vec_city_tiles[i].y_hex+_world_size_y*y_shift)*0.5;
            float y_pos=((float)m_vec_city_tiles[i].y_hex+_world_size_y*y_shift)*0.75;

            glPushMatrix();

            glTranslatef(x_pos*scale,y_pos*scale,0);

            //draw tile
            glEnable(GL_BLEND);
            glBegin(GL_TRIANGLE_FAN);
            //set player prim color
            switch(m_player_owner)
            {
                case 0: glColor4f(0.8,0.2,0.2,0.9);break;
                case 1: glColor4f(0.2,0.8,0.2,0.9);break;
                case 2: glColor4f(0.2,0.2,0.8,0.9);break;
                case 3: glColor4f(0.8,0.8,0.2,0.9);break;
                case 4: glColor4f(0.8,0.8,0.8,0.9);break;
                case 5: glColor4f(0.2,0.8,0.8,0.9);break;
                case 6: glColor4f(0.8,0.2,0.8,0.9);break;
                case 7: glColor4f(0.2,0.2,0.2,0.9);break;
                case 8: glColor4f(0.4,0.8,0.2,0.9);break;
                case 9: glColor4f(0.2,0.4,0.8,0.9);break;

                default: glColor4f(1.0,0.0,1.0,1.0);break;
            }

            //float col[4]={0.2,0.2,0.2,1.0};
            //float col2[4]={col[0]*0.5,col[1]*0.5,col[2]*0.5,0.8};

            //glColor4f(0.8,0.8,0.1,1.0);
            //glColor4fv(col);

            glVertex2f(0*scale,0*scale);//center
            //set player sec color (darker)
            switch(m_player_owner)
            {
                case 0: glColor4f(0.4,0.1,0.1,0.5);break;
                case 1: glColor4f(0.1,0.4,0.1,0.5);break;
                case 2: glColor4f(0.1,0.1,0.4,0.5);break;
                case 3: glColor4f(0.4,0.4,0.1,0.5);break;
                case 4: glColor4f(0.4,0.4,0.4,0.5);break;
                case 5: glColor4f(0.1,0.4,0.4,0.5);break;
                case 6: glColor4f(0.4,0.1,0.4,0.5);break;
                case 7: glColor4f(0.1,0.1,0.1,0.5);break;
                case 8: glColor4f(0.2,0.4,0.1,0.5);break;
                case 9: glColor4f(0.1,0.2,0.4,0.5);break;

                default: glColor4f(1.0,0.0,1.0,0.5);break;
            }
            //glColor4f(0.4,0.4,0.05,0.1);
            //glColor4fv(col2);


            glVertex2f(0*scale,0.45*scale);//top
            glVertex2f(-0.45*scale,0.225*scale);
            glVertex2f(-0.45*scale,-0.225*scale);
            glVertex2f(0*scale,-0.45*scale);
            glVertex2f(0.45*scale,-0.225*scale);
            glVertex2f(0.45*scale,0.225*scale);
            glVertex2f(0*scale,0.45*scale);
            glEnd();
            //glDisable(GL_BLEND);

            //draw border (white)
            //float bord_col[4]={0.2,1.0,0.2,1.0};
            //float bord_col2[4]={0.1,0.9,0.1,1.0};
            float bord_col[4]={0.9,0.9,0.9,0.5};
            float bord_col2[4]={0.9,0.9,0.9,1.0};
            if(zoom_level<25) glLineWidth(2);
            else glLineWidth(3);
            glBegin(GL_LINES);
            for(int side=0;side<6;side++)
            {
                if( m_vec_city_tiles[i].border[side]=='b' )
                {
                    switch(side)
                    {
                        case 0://top left
                        {
                            glColor4fv(bord_col);
                            glVertex2f(0*scale,0.5*scale);
                            glColor4fv(bord_col2);
                            glVertex2f(-0.5*scale,0.25*scale);
                        }break;

                        case 1://left
                        {
                            glColor4fv(bord_col2);
                            glVertex2f(-0.5*scale,0.25*scale);
                            glColor4fv(bord_col);
                            glVertex2f(-0.5*scale,-0.25*scale);
                        }break;

                        case 2://down left
                        {
                            glColor4fv(bord_col);
                            glVertex2f(-0.5*scale,-0.25*scale);
                            glColor4fv(bord_col2);
                            glVertex2f(0*scale,-0.5*scale);
                        }break;

                        case 3://down right
                        {
                            glColor4fv(bord_col2);
                            glVertex2f(0*scale,-0.5*scale);
                            glColor4fv(bord_col);
                            glVertex2f(0.5*scale,-0.25*scale);
                        }break;

                        case 4://right
                        {
                            glColor4fv(bord_col);
                            glVertex2f(0.5*scale,-0.25*scale);
                            glColor4fv(bord_col2);
                            glVertex2f(0.5*scale,0.25*scale);
                        }break;

                        case 5://top right
                        {
                            glColor4fv(bord_col2);
                            glVertex2f(0.5*scale,0.25*scale);
                            glColor4fv(bord_col);
                            glVertex2f(0*scale,0.5*scale);
                        }break;

                    }
                }
            }
            glEnd();

            //draw population (dots)
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            //glColor3f(0.9,0.9,0.9);
            if(zoom_level<25) glPointSize(1);
            else glPointSize(2);
            glBegin(GL_POINTS);
            for(int pop=0;pop<(int)m_vec_city_tiles[i].pop_pos.size();pop++)
            {
                glColor4f( 1.0,1.0,1.0, m_vec_city_tiles[i].pop_pos[pop].brightness );
                glVertex2f( m_vec_city_tiles[i].pop_pos[pop].x_pos*scale, m_vec_city_tiles[i].pop_pos[pop].y_pos*scale );
            }
            glEnd();
            glDisable(GL_BLEND);

            glPopMatrix();
        }

        //draw city center
        /*glColor3f(0.8,0.8,0.8);
        float x_pos=(float)m_center_pos_x_hex+_world_size_x*x_shift+((float)m_center_pos_y_hex+_world_size_y*y_shift)*0.5;
        float y_pos=((float)m_center_pos_y_hex+_world_size_y*y_shift)*0.75;

        glPushMatrix();

        glTranslatef(x_pos*scale,y_pos*scale,0);

        //draw tile
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0*scale,0*scale);//center
        glVertex2f(0*scale,0.25*scale);//top
        glVertex2f(-0.25*scale,0.125*scale);
        glVertex2f(-0.25*scale,-0.125*scale);
        glVertex2f(0*scale,-0.25*scale);
        glVertex2f(0.25*scale,-0.125*scale);
        glVertex2f(0.25*scale,0.125*scale);
        glVertex2f(0*scale,0.25*scale);
        glEnd();

        glPopMatrix();*/

    }

    //the rest is always drawn

    //draw temp expansion route
    if(m_city_mode_work==cm_listen_expansion)
    {
        glEnable(GL_BLEND);
        float color[4]={0.9,0.9,0.9,1.0};
        //set player color (or general color)
        /*switch(m_player_owner)
        {
            case 0: color[0]=0.9;color[1]=0.3;color[1]=0.3;color[3]=1.0;break;
            case 1: color[0]=0.3;color[1]=0.9;color[1]=0.3;color[3]=1.0;break;
            case 2: color[0]=0.3;color[1]=0.3;color[1]=0.9;color[3]=1.0;break;
            case 3: color[0]=0.9;color[1]=0.9;color[1]=0.3;color[3]=1.0;break;
            default:color[0]=1.0;color[1]=1.0;color[1]=1.0;color[3]=1.0;break;
        }*/
        for(int box_index=0;box_index<9;box_index++) //for all boxes
         //for(int i=0;i<(int)m_vec_expansion_route.size();i++)
        {
            //calc tile pos
            int x_shift,y_shift;
            switch(box_index)
            {
                case 0: x_shift=-1; y_shift=-1; break;
                case 1: x_shift=-1; y_shift=0; break;
                case 2: x_shift=-1; y_shift=1; break;
                case 3: x_shift=0; y_shift=-1; break;
                case 4: x_shift=0; y_shift=0; break;
                case 5: x_shift=0; y_shift=1; break;
                case 6: x_shift=1; y_shift=-1; break;
                case 7: x_shift=1; y_shift=0; break;
                case 8: x_shift=1; y_shift=1; break;
            }

            //smooth path
            //start part
            /*float x_pos_start=(float)m_vec_expansion_route.front().x_hex+(m_vec_expansion_route.front().x_box_shift+x_shift)*_world_size_x+
                              ((float)m_vec_expansion_route.front().y_hex+(m_vec_expansion_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos_start=((float)m_vec_expansion_route.front().y_hex+(m_vec_expansion_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
            glVertex2d( x_pos_start*scale,y_pos_start*scale );*/
            glColor4fv(color);
            //central part
            glBegin(GL_LINE_STRIP);
            for(int i=1;i<(int)m_vec_expansion_route.size()-1;i++)
            {
                //calc tile pos
                float x_pos=(float)m_vec_expansion_route[i].x_hex+(m_vec_expansion_route[i].x_box_shift+x_shift)*_world_size_x+
                            ((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos=((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                switch(m_vec_expansion_route[i].road[0])
                {
                    case 0: switch(m_vec_expansion_route[i].road[1])
                    {

                        case 1:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 1: switch(m_vec_expansion_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;

                        case 2:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 2: switch(m_vec_expansion_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale, -0.00*scale+y_pos*scale );
                        }break;

                        case 3:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 3: switch(m_vec_expansion_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;

                        case 4:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 4: switch(m_vec_expansion_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;

                        case 5:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 5: switch(m_vec_expansion_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;

                    }break;
                }
            }
            //final part
            /*if( (int)m_vec_expansion_route.size()>1 )
            {
                float x_pos_end=(float)m_vec_expansion_route[].x_hex+(m_vec_expansion_route.back().x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos_end=((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
                glVertex2d( x_pos_end*scale,y_pos_end*scale );
            }*/
            glColor4f(0.0,0.0,0.0,0.0);
            float x_pos_end=(float)m_vec_expansion_route.back().x_hex+(m_vec_expansion_route.back().x_box_shift+x_shift)*_world_size_x+
                            ((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos_end=((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
            glVertex2d( x_pos_end*scale,y_pos_end*scale );
            glEnd();
            //end blob
            glBegin(GL_QUADS);
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);//topt
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(-0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//2t

            glVertex2f(-0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//2t
            glColor4fv(color);
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(-0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//3t

            glVertex2f(-0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//3t
            glColor4fv(color);
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,-0.5*scale+y_pos_end*scale);//4t

            glVertex2f(0*scale+x_pos_end*scale,-0.5*scale+y_pos_end*scale);//4t
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//5t

            glVertex2f(0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//5t
            glColor4fv(color);
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//6t

            glVertex2f(0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//6t
            glColor4fv(color);
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);//topt
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0*scale+y_pos_end*scale);//center
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glEnd();



            /*//old
            float x_pos=(float)m_vec_expansion_route[i].x_hex+(m_vec_expansion_route[i].x_box_shift+x_shift)*_world_size_x+
                        ((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos=((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

            glPushMatrix();

            glTranslatef(x_pos*scale,y_pos*scale,0);

            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0*scale,0*scale);//center
            glVertex2f(0*scale,0.25*scale);//top
            glVertex2f(-0.25*scale,0.125*scale);
            glVertex2f(-0.25*scale,-0.125*scale);
            glVertex2f(0*scale,-0.25*scale);
            glVertex2f(0.25*scale,-0.125*scale);
            glVertex2f(0.25*scale,0.125*scale);
            glVertex2f(0*scale,0.25*scale);
            glEnd();

            glPopMatrix();*/
        }
        glDisable(GL_BLEND);
    }

    //draw temp trade route
    if(m_city_mode_work==cm_listen_trade)
    {
        /*switch(m_player_owner)
        {
            case 0: glColor3f(0.5,0.1,0.1);break;
            case 1: glColor3f(0.1,0.5,0.1);break;
            case 2: glColor3f(0.1,0.1,0.5);break;
            case 3: glColor3f(0.5,0.5,0.1);break;
            default: glColor3f(1.0,0.0,1.0);break;
        }*/

        float color[4]={0.9,0.9,0.9,1.0};
        glEnable(GL_BLEND);

        for(int box_index=0;box_index<9;box_index++) //for all boxes
         //for(int i=0;i<(int)m_vec_trade_route.size();i++)
        {
            //calc tile pos
            int x_shift,y_shift;
            switch(box_index)
            {
                case 0: x_shift=-1; y_shift=-1; break;
                case 1: x_shift=-1; y_shift=0; break;
                case 2: x_shift=-1; y_shift=1; break;
                case 3: x_shift=0; y_shift=-1; break;
                case 4: x_shift=0; y_shift=0; break;
                case 5: x_shift=0; y_shift=1; break;
                case 6: x_shift=1; y_shift=-1; break;
                case 7: x_shift=1; y_shift=0; break;
                case 8: x_shift=1; y_shift=1; break;
            }

            //smooth path
            //start part
            /*float x_pos_start=(float)m_vec_trade_route.front().x_hex+(m_vec_trade_route.front().x_box_shift+x_shift)*_world_size_x+
                              ((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos_start=((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
            glVertex2d( x_pos_start*scale,y_pos_start*scale );*/
            //central part
            glColor4fv(color);
            glBegin(GL_LINE_STRIP);
            for(int i=1;i<(int)m_vec_trade_route.size()-1;i++)
            {
                //calc tile pos
                float x_pos=(float)m_vec_trade_route[i].x_hex+(m_vec_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                            ((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos=((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                switch(m_vec_trade_route[i].road[0])
                {
                    case 0: switch(m_vec_trade_route[i].road[1])
                    {

                        case 1:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 1: switch(m_vec_trade_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;

                        case 2:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 2: switch(m_vec_trade_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                            glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                            glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale, -0.00*scale+y_pos*scale );
                        }break;

                        case 3:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 3: switch(m_vec_trade_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;

                        case 4:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 5:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 4: switch(m_vec_trade_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                            glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                            glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                            glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;

                        case 5:
                        {
                            glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                    }break;
                    case 5: switch(m_vec_trade_route[i].road[1])
                    {
                        case 0:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                        }break;
                        case 1:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                            glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                            glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                            glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                            //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;
                        case 2:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 3:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                            glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                            glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                            //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                        }break;
                        case 4:
                        {
                            glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                            glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                            glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                            glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                            //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                        }break;

                    }break;
                }
            }
            //final part
            float x_pos_end=(float)m_vec_trade_route.back().x_hex+(m_vec_trade_route.back().x_box_shift+x_shift)*_world_size_x+
                            ((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos_end=((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2d( x_pos_end*scale,y_pos_end*scale );
            glEnd();

            //end blob
            glBegin(GL_QUADS);
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);//topt
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(-0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//2t

            glVertex2f(-0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//2t
            glColor4fv(color);
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(-0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//3t

            glVertex2f(-0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//3t
            glColor4fv(color);
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,-0.5*scale+y_pos_end*scale);//4t

            glVertex2f(0*scale+x_pos_end*scale,-0.5*scale+y_pos_end*scale);//4t
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//5t

            glVertex2f(0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//5t
            glColor4fv(color);
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//6t

            glVertex2f(0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//6t
            glColor4fv(color);
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);//topt
            glEnd();

            glBegin(GL_TRIANGLE_FAN);
            glColor4f(0.0,0.0,0.0,0.0);
            glVertex2f(0*scale+x_pos_end*scale,0*scale+y_pos_end*scale);//center
            glColor4fv(color);
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glVertex2f(-0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//2
            glVertex2f(-0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//3
            glVertex2f(0*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);//4
            glVertex2f(0.25*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);//5
            glVertex2f(0.25*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//6
            glVertex2f(0*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);//top
            glEnd();


            /*//old
            float x_pos=(float)m_vec_trade_route[i].x_hex+(m_vec_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                        ((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos=((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

            glPushMatrix();

            glTranslatef(x_pos*scale,y_pos*scale,0);

            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(0*scale,0*scale);//center
            glVertex2f(0*scale,0.25*scale);//top
            glVertex2f(-0.25*scale,0.125*scale);
            glVertex2f(-0.25*scale,-0.125*scale);
            glVertex2f(0*scale,-0.25*scale);
            glVertex2f(0.25*scale,-0.125*scale);
            glVertex2f(0.25*scale,0.125*scale);
            glVertex2f(0*scale,0.25*scale);
            glEnd();

            glPopMatrix();*/
        }

        glDisable(GL_BLEND);
    }

    //draw expansion mission progress
    if(m_city_mode_work==cm_expansion_mission)
    {
        if(m_mission_expansion_time!=0)
        {
            float color_intensity=sinf(m_lifetime);
            //distance calc
            float rel_progress=m_mission_expansion_progress/m_mission_expansion_time;
            float distance_tot=(int)m_vec_expansion_route.size()-1;
            float distance_progress=distance_tot*rel_progress;
            int distance_done=(int)distance_progress;
            float distance_part=distance_progress-(float)distance_done;
            int distance_part_fifths=distance_part*5.0;

            glEnable(GL_BLEND);

            for(int box_index=0;box_index<9;box_index++) //for all boxes
            {
                //calc tile pos
                int x_shift,y_shift;
                switch(box_index)
                {
                    case 0: x_shift=-1; y_shift=-1; break;
                    case 1: x_shift=-1; y_shift=0; break;
                    case 2: x_shift=-1; y_shift=1; break;
                    case 3: x_shift=0; y_shift=-1; break;
                    case 4: x_shift=0; y_shift=0; break;
                    case 5: x_shift=0; y_shift=1; break;
                    case 6: x_shift=1; y_shift=-1; break;
                    case 7: x_shift=1; y_shift=0; break;
                    case 8: x_shift=1; y_shift=1; break;
                }

                //smooth path
                //start part
                //set color to player color...
                float x_pos_start=(float)m_vec_expansion_route.front().x_hex+(m_vec_expansion_route.front().x_box_shift+x_shift)*_world_size_x+
                                  ((float)m_vec_expansion_route.front().y_hex+(m_vec_expansion_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos_start=((float)m_vec_expansion_route.front().y_hex+(m_vec_expansion_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
                glVertex2d( x_pos_start*scale,y_pos_start*scale );
                //central part

                glBegin(GL_LINE_STRIP);
                for(int i=1;i<(int)m_vec_expansion_route.size()-1;i++)
                {
                    //set color progress
                    float colors[5][4];
                    if( i==distance_done )
                    {
                        for(int col_i=0;col_i<5;col_i++)
                        {
                            colors[col_i][0]=0.2+0.1*color_intensity;
                            colors[col_i][1]=0.2+0.1*color_intensity;
                            colors[col_i][2]=0.2+0.1*color_intensity;
                            colors[col_i][3]=1.0;
                        }
                        //highlighted part
                        if( distance_part_fifths>0 )
                        {
                            colors[distance_part_fifths-1][0]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][1]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][2]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][3]=1.0;

                            colors[distance_part_fifths][0]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][1]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][2]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][3]=1.0;
                        }
                        if( distance_part_fifths<4 )
                        {
                            colors[distance_part_fifths+1][0]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][1]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][2]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][3]=1.0;
                        }
                    }
                    else//outside range
                    {
                        for(int col_i=0;col_i<5;col_i++)
                        {
                            colors[col_i][0]=0.2+0.1*color_intensity;
                            colors[col_i][1]=0.2+0.1*color_intensity;
                            colors[col_i][2]=0.2+0.1*color_intensity;
                            colors[col_i][3]=1.0;
                        }
                    }

                    //calc tile pos
                    float x_pos=(float)m_vec_expansion_route[i].x_hex+(m_vec_expansion_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    switch(m_vec_expansion_route[i].road[0])
                    {
                        case 0: switch(m_vec_expansion_route[i].road[1])
                        {

                            case 1:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.260*scale+x_pos*scale,  0.430*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 1: switch(m_vec_expansion_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;

                            case 2:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 2: switch(m_vec_expansion_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale, -0.00*scale+y_pos*scale );
                            }break;

                            case 3:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f( -0.260*scale+x_pos*scale, -0.430*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f( -0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f( -0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 3: switch(m_vec_expansion_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.260*scale+x_pos*scale, -0.430*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;

                            case 4:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 4: switch(m_vec_expansion_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;

                            case 5:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 5: switch(m_vec_expansion_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.260*scale+x_pos*scale,  0.430*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f( -0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f( -0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor4fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor4fv(colors[1]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                                glColor4fv(colors[2]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                                glColor4fv(colors[3]);
                                glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                                glColor4fv(colors[4]);
                                glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;

                        }break;
                    }
                }
                //final part
                //set color to player color...
                float x_pos_end=(float)m_vec_expansion_route.back().x_hex+(m_vec_expansion_route.back().x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos_end=((float)m_vec_expansion_route.back().y_hex+(m_vec_expansion_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
                glVertex2d( x_pos_end*scale,y_pos_end*scale );
                glEnd();
                //end blob
                glBegin(GL_TRIANGLE_FAN);//center
                glVertex2f(0*scale+x_pos_end*scale,0*scale+y_pos_end*scale);//center
                glVertex2f(0*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);//top
                glVertex2f(-0.125*scale+x_pos_end*scale,0.0625*scale+y_pos_end*scale);
                glVertex2f(-0.125*scale+x_pos_end*scale,-0.0625*scale+y_pos_end*scale);
                glVertex2f(0*scale+x_pos_end*scale,-0.125*scale+y_pos_end*scale);
                glVertex2f(0.125*scale+x_pos_end*scale,-0.0625*scale+y_pos_end*scale);
                glVertex2f(0.125*scale+x_pos_end*scale,0.0625*scale+y_pos_end*scale);
                glVertex2f(0*scale+x_pos_end*scale,0.125*scale+y_pos_end*scale);
                glEnd();

                glBegin(GL_TRIANGLE_FAN);//outer
                glColor4f(0.5,0.5,0.5,1.0);
                glVertex2f(0*scale+x_pos_end*scale,0*scale+y_pos_end*scale);//center
                glColor4f(0.0,0.0,0.0,0.0);
                glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);//top
                glVertex2f(-0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);
                glVertex2f(-0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);
                glVertex2f(0*scale+x_pos_end*scale,-0.5*scale+y_pos_end*scale);
                glVertex2f(0.5*scale+x_pos_end*scale,-0.25*scale+y_pos_end*scale);
                glVertex2f(0.5*scale+x_pos_end*scale,0.25*scale+y_pos_end*scale);
                glVertex2f(0*scale+x_pos_end*scale,0.5*scale+y_pos_end*scale);
                glEnd();
            }

            glDisable(GL_BLEND);


            /*//old
            //distance calc
            float rel_progress=m_mission_expansion_progress/m_mission_expansion_time;
            float distance_tot=(int)m_vec_expansion_route.size()-1;
            float distance_progress=distance_tot*rel_progress;
            int distance_done=(int)distance_progress;
            float distance_part=distance_progress-(float)distance_done;

            for(int box_index=0;box_index<9;box_index++) //for all boxes
            {
                //calc tile pos
                int x_shift,y_shift;
                switch(box_index)
                {
                    case 0: x_shift=-1; y_shift=-1; break;
                    case 1: x_shift=-1; y_shift=0; break;
                    case 2: x_shift=-1; y_shift=1; break;
                    case 3: x_shift=0; y_shift=-1; break;
                    case 4: x_shift=0; y_shift=0; break;
                    case 5: x_shift=0; y_shift=1; break;
                    case 6: x_shift=1; y_shift=-1; break;
                    case 7: x_shift=1; y_shift=0; break;
                    case 8: x_shift=1; y_shift=1; break;
                }

                //draw thick line as progress
                glLineWidth(4);
                glColor3f(0.2,0.8,0.2);
                glBegin(GL_LINE_STRIP);
                for(int i=0;i<=distance_done;i++)
                {
                    //calc tile pos
                    float x_pos=(float)m_vec_expansion_route[i].x_hex+(m_vec_expansion_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    glVertex2d( x_pos*scale,y_pos*scale );
                }
                glEnd();

                //draw fragments
                float x_pos_start=(float)m_vec_expansion_route[distance_done].x_hex+(m_vec_expansion_route[distance_done].x_box_shift+x_shift)*_world_size_x+
                                  ((float)m_vec_expansion_route[distance_done].y_hex+(m_vec_expansion_route[distance_done].y_box_shift+y_shift)*_world_size_y )*0.5;
                float y_pos_start=((float)m_vec_expansion_route[distance_done].y_hex+(m_vec_expansion_route[distance_done].y_box_shift+y_shift)*_world_size_y )*0.75;
                float x_pos_end=(float)m_vec_expansion_route[distance_done+1].x_hex+(m_vec_expansion_route[distance_done+1].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route[distance_done+1].y_hex+(m_vec_expansion_route[distance_done+1].y_box_shift+y_shift)*_world_size_y )*0.5;
                float y_pos_end=((float)m_vec_expansion_route[distance_done+1].y_hex+(m_vec_expansion_route[distance_done+1].y_box_shift+y_shift)*_world_size_y )*0.75;
                float x_pos_mid=x_pos_start+(x_pos_end-x_pos_start)*distance_part;
                float y_pos_mid=y_pos_start+(y_pos_end-y_pos_start)*distance_part;

                glBegin(GL_LINES);
                glVertex2d( x_pos_start*scale,y_pos_start*scale );
                glVertex2d( x_pos_mid*scale,y_pos_mid*scale );
                glEnd();

                //draw the rest of the fragment as a thin line
                glLineWidth(2);
                glColor3f(0.1,0.4,0.1);
                glBegin(GL_LINES);
                glVertex2d( x_pos_mid*scale,y_pos_mid*scale );
                glVertex2d( x_pos_end*scale,y_pos_end*scale );
                glEnd();

                //draw the rest of the line
                glBegin(GL_LINE_STRIP);
                for(int i=distance_done+1;i<(int)m_vec_expansion_route.size();i++)
                {
                    //calc tile pos
                    float x_pos=(float)m_vec_expansion_route[i].x_hex+(m_vec_expansion_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_expansion_route[i].y_hex+(m_vec_expansion_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    glVertex2d( x_pos*scale,y_pos*scale );
                }
                glEnd();
            }

            glLineWidth(1);
            */
        }
    }

    //draw trade mission progress
    if(m_city_mode_work==cm_trade_mission)
    {
        if(m_mission_trade_time!=0)
        {
            float color_intensity=sinf(m_lifetime);
            //distance calc
            float rel_progress=m_mission_trade_progress/m_mission_trade_time;
            float distance_tot=(int)m_vec_new_trade_route.size()-1;
            float distance_progress=distance_tot*rel_progress;
            int distance_done=(int)distance_progress;
            float distance_part=distance_progress-(float)distance_done;
            int distance_part_fifths=distance_part*5.0;

            glEnable(GL_BLEND);

            for(int box_index=0;box_index<9;box_index++) //for all boxes
            {
                //calc tile pos
                int x_shift,y_shift;
                switch(box_index)
                {
                    case 0: x_shift=-1; y_shift=-1; break;
                    case 1: x_shift=-1; y_shift=0; break;
                    case 2: x_shift=-1; y_shift=1; break;
                    case 3: x_shift=0; y_shift=-1; break;
                    case 4: x_shift=0; y_shift=0; break;
                    case 5: x_shift=0; y_shift=1; break;
                    case 6: x_shift=1; y_shift=-1; break;
                    case 7: x_shift=1; y_shift=0; break;
                    case 8: x_shift=1; y_shift=1; break;
                }

                //smooth path
                //start part
                //set color to player color...
                /*float x_pos_start=(float)m_vec_new_trade_route.front().x_hex+(m_vec_new_trade_route.front().x_box_shift+x_shift)*_world_size_x+
                                  ((float)m_vec_new_trade_route.front().y_hex+(m_vec_new_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos_start=((float)m_vec_new_trade_route.front().y_hex+(m_vec_new_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
                glVertex2d( x_pos_start*scale,y_pos_start*scale );*/
                //central part
                glBegin(GL_LINE_STRIP);
                for(int i=1;i<(int)m_vec_new_trade_route.size()-1;i++)
                {
                    //set color progress
                    float colors[5][4];
                    if( i==distance_done )
                    {
                        for(int col_i=0;col_i<5;col_i++)
                        {
                            colors[col_i][0]=0.2+0.1*color_intensity;
                            colors[col_i][1]=0.2+0.1*color_intensity;
                            colors[col_i][2]=0.2+0.1*color_intensity;
                            colors[col_i][3]=1.0;
                        }
                        //highlighted part
                        if( distance_part_fifths>0 )
                        {
                            colors[distance_part_fifths-1][0]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][1]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][2]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths-1][3]=1.0;

                            colors[distance_part_fifths][0]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][1]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][2]=0.9+0.1*color_intensity;
                            colors[distance_part_fifths][3]=1.0;
                        }
                        if( distance_part_fifths<4 )
                        {
                            colors[distance_part_fifths+1][0]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][1]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][2]=0.5+0.1*color_intensity;
                            colors[distance_part_fifths+1][3]=1.0;
                        }
                    }
                    else//outside range
                    {
                        for(int col_i=0;col_i<5;col_i++)
                        {
                            colors[col_i][0]=0.2+0.1*color_intensity;
                            colors[col_i][1]=0.2+0.1*color_intensity;
                            colors[col_i][2]=0.2+0.1*color_intensity;
                            colors[col_i][3]=1.0;
                        }
                    }

                    //calc tile pos
                    float x_pos=(float)m_vec_new_trade_route[i].x_hex+(m_vec_new_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    switch(m_vec_new_trade_route[i].road[0])
                    {
                        case 0: switch(m_vec_new_trade_route[i].road[1])
                        {

                            case 1:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.260*scale+x_pos*scale,  0.430*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 1: switch(m_vec_new_trade_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.37*scale+x_pos*scale,  0.03*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.28*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.24*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.22*scale+x_pos*scale,  0.33*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;

                            case 2:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 2: switch(m_vec_new_trade_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.33*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.22*scale+x_pos*scale, -0.24*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.28*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.37*scale+x_pos*scale, -0.03*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale, -0.00*scale+y_pos*scale );
                            }break;

                            case 3:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f( -0.260*scale+x_pos*scale, -0.430*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f( -0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f( -0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 3: switch(m_vec_new_trade_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.260*scale+x_pos*scale, -0.430*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.09*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.20*scale+x_pos*scale, -0.34*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;

                            case 4:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 5:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 4: switch(m_vec_new_trade_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.14*scale+x_pos*scale,  0.06*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.03*scale+x_pos*scale,  0.16*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.10*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.30*scale+x_pos*scale, -0.01*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.14*scale+x_pos*scale, -0.05*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.03*scale+x_pos*scale, -0.15*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.15*scale+x_pos*scale, -0.27*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.37*scale+x_pos*scale, -0.02*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.28*scale+x_pos*scale, -0.09*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.23*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.22*scale+x_pos*scale, -0.32*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;

                            case 5:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                        }break;
                        case 5: switch(m_vec_new_trade_route[i].road[1])
                        {
                            case 0:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.09*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.20*scale+x_pos*scale,  0.34*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                            }break;
                            case 1:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.15*scale+x_pos*scale,  0.27*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.03*scale+x_pos*scale,  0.15*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.14*scale+x_pos*scale,  0.05*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.30*scale+x_pos*scale,  0.01*scale+y_pos*scale );
                                //glVertex2f( -0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;
                            case 2:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.260*scale+x_pos*scale,  0.430*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.156*scale+x_pos*scale,  0.258*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.052*scale+x_pos*scale,  0.086*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f( -0.052*scale+x_pos*scale, -0.086*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f( -0.156*scale+x_pos*scale, -0.258*scale+y_pos*scale );
                                //glVertex2f( -0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 3:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.17*scale+x_pos*scale,  0.26*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.13*scale+x_pos*scale,  0.10*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.13*scale+x_pos*scale, -0.10*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.17*scale+x_pos*scale, -0.26*scale+y_pos*scale );
                                //glVertex2f(  0.26*scale+x_pos*scale, -0.43*scale+y_pos*scale );
                            }break;
                            case 4:
                            {
                                glColor3fv(colors[0]);
                                glVertex2f(  0.26*scale+x_pos*scale,  0.43*scale+y_pos*scale );
                                glColor3fv(colors[1]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.32*scale+y_pos*scale );
                                glColor3fv(colors[2]);
                                glVertex2f(  0.22*scale+x_pos*scale,  0.23*scale+y_pos*scale );
                                glColor3fv(colors[3]);
                                glVertex2f(  0.28*scale+x_pos*scale,  0.09*scale+y_pos*scale );
                                glColor3fv(colors[4]);
                                glVertex2f(  0.37*scale+x_pos*scale,  0.02*scale+y_pos*scale );
                                //glVertex2f(  0.50*scale+x_pos*scale,  0.00*scale+y_pos*scale );
                            }break;

                        }break;
                    }
                }
                //final part
                //set color to player color...
                float x_pos_end=(float)m_vec_new_trade_route.back().x_hex+(m_vec_new_trade_route.back().x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_new_trade_route.back().y_hex+(m_vec_new_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
                float y_pos_end=((float)m_vec_new_trade_route.back().y_hex+(m_vec_new_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
                glColor4f(0.0,0.0,0.0,0.0);
                glVertex2d( x_pos_end*scale,y_pos_end*scale );
                glEnd();
                //end blob
            }

            glDisable(GL_BLEND);

            /*//old
            float rel_progress=m_mission_trade_progress/m_mission_trade_time;
            float distance_tot=(int)m_vec_new_trade_route.size()-1;
            float distance_progress=distance_tot*rel_progress;
            int distance_done=(int)distance_progress;
            float distance_part=distance_progress-(float)distance_done;

            for(int box_index=0;box_index<9;box_index++) //for all boxes
            {
                int x_shift,y_shift;
                switch(box_index)
                {
                    case 0: x_shift=-1; y_shift=-1; break;
                    case 1: x_shift=-1; y_shift=0; break;
                    case 2: x_shift=-1; y_shift=1; break;
                    case 3: x_shift=0; y_shift=-1; break;
                    case 4: x_shift=0; y_shift=0; break;
                    case 5: x_shift=0; y_shift=1; break;
                    case 6: x_shift=1; y_shift=-1; break;
                    case 7: x_shift=1; y_shift=0; break;
                    case 8: x_shift=1; y_shift=1; break;
                }

                //draw thick line as progress
                glLineWidth(4);
                glColor3f(0.3,0.3,0.9);
                glBegin(GL_LINE_STRIP);
                for(int i=0;i<=distance_done;i++)
                {
                    //calc tile pos
                    float x_pos=(float)m_vec_new_trade_route[i].x_hex+(m_vec_new_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    glVertex2d( x_pos*scale,y_pos*scale );
                }
                glEnd();

                //draw fragments
                float x_pos_start=(float)m_vec_new_trade_route[distance_done].x_hex+(m_vec_new_trade_route[distance_done].x_box_shift+x_shift)*_world_size_x+
                                  ((float)m_vec_new_trade_route[distance_done].y_hex+(m_vec_new_trade_route[distance_done].y_box_shift+y_shift)*_world_size_y )*0.5;
                float y_pos_start=((float)m_vec_new_trade_route[distance_done].y_hex+(m_vec_new_trade_route[distance_done].y_box_shift+y_shift)*_world_size_y )*0.75;
                float x_pos_end=(float)m_vec_new_trade_route[distance_done+1].x_hex+(m_vec_new_trade_route[distance_done+1].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_new_trade_route[distance_done+1].y_hex+(m_vec_new_trade_route[distance_done+1].y_box_shift+y_shift)*_world_size_y )*0.5;
                float y_pos_end=((float)m_vec_new_trade_route[distance_done+1].y_hex+(m_vec_new_trade_route[distance_done+1].y_box_shift+y_shift)*_world_size_y )*0.75;
                float x_pos_mid=x_pos_start+(x_pos_end-x_pos_start)*distance_part;
                float y_pos_mid=y_pos_start+(y_pos_end-y_pos_start)*distance_part;
                glBegin(GL_LINES);
                glVertex2d( x_pos_start*scale,y_pos_start*scale );
                glVertex2d( x_pos_mid*scale,y_pos_mid*scale );
                glEnd();

                //draw the rest of the fragment as a thin line
                glLineWidth(2);
                glColor3f(0.4,0.2,0.3);
                glBegin(GL_LINES);
                glVertex2d( x_pos_mid*scale,y_pos_mid*scale );
                glVertex2d( x_pos_end*scale,y_pos_end*scale );
                glEnd();

                //draw the rest of the line
                glBegin(GL_LINE_STRIP);
                for(int i=distance_done+1;i<(int)m_vec_new_trade_route.size();i++)
                {
                    //calc tile pos
                    float x_pos=(float)m_vec_new_trade_route[i].x_hex+(m_vec_new_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                                ((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
                    float y_pos=((float)m_vec_new_trade_route[i].y_hex+(m_vec_new_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

                    glVertex2d( x_pos*scale,y_pos*scale );
                }
                glEnd();

            }

            glLineWidth(1);
            */
        }
    }


    return true;
}

bool city::get_center_hex_coordinates(int& x_hex,int& y_hex)
{
    x_hex=m_center_pos_x_hex, y_hex=m_center_pos_y_hex;

    return true;
}

bool city::add_tile_to_city(int pos_x_hex,int pos_y_hex)
{
    m_vec_city_tiles.push_back( st_coord_chararr(pos_x_hex,pos_y_hex) );
    //recalc center
    calc_city_center_tile();//temp

    return true;
}

bool city::remove_city_tile(int pos_x_hex,int pos_y_hex)
{
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        if( m_vec_city_tiles[i].x_hex==pos_x_hex && m_vec_city_tiles[i].y_hex==pos_y_hex )
        {
            m_vec_city_tiles.erase(m_vec_city_tiles.begin()+i);
            //recalc center
            calc_city_center_tile();//temp
            recalc_border();
            return true;
        }
    }

    cout<<"ERROR: Removing tile from city: Tile not found\n";

    return false;//tile not found
}

bool city::get_city_tiles_list(vector<st_coord_chararr>& city_tiles_list)
{
    city_tiles_list=m_vec_city_tiles;

    return true;
}

bool city::get_new_city_expansion_info(int& city_old_hex_x,int& city_old_hex_y,int& city_new_hex_x,int& city_new_hex_y)
{
    city_old_hex_x=m_city_old_hex_x;
    city_old_hex_y=m_city_old_hex_y;
    city_new_hex_x=m_city_new_hex_x;
    city_new_hex_y=m_city_new_hex_y;

    return true;
}

bool city::get_new_expansion_route(vector<st_coord_route>& vec_expansion_route)
{
    vec_expansion_route=m_vec_expansion_route;

    return true;
}

bool city::set_new_expansion_route(vector<st_coord_route> vec_expansion_route)
{
    m_vec_expansion_route=vec_expansion_route;

    return true;
}

bool city::get_new_trade_route(vector<st_coord_route>& vec_trade_route)
{
    vec_trade_route=m_vec_new_trade_route;

    return true;
}

bool city::set_new_trade_route(vector<st_coord_route> vec_trade_route)
{
    m_vec_new_trade_route=vec_trade_route;

    return true;
}

int city::get_city_id(void)
{
    return m_city_id;
}

int city::get_city_size(void)
{
    return (int)m_vec_city_tiles.size();
}

int city::get_city_owner_id(void)
{
    return m_player_owner;
}

bool city::abort_trade_mission(void)
{
    if(m_city_mode_work==cm_trade_mission)
    {
        cout<<"Trade: Trade mission aborted successfully\n";
        m_city_mode_work=cm_idle;
        m_mission_trade_time=0;
        m_vec_trade_route.clear();
        m_vec_new_trade_route.clear();
        return true;
    }
    cout<<"ERROR: Trade: Trade mission could not be aborted\n";

    return false;
}

bool city::abort_current_mission(void)
{
    if(m_city_mode_work==cm_trade_mission)
    {
        cout<<"Trade: Trade mission aborted successfully\n";
        m_city_mode_work=cm_idle;
        m_mission_trade_time=0;
        m_vec_trade_route.clear();
        m_vec_new_trade_route.clear();
        return true;
    }
    else if(m_city_mode_work==cm_expansion_mission)
    {
        cout<<"Expansion: Expansion mission aborted successfully\n";
        m_city_mode_work=cm_idle;
        m_mission_expansion_time=0;
        m_vec_expansion_route.clear();
        return true;
    }
    cout<<"ERROR: Mission abort: Current mission could not be aborted\n";

    return false;
}

bool city::get_starvation_coord(int& x_hex,int& y_hex)
{
    x_hex=m_starve_hex_x;
    y_hex=m_starve_hex_y;

    return true;
}

bool city::set_city_mode(int mode_pop,int mode_work)
{
    if(mode_pop!=-1) m_city_mode_pop=mode_pop;
    if(mode_work!=-1)m_city_mode_work=mode_work;
    return true;
}

int city::get_city_color(void)
{
    return m_player_owner;
}

bool city::set_recalc_border_flag(bool flag)
{
    m_recalc_border=flag;

    return true;
}

bool city::set_new_city_id(int new_id)
{
    m_new_city_id=new_id;

    return true;
}

int city::get_new_city_id(void)
{
    return m_new_city_id;
}

bool city::reset_new_city_id(void)
{
    m_new_city_id=-1;//reset value
    return true;
}

bool city::force_finish_new_city(void)
{
    m_city_old_hex_x=m_vec_expansion_route.front().x_hex;
    m_city_old_hex_y=m_vec_expansion_route.front().y_hex;
    m_city_new_hex_x=m_vec_expansion_route.back().x_hex;
    m_city_new_hex_y=m_vec_expansion_route.back().y_hex;

    m_mission_expansion_time=0;
    m_city_mode_work=cm_idle;

    m_vec_expansion_route.clear();

    return true;
}



//---Private---


bool city::recalc_border(void)
{
    //turn all borders on
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        m_vec_city_tiles[i].border_counter=6;
        for(int b=0;b<6;b++)
         m_vec_city_tiles[i].border[b]='b';
    }

    //turn off unwanted borders
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        for(int j=0;j<(int)m_vec_city_tiles.size();j++)
        {
            if(i==j) continue; //same tile

            //test if contact with outside
            for(int x_add=-1;x_add<2;x_add++)
            for(int y_add=-1;y_add<2;y_add++)
            {
                if(x_add==y_add) continue; //skip -1,-1 and +1,+1 and center 0,0

                //box border jump
                int new_x=m_vec_city_tiles[i].x_hex+x_add;
                int new_y=m_vec_city_tiles[i].y_hex+y_add;
                if(new_x<0) new_x+=_world_size_x;
                else if(new_x>=_world_size_x) new_x-=_world_size_x;
                if(new_y<0) new_y+=_world_size_y;
                else if(new_y>=_world_size_y) new_y-=_world_size_y;

                if( new_x==m_vec_city_tiles[j].x_hex &&
                    new_y==m_vec_city_tiles[j].y_hex )
                {//no border needed 'n'
                    m_vec_city_tiles[i].border_counter--;
                    //translate x/y add to side to border char array
                    if( x_add==-1 && y_add==1 ) m_vec_city_tiles[i].border[0]='n';//top left
                    if( x_add==-1 && y_add==0 ) m_vec_city_tiles[i].border[1]='n';//left
                    if( x_add==0 && y_add==-1 ) m_vec_city_tiles[i].border[2]='n';//down left
                    if( x_add==1 && y_add==-1 ) m_vec_city_tiles[i].border[3]='n';//down right
                    if( x_add==1 && y_add==0 ) m_vec_city_tiles[i].border[4]='n';//right
                    if( x_add==0 && y_add==1 ) m_vec_city_tiles[i].border[5]='n';//top right
                }
            }
        }
    }

    m_recalc_border=false;

    //recount the population
    population_count();

    return true;
}

bool city::population_count(void)
{
    //count the number of borders for neighbour city tiles
    for(int tile_i=0;tile_i<(int)m_vec_city_tiles.size();tile_i++)
    {
        int neighbours_border_counter=0;
        for(int tile_i2=0;tile_i2<(int)m_vec_city_tiles.size();tile_i2++)
        {
            if(tile_i==tile_i2) continue; //same tile

            //test if contact with outside
            for(int x_add=-1;x_add<2;x_add++)
            for(int y_add=-1;y_add<2;y_add++)
            {
                if(x_add==y_add) continue; //skip -1,-1 and +1,+1 and center 0,0

                //box border jump
                int new_x=m_vec_city_tiles[tile_i].x_hex+x_add;
                int new_y=m_vec_city_tiles[tile_i].y_hex+y_add;
                if(new_x<0) new_x+=_world_size_x;
                else if(new_x>=_world_size_x) new_x-=_world_size_x;
                if(new_y<0) new_y+=_world_size_y;
                else if(new_y>=_world_size_y) new_y-=_world_size_y;

                if( new_x==m_vec_city_tiles[tile_i2].x_hex &&
                    new_y==m_vec_city_tiles[tile_i2].y_hex )
                {//neighbour tile
                    neighbours_border_counter+=m_vec_city_tiles[tile_i2].border_counter;
                }
            }
        }
        m_vec_city_tiles[tile_i].neighbour_border_counter=neighbours_border_counter;

        //adjust population limit
        int tile_pop=0;
        if(m_vec_city_tiles[tile_i].border_counter>0)
        {
            switch(m_vec_city_tiles[tile_i].border_counter)
            {
                case 1: tile_pop=6; break;
                case 2: tile_pop=5; break;
                case 3: tile_pop=4; break;
                case 4: tile_pop=3; break;
                case 5: tile_pop=2; break;
                case 6: tile_pop=1; break;
            }
        }
        else//if no borders on this tile, use counter for neighbour tiles
        {
            if(m_vec_city_tiles[tile_i].neighbour_border_counter==0) tile_pop=25;
            else if(m_vec_city_tiles[tile_i].neighbour_border_counter<3) tile_pop=18;
            else if(m_vec_city_tiles[tile_i].neighbour_border_counter<7) tile_pop=16;
            else if(m_vec_city_tiles[tile_i].neighbour_border_counter<10) tile_pop=14;
            else if(m_vec_city_tiles[tile_i].neighbour_border_counter<12) tile_pop=12;
            else if(m_vec_city_tiles[tile_i].neighbour_border_counter<18) tile_pop=10;
            else tile_pop=8;
        }

        m_vec_city_tiles[tile_i].pop_max=tile_pop;
    }

    return true;
}

bool city::adjust_pop_count(void)
{//adjust population
    //select tile by random
    int tile_i=rand()%int( m_vec_city_tiles.size() );
    int error_tol=2;
    int add_or_remove=rand()%2;

    if(add_or_remove==1)//add test
    {
        if( (int)m_vec_city_tiles[tile_i].pop_pos.size()<m_vec_city_tiles[tile_i].pop_max+error_tol )
        {//add dot
            int box_size_type=rand()%2;
            if(box_size_type==1)
            {//wide x (-0.40 - 0.40) narrow y (-0.22 - 0.22)
                float x_pos=float(rand()%800-400)/1000.0;
                float y_pos=float(rand()%440-220)/1000.0;
                m_vec_city_tiles[tile_i].pop_pos.push_back( st_pop(x_pos,y_pos) );
            }
            else
            {//wide y (-0.35 - 0.35) narrow x (-0.22 - 0.22)
                float x_pos=float(rand()%440-220)/1000.0;
                float y_pos=float(rand()%700-350)/1000.0;
                m_vec_city_tiles[tile_i].pop_pos.push_back( st_pop(x_pos,y_pos) );
            }
        }
    }
    else//remove test
    {
        if( (int)m_vec_city_tiles[tile_i].pop_pos.size()>m_vec_city_tiles[tile_i].pop_max-error_tol )
        {//remove dot
            if( !m_vec_city_tiles[tile_i].pop_pos.empty() )
            {
                int pop_index=rand()%int(m_vec_city_tiles[tile_i].pop_pos.size() );
                m_vec_city_tiles[tile_i].pop_pos[pop_index].flag_remove=true;
            }
        }
    }

    return true;
}

int city::get_city_max_size(int numof_trades,int longest_trade_distance)
{
    //cout<<"Numof trades: "<<numof_trades<<",  longest dist: "<<longest_trade_distance<<endl;
    int max_size_numof_trades=0;
    int max_size_trade_dist=0;

    //test if enough trades
    switch(numof_trades)
    {
        case 0: max_size_numof_trades=3; break;
        case 1: max_size_numof_trades=7; break;
        case 2: max_size_numof_trades=12; break;
        case 3: max_size_numof_trades=18; break;
        case 4: max_size_numof_trades=25; break;
        case 5: max_size_numof_trades=33; break;
        case 6: max_size_numof_trades=42; break;
        case 7: max_size_numof_trades=52; break;
        case 8: max_size_numof_trades=63; break;
        case 9: max_size_numof_trades=75; break;
        case 10: max_size_numof_trades=88; break;
        default: max_size_numof_trades=0; break;//or no limit
    }

    //test if trade dist is enough FIX THIS
    switch(longest_trade_distance)
    {
        case 0: max_size_trade_dist=4; break;
        case 1: max_size_trade_dist=4; break;//impossible
        case 2: max_size_trade_dist=4; break;
        case 3: max_size_trade_dist=8; break;
        case 4: max_size_trade_dist=16; break;
        case 5: max_size_trade_dist=32; break;
        case 6: max_size_trade_dist=64; break;
        case 7: max_size_trade_dist=128; break;
        default: max_size_trade_dist=128; break;//or no limit
    }

    if( max_size_numof_trades==0 && max_size_trade_dist==0 ) return 0;//no limit
    if( max_size_numof_trades<max_size_trade_dist ) return max_size_numof_trades;
    else return max_size_trade_dist;

    return 0;//will not get here
}

bool city::calc_city_center_tile(void)
{
    //quick test, if city is smaller than 7 tiles, center unchanged (same as first city tile)
    if( (int)m_vec_city_tiles.size()<8 )
    {
        //make sure that center tile is still part of city
        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            if( m_vec_city_tiles[i].x_hex==m_center_pos_x_hex && m_vec_city_tiles[i].y_hex==m_center_pos_y_hex )
            {
                //tile still there
                return true;
            }
        }
        //if here, old center is lost, firsttile in vec is new center
        m_center_pos_x_hex=m_vec_city_tiles[0].x_hex;
        m_center_pos_y_hex=m_vec_city_tiles[0].y_hex;
        return true;
    }

    //make all tile hex coord relative

    //start with old center
    int center_x=m_center_pos_x_hex;
    int center_y=m_center_pos_y_hex;

    //find center index in vec
    int center_index=-1;
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        if( m_vec_city_tiles[i].x_hex==center_x && m_vec_city_tiles[i].y_hex==center_y )
        {
            center_index=i;
            break;
        }
    }
    if(center_index==-1)//bad
    {
        //if old center not part of city anymore, pick first element in vector
        if( m_vec_city_tiles.empty() )
        {
            cout<<"ERROR: City center calc: Could not find center tile in vector\n";
            return false;
        }
        else center_index=0;
    }

    //go through all tiles to find surrounding tiles
    vector< vector<int> > vec_dist_vec_tile_indices;
    int curr_dist=0;//first element will only contain center tile
    while(true)
    {
        vec_dist_vec_tile_indices.push_back( vector<int>() );//prepare empty element
        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            if( m_vec_city_tiles[i].distance( m_vec_city_tiles[center_index] ) == curr_dist )
            {
                vec_dist_vec_tile_indices[curr_dist].push_back(i);
                m_vec_city_tiles[i].x_shift_box=0; m_vec_city_tiles[i].y_shift_box=0;//reset box shift to center
            }
        }
        //if no new tiles found, break
        if( vec_dist_vec_tile_indices[curr_dist].empty() ) break;

        //go to next distance
        curr_dist++;
    }

    //count number of city tiles found
    int tiles_found=0;
    for(int i=0;i<(int)vec_dist_vec_tile_indices.size();i++)
    {
        tiles_found+=(int)vec_dist_vec_tile_indices[i].size();
    }

    //if not all tiles are accounted for there is a border crossing
    if( tiles_found<(int)m_vec_city_tiles.size() )
    {
        cout<<"City center calc: City crosses box border\n";
        //calc rel hex coord

        //find tile indices outside border

        //make bool vec to remember tiles not found
        vector<bool> vec_tile_ind_found;
        for(int i=0;i<(int)m_vec_city_tiles.size();i++) vec_tile_ind_found.push_back(false);
        //test tiles if found
        for(int dist=0;dist<(int)vec_dist_vec_tile_indices.size();dist++)
        for(int tile_ind=0;tile_ind<(int)vec_dist_vec_tile_indices[dist].size();tile_ind++)
        {
            vec_tile_ind_found[ vec_dist_vec_tile_indices[dist][tile_ind] ]=true;
        }
        //test if adding/reducing x/y with world_size will shorten dist to center
        for(int i=0;i<(int)vec_tile_ind_found.size();i++)
        {
            if(!vec_tile_ind_found[i])//tile i not found
            {//find shorter dist, update shift_box
                st_coord_chararr coord_test_right( m_vec_city_tiles[i].x_hex+_world_size_x,
                                                   m_vec_city_tiles[i].y_hex,1,0 );
                st_coord_chararr coord_test_down_right( m_vec_city_tiles[i].x_hex+_world_size_x,
                                                        m_vec_city_tiles[i].y_hex+_world_size_y,1,1 );
                st_coord_chararr coord_test_down( m_vec_city_tiles[i].x_hex,
                                                  m_vec_city_tiles[i].y_hex+_world_size_y,0,1 );
                st_coord_chararr coord_test_down_left( m_vec_city_tiles[i].x_hex-_world_size_x,
                                                       m_vec_city_tiles[i].y_hex+_world_size_y,-1,1 );
                st_coord_chararr coord_test_left( m_vec_city_tiles[i].x_hex-_world_size_x,
                                                  m_vec_city_tiles[i].y_hex,-1,0 );
                st_coord_chararr coord_test_up_left( m_vec_city_tiles[i].x_hex-_world_size_x,
                                                     m_vec_city_tiles[i].y_hex-_world_size_y,-1,-1 );
                st_coord_chararr coord_test_up( m_vec_city_tiles[i].x_hex,
                                                m_vec_city_tiles[i].y_hex-_world_size_y,0,-1 );
                st_coord_chararr coord_test_up_right( m_vec_city_tiles[i].x_hex+_world_size_x,
                                                      m_vec_city_tiles[i].y_hex-_world_size_y,1,-1 );

                //test dist x shift
                int shortest_dist=m_vec_city_tiles[center_index].distance( m_vec_city_tiles[i] );
                //right
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_right ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_right );
                    m_vec_city_tiles[i].x_shift_box=coord_test_right.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_right.y_shift_box;
                }
                //down right
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_down_right ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_down_right );
                    m_vec_city_tiles[i].x_shift_box=coord_test_down_right.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_down_right.y_shift_box;
                }
                //down
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_down ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_down );
                    m_vec_city_tiles[i].x_shift_box=coord_test_down.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_down.y_shift_box;
                }
                //down left
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_down_left ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_down_left );
                    m_vec_city_tiles[i].x_shift_box=coord_test_down_left.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_down_left.y_shift_box;
                }
                //left
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_left ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_left );
                    m_vec_city_tiles[i].x_shift_box=coord_test_left.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_left.y_shift_box;
                }
                //up left
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_up_left ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_up_left );
                    m_vec_city_tiles[i].x_shift_box=coord_test_up_left.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_up_left.y_shift_box;
                }
                //up
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_up ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_up );
                    m_vec_city_tiles[i].x_shift_box=coord_test_up.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_up.y_shift_box;
                }
                //up right
                if( shortest_dist >= m_vec_city_tiles[center_index].distance( coord_test_up_right ) )
                {
                    shortest_dist=m_vec_city_tiles[center_index].distance( coord_test_up_right );
                    m_vec_city_tiles[i].x_shift_box=coord_test_up_right.x_shift_box;
                    m_vec_city_tiles[i].y_shift_box=coord_test_up_right.y_shift_box;
                }
            }
        }
    }
    if( tiles_found>(int)m_vec_city_tiles.size() )
    {
        cout<<"ERROR: City center calc: Found to many tiles\n";
        return false;
    }
    if( tiles_found==(int)m_vec_city_tiles.size() )
    {
        cout<<"City center calc: All city tiles are in one box\n";
    }

    //get center by average tile pos, regarding box shift
    float x_sum=0;
    float y_sum=0;
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        x_sum+=m_vec_city_tiles[i].x_hex+m_vec_city_tiles[i].x_shift_box*_world_size_x;
        y_sum+=m_vec_city_tiles[i].y_hex+m_vec_city_tiles[i].y_shift_box*_world_size_y;
    }
    int city_size=(int)m_vec_city_tiles.size();
    int new_center_x=x_sum/(float)city_size+0.5;
    int new_center_y=y_sum/(float)city_size+0.5;

    //limits within box, this will make box shift values for tiles wrong
    while(new_center_x<0)              new_center_x+=_world_size_x;
    while(new_center_x>=_world_size_x) new_center_x-=_world_size_x;
    while(new_center_y<0)              new_center_y+=_world_size_y;
    while(new_center_y>=_world_size_y) new_center_y-=_world_size_y;

    //test that this tile is part of city
    bool tile_found=false;
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        if( m_vec_city_tiles[i].x_hex==new_center_x && m_vec_city_tiles[i].y_hex==new_center_y )
        {
            tile_found=true;
            break;
        }
    }
    if(!tile_found)//center tile is not part of city
    {//find closest city tile
        int new_x=m_vec_city_tiles[0].x_hex;
        int new_y=m_vec_city_tiles[0].y_hex;
        int shortest_dist=m_vec_city_tiles[0].distance( st_coord_chararr( new_center_x,new_center_y ) );
        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            if( m_vec_city_tiles[i].distance( st_coord_chararr( new_center_x,new_center_y ) ) < shortest_dist )
            {
                shortest_dist=m_vec_city_tiles[i].distance( st_coord_chararr( new_center_x,new_center_y ) );
                new_x=m_vec_city_tiles[i].x_hex;
                new_y=m_vec_city_tiles[i].y_hex;
            }
        }
        new_center_x=new_x;
        new_center_y=new_y;
    }

    m_center_pos_x_hex=new_center_x;
    m_center_pos_y_hex=new_center_y;

    return true;
}

bool city::get_random_city_tile_furthest_from_center(int& x_hex,int& y_hex)
{
    //update center
    calc_city_center_tile();

    //find center tile index
    int center_index=-1;
    for(int i=0;i<(int)m_vec_city_tiles.size();i++)
    {
        if( m_vec_city_tiles[i].x_hex==m_center_pos_x_hex && m_vec_city_tiles[i].y_hex==m_center_pos_y_hex )
        {
            center_index=i;
            break;
        }
    }
    if(center_index==-1)
    {
        cout<<"ERROR: Getting random tile for removal: Could not find center tile index\n";
        return false;
    }

    //go throu all tile to find those that are closest
    int curr_dist=0;
    vector<int> vec_tile_indices;
    vector<int> vec_tile_indices_last_round;
    while(true)
    {
        for(int i=0;i<(int)m_vec_city_tiles.size();i++)
        {
            if( m_vec_city_tiles[i].distance( m_vec_city_tiles[center_index] ) == curr_dist )
            {
                vec_tile_indices.push_back(i);
                m_vec_city_tiles[i].x_shift_box=0; m_vec_city_tiles[i].y_shift_box=0;//reset box shift to center
            }
        }
        //test if done, if no new tiles found
        //cout<<(int)vec_tile_indices.size()<<endl;
        if( vec_tile_indices.empty() ) break;

        //store old values
        vec_tile_indices_last_round.clear();
        vec_tile_indices_last_round.insert( vec_tile_indices_last_round.begin(),vec_tile_indices.begin(),vec_tile_indices.end() );
        vec_tile_indices.clear();

        curr_dist++;//go to next circle of tiles
    }
    if( vec_tile_indices_last_round.empty() )
    {
        cout<<"ERROR: Getting random tile for removal: Could not find correct tile\n";
        return false;
    }

    //pick random tile
    int numof_tiles=(int)vec_tile_indices_last_round.size();
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

    x_hex=m_vec_city_tiles[ vec_tile_indices_last_round[winner] ].x_hex;
    y_hex=m_vec_city_tiles[ vec_tile_indices_last_round[winner] ].y_hex;

    return true;
}

bool city::make_path(vector<st_coord_route>& vec_temp_path,int& first_direction_out,
                     int new_x,int new_y,int last_x,int last_y,int test_type,int x_box_shift,int y_box_shift)
{
    cout<<"Pathing: Try to make path from: "<<last_x<<", "<<last_y<<" to: "<<new_x<<", "<<new_y<<" BOX: "<<x_box_shift<<", "<<y_box_shift<<endl;
    //try to connect new pos with last pos with a path
    //test if pos valid
    if(new_x<0||new_x>=_world_size_x||new_y<0||new_y>=_world_size_y||
       last_x<0||last_x>=_world_size_x||last_y<0||last_y>=_world_size_y)
    {
        return false;
    }
    //test if box shift needed
    st_coord_route start_coord(last_x,last_y,x_box_shift,y_box_shift);
    vector<st_coord_route> vec_target_coords;
    for(int x=-1;x<2;x++)
    for(int y=-1;y<2;y++)
    {
        vec_target_coords.push_back( st_coord_route(new_x,new_y,x_box_shift+x,y_box_shift+y) );
    }
    //find closest box
    int shortest_dist=start_coord.distance( vec_target_coords.front() );
    int shortest_box_index=0;
    for(int i=0;i<(int)vec_target_coords.size();i++)
    {
        if( start_coord.distance( vec_target_coords[i] ) < shortest_dist )
        {
            shortest_box_index=i;
            shortest_dist=start_coord.distance( vec_target_coords[i] );
        }
    }
    //cout<<"Closest box: "<<vec_target_coords[shortest_box_index].x_box_shift<<", "<<vec_target_coords[shortest_box_index].y_box_shift<<endl;
    st_coord_route target_coord=vec_target_coords[shortest_box_index];

    //take steps in new pos direction
    st_coord_route curr_coord(last_x,last_y,x_box_shift,y_box_shift);
    while(true)
    {
        //take one step
        int diff_x=(curr_coord.x_hex+curr_coord.x_box_shift*_world_size_x)-(target_coord.x_hex+curr_coord.x_box_shift*_world_size_x);
        int diff_y=(curr_coord.y_hex+curr_coord.y_box_shift*_world_size_y)-(target_coord.y_hex+curr_coord.y_box_shift*_world_size_y);

        //test if target reached
        if(diff_x==0 && diff_y==0) return true;//done

        //alternatives: -1,0 -1,1 0,-1 0,1 1,-1 1,0
        vector<st_coord_route> vec_possible_tiles;
        for(int x=-1;x<2;x++)
        for(int y=-1;y<2;y++)
        {
            if(x==y) continue;

            int temp_x=(curr_coord.x_hex+curr_coord.x_box_shift*_world_size_x)+x;
            int temp_y=(curr_coord.y_hex+curr_coord.y_box_shift*_world_size_y)+y;
            int box_shift_x=0;
            int box_shift_y=0;
            //test if within world
            while(temp_x<0) { temp_x+=_world_size_x; box_shift_x-=1; }
            while(temp_y<0) { temp_y+=_world_size_y; box_shift_y-=1; }
            while(temp_x>=_world_size_x) { temp_x-=_world_size_x; box_shift_x+=1; }
            while(temp_y>=_world_size_y) { temp_y-=_world_size_y; box_shift_y+=1; }

            switch(test_type)
            {
                case 1://EXP test, only on water/land, cant cross roads, have to be new in exp route
                {
                    bool do_not_add=false;
                    //test if tile is in path(temp)
                    for(int tile_i=0;tile_i<(int)vec_temp_path.size();tile_i++)
                    {
                        if( vec_temp_path[tile_i].x_hex==temp_x && vec_temp_path[tile_i].y_hex==temp_y )
                        {
                            do_not_add=true;
                            break;
                        }
                    }
                    if(do_not_add) break;

                    //test if tile is in exp route
                    for(int tile_i=0;tile_i<(int)m_vec_expansion_route.size();tile_i++)
                    {
                        if( m_vec_expansion_route[tile_i].x_hex==temp_x && m_vec_expansion_route[tile_i].y_hex==temp_y )
                        {
                            do_not_add=true;
                            break;
                        }
                    }
                    if(do_not_add) break;

                    //test if buildable (water or land)
                    if( m_p_arr_tiles[temp_x*_world_size_y+temp_y].get_tile_type()!=tt_water &&
                        m_p_arr_tiles[temp_x*_world_size_y+temp_y].get_tile_type()!=tt_land )
                    {//not buildable
                        break;
                    }

                    //tile is ok, add to vec
                    vec_possible_tiles.push_back( st_coord_route(temp_x,temp_y,box_shift_x,box_shift_y) );

                }break;

                case 2://trade test, can cross roads, test if on city(done)
                {
                    bool do_not_add=false;
                    //test if tile is in path(temp)
                    for(int tile_i=0;tile_i<(int)vec_temp_path.size();tile_i++)
                    {
                        if( vec_temp_path[tile_i].x_hex==temp_x && vec_temp_path[tile_i].y_hex==temp_y )
                        {
                            do_not_add=true;
                            break;
                        }
                    }
                    if(do_not_add) break;

                    //test if tile is in trade route
                    for(int tile_i=0;tile_i<(int)m_vec_trade_route.size();tile_i++)
                    {
                        if( m_vec_trade_route[tile_i].x_hex==temp_x && m_vec_trade_route[tile_i].y_hex==temp_y )
                        {
                            do_not_add=true;
                            break;
                        }
                    }
                    if(do_not_add) break;

                    //test if buildable (water or land) NO CROSSROADTEST YET...
                    if( m_p_arr_tiles[temp_x*_world_size_y+temp_y].get_tile_type()!=tt_water &&
                        m_p_arr_tiles[temp_x*_world_size_y+temp_y].get_tile_type()!=tt_land )
                    {//not buildable
                        break;
                    }

                    //tile is ok, add to vec
                    vec_possible_tiles.push_back( st_coord_route(temp_x,temp_y,box_shift_x,box_shift_y) );
                }break;
            }


        }
        if( vec_possible_tiles.empty() )
        {
            cout<<"Error: Pathing: Could not find a possible step, cancel route\n";
            return false;//could not make a path
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

        //fix road
        int last_box_shift_x=curr_coord.x_box_shift;
        int last_box_shift_y=curr_coord.y_box_shift;
        int direction_this_in=-1;
        int direction_last_out=-1;
        int dif_x=(curr_coord.x_hex+curr_coord.x_box_shift*_world_size_x) -
                  (vec_possible_tiles[shortest_dist_index].x_hex+vec_possible_tiles[shortest_dist_index].x_box_shift*_world_size_x);
        int dif_y=(curr_coord.y_hex+curr_coord.y_box_shift*_world_size_y) -
                  (vec_possible_tiles[shortest_dist_index].y_hex+vec_possible_tiles[shortest_dist_index].y_box_shift*_world_size_y);

        if(dif_x<-1 || dif_x>1 || dif_y<-1 || dif_y>1 || dif_x==dif_y)//bad
        {//cancel route
            cout<<"ERROR: Pathing: Bad tile shift: "<<dif_x<<", "<<dif_y<<endl;
            //cout<<" Curr coord: "<<curr_coord.x_hex<<", "<<curr_coord.y_hex<<endl;
            //cout<<" Next pos  : "<<vec_possible_tiles[shortest_dist_index].x_hex<<", "<<vec_possible_tiles[shortest_dist_index].y_hex<<endl;
            //cout<<" curr box  : "<<curr_coord.x_box_shift<<", "<<curr_coord.y_box_shift<<endl;
            //cout<<" next box  : "<<vec_possible_tiles[shortest_dist_index].x_box_shift<<", "<<vec_possible_tiles[shortest_dist_index].y_box_shift<<endl;
            return false;
        }

        switch(dif_x)
        {
            case -1:
            {
                switch(dif_y)
                {
                    case 0: direction_this_in=1; direction_last_out=4; break;
                    case 1: direction_this_in=0; direction_last_out=3; break;
                }
            }break;

            case 0:
            {
                switch(dif_y)
                {
                    case -1: direction_this_in=2; direction_last_out=5; break;
                    case 1: direction_this_in=5; direction_last_out=2; break;
                }
            }break;

            case 1:
            {
                switch(dif_y)
                {
                    case -1: direction_this_in=3; direction_last_out=0; break;
                    case 0: direction_this_in=4; direction_last_out=1; break;
                }
            }break;
        }

        //assign last out dir, if first, save value
        if(vec_temp_path.empty()) first_direction_out=direction_last_out;
        else vec_temp_path.back().road[1]=direction_last_out;

        //store path tile
        vec_temp_path.push_back(vec_possible_tiles[shortest_dist_index]);

        //assign this in dir
        vec_temp_path.back().road[0]=direction_this_in;

        curr_coord=vec_possible_tiles[shortest_dist_index];

        cout<<"Step taken to: "<<vec_temp_path.back().x_hex<<", "<<vec_temp_path.back().y_hex<<endl;

        //repeat
    }

    cout<<"Pathing: Complete\n";
    return true;
}
