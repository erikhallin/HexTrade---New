#ifndef TEXT_LIST_H
#define TEXT_LIST_H

#include "display.h"
#include "button.h"
#include <string>
#include <vector>

#define _click_delay 0.2

using namespace std;

class text_list
{
    public:
        //Constructors
        text_list();
        //Variables
        bool m_ready;
        //Functions
        bool init(int x_pos,int y_pos,int width,int height,int font_texture[3]);
        bool add_player(string name);
        bool remove_player(string name);
        bool clear_list(void);
        bool draw(void);
        bool set_text_alignment(int value);//0 left, 1 right, 2 center
        bool set_text_color(float colorRGB[3]);

    private:
        //Variables
        int    m_x_pos,m_y_pos,m_width,m_height,m_texture;
        int    m_numof_players;
        int    m_font_textures[3];
        int    m_text_alignment;
        float  m_text_color[3];

        float  m_LMB_delay;

        vector<display>  m_vDisplays_names;//have constant index value
        vector<string>   m_vPlayer_names;
        //Functions
        bool   sort_list(void);
};

#endif // TEXT_LIST_H
