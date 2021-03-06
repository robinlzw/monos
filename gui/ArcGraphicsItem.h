/**
 *  Copyright 2015 -- 2019 Peter Palfraader
 *              2018, 2019 Günther Eder - geder@cs.sbg.ac.at
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/Qt/PainterOstream.h>

#include <QPen>

#include "BasicInput.h"
#include "Wavefront.h"
#include "cgTypes.h"

class ArcGraphicsItem :
  public CGAL::Qt::GraphicsItem
{
  private:
    using Base = CGAL::Qt::GraphicsItem;

  private:
    const Nodes * const nodes;
    const ArcList * const arcs;
    CGAL::Qt::PainterOstream<K> painterostream;
    QPen vertices_pen;
    QPen segments_pen;
    QPen segments_debug_pen;
    QPen labels_pen;

    bool visible_labels 	 = false;
    bool visible_arc_labels  = false;
    bool visible_node_labels = false;

    bool drawNode(const Node& node) const;

  protected:
    QRectF bounding_rect;
    void updateBoundingBox();

  public:
    ArcGraphicsItem(const Nodes * const nodes, const ArcList * arcs);

    QRectF boundingRect() const { return bounding_rect; };
    void setBoundingRect(const QRectF& br) {bounding_rect = br;}

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


    void setVerticesPen(const QPen& pen) { vertices_pen = pen; };
    void setSegmentsPen(const QPen& pen) { segments_pen = pen; };
    void setLabelsPen(const QPen& pen) { labels_pen = pen; };
    const QPen& verticesPen() const { return vertices_pen; }
    const QPen& segmentsPen() const { return segments_pen; }
    const QPen& segmentsDebugPen() const { return segments_debug_pen; }
    const QPen& labelsPen() const { return labels_pen; }
    void setVisibleLabels(bool visible) { if (visible_labels != visible) { prepareGeometryChange(); }; visible_labels = visible; }
    void setVisibleArcLabels(bool visible) { if (visible_arc_labels != visible) { prepareGeometryChange(); }; visible_arc_labels = visible; }
    void setVisibleNodeLabels(bool visible) { if (visible_node_labels != visible) { prepareGeometryChange(); }; visible_node_labels = visible; }

    void modelChanged();
};
