/* -*- C++ -*-
 * 
 *  ScriptParser.h - Define block parser of ONScripter
 *
 *  Copyright (c) 2001-2016 Ogapee. All rights reserved.
 *            (C) 2014-2016 jh10001 <jh10001@live.cn>
 *
 *  ogapee@aqua.dti2.ne.jp
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SCRIPT_PARSER_H__
#define __SCRIPT_PARSER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "ScriptHandler.h"
#include "NsaReader.h"
#include "DirectReader.h"
#include "AnimationInfo.h"
#include "FontInfo.h"
#ifdef USE_LUA
#include "LUAHandler.h"
#endif
#include "coding2utf16.h"
#ifdef USE_BUILTIN_LAYER_EFFECTS
#include "builtin_layer.h"
#endif

extern Coding2UTF16 *coding2utf16;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_FONT_SIZE 26
#define DEFAULT_DIALOG_FONT_SIZE 18

#define DEFAULT_LOOKBACK_NAME0 "uoncur.bmp"
#define DEFAULT_LOOKBACK_NAME1 "uoffcur.bmp"
#define DEFAULT_LOOKBACK_NAME2 "doncur.bmp"
#define DEFAULT_LOOKBACK_NAME3 "doffcur.bmp"

#define DEFAULT_START_KINSOKU coding2utf16->DEFAULT_START_KINSOKU
#define DEFAULT_END_KINSOKU   coding2utf16->DEFAULT_END_KINSOKU

typedef unsigned char uchar3[3];

class ScriptParser
{
public:
    ScriptParser();
    ~ScriptParser();

    void reset();
    int  openScript();
    void setCurrentLabel( const char *label );
    void gosubReal( const char *label, char *next_script, bool textgosub_flag=false );
    int getStringBufferOffset(){return string_buffer_offset;};

    FILE *fopen(const char *path, const char *mode, bool use_save_dir=false);
    void saveGlovalData();

    /* Command */
    int zenkakkoCommand();
    int windowchipCommand();
    int windowbackCommand();
    int versionstrCommand();
    int usewheelCommand();
    int useescspcCommand();
    int underlineCommand();
    int transmodeCommand();
    int timeCommand();
    int textgosubCommand();
    int tanCommand();
    int subCommand();
    int straliasCommand();
    int soundpressplginCommand();
    int skipCommand();
    int sinCommand();
    int shadedistanceCommand();
    int setlayerCommand();
    int setkinsokuCommand();
    int selectvoiceCommand();
    int selectcolorCommand();
    int savenumberCommand();
    int savenameCommand();
    int savedirCommand();
    int rubyonCommand();
    int rubyoffCommand();
    int roffCommand();
    int rmenuCommand();
    int returnCommand();
    int pretextgosubCommand();
    int pagetagCommand();
    int numaliasCommand();
    int nsadirCommand();
    int nsaCommand();
    int nextCommand();
    int mulCommand();
    int movCommand();
    int mode_wave_demoCommand();
    int mode_sayaCommand();
    int mode_extCommand();
    int modCommand();
    int midCommand();
    int menusetwindowCommand();
    int menuselectvoiceCommand();
    int menuselectcolorCommand();
    int maxkaisoupageCommand();
    int luasubCommand();
    int luacallCommand();
    int lookbackspCommand();
    int lookbackcolorCommand();
    //int lookbackbuttonCommand();
    int loadgosubCommand();
    int linepageCommand();
    int lenCommand();
    int labellogCommand();
    int kidokuskipCommand();
    int kidokumodeCommand();
    int itoaCommand();
    int intlimitCommand();
    int incCommand();
    int ifCommand();
    int humanzCommand();
    int gotoCommand();
    int gosubCommand();
    int globalonCommand();
    int getparamCommand();
    //int gameCommand();
    int forCommand();
    int filelogCommand();
    int englishCommand();
    int effectcutCommand();
    int effectblankCommand();
    int effectCommand();
    int divCommand();
    int dimCommand();
    int defvoicevolCommand();
    int defsubCommand();
    int defsevolCommand();
    int defmp3volCommand();
    int defaultspeedCommand();
    int defaultfontCommand();
    int decCommand();
    int dateCommand();
    int cosCommand();
    int cmpCommand();
    int clickvoiceCommand();
    int clickstrCommand();
    int breakCommand();
    int autosaveoffCommand();
    int atoiCommand();
    int arcCommand();
    int addkinsokuCommand();
    int addCommand();
    
