/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2018 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include<iostream>
#include "rs_dimangular.h"

#include "rs_arc.h"
#include "rs_constructionline.h"
#include "rs_debug.h"
#include "rs_information.h"
#include "rs_line.h"
#include "rs_math.h"
#include "rs_pen.h"
#include "rs_solid.h"
#include "rs_units.h"

class RS_Arc;

RS_DimAngularData::RS_DimAngularData():
    definitionPoint1( false),
    definitionPoint2( false),
    definitionPoint3( false),
    definitionPoint4( false)
{
}

RS_DimAngularData::RS_DimAngularData(const RS_DimAngularData &ed):
    definitionPoint1( ed.definitionPoint1),
    definitionPoint2( ed.definitionPoint2),
    definitionPoint3( ed.definitionPoint3),
    definitionPoint4( ed.definitionPoint4)
{
}

/**
 * Constructor with initialisation.
 *
 * @param definitionPoint Definition point of the angular dimension.
 * @param leader Leader length.
 */
RS_DimAngularData::RS_DimAngularData(const RS_Vector& _definitionPoint1,
                                     const RS_Vector& _definitionPoint2,
                                     const RS_Vector& _definitionPoint3,
                                     const RS_Vector& _definitionPoint4):
    definitionPoint1( _definitionPoint1),
    definitionPoint2( _definitionPoint2),
    definitionPoint3( _definitionPoint3),
    definitionPoint4( _definitionPoint4)
{
}

/**
 * Constructor with initialisation.
 *
 * @param dimscale  general scale (DIMSCALE)
 * @param dimexo  distance from entities (DIMEXO)
 * @param dimexe  extension line extension (DIMEXE)
 * @param dimtxt  text height (DIMTXT)
 * @param dimgap  text distance to line (DIMGAP)
 * @param arrowSize  arrow length
 */
LC_DimAngularVars::LC_DimAngularVars(const double _dimscale,
                                     const double _dimexo,
                                     const double _dimexe,
                                     const double _dimtxt,
                                     const double _dimgap,
                                     const double _arrowSize,
                                     const double _tickSize) :
    dimscale( _dimscale),
    dimexo( _dimexo * _dimscale),
    dimexe( _dimexe * _dimscale),
    dimtxt( _dimtxt * _dimscale),
    dimgap( _dimgap * _dimscale),
    arrowSize( _arrowSize * _dimscale),
    m_tickSize( _tickSize * _dimscale)
{
}

LC_DimAngularVars::LC_DimAngularVars(const LC_DimAngularVars& av) :
    dimscale( av.dimscale),
    dimexo( av.dimexo),
    dimexe( av.dimexe),
    dimtxt( av.dimtxt),
    dimgap( av.dimgap),
    arrowSize( av.arrowSize)
{
}

/**
 * Constructor.
 *
 * @para parent Parent Entity Container.
 * @para d Common dimension geometrical data.
 * @para ed Extended geometrical data for angular dimension.
 */
RS_DimAngular::RS_DimAngular(RS_EntityContainer* parent,
                             const RS_DimensionData& d,
                             const RS_DimAngularData& ed) :
    RS_Dimension( parent, d),
    m_dimAngularData( ed)
{
    calcDimension();
    calculateBorders();
}

RS_Entity* RS_DimAngular::clone() const
{
    auto *d  = new RS_DimAngular(getParent(), getData(), getEData());

    d->setOwner( isOwner());
    d->init();
    d->detach();

    return d;
}

/**
 * @return Automatically created label for the default
 * measurement of this dimension.
 */
QString RS_DimAngular::getMeasuredLabel()
{
    int dimaunit {getGraphicVariableInt( QStringLiteral( "$DIMAUNIT"), 0)};
    int dimadec {getGraphicVariableInt( QStringLiteral( "$DIMADEC"), 0)};
    int dimazin {getGraphicVariableInt( QStringLiteral( "$DIMAZIN"), 0)};
    RS2::AngleFormat format {RS_Units::numberToAngleFormat( dimaunit)};
    QString strLabel( RS_Units::formatAngle( dimAngle, format, dimadec));

    if (RS2::DegreesMinutesSeconds != format  && RS2::Surveyors != format) {
        strLabel = stripZerosAngle( strLabel, dimazin);
    }

    //verify if units are decimal and comma separator
    if (RS2::DegreesMinutesSeconds != dimaunit) {
        if (',' == getDimDecimalFormatSeparatorChar()) {
            strLabel.replace( QChar('.'), QChar(','));
        }
    }
    return strLabel;
}

/**
 * @return Center of the measured dimension.
 */
RS_Vector RS_DimAngular::getCenter() const{
    return dimCenter;
}

