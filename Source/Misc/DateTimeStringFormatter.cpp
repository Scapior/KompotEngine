/*
*  DateTimeStringFormatter.cpp
*  Copyright (C) 2020 by Maxim Stoyanov
*  scapior.github.io
*/

#include "DateTimeStringFormatter.hpp"

using namespace KompotEngine;


std::string DateTimeStringFormatter::now()
{
    return now(defaultFormat);
}
