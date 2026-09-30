#pragma once

#include "SketchObjectTransformed.h"

namespace Sketcher
{

class SketcherExport SketchObjectLinearPattern: public SketchObjectTransformed
{
    typedef SketchObjectTransformed inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::SketchObjectLinearPattern);

public:
    SketchObjectLinearPattern();

    App::PropertyVector Direction;
    App::PropertyDistance Length;
    App::PropertyInteger Occurrences;

    short mustExecute() const override;
    std::list<gp_Trsf> getTransformations() const override;
};

}  // namespace Sketcher
