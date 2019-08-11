#include "menu.h"

#define _click_delay 0.2
#define _graph_delay 0.05

menu::menu()
{
    m_ready=false;
}

bool menu::init(int window_width,int window_height,int texture_gameover,int texture_buttons,int texture_title,int texture_help[8],
                int texture_font[3],float time,
                bool* pKeys,bool* pMouse_buttons,int* pMouse_pos,sound* pSound)
{
    m_pKeys=pKeys;
    m_pMouse_buttons=pMouse_buttons;
    m_pMouse_pos=pMouse_pos;
    m_pSound=pSound;

    m_last_time=0;
    float x_pos,y_pos,width,height;
    float tex_x_min,tex_x_max,tex_y_min,tex_y_max;
    m_help_curr=0;

    //create title decal
    x_pos= 0.0 *(float)window_width; y_pos= 0.0 *(float)window_height; width= 1.00 *(float)window_width; height= 0.477 *(float)window_height;
    //x_pos= 0.045 *(float)window_width; y_pos= 0.10 *(float)window_height; width= 0.90 *(float)window_width; height= 0.40 *(float)window_height;
    //tex_x_min=0; tex_x_max=720; tex_y_min=0; tex_y_max=257;
    tex_x_min=1; tex_x_max=1023; tex_y_min=1; tex_y_max=365;
    m_deTitle=decal( x_pos,y_pos,width,height,texture_title,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    //game over decal
    x_pos= 0.0 *(float)window_width; y_pos= 0.07 *(float)window_height; width= 1.00 *(float)window_width; height= 0.2344 *(float)window_height;
    tex_x_min=1; tex_x_max=1023; tex_y_min=1; tex_y_max=179;
    m_deGameover=decal( x_pos,y_pos,width,height,texture_gameover,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    //Help decals
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[0]=decal( 0,0,window_width,window_height,texture_help[0],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[0].show_decal(false);
    m_deHelp[0].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[1]=decal( 0,0,window_width,window_height,texture_help[1],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[1].show_decal(false);
    m_deHelp[1].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[2]=decal( 0,0,window_width,window_height,texture_help[2],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[2].show_decal(false);
    m_deHelp[2].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[3]=decal( 0,0,window_width,window_height,texture_help[3],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[3].show_decal(false);
    m_deHelp[3].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[4]=decal( 0,0,window_width,window_height,texture_help[4],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[4].show_decal(false);
    m_deHelp[4].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[5]=decal( 0,0,window_width,window_height,texture_help[5],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[5].show_decal(false);
    m_deHelp[5].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[6]=decal( 0,0,window_width,window_height,texture_help[6],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[6].show_decal(false);
    m_deHelp[6].masking(false);
    tex_x_min=0; tex_x_max=window_width; tex_y_min=0; tex_y_max=window_height;
    m_deHelp[7]=decal( 0,0,window_width,window_height,texture_help[7],tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deHelp[7].show_decal(false);
    m_deHelp[7].masking(false);

    //credits decal
    x_pos= window_width - 0.363 *(float)window_width-10; y_pos= window_height - 0.0482 *(float)window_height-1;
    width= 0.363 *(float)window_width; height= 0.0482 *(float)window_height;
    tex_x_min=0; tex_x_max=372; tex_y_min=986; tex_y_max=1023;
    m_deCred=decal( x_pos,y_pos,width,height,texture_title,tex_x_min,tex_x_max,tex_y_min,tex_y_max );
    m_deCred.masking(false);
    m_deCred.draw_transparent(true);

    //version display
    stringstream ss;
    ss<<_version;
    x_pos= 0.01 *(float)window_width; y_pos= 0.97 *(float)window_height; width= 0.15 *(float)window_width; height= 0.032 *(float)window_height;
    m_diVersion=display( x_pos,y_pos,width,height,10,texture_font,ss.str() );
    m_diVersion.setting_flags(false);

    //create buttons
    x_pos= 0.09 *(float)window_width; y_pos= 0.50 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=1; tex_x_max=318; tex_y_min=0; tex_y_max=99;
    m_buTest= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.09 *(float)window_width; y_pos= 0.65 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=684; tex_x_max=1001; tex_y_min=0; tex_y_max=99;
    m_buHost= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.09 *(float)window_width; y_pos= 0.80 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=343; tex_x_max=660; tex_y_min=317; tex_y_max=416;
    m_buEdit= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.50 *(float)window_height; width= 0.2197 *(float)window_width; height= 0.099 *(float)window_height;
    tex_x_min=338; tex_x_max=560; tex_y_min=733; tex_y_max=809;
    m_buEdit_sound= button( x_pos,y_pos,width,height,texture_title,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.65 *(float)window_height; width= 0.2197 *(float)window_width; height= 0.099 *(float)window_height;
    tex_x_min=584; tex_x_max=806; tex_y_min=733; tex_y_max=809;
    m_buEdit_music= button( x_pos,y_pos,width,height,texture_title,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.50 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=343; tex_x_max=660; tex_y_min=0; tex_y_max=99;
    m_buHelp=button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.65 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=1; tex_x_max=318; tex_y_min=317; tex_y_max=416;
    m_buJoin= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.80 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=684; tex_x_max=1001; tex_y_min=317; tex_y_max=416;
    m_buExit= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.586 *(float)window_width; y_pos= 0.80 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=343; tex_x_max=660; tex_y_min=634; tex_y_max=733;
    m_buNext= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    x_pos= 0.09 *(float)window_width; y_pos= 0.80 *(float)window_height; width= 0.322 *(float)window_width; height= 0.130 *(float)window_height;
    tex_x_min=1; tex_x_max=318; tex_y_min=634; tex_y_max=733;
    m_buBack= button( x_pos,y_pos,width,height,texture_buttons,tex_x_min,tex_x_max,tex_y_min,tex_y_max );

    //create text boxes
    //float color[3]={0.4,0.4,0.4};
    x_pos= 0.09 *(float)window_width; y_pos= 0.58 *(float)window_height; width= 0.33 *(float)window_width; height= 0.06 *(float)window_height;
    m_tibPlayer_name=text_input_box( x_pos,y_pos,width,height,20,texture_font,"Player Name",m_pSound );
    //m_tibPlayer_name.change_border_color(color);
    m_tibPlayer_name.set_text_alignment(alig_center);
    m_tibPlayer_name.set_back_texture(texture_title,0,339,733,783,50);

    x_pos= 0.586 *(float)window_width; y_pos= 0.58 *(float)window_height; width= 0.33 *(float)window_width; height= 0.06 *(float)window_height;
    m_tibIP_number  =text_input_box( x_pos,y_pos,width,height,20,texture_font," IP number",m_pSound );
    //m_tibIP_number.change_border_color(color);
    m_tibIP_number.set_back_texture(texture_title,0,339,733,783,50);

    x_pos= 0.586 *(float)window_width; y_pos= 0.51 *(float)window_height; width= 0.33 *(float)window_width; height= 0.06 *(float)window_height;
    m_tibLimit_time  =text_input_box( x_pos,y_pos,width,height,20,texture_font,"Time Limit",m_pSound );
    //m_tibLimit_time.change_border_color(color);
    m_tibLimit_time.set_only_number(true);
    m_tibLimit_time.set_text_alignment(alig_center);
    m_tibLimit_time.set_back_texture(texture_title,0,339,733,783,50);

    x_pos= 0.586 *(float)window_width; y_pos= 0.65 *(float)window_height; width= 0.33 *(float)window_width; height= 0.06 *(float)window_height;
    m_tibLimit_tile  =text_input_box( x_pos,y_pos,width,height,20,texture_font,"Tile Limit",m_pSound );
    //m_tibLimit_tile.change_border_color(color);
    m_tibLimit_tile.set_only_number(true);
    m_tibLimit_tile.set_text_alignment(alig_center);
    m_tibLimit_tile.set_back_texture(texture_title,0,339,733,783,50);

    //player list for lobby
    x_pos=0.2 *(float)window_width; y_pos=0.45 *(float)window_height; width=0.6 *(float)window_width; height=0.35 *(float)window_height;
    m_tlLobby_players.init( x_pos,y_pos,width,height,texture_font );
    m_tlLobby_players.set_text_alignment(alig_center);
    float text_color[]={0.9,0.9,0.9};
    m_tlLobby_players.set_text_color(text_color);

    //player list for game over
    x_pos=0.2 *(float)window_width; y_pos=0.33 *(float)window_height; width=0.3 *(float)window_width; height=0.40 *(float)window_height;
    m_tlPlayers.init( x_pos,y_pos,width,height,texture_font );
    m_tlPlayers.set_text_alignment(alig_left);
    m_tlPlayers.set_text_color(text_color);
    m_tlPlayers.add_player("Player");

    //score list for game over
    x_pos=0.5 *(float)window_width; y_pos=0.33 *(float)window_height; width=0.3 *(float)window_width; height=0.40 *(float)window_height;
    m_tlScore.init( x_pos,y_pos,width,height,texture_font );
    m_tlScore.set_text_alignment(alig_right);
    m_tlScore.set_text_color(text_color);
    m_tlScore.add_player("Tiles");

    m_show_help=false;

    m_ready=true;
    m_menu_state=ms_main;

    return true;
}

int menu::update(float time)
{
    if(m_LMB_delay>0)
    {
        m_LMB_delay-=time-m_last_time;
    }

    /*//unmarking tibs
    if( m_pMouse_buttons[0] && m_LMB_delay<=0 )
    {
        //m_LMB_delay=_click_delay;
        //unmark all interactive text box
        m_tibPlayer_name.unmark();
        m_tibIP_number.unmark();
        m_tibLimit_time.unmark();
        m_tibLimit_tile.unmark();
    }*/

    switch(m_menu_state)
    {
        case ms_main:
        {
            //update title decal brightness
            float brightness=0.5+(sinf(time*0.5)*sinf(time*2.0)+1.0)/4.0;
            m_deTitle.set_brightness(brightness);

            if(m_pMouse_buttons[0] && m_LMB_delay<=0 && m_show_help)
            {
                //go to next screen or hide help
                m_deHelp[m_help_curr].show_decal(false);
                m_help_curr++;
                if(m_help_curr>7)//hide help
                {
                    m_help_curr=0;
                    m_show_help=false;
                }
                else//next help
                {
                    m_deHelp[m_help_curr].show_decal(true);
                }

                m_LMB_delay=_click_delay;
            }

            if(m_show_help)
            {
                m_last_time=time;
                return 0;//skip button test if help is shown
            }

            /*if(m_pMouse_buttons[0] && m_LMB_delay<=0)
            {
                //hide help
                m_deHelp.show_decal(false);
                m_show_help=false;
            }*/

            //button test
            //Test
            if( m_buTest.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buTest.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        return 1;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buTest.m_state==0) play_sound=true;
                    m_buTest.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buTest.m_state=0;//change button texture

            //hosting
            if( m_buHost.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buHost.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_host;
                        return 2;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buHost.m_state==0) play_sound=true;
                    m_buHost.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buHost.m_state=0;//change button texture

            //edit
            if( m_buEdit.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buEdit.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_edit;
                        return 3;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buEdit.m_state==0) play_sound=true;
                    m_buEdit.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buEdit.m_state=0;//change button texture

            //joining
            if( m_buJoin.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buJoin.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_join;
                        return 4;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buJoin.m_state==0) play_sound=true;
                    m_buJoin.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buJoin.m_state=0;//change button texture

            //help
            if( m_buHelp.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buHelp.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_deHelp[0].show_decal(true);
                        m_show_help=true;
                        m_help_curr=0;
                        return 5;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buHelp.m_state==0) play_sound=true;
                    m_buHelp.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buHelp.m_state=0;//change button texture

            //exit
            if( m_buExit.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buExit.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        return 6;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buExit.m_state==0) play_sound=true;
                    m_buExit.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buExit.m_state=0;//change button texture

        }break;

        case ms_host:
        {
            //tib mark test
            if(m_pMouse_buttons[0] && m_LMB_delay<=0)
            {
                m_tibPlayer_name.mark_test(m_pMouse_pos[0],m_pMouse_pos[1]);
                m_tibLimit_time.mark_test(m_pMouse_pos[0],m_pMouse_pos[1]);
                m_tibLimit_tile.mark_test(m_pMouse_pos[0],m_pMouse_pos[1]);
            }
            //check for keyboard input for marked text boxes
            m_tibPlayer_name.update(m_pKeys,time);
            m_tibLimit_time.update(m_pKeys,time);
            m_tibLimit_tile.update(m_pKeys,time);

            //back
            if( m_buBack.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buBack.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_main;
                        return 12;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buBack.m_state==0) play_sound=true;
                    m_buBack.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buBack.m_state=0;//change button texture

            //next
            if( m_buNext.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buNext.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        //continue only if have a name
                        if( m_tibPlayer_name.get_text()=="" )
                        {
                            string a_name="Player";
                            int rand_val=rand()%100+1;
                            char buff[10];
                            itoa(rand_val,buff,10);
                            a_name.append(buff);
                            m_tibPlayer_name.set_text(a_name);
                        }
                        m_menu_state=ms_lobby;//temp
                        return 7;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buNext.m_state==0) play_sound=true;
                    m_buNext.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buNext.m_state=0;//change button texture

        }break;

        case ms_join:
        {
            //tib mark test
            if(m_pMouse_buttons[0] && m_LMB_delay<=0)
            {
                m_tibPlayer_name.mark_test(m_pMouse_pos[0],m_pMouse_pos[1]);
                m_tibIP_number.mark_test(m_pMouse_pos[0],m_pMouse_pos[1]);
            }
            //check for keyboard input for marked text boxes
            m_tibPlayer_name.update(m_pKeys,time);
            m_tibIP_number.update(m_pKeys,time);

            //back
            if( m_buBack.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buBack.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_main;
                        return 10;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buBack.m_state==0) play_sound=true;
                    m_buBack.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buBack.m_state=0;//change button texture

            //next
            if( m_buNext.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buNext.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        //continue only if have a name
                        if( m_tibPlayer_name.get_text()=="" )
                        {
                            string a_name="Player";
                            int rand_val=rand()%100+1;
                            char buff[10];
                            itoa(rand_val,buff,10);
                            a_name.append(buff);
                            m_tibPlayer_name.set_text(a_name);
                        }
                        return 8;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buNext.m_state==0) play_sound=true;
                    m_buNext.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buNext.m_state=0;//change button texture

        }break;

        case ms_lobby:
        {

            //back
            if( m_buBack.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buBack.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_main;
                        m_tlLobby_players.clear_list();
                        return 10;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buBack.m_state==0) play_sound=true;
                    m_buBack.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buBack.m_state=0;//change button texture

            //next
            if( m_buNext.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buNext.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        return 9;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buNext.m_state==0) play_sound=true;
                    m_buNext.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buNext.m_state=0;//change button texture

        }break;

        case ms_edit:
        {
            //back
            if( m_buBack.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buBack.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_main;
                        return 12;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buBack.m_state==0) play_sound=true;
                    m_buBack.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buBack.m_state=0;//change button texture

            //sound toggle
            if( m_buEdit_sound.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0]&&m_LMB_delay<=0)
                {
                    m_LMB_delay=_click_delay;
                    if( m_buEdit_sound.m_state==0 )//0==on, 1==off
                    {//turn off
                        m_buEdit_sound.m_state=1;
                        return 13;
                    }
                    else if( m_buEdit_sound.m_state==1 )
                    {//turn on
                        m_buEdit_sound.m_state=0;
                        return 14;
                    }
                }
            }

            //music toggle
            if( m_buEdit_music.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0]&&m_LMB_delay<=0)
                {
                    m_LMB_delay=_click_delay;
                    if( m_buEdit_music.m_state==0 )//0==on, 1==off
                    {//turn off
                        m_buEdit_music.m_state=1;
                        return 15;
                    }
                    else if( m_buEdit_music.m_state==1 )
                    {//turn on
                        m_buEdit_music.m_state=0;
                        return 16;
                    }
                }
            }

        }break;

        case ms_game_over:
        {
            //update title decal brightness
            float brightness=0.5+(sinf(time*0.5)*sinf(time*2.0)+1.0)/4.0;
            m_deGameover.set_brightness(brightness);

            //update button next
            if( m_buNext.overlap_test(m_pMouse_pos[0],m_pMouse_pos[1]) )
            {
                if(m_pMouse_buttons[0])
                {
                    m_buNext.m_state=2;//change button texture
                    if(m_LMB_delay<=0)
                    {
                        m_LMB_delay=_click_delay;
                        m_menu_state=ms_main;
                        m_tlLobby_players.clear_list();
                        m_tlPlayers.clear_list();
                        m_tlScore.clear_list();
                        m_tlPlayers.add_player("Player");
                        m_tlScore.add_player("Tiles");
                        return 11;
                    }
                }
                else
                {
                    bool play_sound=false;
                    if(m_buNext.m_state==0) play_sound=true;
                    m_buNext.m_state=1;//change button texture
                    if(play_sound) return 12;
                }
            }
            else m_buNext.m_state=0;//change button texture

        }break;

    }

    m_last_time=time;
    return 0;
}

bool menu::draw()
{

    switch(m_menu_state)
    {
        case ms_main:
        {
            //help
            if(m_show_help)
            {
                m_deHelp[m_help_curr].draw();
                return true;
            }

            m_deTitle.draw();
            //m_deGameover.draw();//temp

            //buttons
            m_buTest.draw_button();
            m_buHelp.draw_button();
            m_buHost.draw_button();
            m_buJoin.draw_button();
            m_buEdit.draw_button();
            m_buExit.draw_button();
            //credits
            m_deCred.draw();
            //version
            m_diVersion.draw_display();
        }break;

        case ms_host:
        {
            m_deTitle.draw();
            m_buBack.draw_button();
            m_buNext.draw_button();
            m_tibPlayer_name.draw();
            m_tibLimit_time.draw();
            m_tibLimit_tile.draw();
            //credits
            m_deCred.draw();
            //version
            m_diVersion.draw_display();
        }break;

        case ms_join:
        {
            m_deTitle.draw();
            m_buBack.draw_button();
            m_buNext.draw_button();
            m_tibPlayer_name.draw();
            m_tibIP_number.draw();
            //credits
            m_deCred.draw();
            //version
            m_diVersion.draw_display();
        }break;

        case ms_lobby:
        {
            m_deTitle.draw();
            m_buBack.draw_button();
            m_buNext.draw_button();
            m_tlLobby_players.draw();
            //credits
            m_deCred.draw();
            //version
            m_diVersion.draw_display();
        }break;

        case ms_edit:
        {
            m_deTitle.draw();
            m_buBack.draw_button();
            m_buEdit_sound.draw_button();
            m_buEdit_music.draw_button();
        }break;

        case ms_game_over:
        {
            m_deGameover.draw();
            m_tlPlayers.draw();
            m_tlScore.draw();
            m_buNext.draw_button();
        }break;

    }



    return true;
}

bool menu::set_IP(string sIP)
{
    m_tibIP_number.set_text(sIP);
    return true;
}

string menu::get_IP(void)
{
    return m_tibIP_number.get_text();
}

string menu::get_player_name(void)
{
    string name=m_tibPlayer_name.get_text();
    //convert spaces to _
    for(int i=0;i<(int)name.size();i++)
    {
        if(name[i]==' ') name[i]='_';
    }

    return name;
}

bool menu::set_player_name(string name)
{
    m_tibPlayer_name.set_text(name);
    return true;
}

bool menu::info_missing(string type)
{
    if(type=="player_name")
    {
        float color[3]={1,0,0};
        m_tibPlayer_name.change_border_color(color);
        return true;
    }
    if(type=="IP_number")
    {
        float color[3]={1,0,0};
        m_tibIP_number.change_border_color(color);
        return true;
    }

    return false;
}

int menu::get_limit_time(void)
{
    return m_tibLimit_time.get_value();
}

int menu::get_limit_tile(void)
{
    return m_tibLimit_tile.get_value();
}

bool menu::add_player_to_list(string name)
{
    return m_tlLobby_players.add_player(name);
}

bool menu::add_player_to_gameover_list(string name,int score)
{
    m_tlPlayers.add_player(name);
    char buff[256];
    itoa(score,buff,10);
    m_tlScore.add_player( string(buff) );

    return true;
}

bool menu::remove_player_from_list(string name)
{
    return m_tlLobby_players.remove_player(name);
}

bool menu::remove_all_players_from_list(void)
{
    m_tlLobby_players.clear_list();

    return true;
}

bool menu::set_menu_state(int state)
{
    m_menu_state=state;

    return true;
}
