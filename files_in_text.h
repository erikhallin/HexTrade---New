#ifndef FILES_IN_TEXT_H
#define FILES_IN_TEXT_H

#include <string>
#include <iostream>

using namespace std;

//Data files converted with Base64 to text

enum data_files//list of all files
{
    file_texture_font_dark,
    file_texture_font_light,
    file_texture_font_mask,
    file_texture_buttons,
    file_texture_gameover,
    file_texture_help1,
    file_texture_help2,
    file_texture_help3,
    file_texture_help4,
    file_texture_help5,
    file_texture_help6,
    file_texture_help7,
    file_texture_help8,
    file_texture_menu,
    file_texture_tile,

    file_sound_key_input,
    file_sound_start_match,
    file_sound_start_mission,
    file_sound_finish_mission,
    file_sound_game_over,
    file_sound_abort_mission,
    file_sound_music_intro,
    file_sound_music_loop
};

string load_base64_file(int file_id);//will return encoded text

#endif
