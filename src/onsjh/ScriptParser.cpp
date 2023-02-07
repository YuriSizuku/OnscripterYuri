/* -*- C++ -*-
 *
 *  ScriptParser.cpp - Define block parser of ONScripter
 *
 *  Copyright (c) 2001-2016 Ogapee. All rights reserved.
 *            (C) 2014-2019 jh10001 <jh10001@live.cn>
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

#include "ScriptParser.h"
#include "Utils.h"
#ifdef USE_BUILTIN_LAYER_EFFECTS
#include "builtin_layer.h"
LayerInfo layer_info[MAX_LAYER_NUM];

void deleteLayerInfo() {
    for (int i=0; i<MAX_LAYER_NUM; ++i) {
        if (layer_info[i].handler) {
            delete layer_info[i].handler;
            layer_info[i].handler = NULL;
        }
    }
}
#endif

#define VERSION_STR1 "ONScripter"
#define VERSION_STR2 "Copyright (C) 2001-2019 Studio O.G.A. All Rights Reserved.\n          (C) 2014-2019 jh10001"

#define DEFAULT_SAVE_MENU_NAME coding2utf16->DEFAULT_SAVE_MENU_NAME
#define DEFAULT_LOAD_MENU_NAME coding2utf16->DEFAULT_LOAD_MENU_NAME
#define DEFAULT_SAVE_ITEM_NAME coding2utf16->DEFAULT_SAVE_ITEM_NAME

#define DEFAULT_TEXT_SPEED_LOW    40
#define DEFAULT_TEXT_SPEED_MIDDLE 20
#define DEFAULT_TEXT_SPEED_HIGHT  10

#define MAX_PAGE_LIST 16

ScriptParser::ScriptParser()
{
    debug_level = 0;
    srand( time(NULL) );
    rand();

    screen_ratio1 = 1;
    screen_ratio2 = 1;

    archive_path = NULL;
    save_dir = NULL;
    version_str = NULL;
    save_dir_envdata = NULL;
    nsa_path = NULL;
    nsa_offset = 0;
    key_table = NULL;
    force_button_shortcut_flag = false;
    
    save_menu_name = NULL;
    load_menu_name = NULL;
    save_item_name = NULL;

    file_io_buf = NULL;
    save_data_buf = NULL;
    file_io_buf_ptr = 0;
    file_io_buf_len = 0;
    save_data_len = 0;

    render_font_outline = false;
    page_list = NULL;

    /* ---------------------------------------- */
    /* Sound related variables */
    int i;
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
             clickvoice_file_name[i] = NULL;
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
            selectvoice_file_name[i] = NULL;
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        menuselectvoice_file_name[i] = NULL;

    start_kinsoku = end_kinsoku = NULL;
    num_start_kinsoku = num_end_kinsoku = 0;
    setKinsoku(DEFAULT_START_KINSOKU, DEFAULT_END_KINSOKU, false);
}

ScriptParser::~ScriptParser()
{
    reset();

    if (version_str) delete[] version_str;
    if (save_menu_name) delete[] save_menu_name;
    if (load_menu_name) delete[] load_menu_name;
    if (save_item_name) delete[] save_item_name;

    if (file_io_buf) delete[] file_io_buf;
    if (save_data_buf) delete[] save_data_buf;

    if (save_dir_envdata) delete[] save_dir_envdata;
}

