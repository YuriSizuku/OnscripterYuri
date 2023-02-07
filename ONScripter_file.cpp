/* -*- C++ -*-
 *
 *  ONScripter_file.cpp - FILE I/O of ONScripter
 *
 *  Copyright (c) 2001-2018 Ogapee. All rights reserved.
 *            (C) 2014-2018 jh10001 <jh10001@live.cn>
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

#include "ONScripter.h"
#include "Utils.h"

#if defined(LINUX) || defined(MACOSX) || defined(IOS)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#elif defined(WIN32) || defined(_WIN32)
#include <windows.h>
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }
#elif defined(MACOS9)
#include <DateTimeUtils.h>
#include <Files.h>
extern "C" void c2pstrcpy(Str255 dst, const char *src);	//#include <TextUtils.h>
#elif defined(PSP)
#include <pspiofilemgr.h>
#endif

#define SAVEFILE_MAGIC_NUMBER "ONS"
#define SAVEFILE_VERSION_MAJOR 2
#define SAVEFILE_VERSION_MINOR 8

#define READ_LENGTH 4096

void ONScripter::searchSaveFile( SaveFileInfo &save_file_info, int no )
{
    char file_name[256];

    script_h.getStringFromInteger( save_file_info.sjis_no, no, (num_save_file >= 10)?2:1 );
#if defined(LINUX) || defined(MACOSX) || defined(IOS)
    sprintf( file_name, "%ssave%d.dat", save_dir?save_dir:archive_path, no );
    struct stat buf;
    struct tm *tm;
    if ( stat( file_name, &buf ) != 0 ){
        save_file_info.valid = false;
        return;
    }
    time_t mtime = buf.st_mtime;
    tm = localtime( &mtime );
        
    save_file_info.month  = tm->tm_mon + 1;
    save_file_info.day    = tm->tm_mday;
    save_file_info.hour   = tm->tm_hour;
    save_file_info.minute = tm->tm_min;
#elif defined(WINRT)
    sprintf(file_name, "%ssave%d.dat", save_dir ? save_dir : archive_path, no);
    WCHAR file_nameW[256];
    MultiByteToWideChar(CP_ACP, 0, file_name, -1, file_nameW, 256);
    WIN32_FILE_ATTRIBUTE_DATA wfad;
    if (!GetFileAttributesEx(file_nameW, GetFileExInfoStandard, &wfad)) {
        save_file_info.valid = false;
        return;
    }

    SYSTEMTIME stm;
    FileTimeToSystemTime( &wfad.ftLastWriteTime, &stm);

    save_file_info.month  = stm.wMonth;
    save_file_info.day    = stm.wDay;
    save_file_info.hour   = stm.wHour;
    save_file_info.minute = stm.wMinute;
#elif defined(WIN32) || defined(_WIN32)
    sprintf( file_name, "%ssave%d.dat", save_dir?save_dir:archive_path, no );
    HANDLE  handle;
    FILETIME    tm, ltm;
    SYSTEMTIME  stm;

    WCHAR file_nameW[256];
    MultiByteToWideChar(CP_ACP, 0, file_name, -1, file_nameW, 256);
    handle = CreateFile( file_nameW, GENERIC_READ, 0, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    if ( handle == INVALID_HANDLE_VALUE ){
        save_file_info.valid = false;
        return;
    }
            
    GetFileTime( handle, NULL, NULL, &tm );
    FileTimeToLocalFileTime( &tm, &ltm );
    FileTimeToSystemTime( &ltm, &stm );
    CloseHandle( handle );

    save_file_info.month  = stm.wMonth;
    save_file_info.day    = stm.wDay;
    save_file_info.hour   = stm.wHour;
    save_file_info.minute = stm.wMinute;
#elif defined(MACOS9)
    sprintf( file_name, "%ssave%d.dat", save_dir?save_dir:archive_path, no );
    CInfoPBRec  pb;
    Str255      p_file_name;
    FSSpec      file_spec;
    DateTimeRec tm;
    c2pstrcpy( p_file_name, file_name );
    if ( FSMakeFSSpec(0, 0, p_file_name, &file_spec) != noErr ){
        save_file_info.valid = false;
        return;
    }
    pb.hFileInfo.ioNamePtr = file_spec.name;
    pb.hFileInfo.ioVRefNum = file_spec.vRefNum;
    pb.hFileInfo.ioFDirIndex = 0;
    pb.hFileInfo.ioDirID = file_spec.parID;
    if (PBGetCatInfoSync(&pb) != noErr) {
        save_file_info.valid = false;
        return;
    }
    SecondsToDate( pb.hFileInfo.ioFlMdDat, &tm );
    save_file_info.month  = tm.month;
    save_file_info.day    = tm.day;
    save_file_info.hour   = tm.hour;
    save_file_info.minute = tm.minute;
#elif defined(PSP)
    sprintf( file_name, "%ssave%d.dat", save_dir?save_dir:archive_path, no );
    SceIoStat buf;
    if ( sceIoGetstat(file_name, &buf)<0 ){
        save_file_info.valid = false;
        return;
    }

    save_file_info.month  = buf.st_mtime.month;
    save_file_info.day    = buf.st_mtime.day;
    save_file_info.hour   = buf.st_mtime.hour;
    save_file_info.minute = buf.st_mtime.minute;
#else
    sprintf( file_name, "save%d.dat", no );
    FILE *fp;
    if ( (fp = fopen( file_name, "rb", true )) == NULL ){
        save_file_info.valid = false;
        return;
    }
    fclose( fp );

    save_file_info.month  = 1;
    save_file_info.day    = 1;
    save_file_info.hour   = 0;
    save_file_info.minute = 0;
#endif
    save_file_info.valid = true;
    script_h.getStringFromInteger( save_file_info.sjis_month,  save_file_info.month,  2 );
    script_h.getStringFromInteger( save_file_info.sjis_day,    save_file_info.day,    2 );
    script_h.getStringFromInteger( save_file_info.sjis_hour,   save_file_info.hour,   2 );
    script_h.getStringFromInteger( save_file_info.sjis_minute, save_file_info.minute, 2, true );
}

char *ONScripter::readSaveStrFromFile( int no )
{
    char filename[32];
    sprintf( filename, "save%d.dat", no );
    size_t len = loadFileIOBuf( filename );
    if (len == 0){
        utils::printError("readSaveStrFromFile: can't open save file %s\n", filename );
        return NULL;
    }

    int p = len - 1;
    if ( p < 3 || file_io_buf[p] != '*' || file_io_buf[p-1] != '"' ) return NULL;
    p -= 2;
    
    while( file_io_buf[p] != '"' && p>0 ) p--;
    if ( file_io_buf[p] != '"' ) return NULL;

    len = len - p - 3;
    char *buf = new char[len+1];
    
    unsigned int i;
    for (i=0 ; i<len ; i++)
        buf[i] = file_io_buf[p+i+1];
    buf[i] = 0;

    return buf;
}

int ONScripter::loadSaveFile( int no )
{
    char filename[32];
    sprintf( filename, "save%d.dat", no );
    if (loadFileIOBuf( filename ) == 0){
        utils::printError("can't open save file %s\n", filename );
        return -1;
    }
    
    /* ---------------------------------------- */
    /* Load magic number */
    int i;
    for ( i=0 ; i<(int)strlen( SAVEFILE_MAGIC_NUMBER ) ; i++ )
        if ( readChar() != SAVEFILE_MAGIC_NUMBER[i] ) break;

    if ( i != (int)strlen( SAVEFILE_MAGIC_NUMBER ) ){
        file_io_buf_ptr = 0;
        printf("Save file version is unknown\n" );
        return loadSaveFile2( SAVEFILE_VERSION_MAJOR*100 + SAVEFILE_VERSION_MINOR );
    }
    
    int file_version = readChar() * 100;
    file_version += readChar();
    printf("Save file version is %d.%d\n", file_version/100, file_version%100 );
    if ( file_version > SAVEFILE_VERSION_MAJOR*100 + SAVEFILE_VERSION_MINOR ){
        utils::printError("Save file is newer than %d.%d, please use the latest ONScripter.\n", SAVEFILE_VERSION_MAJOR, SAVEFILE_VERSION_MINOR );
        return -1;
    }

    if ( file_version >= 200 )
        return loadSaveFile2( file_version );
    
    utils::printError("Save file is too old.\n");

    return -1;
}

