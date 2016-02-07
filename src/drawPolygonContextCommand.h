#ifndef DRAWPOLYGONCONTEXTCOMMAND_H
#define DRAWPOLYGONCONTEXTCOMMAND_H
#include <maya/MPxContextCommand.h>
#include "drawPolygonContext.h"

#define LENGTH_FLAG         "-l"
#define LENGTH_FLAG_LONG    "-length"

class DrawPolygonContextCommand : public MPxContextCommand
{
    public:
        DrawPolygonContextCommand();
        static void* creator();

        virtual MStatus doEditFlags();
        virtual MStatus doQueryFlags();
        virtual MStatus appendSyntax();
        virtual MPxContext* makeObj();

    protected:
        DrawPolygonContext* m_pContext;
};

#endif
