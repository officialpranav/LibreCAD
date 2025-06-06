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

#ifndef RS_ACTIONMODIFYBEVEL_H
#define RS_ACTIONMODIFYBEVEL_H

#include "rs_previewactioninterface.h"

class RS_AtomicEntity;
class RS_Entity;


/**
 * This action class can handle user events to bevel corners.
 *
 * @author Andrew Mustun
 */
class RS_ActionModifyBevel : public RS_PreviewActionInterface {
    Q_OBJECT
public:
    RS_ActionModifyBevel(LC_ActionContext *actionContext);
    ~RS_ActionModifyBevel() override;
    void init(int status) override;
    void finish(bool updateTB) override;
    QStringList getAvailableCommands() override;
    void setLength1(double l1);
    double getLength1() const;
    void setLength2(double l2);
    double getLength2() const;
    void setTrim(bool t);
    bool isTrimOn() const;
    void drawSnapper() override;
protected:
    /**
     * Action States.
     */
    enum Status {
        SetEntity1,      /**< Choosing the 1st entity. */
        SetEntity2,      /**< Choosing the 2nd entity. */
        SetLength1,      /**< Setting length 1 in command line. */
        SetLength2       /**< Setting length 2 in command line. */
    };
    RS_AtomicEntity* m_entity1 = nullptr;
    RS_AtomicEntity* m_entity2 = nullptr;
    struct BevelActionData;
    std::unique_ptr<BevelActionData> m_actionData;
    /** Last status before entering angle. */
    Status m_lastStatus = SetEntity1;

    bool isEntityAccepted(RS_Entity *en) const;
    bool areBothEntityAccepted(RS_Entity *en1, RS_Entity *en2) const;
    void previewLineModifications(const RS_Entity *original, const RS_Entity *trimmed, bool trimOnStart);
    RS2::CursorType doGetMouseCursor(int status) override;
    void onMouseLeftButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseRightButtonRelease(int status, LC_MouseEvent *e) override;
    void onMouseMoveEvent(int status, LC_MouseEvent *event) override;
    bool doProcessCommand(int status, const QString &command) override;
    void updateMouseButtonHints() override;
    LC_ActionOptionsWidget* createOptionsWidget() override;

    void doTrigger() override;
};
#endif
