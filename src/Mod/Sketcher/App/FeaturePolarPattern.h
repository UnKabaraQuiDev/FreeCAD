#pragma once

#include "FeatureTransformed.h"

namespace Sketcher
{

class SketcherExport PolarPattern: public Transformed
{
    typedef Transformed inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::PolarPattern);

public:
    PolarPattern();

    App::PropertyVector Center;
    App::PropertyAngle Angle;
    App::PropertyInteger Occurrences;

    short mustExecute() const override;
    std::list<gp_Trsf> getTransformations() const override;
};

}  // namespace Sketcher
