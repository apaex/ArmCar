#pragma once

#define COMMAND_QUEUE_SIZE 200
#define MAX_COMMAND_SIZE 5

class Queue
{
    int _pb = 0;
    int _pe = 0;
    char _buf[COMMAND_QUEUE_SIZE];
    int _count = 0;
public:
    void push(const char* cmd)
    {
        for(int i=0; cmd[i]; ++i)
        {
            _buf[_pe++] = cmd[i];

            if (_pe >= COMMAND_QUEUE_SIZE)
                _pe = 0;
        }

        _buf[_pe++] = 0;
        if (_pe >= COMMAND_QUEUE_SIZE)
            _pe = 0;

        ++_count;
    }

    void pop(char* cmd)
    {
        int i = 0;

        if (_count)
        {
            for(; _buf[_pb]; ++_pb)
            {
                if (_pb >= COMMAND_QUEUE_SIZE)
                    _pb = 0;

                cmd[i++] = _buf[_pb];
            }

            --_count;
        }

        ++_pb;
        if (_pb >= COMMAND_QUEUE_SIZE)
            _pb = 0;

        cmd[i++] = 0;
    }
};