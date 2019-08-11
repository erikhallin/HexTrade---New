#include "text_input_box.h"

#include <stdlib.h> //atof()
#include <cctype> //isdigit()
#include <sstream>

#define _key_delay 0.2

text_input_box::text_input_box()
{
    m_ready=false;
    m_marked=false;
    m_sound_enabled=false;
}

text_input_box::text_input_box(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text,sound* pSound)
{
    for(int i=0;i<256;i++) m_key_delay[i]=0;//reset delay
    m_time_last_cycle=0;
    m_ready=true;
    m_marked=false;
    m_caps_lock=false;
    m_only_number_input=false;
    m_max_char=max_char;
    m_text_alignment=alig_left;
    m_show_texture=false;

    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_diTextBox=display(x_pos,y_pos,width,height,max_char,font_texture,text);
    m_diTextBox.set_text_alignment(m_text_alignment);

    m_sound_enabled=true;
    m_pSound=pSound;
}

text_input_box::text_input_box(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text)
{
    for(int i=0;i<256;i++) m_key_delay[i]=0;//reset delay
    m_time_last_cycle=0;
    m_ready=true;
    m_marked=false;
    m_caps_lock=false;
    m_only_number_input=false;
    m_max_char=max_char;
    m_text_alignment=alig_left;
    m_show_texture=false;

    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_diTextBox=display(x_pos,y_pos,width,height,max_char,font_texture,text);
    m_diTextBox.set_text_alignment(m_text_alignment);

    m_sound_enabled=false;
}

bool text_input_box::input_text(string text_string)
{
    m_diTextBox.set_text(text_string);

    return true;
}

bool text_input_box::mark_test(int mouse_x_pos,int mouse_y_pos)
{
    if(mouse_x_pos>m_x_pos&&mouse_x_pos<m_x_pos+m_width&&
       mouse_y_pos>m_y_pos&&mouse_y_pos<m_y_pos+m_height)
    {
        if(!m_marked)
        {
            if(m_show_texture)
            {//texture
                m_diTextBox.set_back_texture(m_back_texture,m_tex_max_x,m_tex_min_x,
                                             m_tex_max_y+m_texture_shift,m_tex_min_y+m_texture_shift);
            }
            else
            {
                //border color
                float color[]={0,0,1};
                m_diTextBox.setting_flags(true,color);
            }
            //play sound
            //if(m_sound_enabled) m_pSound->playSimpleSound(wav_button_push,1.0f);
        }

        m_marked=true;

        return true;
    }
    else //unmark
    {
        if(m_marked)
        {
            if(m_show_texture)
            {
                m_diTextBox.set_back_texture(m_back_texture,m_tex_max_x,m_tex_min_x,
                                             m_tex_max_y,m_tex_min_y);
            }
            else
            {
                float color[]={0.4,0.4,0.4};
                m_diTextBox.setting_flags(true,color);
            }
        }

        m_marked=false;
    }

    return false;
}

bool text_input_box::is_marked(void)
{
    if(m_marked) return true;

    return false;
}

bool text_input_box::unmark(void)
{
    m_marked=false;

    if(m_show_texture)
    {
        m_diTextBox.set_back_texture(m_back_texture,m_tex_max_x,m_tex_min_x,m_tex_max_y,m_tex_min_y);
    }
    else
    {
        float color[]={0.4,0.4,0.4};
        m_diTextBox.setting_flags(true,color);
    }
    return true;
}

bool text_input_box::change_border_color(float color[3])
{
    m_diTextBox.setting_flags(true,color);

    return true;
}

bool text_input_box::set_only_number(bool flag)
{
    m_only_number_input=flag;

    return true;
}

