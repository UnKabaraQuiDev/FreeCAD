//
// Created by pcy113 on 10/1/26.
//

#include "FeatureMirrored.h"

#include "App/Datums.h"
#include "Mod/PartDesign/App/DatumPlane.h"
#include "opencascade/TopLoc_Location.hxx"
#include "opencascade/gp_Dir.hxx"
#include "opencascade/gp_Pnt.hxx"

using namespace Sketcher;

PROPERTY_SOURCE(Sketcher::Mirrored, Sketcher::Transformed)

Mirrored::Mirrored()
{
    ADD_PROPERTY_TYPE(
        MirrorPlane,
        (nullptr),
        "Mirrored",
        (App::PropertyType)(App::Prop_None),
        "Mirror plane"
    );
}

short Mirrored::mustExecute() const
{
    return inherited::mustExecute() || MirrorPlane.isTouched();
}

std::list<gp_Trsf> Mirrored::getTransformations() const
{
    App::DocumentObject* refObject = MirrorPlane.getValue();

    if (!refObject) {
        throw Base::ValueError("No mirror plane reference specified");
    }

    gp_Pnt axbase;
    gp_Dir axdir;

    if (auto plane = dynamic_cast<PartDesign::Plane*>(refObject)) {
        Base::Vector3d base = plane->getBasePoint();
        Base::Vector3d dir = plane->getNormal();

        axbase = gp_Pnt(base.x, base.y, base.z);
        axdir = gp_Dir(dir.x, dir.y, dir.z);
    }
    else if (auto plane = dynamic_cast<App::Plane*>(refObject)) {
        Base::Vector3d base = plane->getBasePoint();
        Base::Vector3d dir = plane->getDirection();

        axbase = gp_Pnt(base.x, base.y, base.z);
        axdir = gp_Dir(dir.x, dir.y, dir.z);
    }
    else {
        // Handle sketch axes / planar faces here if MirrorPlane
        // is allowed to reference those.
        throw Base::ValueError("Invalid mirror plane reference");
    }

    // Transform the mirror plane into this object's local coordinate system.
    TopLoc_Location invObjLoc = this->getLocation().Inverted();
    axbase.Transform(invObjLoc.Transformation());
    axdir.Transform(invObjLoc.Transformation());

    gp_Ax2 mirrorAxis(axbase, axdir);

    std::list<gp_Trsf> transformations;

    // Mirrored instance.
    gp_Trsf mirror;
    mirror.SetMirror(mirrorAxis);
    transformations.push_back(mirror);

    return transformations;
}