void ScriptParser::reset()
{
    int i;
    for (i='z'-'a' ; i>=0 ; i--){
        UserFuncHash &ufh = user_func_hash[i];
        UserFuncLUT *func = ufh.root.next;
        while(func){
            UserFuncLUT *tmp = func;
            func = func->next;
            delete tmp;
        }
        ufh.root.next = NULL;
        ufh.last = &ufh.root;
    }

    // reset misc variables
    if ( nsa_path ){
        delete[] nsa_path;
        nsa_path = NULL;
    }

    globalon_flag = false;
    labellog_flag = false;
    filelog_flag = false;
    kidokuskip_flag = false;
    kidokumode_flag = true;
    autosaveoff_flag = false;

    rmode_flag = true;
    windowback_flag = false;
    usewheel_flag = false;
    useescspc_flag = false;
    mode_wave_demo_flag = false;
    mode_saya_flag = false;
    mode_ext_flag = false;
    sentence_font.rubyon_flag = false;
    zenkakko_flag = false;
    pagetag_flag = false;
    windowchip_sprite_no = -1;
    string_buffer_offset = 0;

    break_flag = false;
    trans_mode = AnimationInfo::TRANS_TOPLEFT;

    if (version_str) delete[] version_str;
    version_str = new char[strlen(VERSION_STR1)+
                           strlen("\n")+
                           strlen(VERSION_STR2)+
                           strlen("\n")+
                           +1];
    sprintf( version_str, "%s\n%s\n", VERSION_STR1, VERSION_STR2 );
    z_order = 499;

    textgosub_label = NULL;
    pretextgosub_label = NULL;
    pretext_buf = NULL;
    loadgosub_label = NULL;
    textgosub_clickstr_state = CLICK_NONE;

    /* ---------------------------------------- */
    /* Lookback related variables */
    lookback_sp[0] = lookback_sp[1] = -1;
    lookback_color[0] = 0xff;
    lookback_color[1] = 0xff;
    lookback_color[2] = 0x00;

    /* ---------------------------------------- */
    /* Save/Load related variables */
    setStr( &save_menu_name, DEFAULT_SAVE_MENU_NAME );
    setStr( &load_menu_name, DEFAULT_LOAD_MENU_NAME );
    setStr( &save_item_name, DEFAULT_SAVE_ITEM_NAME );
    num_save_file = 9;

    /* ---------------------------------------- */
    /* Text related variables */
    sentence_font.reset();
    menu_font.reset();
    ruby_font.reset();
    dialog_font.reset();

    current_font = &sentence_font;
    shade_distance[0] = 1;
    shade_distance[1] = 1;
    
    default_text_speed[0] = DEFAULT_TEXT_SPEED_LOW;
    default_text_speed[1] = DEFAULT_TEXT_SPEED_MIDDLE;
    default_text_speed[2] = DEFAULT_TEXT_SPEED_HIGHT;
    max_page_list = MAX_PAGE_LIST+1;
    num_chars_in_sentence = 0;
    if (page_list){
        delete[] page_list;
        page_list = NULL;
    }
    current_page = start_page = NULL;
    
    clickstr_line = 0;
    clickstr_state = CLICK_NONE;
    linepage_mode = 0;
    english_mode = false;
    
    /* ---------------------------------------- */
    /* Sound related variables */
    for ( i=0 ; i<     CLICKVOICE_NUM ; i++ )
        setStr(&clickvoice_file_name[i], NULL);
    for ( i=0 ; i<    SELECTVOICE_NUM ; i++ )
        setStr(&selectvoice_file_name[i], NULL);
    for ( i=0 ; i<MENUSELECTVOICE_NUM ; i++ )
        setStr(&menuselectvoice_file_name[i], NULL);

    /* ---------------------------------------- */
    /* Menu related variables */
    menu_font.font_size_xy[0] = DEFAULT_FONT_SIZE;
    menu_font.font_size_xy[1] = DEFAULT_FONT_SIZE;
    menu_font.top_xy[0] = 0;
    menu_font.top_xy[1] = 16;
    menu_font.num_xy[0] = 32;
    menu_font.num_xy[1] = 23;
    menu_font.pitch_xy[0] = menu_font.font_size_xy[0];
    menu_font.pitch_xy[1] = 2 + menu_font.font_size_xy[1];
    menu_font.window_color[0] = menu_font.window_color[1] = menu_font.window_color[2] = 0xcc;

    deleteRMenuLink();

    /* ---------------------------------------- */
    /* Dialog related variables */
    dialog_font.font_size_xy[0] = DEFAULT_DIALOG_FONT_SIZE;
    dialog_font.font_size_xy[1] = DEFAULT_DIALOG_FONT_SIZE;
    dialog_font.pitch_xy[0] = dialog_font.font_size_xy[0];
    dialog_font.pitch_xy[1] = 2 + dialog_font.font_size_xy[1];
    dialog_font.is_bold = false;
    dialog_font.is_shadow = false;

    /* ---------------------------------------- */
    /* Effect related variables */
    effect_blank = 10;
    effect_cut_flag = false;

    window_effect.effect = 1;
    window_effect.duration = 0;
    root_effect_link.no = 0;
    root_effect_link.effect = 0;
    root_effect_link.duration = 0;

    EffectLink *link = root_effect_link.next;
    while(link){
        EffectLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    last_effect_link = &root_effect_link;
    last_effect_link->next = NULL;

    current_mode = DEFINE_MODE;

#ifdef USE_BUILTIN_LAYER_EFFECTS
    deleteLayerInfo();
#endif
}

int ScriptParser::openScript()
{
    script_h.cBR = new NsaReader( 0, archive_path, BaseReader::ARCHIVE_TYPE_NS2, key_table );
    if (script_h.cBR->open( nsa_path )){
        delete script_h.cBR;
        script_h.cBR = new DirectReader( archive_path, key_table );
        script_h.cBR->open();
    }
    
    if ( script_h.openScript( archive_path ) ) return -1;

    screen_width  = script_h.screen_width;
    screen_height = script_h.screen_height;

    return 0;
}

unsigned char ScriptParser::convHexToDec( char ch )
{
    if      ( '0' <= ch && ch <= '9' ) return ch - '0';
    else if ( 'a' <= ch && ch <= 'f' ) return ch - 'a' + 10;
    else if ( 'A' <= ch && ch <= 'F' ) return ch - 'A' + 10;
    else errorAndExit("convHexToDec: not valid character for color.");

    return 0;
}

void ScriptParser::readColor( uchar3 *color, const char *buf ){
    if ( buf[0] != '#' ) errorAndExit("readColor: no preceding #.");
    (*color)[0] = convHexToDec( buf[1] ) << 4 | convHexToDec( buf[2] );
    (*color)[1] = convHexToDec( buf[3] ) << 4 | convHexToDec( buf[4] );
    (*color)[2] = convHexToDec( buf[5] ) << 4 | convHexToDec( buf[6] );
}

void ScriptParser::deleteRMenuLink()
{
    RMenuLink *link = root_rmenu_link.next;
    while(link){
        RMenuLink *tmp = link;
        link = link->next;
        delete tmp;
    }
    root_rmenu_link.next = NULL;

    rmenu_link_num   = 0;
    rmenu_link_width = 0;
}

int ScriptParser::getSystemCallNo( const char *buffer )
{
    if      ( !strcmp( buffer, "skip" ) )        return SYSTEM_SKIP;
    else if ( !strcmp( buffer, "reset" ) )       return SYSTEM_RESET;
    else if ( !strcmp( buffer, "save" ) )        return SYSTEM_SAVE;
    else if ( !strcmp( buffer, "load" ) )        return SYSTEM_LOAD;
    else if ( !strcmp( buffer, "lookback" ) )    return SYSTEM_LOOKBACK;
    else if ( !strcmp( buffer, "windowerase" ) ) return SYSTEM_WINDOWERASE;
    else if ( !strcmp( buffer, "rmenu" ) )       return SYSTEM_MENU;
    else if ( !strcmp( buffer, "automode" ) )    return SYSTEM_AUTOMODE;
    else if ( !strcmp( buffer, "end" ) )         return SYSTEM_END;
    else{
        utils::printInfo("Unsupported system call %s\n", buffer );
        return -1;
    }
}

void ScriptParser::saveGlovalData()
{
    if ( !globalon_flag ) return;

    file_io_buf_ptr = 0;
    writeVariables( script_h.global_variable_border, script_h.variable_range, false );
    allocFileIOBuf();
    writeVariables( script_h.global_variable_border, script_h.variable_range, true );

    if (saveFileIOBuf( "gloval.sav" ))
        errorAndExit("can't open gloval.sav for writing.");
}

void ScriptParser::allocFileIOBuf()
{
    if (file_io_buf_ptr > file_io_buf_len){
        file_io_buf_len = file_io_buf_ptr;
        if (file_io_buf) delete[] file_io_buf;
        file_io_buf = new unsigned char[file_io_buf_len];

        if (save_data_buf){
            memcpy(file_io_buf, save_data_buf, save_data_len);
            delete[] save_data_buf;
        }
        save_data_buf = new unsigned char[file_io_buf_len];
        memcpy(save_data_buf, file_io_buf, save_data_len);
    }
    file_io_buf_ptr = 0;
}

int ScriptParser::saveFileIOBuf( const char *filename, int offset, const char *savestr )
{
    bool use_save_dir = false;
    if (strcmp(filename, "envdata") != 0) use_save_dir = true;
    
    FILE *fp;
    if ( (fp = fopen( filename, "wb", use_save_dir )) == NULL ) return -1;
    
    size_t ret = fwrite(file_io_buf+offset, 1, file_io_buf_ptr-offset, fp);

    if (savestr){
        fputc('"', fp);
        fwrite(savestr, 1, strlen(savestr), fp);
        fputc('"', fp);
        fputc('*', fp);
    }

    fclose(fp);

    if (ret != file_io_buf_ptr-offset) return -2;

    return 0;
}

size_t ScriptParser::loadFileIOBuf( const char *filename )
{
    bool use_save_dir = false;
    if (strcmp(filename, "envdata") != 0) use_save_dir = true;

    FILE *fp;
    if ( (fp = fopen( filename, "rb", use_save_dir )) == NULL )
        return 0;
    
    fseek(fp, 0, SEEK_END);
    size_t len = ftell(fp);
    file_io_buf_ptr = len;
    allocFileIOBuf();

    fseek(fp, 0, SEEK_SET);
    size_t ret = fread(file_io_buf, 1, len, fp);
    fclose(fp);

    return ret;
}

void ScriptParser::writeChar(char c, bool output_flag)
{
    if (output_flag)
        file_io_buf[file_io_buf_ptr] = (unsigned char)c;
    file_io_buf_ptr++;
}

char ScriptParser::readChar()
{
    if (file_io_buf_ptr >= file_io_buf_len ) return 0;
    return (char)file_io_buf[file_io_buf_ptr++];
}

void ScriptParser::writeInt(int i, bool output_flag)
{
    if (output_flag){
        file_io_buf[file_io_buf_ptr++] = i & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 8) & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 16) & 0xff;
        file_io_buf[file_io_buf_ptr++] = (i >> 24) & 0xff;
    }
    else{
        file_io_buf_ptr += 4;
    }
}

