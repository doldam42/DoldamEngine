#pragma once

#include <iostream>

class MemoryStreamBuf : public std::streambuf
{
  public:
    MemoryStreamBuf(char *buffer, size_t size)
    {
        setg(buffer, buffer, buffer + size); // 읽기 포인터 설정
        setp(buffer, buffer + size);         // 쓰기 포인터 설정
    }
};

class MemoryIStream : public std::istream
{
  private:
    MemoryStreamBuf m_buffer;

  public:
    MemoryIStream(char *buffer, size_t size) : std::istream(&m_buffer), m_buffer(buffer, size) {}
};

class MemoryOStream : public std::ostream
{
  private:
    MemoryStreamBuf m_buffer;

  public:
    MemoryOStream(char *buffer, size_t size) : std::ostream(&m_buffer), m_buffer(buffer, size) {}
};