/**
 * @brief Add an extension line if necessary
 *
 * @param dimLine  dimension definition line including extension offset
 * @param dimPoint  point where the arc meets the definition line
 * @param dirStart  unit vector defining the lines starting point direction
 * @param dirEnd  unit vector defining the lines ending point direction
 * @param av  DXF variables with offset and extension line length
 * @param pen  pen to draw the extension line
 */
void RS_DimAngular::extensionLine(const RS_ConstructionLine& dimLine,
                                  const RS_Vector& dimPoint,
                                  const RS_Vector& dirStart,
                                  const RS_Vector& dirEnd,
                                  const LC_DimAngularVars& av)
{
    double diffLine {RS_Vector::posInLine( dimLine.getStartpoint(), dimLine.getEndpoint(), dimPoint)};
    double diffCenter {RS_Vector::posInLine( dimLine.getStartpoint(), dimCenter, dimPoint)};

    if( 0.0 <= diffLine && 1.0 >= diffLine) {
        // dimension ends on entity, nothing to extend
        return;
    }
    if( 0.0 > diffLine && 0.0 > diffCenter) {
        addDimExtensionLine(dimLine.getStartpoint(),dimPoint - dirStart * av.exe());
    }
    else if( 1.0 < diffLine && 0.0 < diffCenter) {
        addDimExtensionLine(dimLine.getEndpoint(),dimPoint - dirEnd * av.exe());
    }
    else if( 0.0 > diffLine && 1.0 < diffCenter) {
        addDimExtensionLine(dimCenter - dirStart * av.exo(),dimPoint + dirEnd * av.exe());
    }
}

/**
 * @brief Add an arrow to the dimension arc
 *
 * @param point  arc endpoint, the arrow tip
 * @param angle  the angle from center to the arc endpoint
 * @param direction  this holds the sign for the arrow endpoint direction
 * @param outsideArrow  when the arc becomes too small, arrows are placed outside
 * @param av  DXF variables with offset and extension line length
 * @param pen  pen to draw the extension line
 */
void RS_DimAngular::arrow(const RS_Vector& point,
                          const double angle,
                          const double direction,
                          const bool outsideArrows,
                          const LC_DimAngularVars& av) {
    const double scaledTickSize = av.tickSize();
    if (scaledTickSize < 0.01) {
        double arrowAngle{0.0};

        if (outsideArrows) {
            // for outside arrows use tangent angle on endpoints
            // because for small radius the arrows looked inclined
            arrowAngle = angle + std::copysign(M_PI_2, direction);
        }
        else {
            // compute the angle from center to the endpoint of the arrow on the arc
            double endAngle{0.0};

            if (RS_TOLERANCE_ANGLE < dimRadius) {
                endAngle = av.arrow() / dimRadius;
            }

            // compute the endpoint of the arrow on the arc
            RS_Vector arrowEnd;
            arrowEnd.setPolar(dimRadius, angle + std::copysign(endAngle, direction));
            arrowEnd += dimCenter;
            arrowAngle = arrowEnd.angleTo(point);
        }

        RS_SolidData sd;
        RS_Solid* arrow;

        arrow = new RS_Solid(this, sd);
        arrow->shapeArrow(point, arrowAngle, av.arrow());
        addDimComponentEntity(arrow, getPenExtensionLine());
    }
    else {
        RS_Vector tickVector = RS_Vector::polar(scaledTickSize,
                                                (dimAngleL1 + dimAngleL2) / 2.0);

        addDimComponentLine(point - tickVector, point + tickVector, getPenExtensionLine());
    }
}

/**
 * Updates the sub entities of this dimension. Called when the
 * dimension or the position, alignment, .. changes.
 *
 * @param autoText Automatically reposition the text label
 */
