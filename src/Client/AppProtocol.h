#pragma once

enum CMDCODE
{
    CMD_ERROR = 50,         // ����
    CMD_SND_CHAR_POS = 100, // ĳ���� ��ġ ����
};

struct MYCMD
{
    int code; // ����ڵ�
    int size; // �������� ����Ʈ ���� ũ��
};

struct ERRORDATA
{
    int  errorCode; // �����ڵ�
    char desc[256]; // ���� ����
};

struct SEND_CHAR_POS
{
    int   charId; // ĳ���� ID
    float pos[3]; // ĳ���� ��ġ
};

struct USERSESSION
{
    SOCKET hSocket;

    char buffer[8192]; // 8KB
};
