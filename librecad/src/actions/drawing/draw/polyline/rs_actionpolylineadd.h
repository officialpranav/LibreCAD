/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
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

#ifndef RS_ACTIONPOLYLINEADD_H
#define RS_ACTIONPOLYLINEADD_H

#include "rs_previewactioninterface.h"
class RS_Polyline;

/**
 * This action class can handle user events to move entities.
 *
 * @author Andrew Mustun
 */
class RS_ActionPolylineAdd:public RS_PreviewActionInterface {
    Q_OBJECT
public:
    RS_ActionPolylineAdd(LC_ActionContext *actionContext);
    ~RS_ActionPolylineAdd() override;
    void init(int status) override;
    void finish(bool updateTB) override;
protected:
    /**
     * Action States.
     */
    enum Status {
        ChooseSegment,  /**< Choosing a segment of existing polyline to add node. */
        SetAddCoord   /**< Setting the reference point. */
    };

    RS_Polyline *m_polylineToModify = nullptr;
    RS_Entity *m_addSegment = nullptr;
    std::unique_ptr<RS_Vector> m_addCoord;

    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void updateMouseButtonHints() override;
    void doTrigger() override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
};
#endif
