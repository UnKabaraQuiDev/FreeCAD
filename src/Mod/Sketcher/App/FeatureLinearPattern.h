#pragma once

#include "FeatureTransformed.h"

namespace Sketcher
{

class SketcherExport LinearPattern: public Transformed
{
    typedef Transformed inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::LinearPattern);

public:
    LinearPattern();

    App::PropertyVector Direction;
    App::PropertyDistance Length;
    App::PropertyInteger Occurrences;

    short mustExecute() const override;
    const std::list<gp_Trsf> getTransformations() const override;
};

}  // namespace Sketcher