bool text_input_box::update(bool keys[],float time)
{
    //timing
    for(int i=0;i<256;i++)
    {
        if(m_key_delay[i]>0)
        {
            m_key_delay[i]-=time-m_time_last_cycle;
        }
        //if(keys[i]) cout<<i<<endl; //TEMP
    }
    m_time_last_cycle=time;

    if(!m_marked) return false;

    string sBefore_input=m_text;

    //check for function key input
    bool key_shift=false;
    //bool key_alt_left=false; XXXXX
    //bool key_alt_right=false; XXXXX
    if(keys[16])//shift
    {
        key_shift=true;
    }
    else key_shift=false;

    if(keys[20]&&m_key_delay[20]<=0)//caps toogle
    {
        m_caps_lock=!m_caps_lock;
        m_key_delay[13]=_key_delay;
    }
    if(keys[8] &&m_key_delay[8] <=0)//backspace
    {
        erase_char();
        m_key_delay[8]=_key_delay;
        m_diTextBox.set_text(m_text);
    }

    //special keys
    if(keys[190] &&m_key_delay[190] <=0)//.
    {
        m_key_delay[190]=_key_delay;
        if(key_shift) add_char(':');
        else add_char('.');
        m_diTextBox.set_text(m_text);
    }
    if(keys[188] &&m_key_delay[188] <=0)//,
    {
        m_key_delay[188]=_key_delay;
        if(key_shift) add_char(';');
        else add_char(',');
        m_diTextBox.set_text(m_text);
    }
    if(keys[189] &&m_key_delay[189] <=0)//-
    {
        m_key_delay[189]=_key_delay;
        if(key_shift) add_char('_');
        else add_char('-');
        m_diTextBox.set_text(m_text);
    }
    if(keys[189] &&m_key_delay[189] <=0)//-
    {
        m_key_delay[189]=_key_delay;
        if(key_shift) add_char('_');
        else add_char('-');
        m_diTextBox.set_text(m_text);
    }
    if(keys[187] &&m_key_delay[187] <=0)//+/?
    {
        m_key_delay[187]=_key_delay;
        if(key_shift) add_char('?');
        else add_char('+');
        m_diTextBox.set_text(m_text);
    }

    if(keys[221] &&m_key_delay[221] <=0)//å
    {
        m_key_delay[221]=_key_delay;
        if(key_shift)
        {
            if(m_caps_lock) add_char('\x86');
            else            add_char('\x8F');
        }
        else
        {
            if(m_caps_lock) add_char('\x8F');
            else            add_char('\x86');
        }
        m_diTextBox.set_text(m_text);
    }
    if(keys[222] &&m_key_delay[222] <=0)//ä
    {
        m_key_delay[222]=_key_delay;
        if(key_shift)
        {
            if(m_caps_lock) add_char('\x84');
            else            add_char('\x8E');
        }
        else
        {
            if(m_caps_lock) add_char('\x8E');
            else            add_char('\x84');
        }
        m_diTextBox.set_text(m_text);
    }
    if(keys[192] &&m_key_delay[192] <=0)//ö
    {
        m_key_delay[192]=_key_delay;
        if(key_shift)
        {
            if(m_caps_lock) add_char('\x94');
            else            add_char('\x99');
        }
        else
        {
            if(m_caps_lock) add_char('\x99');
            else            add_char('\x94');
        }
        m_diTextBox.set_text(m_text);
    }
    //Numpad
    if(keys[96] &&m_key_delay[96] <=0)//0
    {
        m_key_delay[96]=_key_delay;
        add_char('0');
        m_diTextBox.set_text(m_text);
    }
    if(keys[97] &&m_key_delay[97] <=0)//1
    {
        m_key_delay[97]=_key_delay;
        add_char('1');
        m_diTextBox.set_text(m_text);
    }
    if(keys[98] &&m_key_delay[98] <=0)//2
    {
        m_key_delay[98]=_key_delay;
        add_char('2');
        m_diTextBox.set_text(m_text);
    }
    if(keys[99] &&m_key_delay[99] <=0)//3
    {
        m_key_delay[99]=_key_delay;
        add_char('3');
        m_diTextBox.set_text(m_text);
    }
    if(keys[100] &&m_key_delay[100] <=0)//4
    {
        m_key_delay[100]=_key_delay;
        add_char('4');
        m_diTextBox.set_text(m_text);
    }
    if(keys[101] &&m_key_delay[101] <=0)//5
    {
        m_key_delay[101]=_key_delay;
        add_char('5');
        m_diTextBox.set_text(m_text);
    }
    if(keys[102] &&m_key_delay[102] <=0)//6
    {
        m_key_delay[102]=_key_delay;
        add_char('6');
        m_diTextBox.set_text(m_text);
    }
    if(keys[103] &&m_key_delay[103] <=0)//7
    {
        m_key_delay[103]=_key_delay;
        add_char('7');
        m_diTextBox.set_text(m_text);
    }
    if(keys[104] &&m_key_delay[104] <=0)//8
    {
        m_key_delay[104]=_key_delay;
        add_char('8');
        m_diTextBox.set_text(m_text);
    }
    if(keys[105] &&m_key_delay[105] <=0)//9
    {
        m_key_delay[105]=_key_delay;
        add_char('9');
        m_diTextBox.set_text(m_text);
    }
    if(keys[111] &&m_key_delay[111] <=0)///
    {
        m_key_delay[111]=_key_delay;
        add_char('/');
        m_diTextBox.set_text(m_text);
    }
    if(keys[106] &&m_key_delay[106] <=0)//*
    {
        m_key_delay[106]=_key_delay;
        add_char('*');
        m_diTextBox.set_text(m_text);
    }
    if(keys[109] &&m_key_delay[109] <=0)//-
    {
        m_key_delay[109]=_key_delay;
        add_char('-');
        m_diTextBox.set_text(m_text);
    }
    if(keys[107] &&m_key_delay[107] <=0)//+
    {
        m_key_delay[107]=_key_delay;
        add_char('+');
        m_diTextBox.set_text(m_text);
    }
    if(keys[110] &&m_key_delay[110] <=0)//,
    {
        m_key_delay[110]=_key_delay;
        add_char(',');
        m_diTextBox.set_text(m_text);
    }

    //check for char input
    for(int i=32;i<256;i++) // 0 / 32 - 91 / 127 / 256
    {
        if(keys[i]&&m_key_delay[i]<=0)
        {
            if(key_shift) //caps or symbol
            {
                if(isalpha(int_to_char(i)))//alphabetic
                {
                    if(m_caps_lock)
                    {
                        add_char(tolower(int_to_char(i)));//small
                    }
                    else add_char(int_to_char(i));//big
                }
                else //convert number to symbol
                {
                    if(i==48) add_char(int_to_char(61));//shift+0 to =
                    else      add_char(int_to_char(i-16));
                }
            }
            else //not shift
            {
                if(m_caps_lock)
                {
                    add_char(int_to_char(i));//big
                }
                else add_char(tolower(int_to_char(i)));//small
            }
            m_key_delay[i]=_key_delay;
            m_diTextBox.set_text(m_text);
        }
    }

    if(m_text!=sBefore_input) return true;//change happend

    return false;
}

