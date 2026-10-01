#include "Feature.h"

#include <App/Document.h>

#include <Mod/Part/App/TopoShapeOpCode.h>
#include "opencascade/TopAbs_ShapeEnum.hxx"
#include "opencascade/TopoDS.hxx"

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