protected:
    struct UserFuncLUT{
        struct UserFuncLUT *next;
        char *command;
        bool lua_flag;
        UserFuncLUT(){
            next = NULL;
            command = NULL;
            lua_flag = false;
        };
        ~UserFuncLUT(){
            if (command) delete[] command;
        };
    };

    struct UserFuncHash{
        UserFuncLUT root;
        UserFuncLUT *last;
    } user_func_hash['z'-'a'+1];

    struct NestInfo{
        enum { LABEL = 0,
               FOR   = 1 };
        struct NestInfo *previous, *next;
        int  nest_mode;
        char *next_script; // used in gosub and for
        int  var_no, to, step; // used in for
        bool textgosub_flag; // used in textgosub and pretextgosub

        NestInfo(){
            previous = next = NULL;
            nest_mode = LABEL;
            textgosub_flag = false;
        };
    } last_tilde;

    enum { SYSTEM_NULL        = 0,
           SYSTEM_SKIP        = 1,
           SYSTEM_RESET       = 2,
           SYSTEM_SAVE        = 3,
           SYSTEM_LOAD        = 4,
           SYSTEM_LOOKBACK    = 5,
           SYSTEM_WINDOWERASE = 6,
           SYSTEM_MENU        = 7,
           SYSTEM_YESNO       = 8,
           SYSTEM_AUTOMODE    = 9,
           SYSTEM_END         = 10
    };
    enum { RET_NOMATCH   = 0,
           RET_SKIP_LINE = 1,
           RET_CONTINUE  = 2,
           RET_NO_READ   = 4,
           RET_EOL       = 8 // end of line (0x0a is found)
    };
    enum { CLICK_NONE    = 0,
           CLICK_WAIT    = 1,
           CLICK_NEWPAGE = 2,
           CLICK_EOL     = 4
    };
    enum{ NORMAL_MODE, DEFINE_MODE };
    int current_mode;
    int debug_level;

    char *archive_path;
    char *save_dir;
    char *nsa_path;
    unsigned int nsa_offset;
    bool globalon_flag;
    bool labellog_flag;
    bool filelog_flag;
    bool kidokuskip_flag;
    bool kidokumode_flag;
    bool autosaveoff_flag;

    int z_order;
    bool rmode_flag;
    bool windowback_flag;
    bool usewheel_flag;
    bool useescspc_flag;
    bool mode_wave_demo_flag;
    bool mode_saya_flag;
    bool mode_ext_flag;
    bool force_button_shortcut_flag;
    bool zenkakko_flag;
    bool pagetag_flag;
    int  windowchip_sprite_no;
    
    int string_buffer_offset;

    NestInfo root_nest_info, *last_nest_info;
    ScriptHandler::LabelInfo current_label_info;
    int current_line;

#ifdef USE_LUA
    LUAHandler lua_handler;