bool text_input_box::set_text(string sText)
{
    m_text=sText;
    m_diTextBox.set_text(sText);
    return true;
}

string text_input_box::get_text(void)
{
    return m_text;
}

bool text_input_box::set_value(float value)
{
    stringstream ss;
    ss<<value;
    string sValue = ss.str();
    m_text=sValue;
    m_diTextBox.set_text(m_text);

    return true;
}

float text_input_box::get_value(void)
{
    float value=atof( m_text.c_str() );

    return value;
}

bool text_input_box::draw(void)
{
    m_diTextBox.draw_display();

    return true;
}

bool text_input_box::set_text_alignment(int value)
{
    m_text_alignment=value;
    m_diTextBox.set_text_alignment(m_text_alignment);

    return true;
}

bool text_input_box::set_back_texture(int texture,int tex_max_x,int tex_min_x,int tex_max_y,int tex_min_y,int shift_y)
{
    m_show_texture=true;
    m_back_texture=texture;
    m_tex_max_x=tex_max_x;
    m_tex_min_x=tex_min_x;
    m_tex_max_y=tex_max_y;
    m_tex_min_y=tex_min_y;
    m_texture_shift=shift_y;

    m_diTextBox.set_back_texture(texture,tex_max_x,tex_min_x,tex_max_y,tex_min_y);

    return true;
}