int ScriptParser::readInt()
{
    if (file_io_buf_ptr+3 >= file_io_buf_len ) return 0;
    
    int i =
        (unsigned int)file_io_buf[file_io_buf_ptr+3] << 24 |
        (unsigned int)file_io_buf[file_io_buf_ptr+2] << 16 |
        (unsigned int)file_io_buf[file_io_buf_ptr+1] << 8 |
        (unsigned int)file_io_buf[file_io_buf_ptr];
    file_io_buf_ptr += 4;

    return i;
}

void ScriptParser::writeStr(char *s, bool output_flag)
{
    if ( s && s[0] ){
        if (output_flag)
            memcpy( file_io_buf + file_io_buf_ptr,
                    s,
                    strlen(s) );
        file_io_buf_ptr += strlen(s);
    }
    writeChar( 0, output_flag );
}

void ScriptParser::readStr(char **s)
{
    int counter = 0;

    while (file_io_buf_ptr+counter < file_io_buf_len){
        if (file_io_buf[file_io_buf_ptr+counter++] == 0) break;
    }
    
    if (*s) delete[] *s;
    *s = NULL;
    
    if (counter > 1){
        *s = new char[counter];
        memcpy(*s, file_io_buf + file_io_buf_ptr, counter);
    }
    file_io_buf_ptr += counter;
}

