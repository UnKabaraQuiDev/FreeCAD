#pragma once

#include "Feature.h"
#include "App/PropertyContainer.h"

namespace Sketcher
{

class Project : public Feature
{
    typedef Feature inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::Project);
public:
    Project();

    App::PropertyLinkSub ProjectPlane;

    short mustExecute() const override;
    App::DocumentObjectExecReturn* executeModifier() override;
};

}  // namespace Sketcher
