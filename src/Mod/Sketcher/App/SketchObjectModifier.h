#pragma once

#include <App/FeaturePython.h>
#include <App/IndexedName.h>
#include <App/PropertyFile.h>
#include <Base/Axis.h>
#include <Base/Bitmask.h>
#include <Mod/Part/App/Part2DObject.h>
#include <Mod/Part/App/PropertyGeometryList.h>
#include <Mod/Sketcher/App/PropertyConstraintList.h>
#include <Mod/Sketcher/App/SketchAnalysis.h>

#include "Analyse.h"
#include "GeoEnum.h"
#include "GeoList.h"
#include "GeometryFacade.h"
#include "Sketch.h"

#include "SketchGeometryExtension.h"
#include "ExternalGeometryExtension.h"
#include "SketchObject.h"

namespace Sketcher
{

using Part::Part2DObject;

class SketcherExport SketchObjectModifier: public SketchObject
{
    typedef SketchObject inherited;
    PROPERTY_HEADER_WITH_OVERRIDE(Sketcher::SketchObjectModifier);

public:
    SketchObjectModifier();
    ~SketchObjectModifier() override;

    App::PropertyLink Base;

    void buildShape2();

    /** @name methods override Feature */
    //@{
    short mustExecute() const override;
    App::DocumentObjectExecReturn* execute() override;
    //@}

protected:
    virtual App::DocumentObjectExecReturn* executeModifier();
};

}  // namespace Sketcher