void ScriptParser::writeVariables( int from, int to, bool output_flag )
{
    for (int i=from ; i<to ; i++){
        writeInt( script_h.getVariableData(i).num, output_flag );
        writeStr( script_h.getVariableData(i).str, output_flag );
    }
}

void ScriptParser::readVariables( int from, int to )
{
    for (int i=from ; i<to ; i++){
        script_h.getVariableData(i).num = readInt();
        readStr( &script_h.getVariableData(i).str );
    }
}

void ScriptParser::writeArrayVariable( bool output_flag )
{
    ScriptHandler::ArrayVariable *av = script_h.getRootArrayVariable();

    while(av){
        int i, dim = 1;
        for ( i=0 ; i<av->num_dim ; i++ )
            dim *= av->dim[i];
        
        for ( i=0 ; i<dim ; i++ ){
            unsigned long ch = av->data[i];
            if (output_flag){
                file_io_buf[file_io_buf_ptr+3] = (unsigned char)((ch>>24) & 0xff);
                file_io_buf[file_io_buf_ptr+2] = (unsigned char)((ch>>16) & 0xff);
                file_io_buf[file_io_buf_ptr+1] = (unsigned char)((ch>>8)  & 0xff);
                file_io_buf[file_io_buf_ptr]   = (unsigned char)(ch & 0xff);
            }
            file_io_buf_ptr += 4;
        }
        av = av->next;
    }
}

