#include "FeatureMirrored.h"

#include <cstdlib>
#include <functional>
#include <list>
#include <string>
#include <vector>

#include <BRepAdaptor_Surface.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <gp_Ax2.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopLoc_Location.hxx>

#include <App/Datums.h>
#include <Mod/Part/App/Part2DObject.h>
#include <Mod/Part/App/PartFeature.h>
#include <Mod/Part/App/TopoShape.h>
#include <Mod/PartDesign/App/DatumPlane.h>

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

const std::list<gp_Trsf> Mirrored::getTransformations() const
{
    using getMirrorAxis = std::function<bool(gp_Pnt&, gp_Dir&)>;

    // 2D part object
    getMirrorAxis axisOfSketch = [this](gp_Pnt& axbase, gp_Dir& axdir) {
        App::DocumentObject* refObject = MirrorPlane.getValue();
        std::vector<std::string> subStrings = MirrorPlane.getSubValues();

        if (auto refSketch = dynamic_cast<Part::Part2DObject*>(refObject)) {
            Base::Axis axis;
            if (subStrings.empty() || subStrings[0].empty()) {
                axis = refSketch->getAxis(Part::Part2DObject::N_Axis);
            }
            else if (subStrings[0] == "H_Axis") {
                axis = refSketch->getAxis(Part::Part2DObject::V_Axis);
            }
            else if (subStrings[0] == "V_Axis") {
                axis = refSketch->getAxis(Part::Part2DObject::H_Axis);
            }
            else if (subStrings[0].compare(0, 4, "Axis") == 0) {
                int AxId = std::atoi(subStrings[0].substr(4, 4000).c_str());
                if (AxId >= 0 && AxId < refSketch->getAxisCount()) {
                    axis = refSketch->getAxis(AxId);
                    axis.setBase(axis.getBase() + 0.5 * axis.getDirection());
                    axis.setDirection(
                        Base::Vector3d(
                            -axis.getDirection().y,
                            axis.getDirection().x,
                            axis.getDirection().z
                        )
                    );
                }
                else {
                    throw Base::ValueError("No valid axis specified");
                }
            }
            axis *= refSketch->Placement.getValue();
            axbase = gp_Pnt(axis.getBase().x, axis.getBase().y, axis.getBase().z);
            axdir = gp_Dir(axis.getDirection().x, axis.getDirection().y, axis.getDirection().z);
            return true;
        }

        return false;
    };

    // Datum plane
    getMirrorAxis axisOfDatumPlane = [this](gp_Pnt& axbase, gp_Dir& axdir) {
        App::DocumentObject* refObject = MirrorPlane.getValue();
        if (auto plane = dynamic_cast<PartDesign::Plane*>(refObject)) {
            Base::Vector3d base = plane->getBasePoint();
            axbase = gp_Pnt(base.x, base.y, base.z);
            Base::Vector3d dir = plane->getNormal();
            axdir = gp_Dir(dir.x, dir.y, dir.z);
            return true;
        }

        return false;
    };

    // Normal plane
    getMirrorAxis axisOfPlane = [this](gp_Pnt& axbase, gp_Dir& axdir) {
        App::DocumentObject* refObject = MirrorPlane.getValue();
        if (auto plane = dynamic_cast<App::Plane*>(refObject)) {
            Base::Vector3d base = plane->getBasePoint();
            axbase = gp_Pnt(base.x, base.y, base.z);
            Base::Vector3d dir = plane->getDirection();
            axdir = gp_Dir(dir.x, dir.y, dir.z);
            return true;
        }

        return false;
    };

    // Planar shape
    getMirrorAxis axisOfPlanarShape = [this](gp_Pnt& axbase, gp_Dir& axdir) {
        App::DocumentObject* refObject = MirrorPlane.getValue();
        std::vector<std::string> subStrings = MirrorPlane.getSubValues();

        if (auto feature = dynamic_cast<Part::Feature*>(refObject)) {
            if (subStrings.empty()) {
                throw Base::ValueError("No mirror plane reference specified");
            }
            if (subStrings[0].empty()) {
                throw Base::ValueError("No direction reference specified");
            }

            Part::TopoShape baseShape = feature->Shape.getShape();
            // TODO: Check for multiple mirror planes?
            TopoDS_Shape shape = baseShape.getSubShape(subStrings[0].c_str());
            TopoDS_Face face = TopoDS::Face(shape);
            if (face.IsNull()) {
                throw Base::ValueError("Failed to extract mirror plane");
            }

            BRepAdaptor_Surface adapt(face);
            if (adapt.GetType() != GeomAbs_Plane) {
                throw Base::TypeError("Mirror face must be planar");
            }

            axbase = getPointFromFace(face);
            axdir = adapt.Plane().Axis().Direction();
            return true;
        }

        return false;
    };

    // ------------------------------------------------------------------------

    App::DocumentObject* refObject = MirrorPlane.getValue();
    if (!refObject) {
        throw Base::ValueError("No mirror plane reference specified");
    }

    gp_Pnt axbase;
    gp_Dir axdir;
    std::vector<getMirrorAxis> axisCheckers;
    axisCheckers.push_back(axisOfSketch);
    axisCheckers.push_back(axisOfDatumPlane);
    axisCheckers.push_back(axisOfPlane);
    axisCheckers.push_back(axisOfPlanarShape);

    for (const auto& getAxis : axisCheckers) {
        if (getAxis(axbase, axdir)) {
            return createTransformations(axbase, axdir);
        }
    }

    throw Base::ValueError(
        "Mirror plane reference must be a sketch axis, a face of a feature or a datum plane"
    );
}

std::list<gp_Trsf> Mirrored::createTransformations(gp_Pnt& axbase, gp_Dir& axdir) const
{
    TopLoc_Location invObjLoc = this->getLocation().Inverted();
    axbase.Transform(invObjLoc.Transformation());
    axdir.Transform(invObjLoc.Transformation());

    gp_Ax2 mirrorAxis(axbase, axdir);

    std::list<gp_Trsf> transformations;
    gp_Trsf trans;
    transformations.push_back(trans);  // identity transformation
    trans.SetMirror(mirrorAxis);
    transformations.push_back(trans);  // mirrored transformation
    return transformations;
}
