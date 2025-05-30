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

#include "lc_refpoint.h"

#include "rs_painter.h"

LC_RefPoint::LC_RefPoint(RS_EntityContainer* parent,
                         const RS_Vector & d,
                         double size, int mode)
    :RS_Point(parent, RS_PointData(d)), pdmode{mode},pdsize{size}  {
    calculateBorders ();
}

RS_Entity* LC_RefPoint::clone() const {
    auto* p = new LC_RefPoint(*this);
    return p;
}

RS2::EntityType LC_RefPoint::rtti() const{
    return RS2::EntityRefPoint;
}

void LC_RefPoint::draw(RS_Painter *painter){
    painter->drawRefPointEntityWCS(data.pos, pdmode, pdsize);
}
