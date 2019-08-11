#include "trade.h"

trade::trade()
{
    //ctor
}

trade::trade(vector<st_coord_route> vec_trade_route,int id,int city_id_a,int city_id_b)
{
    //copy route vector
    m_vec_trade_route.insert( m_vec_trade_route.begin(),vec_trade_route.begin(),vec_trade_route.end() );
    m_city_id_a=city_id_a;
    m_city_id_b=city_id_b;
    m_trade_id=id;
    m_anim_reverse=true;
    m_anim_trade_progress=float(m_vec_trade_route.size()-1.0)*_anim_trade_time;
    m_time_since_start=m_light_pulse=0;

    //calc bird distace (hex)
    m_distance_bird=m_vec_trade_route.front().distance( m_vec_trade_route.back() );
    cout<<"Trade: Distance short: "<<m_distance_bird<<", Distance path: "<<(int)m_vec_trade_route.size()<<endl;;
}

bool trade::update(float time)
{
    m_time_since_start+=time;

    //update light pulse
    if(m_light_pulse>0.0)
    {
        m_light_pulse-=time*1.0;
        if(m_light_pulse<0.0) m_light_pulse=0.0;
    }

    if(m_anim_reverse)//lower time progress
    {
        m_anim_trade_progress-=time;
        if( m_anim_trade_progress<0.0 )
        {
            m_anim_reverse=false;
            m_anim_trade_progress+=time;
        }
    }
    else//increase time progress
    {
        m_anim_trade_progress+=time;
        if( m_anim_trade_progress>float(m_vec_trade_route.size()-1.0)*_anim_trade_time )
        {
            m_anim_reverse=true;
            m_anim_trade_progress-=time;
        }
    }

    return true;
}