void ScriptParser::readArrayVariable()
{
    ScriptHandler::ArrayVariable *av = script_h.getRootArrayVariable();

    while(av){
        int i, dim = 1;
        for ( i=0 ; i<av->num_dim ; i++ )
            dim *= av->dim[i];
        
        for ( i=0 ; i<dim ; i++ ){
            unsigned long ret;
            if (file_io_buf_ptr+3 >= file_io_buf_len ) return;
            ret = file_io_buf[file_io_buf_ptr+3];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr+2];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr+1];
            ret = ret << 8 | file_io_buf[file_io_buf_ptr];
            file_io_buf_ptr += 4;
            av->data[i] = ret;
        }
        av = av->next;
    }
}

void ScriptParser::writeLog( ScriptHandler::LogInfo &info )
{
    file_io_buf_ptr = 0;
    bool output_flag = false;
    for (int n=0 ; n<2 ; n++){
        int  i,j;
        char buf[10];

        sprintf( buf, "%d", info.num_logs );
        for ( i=0 ; i<(int)strlen( buf ) ; i++ ) writeChar( buf[i], output_flag );
        writeChar( 0x0a, output_flag );

        ScriptHandler::LogLink *cur = info.root_log.next;
        for ( i=0 ; i<info.num_logs ; i++ ){
            writeChar( '"', output_flag );
            for ( j=0 ; j<(int)strlen( cur->name ) ; j++ )
                writeChar( cur->name[j] ^ 0x84, output_flag );
            writeChar( '"', output_flag );
            cur = cur->next;
        }

        if (n==1) break;
        allocFileIOBuf();
        output_flag = true;
    }

    if (saveFileIOBuf( info.filename )){
        utils::printError("can't write %s\n", info.filename );
        exit( -1 );
    }
}

void ScriptParser::readLog( ScriptHandler::LogInfo &info )
{
    script_h.resetLog( info );
    
    if (loadFileIOBuf( info.filename ) > 0){
        int i, j, ch, count = 0;
        char buf[100];

        while( (ch = readChar()) != 0x0a ){
            count = count * 10 + ch - '0';
        }

        for ( i=0 ; i<count ; i++ ){
            readChar();
            j = 0; 
            while( (ch = readChar()) != '"' ) buf[j++] = ch ^ 0x84;
            buf[j] = '\0';

            script_h.findAndAddLog( info, buf, true );
        }
    }
}

