/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_ACTIONDRAWARC2POINTSLENGTH_H
#define LC_ACTIONDRAWARC2POINTSLENGTH_H

#include <QObject>
#include "lc_actiondrawarc2pointsbase.h"

class LC_ActionDrawArc2PointsLength: public LC_ActionDrawArc2PointsBase{
    Q_OBJECT
public:
    LC_ActionDrawArc2PointsLength(LC_ActionContext *actionContext);
    ~LC_ActionDrawArc2PointsLength() override = default;
protected:
    bool createArcData(RS_ArcData &data, int status, RS_Vector pos, bool reverse, bool reportErrors) override;
    void doPreviewOnPoint2Custom(RS_Arc *pArc) override;
    QString getParameterCommand() override;
    QString getParameterPromptValue() const override;
    QString getAlternativePoint2Prompt() const override;
    double determineArcAngleByLenAndChord(double arcLen, double chordLen);
};

#endif // LC_ACTIONDRAWARC2POINTSLENGTH_H
