#pragma once

#include "SketchObjectTransformed.h"

namespace Sketcher
{

class SketcherExport SketchObjectPolarPattern: public SketchObjectTransformed
{
    typedef SketchObjectTransformed inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::SketchObjectPolarPattern);

public:
    SketchObjectPolarPattern();

    App::PropertyVector Center;
    App::PropertyAngle Angle;
    App::PropertyInteger Occurrences;

    short mustExecute() const override;
    std::list<gp_Trsf> getTransformations() const override;
};

}  // namespace Sketcher