void ScriptParser::errorAndExit( const char *str, const char *reason )
{
    if ( reason )
        utils::printError(" *** Parse error at %s:%d [%s]; %s ***\n",
                 current_label_info.name,
                 current_line,
                 str, reason );
    else
        utils::printError( " *** Parse error at %s:%d [%s] ***\n",
                 current_label_info.name,
                 current_line,
                 str );
    exit(-1);
}

void ScriptParser::deleteNestInfo()
{
    NestInfo *info = root_nest_info.next;
    while(info){
        NestInfo *tmp = info;
        info = info->next;
        delete tmp;
    }
    root_nest_info.next = NULL;
    last_nest_info = &root_nest_info;
}

void ScriptParser::setStr( char **dst, const char *src, int num )
{
    if ( *dst ) delete[] *dst;
    *dst = NULL;
    
    if ( src ){
        if (num >= 0){
            *dst = new char[ num + 1 ];
            memcpy( *dst, src, num );
            (*dst)[num] = '\0';
        }
        else{
            *dst = new char[ strlen( src ) + 1];
            strcpy( *dst, src );
        }
    }
}

void ScriptParser::setCurrentLabel( const char *label )
{
    current_label_info = script_h.lookupLabel( label );
    current_line = script_h.getLineByAddress( current_label_info.start_address );
    script_h.setCurrent( current_label_info.start_address );
}

void ScriptParser::readToken()
{
    script_h.readToken();
    string_buffer_offset = 0;

    if (script_h.isText() && linepage_mode > 0){
        char ch = '@'; // click wait
        if (linepage_mode == 1 || sentence_font.getRemainingLine() <= clickstr_line)
            ch = '\\'; // newline

        // ugly work around
        unsigned int len = strlen(script_h.getStringBuffer());
        if (script_h.getStringBuffer()[len-1] == 0x0a){
            script_h.getStringBuffer()[len-1] = ch;
            script_h.addStringBuffer(0x0a);
        }
        else{
            script_h.addStringBuffer(ch);
        }
    }
}

int ScriptParser::readEffect( EffectLink *effect )
{
    int num = 1;
    
    effect->effect = script_h.readInt();
    if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
        num++;
        effect->duration = script_h.readInt();
        if ( script_h.getEndStatus() & ScriptHandler::END_COMMA ){
            num++;
            const char *buf = script_h.readStr();
            effect->anim.setImageName( buf );
        }
        else
            effect->anim.remove();
    }
    else if (effect->effect < 0 || effect->effect > 255){
        utils::printError( "Effect %d is out of range and is switched to 0.\n", effect->effect);
        effect->effect = 0; // to suppress error
    }

    //utils::printInfo("readEffect %d: %d %d %s\n", num, effect->effect, effect->duration, effect->anim.image_name );
    return num;
}

ScriptParser::EffectLink *ScriptParser::parseEffect(bool init_flag)
{
    if (init_flag) tmp_effect.anim.remove();

    int num = readEffect(&tmp_effect);

    if (num > 1) return &tmp_effect;
    if (tmp_effect.effect == 0 || tmp_effect.effect == 1) return &tmp_effect;

    EffectLink *link = &root_effect_link;
    while(link){
        if (link->no == tmp_effect.effect) return link;
        link = link->next;
    }

    utils::printError( "Effect No. %d is not found.\n", tmp_effect.effect);
    exit(-1);

    return NULL;
}

FILE *ScriptParser::fopen(const char *path, const char *mode, bool use_save_dir)
{
    return script_h.fopen(path, mode, use_save_dir);
}

void ScriptParser::createKeyTable( const char *key_exe )
{
    if (!key_exe) return;
    
    FILE *fp = ::fopen(key_exe, "rb");
    if (fp == NULL){
        utils::printError( "createKeyTable: can't open EXE file %s\n", key_exe);
        return;
    }

    key_table = new unsigned char[256];

    int i;
    for (i=0 ; i<256 ; i++) key_table[i] = i;

    unsigned char ring_buffer[256];
    int ring_start = 0, ring_last = 0;
    
    int ch, count;
    while((ch = fgetc(fp)) != EOF){
        i = ring_start;
        count = 0;
        while (i != ring_last &&
               ring_buffer[i] != ch ){
            count++;
            i = (i+1)%256;
        }
        if (i == ring_last && count == 255) break;
        if (i != ring_last)
            ring_start = (i+1)%256;
        ring_buffer[ring_last] = ch;
        ring_last = (ring_last+1)%256;
    }
    fclose(fp);

    if (ch == EOF)
        errorAndExit( "createKeyTable: can't find a key table." );

    // Key table creation
    ring_buffer[ring_last] = ch;
    for (i=0 ; i<256 ; i++)
        key_table[ring_buffer[(ring_start+i)%256]] = i;
}

