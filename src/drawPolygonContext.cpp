#include "drawPolygonContext.h"


DrawPolygonToolCommand::DrawPolygonToolCommand()
{
    setCommandString( "drawPolygonTool" );
}


DrawPolygonToolCommand::~DrawPolygonToolCommand()
{
}


void* DrawPolygonToolCommand::creator()
{
    return new DrawPolygonToolCommand;
}


bool DrawPolygonToolCommand::isUndoable() const
{
    return true;   
}


void DrawPolygonToolCommand::setEditPoints(MFloatPointArray& editPoints)
{
    m_editPoints = editPoints;
}


MStatus DrawPolygonToolCommand::doIt(const MArgList& args)
{
    return redoIt();
}


MStatus DrawPolygonToolCommand::redoIt()
{
    MStatus status;

    // Create a mesh.
    MFnMesh fnMesh;
    MObject outData;
    unsigned int numVertices = m_editPoints.length();
    unsigned int numPolygons = 1;

    //  array of vertex counts for each polygon
    // It's 1 polygon so it is the number of vertices.
    MIntArray polygonCounts;
    polygonCounts.append(numVertices);

    // array of vertex connections for each polygon
    MIntArray polygonConnects;
    for (unsigned int i = 0; i < numVertices; ++i )
    {
        polygonConnects.append(i);
    }

    MObject oMesh = fnMesh.create(numVertices,
                                  numPolygons,
                                  m_editPoints,
                                  polygonCounts,
                                  polygonConnects,
                                  outData,
                                  &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDagNode fnDag(oMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = fnDag.getPath(m_pathMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Add initial shading group.
    char buffer[512];
    sprintf(buffer, "sets -e -forceElement initialShadingGroup \"%s\";", m_pathMesh.partialPathName().asChar());
    status = MGlobal::executeCommand(buffer);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}


MStatus DrawPolygonToolCommand::undoIt()
{
    MStatus status;

    if ( m_pathMesh.isValid() )
    {
        char buffer[512];
        sprintf( buffer, "delete %s;", m_pathMesh.partialPathName().asChar() );
        status = MGlobal::executeCommand( buffer );
        CHECK_MSTATUS_AND_RETURN_IT( status );

    }
    return MS::kSuccess;
}


MStatus DrawPolygonToolCommand::finalize()
{
    MArgList command;
    command.addArg( commandString() );
    return MPxToolCommand::doFinalize( command );
}


DrawPolygonContext::DrawPolygonContext()
{
    m_intersects = false;
}


DrawPolygonContext::~DrawPolygonContext()
{
    
}


void DrawPolygonContext::getClassName( MString& name ) const 
{
	name.set( "drawPolygonContext" );
}


void DrawPolygonContext::toolOnSetup( MEvent & event )
{
    MStatus status;

	m_view = M3dView::active3dView();
    status = m_view.getCamera( m_pathCamera );
    CHECK_MSTATUS( status );
    updateCameraNormal();
    

    MString modelPanel;
    status = MGlobal::executeCommand( "getPanel -wf", modelPanel );
    CHECK_MSTATUS( status );
    if ( m_postRenderId == 0 )
    {
        m_postRenderId = MUiMessage::add3dViewPostRenderMsgCallback( 
            modelPanel, &DrawPolygonContext::postRenderCallback, (void *) this, &status );
        CHECK_MSTATUS( status );
    }

    m_geometry.clear();
    status = getSelection( m_geometry );
    CHECK_MSTATUS( status );
}


void DrawPolygonContext::toolOffCleanup()
{
    if ( m_postRenderId )
    {
        MMessage::removeCallback( m_postRenderId );
        m_postRenderId = 0;
    }
}


void DrawPolygonContext::setLength( float length )
{
    m_length = length;
}


float DrawPolygonContext::getLength()
{
    return m_length;
}



MStatus DrawPolygonContext::getSelection( MDagPathArray& geometry )
{
    MStatus status;

    MSelectionList selectionList;
    MGlobal::getActiveSelectionList( selectionList );

    // Get the shape nodes
    MItSelectionList itList( selectionList );
    MDagPath pathNode;
    unsigned int numShapes;
    for ( ; !itList.isDone(); itList.next() )
    {
        status = itList.getDagPath( pathNode );
        CHECK_MSTATUS_AND_RETURN_IT( status );
        numShapes = pathNode.childCount();
        for ( unsigned int i = 0; i < numShapes; ++i )
        {
            status = pathNode.push( pathNode.child( i ) );
            CHECK_MSTATUS_AND_RETURN_IT( status );

            if ( pathNode.node().hasFn( MFn::kMesh ) )
            {
                MFnDagNode fnDag( pathNode );
                if ( !fnDag.isIntermediateObject() )
                {
                    geometry.append( MDagPath( pathNode ) );
                }
            }
            pathNode.pop();
        }
    }

    return MS::kSuccess;
}


void DrawPolygonContext::updateCameraNormal()
{
    m_cameraNormal = MVector( 0.0, 0.0, 1.0 );
    m_cameraNormal *= m_pathCamera.inclusiveMatrix();
}



MStatus	DrawPolygonContext::doPress( MEvent& event )
{
    MStatus status;

    short mouseX, mouseY;
    status = event.getPosition( mouseX, mouseY );
    CHECK_MSTATUS_AND_RETURN_IT( status );

    m_editPoints.clear();
    status = getMeshIntersection( mouseX, mouseY );
    CHECK_MSTATUS_AND_RETURN_IT( status );

    m_distanceFromLastEditPoint = 0.0;
    m_lastProjectedPoint = m_intersectionPoint;

    return MS::kSuccess;
}


MStatus DrawPolygonContext::getMeshIntersection( int x, int y )
{
    MStatus status;

    MPoint nearClip;
    MPoint farClip;
    m_view.viewToWorld( x, y, nearClip, farClip );
    MVector rayDirection = (farClip - nearClip).normal();
    MFloatPoint raySource, hitPoint;
    raySource.setCast( nearClip );
    float intersectionDistance = 0.0f;
    int hitFace, hitTriangle;
    float hitBary1, hitBary2;
    bool initialClosestIntersection = true;
    float minIntersectionDistance = 0.0f;
    m_intersects = false;

    for ( unsigned int i = 0; i < m_geometry.length(); ++i )
    {
        MFnMesh fnMesh( m_geometry[i], &status );
        CHECK_MSTATUS_AND_RETURN_IT( status );

        if ( !fnMesh.closestIntersection( raySource,
                rayDirection, NULL, NULL, false, MSpace::kWorld,
                1000.0f, false, NULL, hitPoint, &intersectionDistance,
                &hitFace, &hitTriangle,
                &hitBary1, &hitBary2,
                0.00001f, &status ) )
        {
            continue;
        }
        CHECK_MSTATUS_AND_RETURN_IT( status );
        if ( initialClosestIntersection || intersectionDistance < minIntersectionDistance )
        {
            m_intersects = true;
            minIntersectionDistance = intersectionDistance;
            initialClosestIntersection = false;
            m_intersectionPoint = MPoint(hitPoint);
        }
    }

    if ( m_intersects )
    {
        MFloatPoint projectedPoint;
        projectedPoint.setCast(m_intersectionPoint);
        m_editPoints.append(projectedPoint);
    }

    return MS::kSuccess;
}


MStatus	DrawPolygonContext::doDrag( MEvent& event )
{
    MStatus status;
    if ( !m_intersects )
    {
        return MS::kSuccess;
    }

    short mouseX, mouseY;
    event.getPosition( mouseX, mouseY );
    MPoint nearClip, farClip;
    status = m_view.viewToWorld( mouseX, mouseY, nearClip, farClip );

    MPoint projPlanePoint = planeLineIntersection( nearClip, farClip, m_intersectionPoint, m_cameraNormal );
    m_distanceFromLastEditPoint += projPlanePoint.distanceTo( m_lastProjectedPoint );

    if ( m_distanceFromLastEditPoint >= m_length )
    {
        MFloatPoint projectedPoint;
        projectedPoint.setCast(projPlanePoint);
        m_editPoints.append( projectedPoint );
        status = createMesh();
        CHECK_MSTATUS_AND_RETURN_IT( status );
        m_distanceFromLastEditPoint = 0.0;
    }
    m_lastProjectedPoint = projPlanePoint;

    return MS::kSuccess;
}

MStatus	DrawPolygonContext::createMesh()
{
    MStatus status;
    
    status = deleteMesh();
    CHECK_MSTATUS_AND_RETURN_IT( status );

    // Create a mesh.
    MFnMesh fnMesh;
    MObject outData;
    unsigned int numVertices = m_editPoints.length();
    unsigned int numPolygons = 1;

    //  array of vertex counts for each polygon
    // It's 1 polygon so it is the number of vertices.
    MIntArray polygonCounts;
    polygonCounts.append(numVertices);

    // array of vertex connections for each polygon
    MIntArray polygonConnects;
    for (unsigned int i = 0; i < numVertices; ++i )
    {
        polygonConnects.append(i);
    }

    MObject oMesh = fnMesh.create(numVertices,
                                  numPolygons,
                                  m_editPoints,
                                  polygonCounts,
                                  polygonConnects,
                                  outData,
                                  &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    MFnDagNode fnDag(oMesh, &status);
    CHECK_MSTATUS_AND_RETURN_IT(status);
    status = fnDag.getPath(m_pathMesh);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Add initial shading group.
    char buffer[512];
    sprintf(buffer, "sets -e -forceElement initialShadingGroup \"%s\";", m_pathMesh.partialPathName().asChar());
    status = MGlobal::executeCommand(buffer);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    m_view.refresh(false, true);
    return MS::kSuccess;
}


MStatus	DrawPolygonContext::deleteMesh()
{
    MStatus status;
    
    if ( m_pathMesh.isValid() )
    {
        char buffer[512];
        sprintf(buffer, "delete %s;", m_pathMesh.partialPathName().asChar());
        status = MGlobal::executeCommand( buffer );
        CHECK_MSTATUS_AND_RETURN_IT( status );
    }

    return MS::kSuccess;
}


MStatus	DrawPolygonContext::doRelease( MEvent& event )
{
    MStatus status;

    if ( !m_intersects )
    {
        return MS::kSuccess;
    }

    short mouseX, mouseY;
    event.getPosition( mouseX, mouseY );
    MPoint nearClip, farClip;
    status = m_view.viewToWorld( mouseX, mouseY, nearClip, farClip );

    MFloatPoint projectedPoint;

    MPoint projPlanePoint = planeLineIntersection( nearClip, farClip, m_intersectionPoint, m_cameraNormal );
    projectedPoint.setCast(projPlanePoint);
    m_editPoints.append( projectedPoint );

    status = deleteMesh();
    CHECK_MSTATUS_AND_RETURN_IT( status );

    // Create the tool command so we can undo the curve creation
    DrawPolygonToolCommand* pCmd = (DrawPolygonToolCommand*)newToolCommand();
    pCmd->setEditPoints( m_editPoints );
    pCmd->redoIt();
    pCmd->finalize();

    return MS::kSuccess;
}


MPoint DrawPolygonContext::planeLineIntersection( const MPoint& lineA,
        const MPoint& lineB, const MPoint& planeP, const MVector& planeN )
{
    MVector v = lineB - lineA;
    return lineA + v * ((planeN * (planeP - lineA)) / (planeN * v));
}


void DrawPolygonContext::postRenderCallback( const MString& panelName, void* data )
{
    DrawPolygonContext *pContext = (DrawPolygonContext*) data;
    if ( !pContext )
    {
        return;
    }

    pContext->updateCameraNormal();
}
