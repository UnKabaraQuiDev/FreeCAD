#pragma once

#include "Feature.h"
#include "App/PropertyContainer.h"

namespace Sketcher
{

class Projected: public Feature
{
    typedef Feature inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::Projected);

public:
    Projected();

    App::PropertyLinkSub ProjectPlane;

    short mustExecute() const override;
    App::DocumentObjectExecReturn* executeModifier() override;
};

}  // namespace Sketcher