void RS_DimAngular::doUpdateDim(){
    RS_DEBUG->print("RS_DimAngular::update");
    if (!dimCenter.valid) {
        return;
    }

    LC_DimAngularVars   av( getGeneralScale(),
                            getExtensionLineOffset(),
                            getExtensionLineExtension(),
                            getTextHeight(),
                            getDimensionLineGap(),
                            getArrowSize(),
                            getTickSize());

    // create new lines with offsets for extension lines
    RS_ConstructionLine line1(dimLine1.getStartpoint() - dimDir1s * av.exo(),
                              dimLine1.getEndpoint() - dimDir1e * av.exo());

    RS_ConstructionLine line2(dimLine2.getStartpoint() - dimDir2s * av.exo(),
                              dimLine2.getEndpoint() - dimDir2e * av.exo());

    RS_Vector p1 {dimCenter + dimDir1e * dimRadius};
    RS_Vector p2 {dimCenter + dimDir2e * dimRadius};

    extensionLine( line1, p1, dimDir1s, dimDir1e, av);
    extensionLine( line2, p2, dimDir2s, dimDir2e, av);

    RS_ArcData arcData( dimCenter, dimRadius, dimAngleL1, dimAngleL2, false);
    // Create dimension line (arc)
    RS_Arc* arc = addDimArc(arcData);

    // do we have to put the arrows outside of the arc?
    bool outsideArrows {arc->getLength() < 3.0 * av.arrow()};

    arrow( p1, dimAngleL1, +1.0, outsideArrows, av);
    arrow( p2, dimAngleL2, -1.0, outsideArrows, av);

    // text label

    RS_Vector textPos {arc->getMiddlePoint()};

    RS_Vector distV;
    double textAngle {0.0};
    double angle1 {textPos.angleTo( dimCenter) - M_PI_2};

    // rotate text so it's readable from the bottom or right (ISO)
    // quadrant 1 & 4
    if (angle1 > M_PI_2 * 3.0 + 0.001
        || angle1 < M_PI_2 + 0.001) {
        distV.setPolar( av.gap(), angle1 + M_PI_2);
        textAngle = angle1;
    }
    // quadrant 2 & 3
    else {
        distV.setPolar( av.gap(), angle1 - M_PI_2);
        textAngle = angle1 + M_PI;
    }

    // move text away from dimension line:
    textPos += distV;

    createDimText(textPos, av.txt(), textAngle);
}

void RS_DimAngular::update()
{
    calcDimension();
    RS_Dimension::update();
}

void RS_DimAngular::move(const RS_Vector& offset)
{
    RS_Dimension::move( offset);

    m_dimAngularData.definitionPoint1.move( offset);
    m_dimAngularData.definitionPoint2.move( offset);
    m_dimAngularData.definitionPoint3.move( offset);
    m_dimAngularData.definitionPoint4.move( offset);
    update();
}

void RS_DimAngular::rotate(const RS_Vector& center, double angle)
{
    rotate( center, RS_Vector( angle));
}

void RS_DimAngular::rotate(const RS_Vector& center, const RS_Vector& angleVector)
{
    RS_Dimension::rotate( center, angleVector);

    m_dimAngularData.definitionPoint1.rotate( center, angleVector);
    m_dimAngularData.definitionPoint2.rotate( center, angleVector);
    m_dimAngularData.definitionPoint3.rotate( center, angleVector);
    m_dimAngularData.definitionPoint4.rotate( center, angleVector);
    update();
}

void RS_DimAngular::scale(const RS_Vector& center, const RS_Vector& factor)
{
    RS_Dimension::scale( center, factor);

    m_dimAngularData.definitionPoint1.scale( center, factor);
    m_dimAngularData.definitionPoint2.scale( center, factor);
    m_dimAngularData.definitionPoint3.scale( center, factor);
    m_dimAngularData.definitionPoint4.scale( center, factor);
    update();
}

void RS_DimAngular::mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2)
{
    RS_Dimension::mirror( axisPoint1, axisPoint2);

    m_dimAngularData.definitionPoint1.mirror( axisPoint1, axisPoint2);
    m_dimAngularData.definitionPoint2.mirror( axisPoint1, axisPoint2);
    m_dimAngularData.definitionPoint3.mirror( axisPoint1, axisPoint2);
    m_dimAngularData.definitionPoint4.mirror( axisPoint1, axisPoint2);
    update();
}

/**
 * @brief Compute all static values for dimension.
 *
 * From DXF reference the lines are P2-P1 and P-P3.
 * The dimension is drawn from line1 (P2-P1) to line2 (P-P3) in CCW direction.
 */
void RS_DimAngular::calcDimension(void)
{
    // get unit vectors for definition points
    dimDir1s = RS_Vector::polar( 1.0, RS_Math::correctAngle( m_dimAngularData.definitionPoint2.angleTo( m_dimAngularData.definitionPoint1)));
    dimDir1e = RS_Vector::polar( 1.0, RS_Math::correctAngle( m_dimAngularData.definitionPoint1.angleTo( m_dimAngularData.definitionPoint2)));
    dimDir2s = RS_Vector::polar( 1.0, RS_Math::correctAngle( m_dimGenericData.definitionPoint.angleTo( m_dimAngularData.definitionPoint3)));
    dimDir2e = RS_Vector::polar( 1.0, RS_Math::correctAngle( m_dimAngularData.definitionPoint3.angleTo( m_dimGenericData.definitionPoint)));

    // create the two dimension definition lines
    dimLine1 = RS_ConstructionLine( nullptr,
                                    RS_ConstructionLineData( m_dimAngularData.definitionPoint2,
                                                             m_dimAngularData.definitionPoint1));
    dimLine2 = RS_ConstructionLine( nullptr,
                                    RS_ConstructionLineData( m_dimGenericData.definitionPoint,
                                                             m_dimAngularData.definitionPoint3));

    RS_VectorSolutions vs {RS_Information::getIntersection( &dimLine1, &dimLine2, false)};
    dimCenter = vs.get(0);
    dimRadius = dimCenter.distanceTo( m_dimAngularData.definitionPoint4);
    dimDirRad = RS_Vector::polar( 1.0, RS_Math::correctAngle( dimCenter.angleTo( m_dimAngularData.definitionPoint4)));

    fixDimension();

    dimAngleL1 = dimLine1.getDirection2();
    dimAngleL2 = dimLine2.getDirection2();

    dimAngle = RS_Math::correctAngle( dimLine2.getDirection1() - dimLine1.getDirection1());
}

