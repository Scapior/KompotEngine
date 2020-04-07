/*
*   Copyright (C) 2019 by Maxim Stoyanov
*
*   scapior.github.io
*/

#pragma once

/**
 * \interface IEngineSystem
 * \brief An interface of the KompotEngine system - the part of ECS pattern.
 *
 * Detailed description.
 *
 *
 */

namespace KompotEngine
{
    class IEngineSystem
    {
    public:
        virtual void run() = 0;

        virtual ~IEngineSystem() {}
    };
}
