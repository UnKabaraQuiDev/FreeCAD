#pragma once

#include "App/PropertyContainer.h"
#include "FeatureTransformed.h"
#include "opencascade/gp_Trsf.hxx"

namespace Sketcher
{

class Mirrored : public Transformed
{
    typedef Transformed inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::Mirrored);
public:
    Mirrored();

    App::PropertyLinkSub MirrorPlane;

    short mustExecute() const override;
    std::list<gp_Trsf> getTransformations() const override;
};

}  // namespace Sketcher
