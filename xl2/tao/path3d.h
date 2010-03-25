#ifndef PATH3D_H
#define PATH3D_H
// ****************************************************************************
//  path3d.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//     Representation of paths in 3D
//
//
//
//
//
//
//
//
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "shapes.h"
#include <GL/glew.h>

struct QPainterPath;

TAO_BEGIN

struct GraphicPath : Shape
// ----------------------------------------------------------------------------
//    An arbitrary graphic path
// ----------------------------------------------------------------------------
{
    GraphicPath(): Shape() {}
    virtual void        Draw(Layout *where);
    virtual void        Draw(Layout *where, GLenum mode, GLenum tessel=0);
    virtual Box3        Bounds();

    // Absolute coordinates
    GraphicPath&        moveTo(Point3 dst);
    GraphicPath&        lineTo(Point3 dst);
    GraphicPath&        curveTo(Point3 control, Point3 dst);
    GraphicPath&        curveTo(Point3 c1, Point3 c2, Point3 dst);
    GraphicPath&        close();

    // Relative coordinates
    GraphicPath&        moveTo(Vector3 dst)     { return moveTo(position+dst); }
    GraphicPath&        lineTo(Vector3 dst)     { return lineTo(position+dst); }

    // Qt path conversion
    GraphicPath&        addQtPath(QPainterPath &path);
    static void         Draw(Layout *where, QPainterPath &path, GLenum tess=0);

public:
    enum Kind { MOVE_TO, LINE_TO, CURVE_TO, CURVE_CONTROL };
    struct Element
    {
        Element(Kind k, const Point3 &p): kind(k), position(p) {}
        Kind    kind;
        Point3  position;
    };
    typedef std::vector<Element> path_elements;
    struct VertexData
    {
        VertexData(const Point3& v, const Point3& t): vertex(v), texture(t) {}
        Vector3  vertex;
        Vector3  texture;
    };
    typedef std::vector<VertexData>   Vertices;
    typedef std::vector<VertexData *> DynamicVertices;
    struct PolygonData
    {
        PolygonData() {}
        ~PolygonData();
        Vertices        vertices;
        DynamicVertices allocated;
    };

public:
    path_elements       elements;
    Point3              start, position;
    Box3                bounds;
};

TAO_END

#endif // PATH3D_H
