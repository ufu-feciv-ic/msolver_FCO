#pragma once

namespace cereal
{
    class access {};
}

#ifndef CEREAL_NVP
#define CEREAL_NVP(name) #name, name
#endif