bool trade::draw(float zoom_level)
{
    float scale=zoom_level;
    float color_intesity=sinf(m_time_since_start)+m_light_pulse;

    if( zoom_level<25 ) glLineWidth(2);
    else glLineWidth(3);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //for(int layer=2;layer<3;layer++)
    for(int box_index=0;box_index<9;box_index++) //for all boxes
    {
        /*switch(layer)
        {
            case 0:
            {
                glLineWidth(6);
                glColor4f(0.9,0.9,0.9,0.3);
            }break;

            case 1:
            {
                glLineWidth(4);
                glColor4f(0.7,0.3,0.3,0.5);
            }break;

            case 2:
            {
                glLineWidth(2);
                glColor4f(0.9,0.2,0.2,0.9);
            }break;
        }*/

        //distance calc
        float rel_progress=m_anim_trade_progress/((float(m_vec_trade_route.size()-1.0)*_anim_trade_time));
        float distance_tot=(int)m_vec_trade_route.size()-1;
        float distance_progress=distance_tot*rel_progress;
        int distance_done=(int)distance_progress;
        float distance_part=distance_progress-(float)distance_done;
        int distance_part_fifths=distance_part*5.0;

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

        glBegin(GL_LINE_STRIP);
        //start part
        float x_pos_start=(float)m_vec_trade_route.front().x_hex+(m_vec_trade_route.front().x_box_shift+x_shift)*_world_size_x+
                    ((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
        float y_pos_start=((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
        glColor4f(0.0,0.0,0.0,0.0);
        glVertex2d( x_pos_start*scale,y_pos_start*scale );
        //central part
        for(int i=1;i<(int)m_vec_trade_route.size()-1;i++)
        {
            //set color progress
            float colors[5][4];
            if( i==distance_done )
            {
                for(int col_i=0;col_i<5;col_i++)
                {
                    colors[col_i][0]=0.3+0.1*color_intesity;
                    colors[col_i][1]=0.2+0.1*color_intesity;
                    colors[col_i][2]=0.2+0.1*color_intesity;
                    colors[col_i][3]=1.0;
                }
                //highlighted part
                if( distance_part_fifths>0 )
                {
                    colors[distance_part_fifths-1][0]=0.6;
                    colors[distance_part_fifths-1][1]=0.5;
                    colors[distance_part_fifths-1][2]=0.5;
                    colors[distance_part_fifths-1][3]=1.0;

                    colors[distance_part_fifths][0]=0.9;
                    colors[distance_part_fifths][1]=0.9;
                    colors[distance_part_fifths][2]=0.9;
                    colors[distance_part_fifths][3]=1.0;
                }
                if( distance_part_fifths<4 )
                {
                    colors[distance_part_fifths+1][0]=0.6;
                    colors[distance_part_fifths+1][1]=0.5;
                    colors[distance_part_fifths+1][2]=0.5;
                    colors[distance_part_fifths+1][3]=1.0;
                }
            }
            else//outside range
            {
                for(int col_i=0;col_i<5;col_i++)
                {
                    colors[col_i][0]=0.3+0.1*color_intesity;
                    colors[col_i][1]=0.2+0.1*color_intesity;
                    colors[col_i][2]=0.2+0.1*color_intesity;
                    colors[col_i][3]=1.0;
                }
            }

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
                case 1: switch(m_vec_trade_route[i].road[1])
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
                case 2: switch(m_vec_trade_route[i].road[1])
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
                case 3: switch(m_vec_trade_route[i].road[1])
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
                case 4: switch(m_vec_trade_route[i].road[1])
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
                case 5: switch(m_vec_trade_route[i].road[1])
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
        float x_pos_end=(float)m_vec_trade_route.back().x_hex+(m_vec_trade_route.back().x_box_shift+x_shift)*_world_size_x+
                    ((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
        float y_pos_end=((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
        glColor4f(0.0,0.0,0.0,0.0);
        glVertex2d( x_pos_end*scale,y_pos_end*scale );

        glEnd();

        /*//old method
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<(int)m_vec_trade_route.size();i++)
        {
            //calc tile pos
            float x_pos=(float)m_vec_trade_route[i].x_hex+(m_vec_trade_route[i].x_box_shift+x_shift)*_world_size_x+
                        ((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.5;
            float y_pos=((float)m_vec_trade_route[i].y_hex+(m_vec_trade_route[i].y_box_shift+y_shift)*_world_size_y)*0.75;

            glVertex2d( x_pos*scale,y_pos*scale );
        }
        glEnd();*/
    }



    //again
    /*glLineWidth(1);
    glBegin(GL_LINE_STRIP);
    for(int box_index=0;box_index<9;box_index++) //for all boxes
    {
        //distance calc
        float rel_progress=m_anim_trade_progress/((float(m_vec_trade_route.size()-1.0)*_anim_trade_time));
        float distance_tot=(int)m_vec_trade_route.size()-1;
        float distance_progress=distance_tot*rel_progress;
        int distance_done=(int)distance_progress;
        float distance_part=distance_progress-(float)distance_done;
        int distance_part_fifths=distance_part*5.0;

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

        glBegin(GL_LINES);
        //start part
        float x_pos_start=(float)m_vec_trade_route.front().x_hex+(m_vec_trade_route.front().x_box_shift+x_shift)*_world_size_x+
                    ((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.5;
        float y_pos_start=((float)m_vec_trade_route.front().y_hex+(m_vec_trade_route.front().y_box_shift+y_shift)*_world_size_y)*0.75;
        glColor4f(0.0,0.0,0.0,0.0);
        glVertex2d( x_pos_start*scale,y_pos_start*scale );
        //central part
        for(int i=1;i<(int)m_vec_trade_route.size()-1;i++)
        {
            //set color progress
            float colors[5][4];
            if( i==distance_done )
            {
                for(int col_i=0;col_i<5;col_i++)
                {
                    colors[col_i][0]=0.1+0.1*color_intesity;
                    colors[col_i][1]=0.1+0.1*color_intesity;
                    colors[col_i][2]=0.1+0.1*color_intesity;
                    colors[col_i][3]=1.0;
                }
                //highlighted part
                if( distance_part_fifths>0 )
                {
                    colors[distance_part_fifths-1][0]=0.6;
                    colors[distance_part_fifths-1][1]=0.5;
                    colors[distance_part_fifths-1][2]=0.5;
                    colors[distance_part_fifths-1][3]=1.0;

                    colors[distance_part_fifths][0]=0.9;
                    colors[distance_part_fifths][1]=0.9;
                    colors[distance_part_fifths][2]=0.9;
                    colors[distance_part_fifths][3]=1.0;
                }
                if( distance_part_fifths<4 )
                {
                    colors[distance_part_fifths+1][0]=0.6;
                    colors[distance_part_fifths+1][1]=0.5;
                    colors[distance_part_fifths+1][2]=0.5;
                    colors[distance_part_fifths+1][3]=1.0;
                }
            }
            else//outside range
            {
                for(int col_i=0;col_i<5;col_i++)
                {
                    colors[col_i][0]=0.1+0.1*color_intesity;
                    colors[col_i][1]=0.1+0.1*color_intesity;
                    colors[col_i][2]=0.1+0.1*color_intesity;
                    colors[col_i][3]=1.0;
                }
            }

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
                case 1: switch(m_vec_trade_route[i].road[1])
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
                case 2: switch(m_vec_trade_route[i].road[1])
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
                case 3: switch(m_vec_trade_route[i].road[1])
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
                case 4: switch(m_vec_trade_route[i].road[1])
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
                case 5: switch(m_vec_trade_route[i].road[1])
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
        float x_pos_end=(float)m_vec_trade_route.back().x_hex+(m_vec_trade_route.back().x_box_shift+x_shift)*_world_size_x+
                    ((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.5;
        float y_pos_end=((float)m_vec_trade_route.back().y_hex+(m_vec_trade_route.back().y_box_shift+y_shift)*_world_size_y)*0.75;
        glColor4f(0.0,0.0,0.0,0.0);
        glVertex2d( x_pos_end*scale,y_pos_end*scale );

        glEnd();
    }*/




    glLineWidth(1);
    glDisable(GL_BLEND);

    return true;
}

bool trade::get_trade_route_list(vector<st_coord_route>& vec_trade_route_list)
{
    vec_trade_route_list=m_vec_trade_route;

    return true;
}

int trade::get_trade_id(void)
{
    return m_trade_id;
}

int trade::get_city_a_id(void)
{
    return m_city_id_a;
}

int trade::get_city_b_id(void)
{
    return m_city_id_b;
}

int trade::get_trade_distance(void)
{
    return m_distance_bird;
}

bool trade::remove_tile_from_route(int x_hex,int y_hex)
{
    for(int i=0;i<(int)m_vec_trade_route.size();i++)
    {
        if( m_vec_trade_route[i].x_hex==x_hex && m_vec_trade_route[i].y_hex==y_hex )
        {
            m_vec_trade_route.erase( m_vec_trade_route.begin()+i );
            return true;//tile found
        }
    }

    return false;
}

bool trade::test_trade(void)
{
    //test if start pos is a city (done in World)


    return true;
}

bool trade::city_expansion_update(int from_x_hex,int from_y_hex,int to_x_hex,int to_y_hex)
{
    //remove tiles from trade route that is between new tile pos and tile with expanding city
    //remove tiles selected tiles range
    //find index of these tiles
    int from_index=-1;
    int to_index=-1;
    for(int i=0;i<(int)m_vec_trade_route.size();i++)
    {
        if( m_vec_trade_route[i].x_hex==from_x_hex && m_vec_trade_route[i].y_hex==from_y_hex )
        {
            from_index=i;
        }
        if( m_vec_trade_route[i].x_hex==to_x_hex && m_vec_trade_route[i].y_hex==to_y_hex )
        {
            to_index=i;
        }
    }
    if( from_index==-1 || to_index==-1 )//bad
    {
        cout<<"ERROR: Trade route update: Could not find selected tiles index\n";
        return false;
    }
    if(from_index!=0)
    {
        //to remove the last element and not the selected
        from_index+=1;
        to_index+=1;
    }

    m_vec_trade_route.erase( m_vec_trade_route.begin()+from_index, m_vec_trade_route.begin()+to_index );

    return true;
}

bool trade::set_light_pulse(float value)
{
    m_light_pulse=value;
    return true;
}


//---Private---
