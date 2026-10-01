#include "Feature.h"

#include <App/Document.h>
#include <Mod/Part/App/TopoShapeOpCode.h>

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
#include <TopAbs_ShapeEnum.hxx>
#include <BRep_Tool.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>

using namespace Sketcher;
FC_LOG_LEVEL_INIT("SketchObjectModifier", true, true)

PROPERTY_SOURCE_ABSTRACT(Sketcher::Feature, Sketcher::SketchObject)

Feature::Feature()
{
    ADD_PROPERTY_TYPE(Base, (nullptr), "Modifier", App::Prop_None, "Base sketch");

    Geometry.setStatusValue(App::Prop_ReadOnly);
    Constraints.setStatusValue(App::Prop_None | App::Prop_Hidden);
    ExternalGeometry.setStatusValue(App::Prop_None | App::Prop_ReadOnly | App::Prop_Hidden);
    ExternalTypes.setStatusValue(App::Prop_None | App::Prop_Hidden);
    FullyConstrained.setStatusValue(App::Prop_ReadOnly | App::Prop_Output | App::Prop_Hidden);
    Exports.setStatusValue(App::Prop_Hidden);
    ExternalGeo.setStatusValue(App::Prop_ReadOnly | App::Prop_Hidden);
    ArcFitTolerance.setStatusValue(App::Prop_None | App::Prop_Hidden);
    _InternalFaceVersion.setStatusValue(App::Prop_ReadOnly | App::Prop_Hidden);
}

Feature::~Feature()
{}

short Feature::mustExecute() const
{
    return inherited::mustExecute() || Base.isTouched();
}

App::DocumentObjectExecReturn* Feature::execute()
{
    try {
        // Position the sketch according to its attachment/support.
        inherited::positionBySupport();

        // Derived modifier fills Geometry.
        auto rtn = executeModifier();
        if (rtn != App::DocumentObject::StdReturn) {
            return rtn;
        }

        buildShape2();

        return rtn;
    }
    catch (const Base::Exception& e) {
        return new App::DocumentObjectExecReturn(e.what(), this);
    }
}

App::DocumentObjectExecReturn* Feature::executeModifier()
{
    return App::DocumentObject::StdReturn;
}

void Feature::buildShape2()
{
    std::vector<Part::TopoShape> shapes;
    std::vector<Part::TopoShape> vertices;

    int geoId = 0;

    auto addVertex = [&vertices](auto vertex, auto name) {
        if (!vertex.hasElementMap()) {
            vertex.resetElementMap(std::make_shared<Data::ElementMap>());
        }

        vertex.setElementName(
            Data::IndexedName::fromConst("Vertex", 1),
            Data::MappedName::fromRawData(name.c_str()),
            0L
        );

        vertices.push_back(vertex);
        vertices.back().copyElementMap(vertex, Part::OpCodes::Sketch);
    };

    auto addEdge = [this, &shapes](auto geo, auto indexedName) {
        shapes.push_back(getEdge(geo, convertSubName(indexedName, false).c_str()));

        if (SketchObject::checkSmallEdge(shapes.back())) {
            FC_WARN("Edge too small: " << indexedName);
        }
    };

    for (const auto* geo : Geometry.getValues()) {
        if (!geo) {
            continue;
        }

        ++geoId;

        if (geo->isDerivedFrom<Part::GeomPoint>()) {
            addVertex(
                Part::TopoShape {TopoDS::Vertex(geo->toShape())},
                inherited::convertSubName(Data::IndexedName::fromConst("Vertex", geoId), false)
            );
        }
        else {
            addEdge(geo, Data::IndexedName::fromConst("Edge", geoId));
        }
    }

    internalElementMap.clear();

    if (shapes.empty() && vertices.empty()) {
        InternalShape.setValue(Part::TopoShape());
        Shape.setValue(Part::TopoShape());
        return;
    }

    Part::TopoShape result(0, getDocument()->getStringHasher());

    if (vertices.empty()) {
        result.makeElementWires(shapes, Part::OpCodes::Sketch);
    }
    else {
        std::vector<Part::TopoShape> results;

        if (!shapes.empty()) {
            auto wires = Part::TopoShape().makeElementWires(shapes, Part::OpCodes::Sketch);

            for (const auto& wire : wires.getSubTopoShapes(TopAbs_WIRE)) {
                results.push_back(wire);
            }
        }

        results.insert(results.end(), vertices.begin(), vertices.end());

        result.makeElementCompound(results);
    }

    result.Tag = getID();

    InternalShape.setValue(buildInternals(result.located(TopLoc_Location())));

    Shape.setValue(result);
}

gp_Pnt Feature::getPointFromFace(const TopoDS_Face& f) const
{
    if (!f.Infinite()) {
        TopExp_Explorer exp;
        exp.Init(f, TopAbs_VERTEX);
        if (exp.More()) {
            return BRep_Tool::Pnt(TopoDS::Vertex(exp.Current()));
        }
        // Else try the other method
    }

    // TODO: Other method, e.g. intersect X,Y,Z axis with the (unlimited?) face?
    // Or get a "corner" point if the face is limited?
    throw Base::NotImplementedError("getPointFromFace(): Not implemented yet for this case");
}