void ONScripter::saveMagicNumber( bool output_flag )
{
    for ( unsigned int i=0 ; i<strlen( SAVEFILE_MAGIC_NUMBER ) ; i++ )
        writeChar( SAVEFILE_MAGIC_NUMBER[i], output_flag );
    writeChar( SAVEFILE_VERSION_MAJOR, output_flag );
    writeChar( SAVEFILE_VERSION_MINOR, output_flag );
}

void ONScripter::storeSaveFile()
{
    file_io_buf_ptr = 0;
    saveMagicNumber( false );
    saveSaveFile2( false );
    allocFileIOBuf();
    saveMagicNumber( true );
    saveSaveFile2( true );
    save_data_len = file_io_buf_ptr;
    memcpy(save_data_buf, file_io_buf, save_data_len);
}

int ONScripter::writeSaveFile( int no, const char *savestr )
{
    saveAll();

    char filename[32];
    sprintf( filename, "save%d.dat", no );
        
    memcpy(file_io_buf, save_data_buf, save_data_len);
    file_io_buf_ptr = save_data_len;
    if (saveFileIOBuf( filename, 0, savestr )){
        utils::printError("can't open save file %s for writing\n", filename );
        return -1;
    }

    size_t magic_len = strlen(SAVEFILE_MAGIC_NUMBER)+2;
    sprintf( filename, RELATIVEPATH "sav%csave%d.dat", DELIMITER, no );
    if (saveFileIOBuf( filename, magic_len, savestr ))
        utils::printError("can't open save file %s for writing (not an error)\n", filename );

    return 0;
}
