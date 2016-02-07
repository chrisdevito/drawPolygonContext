#include "drawPolygonContextCommand.h"

void* DrawPolygonContextCommand::creator()
{
    return new DrawPolygonContextCommand;
}


DrawPolygonContextCommand::DrawPolygonContextCommand() 
{
    m_pContext = NULL;
}


MPxContext* DrawPolygonContextCommand::makeObj()
{
    m_pContext = new DrawPolygonContext();
	return m_pContext;
}


MStatus DrawPolygonContextCommand::appendSyntax()
{
    MStatus status;
    MSyntax mSyntax = syntax();
   
    status = mSyntax.addFlag( LENGTH_FLAG, LENGTH_FLAG_LONG, MSyntax::kDouble );
    CHECK_MSTATUS_AND_RETURN_IT( status );

    return MS::kSuccess;
}


MStatus DrawPolygonContextCommand::doEditFlags()
{
    MStatus status;
    MArgParser argData = parser();

    if ( argData.isFlagSet( LENGTH_FLAG ) )
    {
        float length = (float)argData.flagArgumentDouble( LENGTH_FLAG, 0, &status );
        CHECK_MSTATUS_AND_RETURN_IT( status );
        m_pContext->setLength( length );
    }

    return MS::kSuccess;
}


MStatus DrawPolygonContextCommand::doQueryFlags()
{
    MArgParser argData = parser();
   
    if ( argData.isFlagSet( LENGTH_FLAG ) )
    {
        setResult( m_pContext->getLength() );
    }

    return MS::kSuccess;
}
