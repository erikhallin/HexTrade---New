#include "text_list.h"

text_list::text_list()
{
    m_ready=false;
}

bool text_list::init(int x_pos,int y_pos,int width,int height,int font_texture[3])
{
    //clear vectors to be safe
    m_vDisplays_names.clear();
    m_vPlayer_names.clear();

    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;
    m_numof_players=0;
    m_font_textures[0]=font_texture[0];
    m_font_textures[1]=font_texture[1];
    m_font_textures[2]=font_texture[2];
    m_text_alignment=alig_left;

    m_LMB_delay=0;

    m_ready=true;

    return true;
}

bool text_list::add_player(string name)
{
    if(name!="") //noname, will refresh current player list (used for removing player)
    {
        m_numof_players++;
        m_vPlayer_names.push_back(name);
    }

    //recreate all displays and buyout buttons
    m_vDisplays_names.clear();
    for(int i=0;i<m_numof_players;i++)
    {
        float x_name_start=0.0; float y_start=0.01;
        float y_shift=(1-y_start)/(float)m_numof_players;
        if(y_shift>0.3) y_shift=0.3; //cut-off

        float x_name_end=1.0;

        m_vDisplays_names.push_back( display( int(m_x_pos+float(m_width)*(x_name_start)), int(m_y_pos+float(m_height)*(y_start+y_shift*(float)i)),
                                              int((x_name_end-x_name_start)*(float)m_width), int(y_shift*(float)m_height), 20, m_font_textures, m_vPlayer_names[i] ) );
        //turn off border
        m_vDisplays_names.back().setting_flags(false);
        //align text
        m_vDisplays_names.back().set_text_alignment(m_text_alignment);
        //set text color
        m_vDisplays_names.back().set_text_color(m_text_color);
    }

    return true;
}

bool text_list::remove_player(string name)
{
    //find player
    int player_id=-1;
    for(int i=0;i<(int)m_vPlayer_names.size();i++)
    {
        if(m_vPlayer_names[i]==name)
        {
            player_id=i;
            break;
        }
    }
    if(player_id==-1) return false;

    m_vPlayer_names.erase( m_vPlayer_names.begin() + player_id );
    m_numof_players--;

    add_player(""); //recreates buttons and displays for the rest of the players

    return true;
}

bool text_list::clear_list(void)
{
    m_numof_players=0;
    m_vPlayer_names.clear();
    m_vDisplays_names.clear();

    return true;
}

bool text_list::draw(void)
{
    if(!m_ready) return false;

    for(int i=0;i<(int)m_vDisplays_names.size();i++)
    {
        m_vDisplays_names[i].draw_display();
    }

    return true;
}

bool text_list::set_text_alignment(int value)
{
    m_text_alignment=value;

    return true;
}

bool text_list::set_text_color(float colorRGB[3])
{
    m_text_color[0]=colorRGB[0];
    m_text_color[1]=colorRGB[1];
    m_text_color[2]=colorRGB[2];

    for(int i=0;i<(int)m_vDisplays_names.size();i++)
    {
        m_vDisplays_names[i].set_text_color(colorRGB);
    }

    return true;
}