//Private

bool text_input_box::add_char(char letter)
{
    if( m_only_number_input ) //only numbers
    {
        if( isdigit(letter) )
        {
            if( (int)m_text.size() < m_max_char)
            {
                m_text.append(1,letter);
                //play sound
                if(m_sound_enabled) m_pSound->playSimpleSound(wav_key_input,0.6f);
                return true;
            }
        }
        /*if( letter=='.' || letter==',' ) //allows many ,/. atm
        {
            if( (int)m_text.size() < m_max_char) m_text.append(1,'.');
            return true;
        }*/

        return false;//not number or dot, or max char reached
    }
    else if( (int)m_text.size() < m_max_char)
    {
        m_text.append(1,letter); //all symbols allowed
        //play sound
        if(m_sound_enabled) m_pSound->playSimpleSound(wav_key_input,0.6f);
    }

    return true;
}

bool text_input_box::erase_char(void)
{
    if((int)m_text.size()>0)
    {
        m_text.erase( m_text.end()-1 );
        m_diTextBox.set_text(m_text);
        //play sound
        if(m_sound_enabled) m_pSound->playSimpleSound(wav_key_input,0.6f);
    }

    return true;
}

char text_input_box::int_to_char(int key_id)
{
    //int to char conversion
    switch(key_id)
    {
        case 32: return ' ';
        case 33: return '!';
        case 34: return '"';
        case 35: return '#';
        case 36: return '$';
        case 37: return '%';
        case 38: return '&';
        case 39: return '/';
        case 40: return '(';
        case 41: return ')';
        case 42: return '*';
        case 43: return '+';
        case 44: return ',';
        case 45: return '-';
        case 46: return '.';
        case 47: return '/';

        case 48: return '0';
        case 49: return '1';
        case 50: return '2';
        case 51: return '3';
        case 52: return '4';
        case 53: return '5';
        case 54: return '6';
        case 55: return '7';
        case 56: return '8';
        case 57: return '9';

        case 58: return ':';
        case 59: return ';';
        case 60: return '<';
        case 61: return '=';
        case 62: return '>';
        case 63: return '?';
        case 64: return '@';

        case 65: return 'A';
        case 66: return 'B';
        case 67: return 'C';
        case 68: return 'D';
        case 69: return 'E';
        case 70: return 'F';
        case 71: return 'G';
        case 72: return 'H';
        case 73: return 'I';
        case 74: return 'J';
        case 75: return 'K';
        case 76: return 'L';
        case 77: return 'M';
        case 78: return 'N';
        case 79: return 'O';
        case 80: return 'P';
        case 81: return 'Q';
        case 82: return 'R';
        case 83: return 'S';
        case 84: return 'T';
        case 85: return 'U';
        case 86: return 'V';
        case 87: return 'W';
        case 88: return 'X';
        case 89: return 'Y';
        case 90: return 'Z';

        case 91: return '[';
        case 92: return '\\';
        case 93: return ']';
        case 94: return '^';
        case 95: return '_';
        case 96: return '`';

        case 97: return 'a';
        case 98: return 'b';
        case 99: return 'c';
        case 100: return 'd';
        case 101: return 'e';
        case 102: return 'f';
        case 103: return 'g';
        case 104: return 'h';
        case 105: return 'i';
        case 106: return 'j';
        case 107: return 'k';
        case 108: return 'l';
        case 109: return 'm';
        case 110: return 'n';
        case 111: return 'o';
        case 112: return 'p';
        case 113: return 'q';
        case 114: return 'r';
        case 115: return 's';
        case 116: return 't';
        case 117: return 'u';
        case 118: return 'v';
        case 119: return 'w';
        case 120: return 'x';
        case 121: return 'y';
        case 122: return 'z';

        case 123: return '{';
        case 124: return '|';
        case 125: return '}';
        case 126: return '~';

    }
    return '@';//error
}