void ScriptParser::setKinsoku(const char *start_chrs, const char *end_chrs, bool add)
{
    int i;
    const char *kchr;
    Kinsoku *tmp;

    // count chrs
    int num_start = 0;
    kchr = start_chrs;
    while (*kchr != '\0') {
        if IS_TWO_BYTE(*kchr) kchr++;
        kchr++;
        num_start++;
    }

    int num_end = 0;
    kchr = end_chrs;
    while (*kchr != '\0') {
        if IS_TWO_BYTE(*kchr) kchr++;
        kchr++;
        num_end++;
    }

    if (add) {
        if (start_kinsoku != NULL)
            tmp = start_kinsoku;
        else {
            tmp = new Kinsoku[1];
            num_start_kinsoku = 0;
        }
    } else {
        if (start_kinsoku != NULL)
            delete[] start_kinsoku;
        tmp = new Kinsoku[1];
        num_start_kinsoku = 0;
    }
    start_kinsoku = new Kinsoku[num_start_kinsoku + num_start];
    kchr = start_chrs;
    for (i=0; i<num_start_kinsoku+num_start; i++) {
        if (i < num_start_kinsoku)
            start_kinsoku[i].chr[0] = tmp[i].chr[0];
        else
            start_kinsoku[i].chr[0] = *kchr++;
        if IS_TWO_BYTE(start_kinsoku[i].chr[0]) {
            if (i < num_start_kinsoku)
                start_kinsoku[i].chr[1] = tmp[i].chr[1];
            else
                start_kinsoku[i].chr[1] = *kchr++;
        } else {
            start_kinsoku[i].chr[1] = '\0';
        }
    }
    num_start_kinsoku += num_start;
    delete[] tmp;

    if (add) {
        if (end_kinsoku != NULL)
            tmp = end_kinsoku;
        else {
            tmp = new Kinsoku[1];
            num_end_kinsoku = 0;
        }
    } else {
        if (end_kinsoku != NULL)
            delete[] end_kinsoku;
        tmp = new Kinsoku[1];
        num_end_kinsoku = 0;
    }
    end_kinsoku = new Kinsoku[num_end_kinsoku + num_end];
    kchr = end_chrs;
    for (i=0; i<num_end_kinsoku+num_end; i++) {
        if (i < num_end_kinsoku)
            end_kinsoku[i].chr[0] = tmp[i].chr[0];
        else
            end_kinsoku[i].chr[0] = *kchr++;
        if IS_TWO_BYTE(end_kinsoku[i].chr[0]) {
            if (i < num_end_kinsoku)
                end_kinsoku[i].chr[1] = tmp[i].chr[1];
            else
                end_kinsoku[i].chr[1] = *kchr++;
        } else {
            end_kinsoku[i].chr[1] = '\0';
        }
    }
    num_end_kinsoku += num_end;
    delete[] tmp;
}

bool ScriptParser::isStartKinsoku(const char *str)
{
    for (int i=0; i<num_start_kinsoku; i++) {
        if ((start_kinsoku[i].chr[0] == *str) &&
            (start_kinsoku[i].chr[1] == *(str+1)))
            return true;
    }
    return false;
}

bool ScriptParser::isEndKinsoku(const char *str)
{
    for (int i=0; i<num_end_kinsoku; i++) {
        if ((end_kinsoku[i].chr[0] == *str) &&
            (end_kinsoku[i].chr[1] == *(str+1)))
            return true;
    }
    return false;
}