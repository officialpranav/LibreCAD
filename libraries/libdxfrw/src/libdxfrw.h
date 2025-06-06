/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2016-2021 A. Stebich (librecad@mail.lordofbikes.de)        **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef LIBDXFRW_H
#define LIBDXFRW_H

#include <string>
#include <unordered_map>
#include "drw_entities.h"
#include "drw_objects.h"
#include "drw_header.h"
#include "drw_interface.h"


class dxfReader;
class dxfWriter;

class dxfRW {
public:
    dxfRW(const char* name);
    virtual ~dxfRW();
    void setDebug(DRW::DebugLevel lvl);
    /// reads the file specified in constructor
    /*!
     * An interface must be provided. It is used by the class to signal various
     * components being added.
     * @param interface_ the interface to use
     * @param ext should the extrusion be applied to convert in 2D?
     * @return true for success
     */
    bool read(DRW_Interface *interface_, bool ext);
    void setBinary(bool b) {binFile = b;}

    bool write(DRW_Interface *interface_, DRW::Version ver, bool bin);
    bool writeLineType(DRW_LType *ent);
    bool writeLayer(DRW_Layer *ent);
    bool writeView(DRW_View *ent);
    bool writeUCS(DRW_UCS* ent);
    bool writeDimstyle(DRW_Dimstyle *ent);
    bool writeTextstyle(DRW_Textstyle *ent);
    bool writeVport(DRW_Vport *ent);
    bool writeAppId(DRW_AppId *ent);
    bool writePoint(DRW_Point *ent);
    bool writeLine(DRW_Line *ent);
    bool writeRay(DRW_Ray *ent);
    bool writeXline(DRW_Xline *ent);
    bool writeCircle(DRW_Circle *ent);
    bool writeArc(DRW_Arc *ent);
    bool writeEllipse(DRW_Ellipse *ent);
    bool writeTrace(DRW_Trace *ent);
    bool writeSolid(DRW_Solid *ent);
    bool write3dface(DRW_3Dface *ent);
    bool writeLWPolyline(DRW_LWPolyline *ent);
    bool writePolyline(DRW_Polyline *ent);
    bool writeSpline(DRW_Spline *ent);
    bool writeBlockRecord(std::string name);
    bool writeBlock(DRW_Block *ent);
    bool writeInsert(DRW_Insert *ent);
    bool writeMText(DRW_MText *ent);
    bool writeText(DRW_Text *ent);
    bool writeHatch(DRW_Hatch *ent);
    bool writeViewport(DRW_Viewport *ent);
    DRW_ImageDef *writeImage(DRW_Image *ent, std::string name);
    bool writeLeader(DRW_Leader *ent);
    bool writeDimension(DRW_Dimension *ent);
    void setEllipseParts(int parts){elParts = parts;} /*!< set parts number when convert ellipse to polyline */
    bool writePlotSettings(DRW_PlotSettings *ent);

    DRW::Version getVersion() const;
    DRW::error getError() const;

private:
    /// used by read() to parse the content of the file
    bool processDxf();
    bool processHeader();
    bool processTables();
    bool processBlocks();
    bool processBlock();
    bool processEntities(bool isblock);
    bool processObjects();

    bool processLType();
    bool processLayer();
    bool processDimStyle();
    bool processTextStyle();
    bool processVports();
    bool processView();
    bool processUCS();
    bool processAppId();

    bool processPoint();
    bool processLine();
    bool processRay();
    bool processXline();
    bool processCircle();
    bool processArc();
    bool processEllipse();
    bool processTrace();
    bool processSolid();
    bool processInsert();
    bool processLWPolyline();
    bool processPolyline();
    bool processVertex(DRW_Polyline* pl);
    bool processTolerance();
    bool processText();
    bool processMText();
    bool processHatch();
    bool processSpline();
    bool process3dface();
    bool processViewport();
    bool processImage();
    bool processImageDef();
    bool processDimension();
    bool processLeader();
    bool processPlotSettings();

//    bool writeHeader();
    bool writeEntity(DRW_Entity *ent);
    bool writeTables();
    bool writeBlocks();
    bool writeObjects();
    bool writeExtData(const std::vector<DRW_Variant*> &ed);
    /*use version from dwgutil.h*/
    std::string toHexStr(int n);//RLZ removeme
    bool writeAppData(const std::list<std::list<DRW_Variant>> &appData);

    bool setError(const DRW::error lastError);

private:
    DRW::Version version;
    DRW::error error {DRW::BAD_NONE};
    std::string fileName;
    std::string codePage;
    bool binFile = false;
    dxfReader *reader = nullptr;
    dxfWriter *writer = nullptr;
    DRW_Interface *iface = nullptr;
    DRW_Header header;
//    int section;
    std::string nextentity;
    int entCount = 0;
    bool wlayer0 = false;
    bool dimstyleStd = false;
    bool applyExt =false;
    bool writingBlock;
    int elParts;  /*!< parts number when convert ellipse to polyline */
    std::unordered_map<std::string,int> blockMap;
    std::unordered_map<std::string,int> textStyleMap;
    std::vector<DRW_ImageDef*> imageDef;  /*!< imageDef list */

    int currHandle;


};

#endif // LIBDXFRW_H
