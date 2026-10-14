#include "cropoverlay.h"

#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>

CropOverlayItem::CropOverlayItem(QGraphicsItem *parent) : QGraphicsObject(parent)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

QRectF CropOverlayItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), videoSize_);
}

void CropOverlayItem::setVideoSize(const QSize &size)
{
    if (size == videoSize_) return;
    prepareGeometryChange();
    videoSize_ = size;
    if (crop_.isEmpty()) crop_ = QRect(QPoint(0, 0), size);
    crop_ = clampedCrop(crop_);
    update();
}

void CropOverlayItem::setCrop(const QRect &rect)
{
    const QRect next = clampedCrop(rect);
    if (next == crop_) return;
    crop_ = next;
    update();
}

void CropOverlayItem::setHandleSceneSize(qreal size)
{
    handleSize_ = qMax(1.0, size);
    update();
}

void CropOverlayItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (videoSize_.isEmpty() || crop_.isEmpty()) return;
    QPainterPath shade;
    shade.addRect(boundingRect());
    shade.addRect(crop_);
    painter->fillPath(shade, QColor(0, 0, 0, 120));
    QPen pen(QColor("#7eb6ff"));
    pen.setCosmetic(true);
    pen.setWidthF(2);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(crop_);
}

int CropOverlayItem::edgeAt(const QPointF &position) const
{
    const int handle = qMax(1, qRound(handleSize_));
    const QPoint point = position.toPoint();
    if (!crop_.adjusted(-handle, -handle, handle, handle).contains(point))
        return NoEdge;
    const bool left = qAbs(point.x() - crop_.left()) <= handle;
    const bool right = qAbs(point.x() - crop_.right()) <= handle;
    const bool top = qAbs(point.y() - crop_.top()) <= handle;
    const bool bottom = qAbs(point.y() - crop_.bottom()) <= handle;
    int edge = NoEdge;
    if (left) edge |= Left;
    if (right) edge |= Right;
    if (top) edge |= Top;
    if (bottom) edge |= Bottom;
    if (edge != NoEdge) return edge;
    return crop_.contains(point) ? Move : NoEdge;
}

QRect CropOverlayItem::clampedCrop(QRect rect) const
{
    if (videoSize_.isEmpty()) return {};
    rect = rect.normalized();
    rect.setWidth(qMax(2, rect.width()));
    rect.setHeight(qMax(2, rect.height()));
    if (rect.left() < 0) rect.moveLeft(0);
    if (rect.top() < 0) rect.moveTop(0);
    if (rect.right() > videoSize_.width()) rect.moveRight(videoSize_.width());
    if (rect.bottom() > videoSize_.height()) rect.moveBottom(videoSize_.height());
    if (rect.left() < 0) rect.setLeft(0);
    if (rect.top() < 0) rect.setTop(0);
    rect.setWidth(qBound(2, rect.width(), videoSize_.width()));
    rect.setHeight(qBound(2, rect.height(), videoSize_.height()));
    return rect.intersected(QRect(QPoint(0, 0), videoSize_));
}

void CropOverlayItem::updateCursor(int edge)
{
    if (edge == Move) setCursor(Qt::SizeAllCursor);
    else if ((edge & Left && edge & Top) || (edge & Right && edge & Bottom)) setCursor(Qt::SizeFDiagCursor);
    else if ((edge & Right && edge & Top) || (edge & Left && edge & Bottom)) setCursor(Qt::SizeBDiagCursor);
    else if (edge & (Left | Right)) setCursor(Qt::SizeHorCursor);
    else if (edge & (Top | Bottom)) setCursor(Qt::SizeVerCursor);
    else unsetCursor();
}

void CropOverlayItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    updateCursor(edgeAt(event->pos()));
    QGraphicsObject::hoverMoveEvent(event);
}

void CropOverlayItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    activeEdge_ = edgeAt(event->pos());
    if (activeEdge_ == NoEdge) {
        event->ignore();
        return;
    }
    pressPosition_ = event->pos();
    pressCrop_ = crop_;
    updateCursor(activeEdge_);
    event->accept();
}

void CropOverlayItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (activeEdge_ == NoEdge) return;
    const QPoint delta = (event->pos() - pressPosition_).toPoint();
    QRect next = pressCrop_;
    if (activeEdge_ == Move) {
        next.translate(delta);
    } else {
        if (activeEdge_ & Left) next.setLeft(pressCrop_.left() + delta.x());
        if (activeEdge_ & Right) next.setRight(pressCrop_.right() + delta.x());
        if (activeEdge_ & Top) next.setTop(pressCrop_.top() + delta.y());
        if (activeEdge_ & Bottom) next.setBottom(pressCrop_.bottom() + delta.y());
    }
    next = clampedCrop(next);
    if (next == crop_) return;
    crop_ = next;
    update();
    emit cropEdited(crop_);
}

void PreviewView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    emit resized();
}
