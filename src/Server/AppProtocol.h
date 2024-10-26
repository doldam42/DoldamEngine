#pragma once

enum CMDCODE
{
    CMD_ERROR = 50,              // 에러
    CMD_SND_CHAR_ENTER = 100,
    CMD_SND_CHAR_POS = 101,      // 캐릭터 위치 전송
};

struct MYCMD
{
    int code;                    // 명령코드
    int size;                    // 데이터의 바이트 단위 크기
};

struct ERRORDATA
{
    int errorCode;               // 에러코드
    char desc[256];              // 에러 내용
};

struct SEND_CHAR_POS
{
    int   charId;                // 캐릭터 ID
    float pos[3];                // 캐릭터 위치
};
