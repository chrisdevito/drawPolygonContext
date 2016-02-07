#include "drawPolygonContext.h"
#include "drawPolygonContextCommand.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{
    MStatus status;
    MFnPlugin fnPlugin( obj, "Christopher DeVito", "1.0", "Any" );

    status = fnPlugin.registerContextCommand(
        "drawPolygonContext",
        DrawPolygonContextCommand::creator,
        "drawPolygonTool",
        DrawPolygonToolCommand::creator);
    CHECK_MSTATUS_AND_RETURN_IT(status);

    // Execute the command.
    MGlobal::executeCommand("drawPolygonContext drawPolygonContext1;");

    return MS::kSuccess;
}

MStatus uninitializePlugin( MObject obj )
{
    MStatus status;
    MFnPlugin fnPlugin(obj);

    status = fnPlugin.deregisterContextCommand("drawPolygonContext");
    CHECK_MSTATUS_AND_RETURN_IT(status);

    return MS::kSuccess;
}