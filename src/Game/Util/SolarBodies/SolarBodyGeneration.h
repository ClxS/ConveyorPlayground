#pragma once
#include "SolarBodyComponent.h"

namespace cpp_conv::util::solar_bodies
{
    enum class SolarBodySpecification
    {
        CrateredMoon
    };

    components::SquareSolarBodyComponent createSolarBody(SolarBodySpecification type);
}
