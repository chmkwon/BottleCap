#ifndef Protocol_h
#define Protocol_h

namespace Protocol
{
    const unsigned char DISCONNECT = 0x03;
    const unsigned char SUCCESS = 0x01;
    const unsigned char FAIL = 0x02;

    const unsigned char CHECK_RESULT = 0x01;
    const unsigned char CHAR_DATA = 0x02;
}

namespace Error_Protocol
{
    const unsigned char CONNECT_ERROR = 0x00;
    const unsigned char DB_ERROR = 0x01;
}

#endif