#endif

    /* ---------------------------------------- */
    /* Global definitions */
    int screen_ratio1, screen_ratio2;
    int screen_width, screen_height;
    int screen_device_width, screen_device_height;
    int device_width, device_height;
    float screen_scale_ratio1, screen_scale_ratio2;
    SDL_Rect screen_rect;
    SDL_Rect render_view_rect;
    int screen_bpp;
    char *version_str;
    int underline_value;
    char *save_dir_envdata;

    void deleteNestInfo();
    void setStr( char **dst, const char *src, int num=-1 );
    
    void readToken();
    
    /* ---------------------------------------- */
    /* Effect related variables */
    struct EffectLink{
        struct EffectLink *next;
        int no;
        int effect;
        int duration;
        AnimationInfo anim;

        EffectLink(){
            next = NULL;
            effect = 10;
            duration = 0;
        };
    };
    
    EffectLink root_effect_link, *last_effect_link, window_effect, tmp_effect;
    
    int effect_blank;
    bool effect_cut_flag;

    int readEffect( EffectLink *effect );
    EffectLink *parseEffect(bool init_flag);

    /* ---------------------------------------- */
    /* Lookback related variables */
    //char *lookback_image_name[4];
    int lookback_sp[2];
    uchar3 lookback_color;
    
    /* ---------------------------------------- */
    /* For loop related variables */
    bool break_flag;
    
    /* ---------------------------------------- */
    /* Transmode related variables */
    int trans_mode;
    
    /* ---------------------------------------- */
    /* Save/Load related variables */
    struct SaveFileInfo{
        bool valid;
        int  month, day, hour, minute;
        char sjis_no[5];
        char sjis_month[5];
        char sjis_day[5];
        char sjis_hour[5];
        char sjis_minute[5];
    };
    unsigned int num_save_file;
    char *save_menu_name;
    char *load_menu_name;
    char *save_item_name;

    unsigned char *save_data_buf;
    unsigned char *file_io_buf;
    size_t file_io_buf_ptr;
    size_t file_io_buf_len;
    size_t save_data_len;
    
    /* ---------------------------------------- */
    /* Text related variables */
    bool render_font_outline;
    char *default_env_font;
    int default_text_speed[3];
    struct Page{
        struct Page *next, *previous;

        char *text;
        int max_text;
        int text_count;

        char *tag;

        Page(){
            text = NULL;
            text_count = 0;
            tag = NULL;
        }
        ~Page(){
            if (text) delete[] text;
            if (tag)  delete[] tag;
        }
        char add(char ch){
            if (text_count >= max_text){
                char *text2 = new char[max_text*2];
                memcpy(text2, text, max_text);
                delete[] text;
                text = text2;
                max_text *= 2;
            }
            text[text_count++] = ch;
            return ch;
        };
    } *page_list, *start_page, *current_page; // ring buffer
    int  max_page_list;
    int  clickstr_line;
    int  clickstr_state;
    int  linepage_mode;
    int  num_chars_in_sentence;
    int  line_enter_status; // 0 ... no enter, 1 ... pretext, 2 ... body
    int  page_enter_status; // 0 ... no enter, 1 ... body until @,\ used when pagetag is enabled
    bool in_textbtn_flag;
    bool english_mode;

    struct Kinsoku {
        char chr[2];
    } *start_kinsoku, *end_kinsoku;
    bool is_kinsoku;
    int num_start_kinsoku, num_end_kinsoku;
    void setKinsoku(const char *start_chrs, const char *end_chrs, bool add);
    bool isStartKinsoku(const char *str);
    bool isEndKinsoku(const char *str);
    
    /* ---------------------------------------- */
    /* Sound related variables */
    int music_volume;
    int voice_volume;
    int se_volume;

    enum { CLICKVOICE_NORMAL  = 0,
           CLICKVOICE_NEWPAGE = 1,
           CLICKVOICE_NUM     = 2
    };
    char *clickvoice_file_name[CLICKVOICE_NUM];

    enum { SELECTVOICE_OPEN   = 0,
           SELECTVOICE_OVER   = 1,
           SELECTVOICE_SELECT = 2,
           SELECTVOICE_NUM    = 3
    };
    char *selectvoice_file_name[SELECTVOICE_NUM];

    enum { MENUSELECTVOICE_OPEN   = 0,
           MENUSELECTVOICE_CANCEL = 1,
           MENUSELECTVOICE_OVER   = 2,
           MENUSELECTVOICE_CLICK  = 3,
           MENUSELECTVOICE_WARN   = 4,
           MENUSELECTVOICE_YES    = 5,
           MENUSELECTVOICE_NO     = 6,
           MENUSELECTVOICE_NUM    = 7
    };
    char *menuselectvoice_file_name[MENUSELECTVOICE_NUM];
     
    /* ---------------------------------------- */
    /* Font related variables */
    FontInfo *current_font, sentence_font, menu_font, ruby_font, dialog_font;
    struct RubyStruct{
        enum { NONE,
               BODY,
               RUBY };
        int stage;
        int body_count;
        const char *ruby_start;
        const char *ruby_end;
        int ruby_count;
        int margin;

        int font_size_xy[2];
        char *font_name;

        RubyStruct(){
            stage = NONE;
            font_size_xy[0] = 0;
            font_size_xy[1] = 0;
            font_name = NULL;
        };
        ~RubyStruct(){
            if ( font_name ) delete[] font_name;
        };
    } ruby_struct;
    int shade_distance[2];

    /* ---------------------------------------- */
    /* RMenu related variables */
    struct RMenuLink{
        RMenuLink *next;
        char *label;
        int system_call_no;

        RMenuLink(){
            next  = NULL;
            label = NULL;
        };
        ~RMenuLink(){
            if (label) delete[] label;
        };
    } root_rmenu_link;
    unsigned int rmenu_link_num, rmenu_link_width;

    void deleteRMenuLink();
    int getSystemCallNo( const char *buffer );
    unsigned char convHexToDec( char ch );
    void readColor( uchar3 *color, const char *buf );
    
    void errorAndExit( const char *str, const char *reason=NULL );

    void allocFileIOBuf();
    int saveFileIOBuf( const char *filename, int offset=0, const char *savestr=NULL );
    size_t loadFileIOBuf( const char *filename );

    void writeChar( char c, bool output_flag );
    char readChar();
    void writeInt( int i, bool output_flag );
    int readInt();
    void writeStr( char *s, bool output_flag );
    void readStr( char **s );
    void writeVariables( int from, int to, bool output_flag );
    void readVariables( int from, int to );
    void writeArrayVariable( bool output_flag );
    void readArrayVariable();
    void writeLog( ScriptHandler::LogInfo &info );
    void readLog( ScriptHandler::LogInfo &info );

    /* ---------------------------------------- */
    /* System customize related variables */
    char *textgosub_label;
    char *pretextgosub_label;
    char *pretext_buf;
    char *loadgosub_label;
    int  textgosub_clickstr_state;

    ScriptHandler script_h;
    
    unsigned char *key_table;

    void createKeyTable( const char *key_exe );
};

#endif // __SCRIPT_PARSER_H__