/**
 * @brief check the dimension and fix non conform values from foreign CAD systems
 *
 * check if the radius definition point is on the arc,
 * from line1 to line2 in counter clockwise direction
 * LibreCAD takes care on correct orientation and line order in RS_ActionDimAngular
 * but angular dimensions, created in other CAD software, may fail and must be fixed here
 */
void RS_DimAngular::fixDimension(void){
    if( ! RS_Math::isAngleBetween( dimDirRad.angle(), dimDir2s.angle(), dimDir1s.angle(), false)) {
        double distance0 {m_dimGenericData.definitionPoint.distanceTo( dimCenter)};
        double distance1 {m_dimAngularData.definitionPoint1.distanceTo( dimCenter)};
        double distance2 {m_dimAngularData.definitionPoint2.distanceTo( dimCenter)};
        double distance3 {m_dimAngularData.definitionPoint3.distanceTo( dimCenter)};
        double  angle0 {0.0};
        double  angle1 {0.0};
        double  angle2 {0.0};
        double  angle3 {0.0};
        if( RS_TOLERANCE >= distance0) {
            angle3 = (m_dimAngularData.definitionPoint3 - dimCenter).angle();
            angle0 = angle3;
        }
        else if( RS_TOLERANCE >= distance3) {
            angle0 = (m_dimGenericData.definitionPoint - dimCenter).angle();
            angle3 = angle0;
        }
        else {
            angle0 = (m_dimGenericData.definitionPoint - dimCenter).angle();
            angle3 = (m_dimAngularData.definitionPoint3 - dimCenter).angle();
        }

        if( RS_TOLERANCE >= distance1) {
            angle2 = (m_dimAngularData.definitionPoint2- dimCenter).angle();
            angle1 = angle2;
        }
        else if( RS_TOLERANCE >= distance2) {
            angle1 = (m_dimAngularData.definitionPoint1 - dimCenter).angle();
            angle2 = angle1;
        }
        else {
            angle1 = (m_dimAngularData.definitionPoint1 - dimCenter).angle();
            angle2 = (m_dimAngularData.definitionPoint2 - dimCenter).angle();
        }

        if( angle2 == angle1
            && distance2 < distance1
            && angle0 == angle3
            && distance0 < distance3) {
            // revert both lines
            dimLine1 = RS_ConstructionLine( nullptr,
                                            RS_ConstructionLineData( dimLine1.getEndpoint(),
                                                                     dimLine1.getStartpoint()));
            dimLine2 = RS_ConstructionLine( nullptr,
                                            RS_ConstructionLineData( dimLine2.getEndpoint(),
                                                                     dimLine2.getStartpoint()));

            // and their unit vectors
            RS_Vector swapDir {dimDir1s};
            dimDir1s = dimDir1e;
            dimDir1e = swapDir;

            swapDir = dimDir2s;
            dimDir2s = dimDir2e;
            dimDir2e = swapDir;
        }

        // check again, as the previous revert may have made this condition false
        if( ! RS_Math::isAngleBetween( dimDirRad.angle(), dimDir2s.angle(), dimDir1s.angle(), false)) {
            // swap the lines
            RS_ConstructionLine swapLine {dimLine1};
            dimLine1 = dimLine2;
            dimLine2 = swapLine;

            // and their unit vectors
            RS_Vector swapDir {dimDir1s};
            dimDir1s = dimDir2s;
            dimDir2s = swapDir;

            swapDir = dimDir1e;
            dimDir1e = dimDir2e;
            dimDir2e = swapDir;
        }
    }
}

/**
 * Dumps the point's data to stdout.
 */
std::ostream& operator <<(std::ostream& os, const RS_DimAngular& d) {
    os << " DimAngular: "
        << d.getData() << std::endl
        << d.getEData() << std::endl;

    return os;
}

std::ostream& operator <<(std::ostream& os, const RS_DimAngularData& dd) {
    os << "(" << dd.definitionPoint1
        << "," << dd.definitionPoint2
        << "," << dd.definitionPoint3
        << "," << dd.definitionPoint3
        << ")";

    return os;
}
