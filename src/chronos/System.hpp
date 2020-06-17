#pragma once
#include <bits/stdc++.h>


namespace chronos
{
    struct SystemCall
    {
        bool operator () (const std::string &command)
        {
            const int execution_status { system(command.c_str()) };
            return execution_status == 0;
        }
    };
}
