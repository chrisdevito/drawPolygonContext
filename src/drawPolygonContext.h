#ifndef DRAWPOLYGONCONTEXT_H
#define DRAWPOLYGONCONTEXT_H
#define _CRT_SECURE_NO_WARNINGS

#include <maya/MItSelectionList.h>
#include <maya/M3dView.h>
#include <maya/MArgList.h>
#include <maya/MCursor.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MEvent.h>
#include <maya/MFloatArray.h>
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItSelectionList.h>
#include <maya/MMeshIntersector.h>
#include <maya/MPoint.h>
#include <maya/MFloatPoint.h>
#include <maya/MFloatPointArray.h>
#include <maya/MSelectionList.h>
#include <maya/MString.h>
#include <maya/MToolsInfo.h>
#include <maya/MUiMessage.h>
#include <maya/MVector.h>

#include <maya/MFnDagNode.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnMesh.h>
#include <maya/MPxContext.h>
#include <maya/MPxToolCommand.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <vector>
#include <math.h>
#include <stdlib.h>

class DrawPolygonContext;

class DrawPolygonToolCommand : public MPxToolCommand
{
    public:
        DrawPolygonToolCommand();
        virtual ~DrawPolygonToolCommand();
        static void* creator();

        virtual MStatus doIt(const MArgList& args);
        virtual MStatus redoIt();
        virtual MStatus undoIt();
        bool isUndoable() const;

        virtual MStatus finalize();
        void setEditPoints(MFloatPointArray& editPoints);

    private:
        MFloatPointArray m_editPoints;
        MDagPath m_pathMesh;
};


class DrawPolygonContext : public MPxContext
{
    public:
        DrawPolygonContext();
        virtual ~DrawPolygonContext();

        virtual void toolOnSetup(MEvent& event);
        virtual void toolOffCleanup();
        virtual void getClassName(MString& name) const;

        virtual MStatus doPress(MEvent& event);
        virtual MStatus doDrag(MEvent& event);
        virtual MStatus doRelease(MEvent& event);

        void setLength(float length);
        float getLength();
        MStatus getSelection( MDagPathArray& geometry );
        MStatus getMeshIntersection( int x, int y );

        MPoint planeLineIntersection(
            const MPoint& lineA,
            const MPoint& lineB,
            const MPoint& planeP, 
            const MVector& planeN);

        MStatus createMesh();
        MStatus deleteMesh();

        void updateCameraNormal();

        static void postRenderCallback( const MString& panelName, void* data );

    private:
        float            m_length;
        MDagPathArray    m_geometry;
        M3dView          m_view;
        bool             m_intersects;
        MPoint           m_intersectionPoint;
        MVector          m_cameraNormal;
        MDagPath         m_pathCamera;
        MCallbackId      m_postRenderId;
        double           m_distanceFromLastEditPoint;
        MPoint           m_lastProjectedPoint;
        MFloatPointArray m_editPoints;
        MDagPath         m_pathMesh;
};

#endif
