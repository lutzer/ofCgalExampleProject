#include "ofApp.h"

#define CGAL_EIGEN3_ENABLED 1;

#include <CGAL/trace.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Surface_mesh_default_triangulation_3.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/Implicit_surface_3.h>
#include <CGAL/IO/output_surface_facets_to_polyhedron.h>
#include <CGAL/Poisson_reconstruction_function.h>
#include <CGAL/Point_with_normal_3.h>
#include <CGAL/property_map.h>
#include <CGAL/IO/read_xyz_points.h>
#include <CGAL/compute_average_spacing.h>
#include <vector>
#include <fstream>

static const int SQRT_NUMBER_OF_POINTS = 4;
static const int NUMBER_OF_POINTS = SQRT_NUMBER_OF_POINTS * SQRT_NUMBER_OF_POINTS;

static const float POINT_RADIUS = 0.5;
static const float NORMALS_LENGTH = 2.0;

// Types
typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef Kernel::FT FT;
typedef Kernel::Point_3 cgPoint;
typedef Kernel::Vector_3 cgVector;
typedef CGAL::Point_with_normal_3<Kernel> Point_with_normal;
typedef Kernel::Sphere_3 Sphere;
typedef std::vector<Point_with_normal> PointList;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
typedef CGAL::Poisson_reconstruction_function<Kernel> Poisson_reconstruction_function;
typedef CGAL::Surface_mesh_default_triangulation_3 STr;
typedef CGAL::Surface_mesh_complex_2_in_triangulation_3<STr> C2t3;
typedef CGAL::Implicit_surface_3<Kernel, Poisson_reconstruction_function> Surface_3;

Polyhedron calculateMesh(vector<ofVec3f> inputPoints, vector<ofVec3f> inputPointsNormals) {
    // Poisson options
    FT sm_angle = 20.0; // Min triangle angle in degrees.
    FT sm_radius = 1.0; // Max triangle size w.r.t. point set average spacing.
    FT sm_distance = 0.075; // Surface Approximation error w.r.t. point set average spacing.

    // saves reconstructed surface mesh
    Polyhedron output_mesh;

    // Reads the point set file in points[].
    // Note: read_xyz_points_and_normals() requires an iterator over points
    // + property maps to access each point's position and normal.
    // The position property map can be omitted here as we use iterators over Point_3 elements.
    PointList points;

    for (int i=0; i < inputPoints.size(); i++) {
        cgPoint point(
          inputPoints[i].x,
          inputPoints[i].y,
          inputPoints[i].z
        );

        cgVector normal(
          inputPointsNormals[i].x,
          inputPointsNormals[i].y,
          inputPointsNormals[i].z
        );

        CGAL::Point_with_normal_3<Kernel> point_normal(point, normal);

        points.push_back(point_normal);
    }

    Poisson_reconstruction_function function(points.begin(), points.end(),
                                             CGAL::make_normal_of_point_with_normal_pmap(PointList::value_type()) );
    // Computes the Poisson indicator function f()
    // at each vertex of the triangulation.
    if ( ! function.compute_implicit_function() ) {
        ofLogError() << "could not compute implicit function";
        return output_mesh;
    }


    // Computes average spacing
    FT average_spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>(points.begin(), points.end(),
                                                                             6 /* knn = 1 ring */);
    // Gets one point inside the implicit surface
    // and computes implicit function bounding sphere radius.
    cgPoint inner_point = function.get_inner_point();
    Sphere bsphere = function.bounding_sphere();
    FT radius = std::sqrt(bsphere.squared_radius());
    // Defines the implicit surface: requires defining a
    // conservative bounding sphere centered at inner point.
    FT sm_sphere_radius = 5.0 * radius;
    FT sm_dichotomy_error = sm_distance*average_spacing/1000.0; // Dichotomy error must be << sm_distance
    Surface_3 surface(function,
                      Sphere(inner_point,sm_sphere_radius*sm_sphere_radius),
                      sm_dichotomy_error/sm_sphere_radius);
    // Defines surface mesh generation criteria
    CGAL::Surface_mesh_default_criteria_3<STr> criteria(sm_angle,  // Min triangle angle (degrees)
                                                        sm_radius*average_spacing,  // Max triangle size
                                                        sm_distance*average_spacing); // Approximation error
    // Generates surface mesh with manifold option
    STr tr; // 3D Delaunay triangulation for surface mesh generation
    C2t3 c2t3(tr); // 2D complex in 3D Delaunay triangulation
    CGAL::make_surface_mesh(c2t3,                                 // reconstructed mesh
                            surface,                              // implicit surface
                            criteria,                             // meshing criteria
                            CGAL::Manifold_with_boundary_tag());  // require manifold mesh
    if(tr.number_of_vertices() == 0) {
        ofLogError() << "No vertices";
        return output_mesh;
    }

    CGAL::output_surface_facets_to_polyhedron(c2t3, output_mesh);

    return output_mesh;

}

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetVerticalSync(true);

    cam.setDistance(80);

    //setup light
    ambLight.setAmbientColor(ofColor(20.0, 20.0, 20.0, 20.0));
    //ambLight.enable();

    static float circleRadius = 10;

    // create a set of randomly distirbuted points around a sphere
    for (int i=0; i < NUMBER_OF_POINTS; i++) {

        float angle1 = ((i % SQRT_NUMBER_OF_POINTS) + 1) / (float)(SQRT_NUMBER_OF_POINTS + 2) * PI;
        float angle2 = (i / SQRT_NUMBER_OF_POINTS) / (float)SQRT_NUMBER_OF_POINTS * PI * 2;

        ofVec3f point = {
            sin(angle1) * cos(angle2),
            sin(angle1) * sin(angle2),
            cos(angle1)
        };

        point *= ofRandom(circleRadius*0.5, circleRadius*1.5);

//        ofVec3f point = {
//            ofRandom(-circleRadius, circleRadius),
//            ofRandom(-circleRadius, circleRadius),
//            ofRandom(-circleRadius, circleRadius)
//        };
        points.push_back(point);
        normals.push_back(point.normalize());
    }

    //construct mesh from points
    Polyhedron cgalMesh = calculateMesh(points,normals);

    map<cgPoint, int> point_indices;
    int count = 0;

    for ( auto v = cgalMesh.vertices_begin(); v != cgalMesh.vertices_end(); ++v) {
        cgPoint point = v->point();
        ofPoint vertex(
            point.x(),
            point.y(),
            point.z()
        );
        surface.addVertex(vertex);
        point_indices[point] = count++;
    }

    for (auto it =cgalMesh.facets_begin(); it!=cgalMesh.facets_end(); ++it) {
        surface.addIndex(point_indices[it->halfedge()->vertex()->point()]);
        surface.addIndex(point_indices[it->halfedge()->next()->vertex()->point()]);
        surface.addIndex(point_indices[it->halfedge()->prev()->vertex()->point()]);
    }

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackgroundGradient(ofColor::gray, ofColor::black, OF_GRADIENT_CIRCULAR);

    ofEnableDepthTest();
    cam.begin();
    for (int i=0; i < NUMBER_OF_POINTS; i++) {
        ofSetColor(200, 200, 200);
        ofDrawSphere(points[i], POINT_RADIUS);
        ofSetColor(0, 255, 0);
        ofDrawLine(points[i], points[i] + normals[i] * NORMALS_LENGTH);
    }

    ofSetColor(255, 0, 0, 80);
    glPointSize(1.0);
    surface.drawWireframe();
    //ofSetColor(100, 100, 100);
    //surface.draw();

    cam.end();
    ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
