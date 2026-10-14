#pragma once

#include <QGraphicsObject>
#include <QGraphicsView>
#include <QRect>

class CropOverlayItem final : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit CropOverlayItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setVideoSize(const QSize &size);
    void setCrop(const QRect &rect);
    void setHandleSceneSize(qreal size);

signals:
    void cropEdited(const QRect &rect);

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    enum Edge {
        NoEdge = 0,
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8,
        Move = 16
    };

    int edgeAt(const QPointF &position) const;
    QRect clampedCrop(QRect rect) const;
    void updateCursor(int edge);

    QSize videoSize_;
    QRect crop_;
    QRect pressCrop_;
    QPointF pressPosition_;
    int activeEdge_ = NoEdge;
    qreal handleSize_ = 12;
};

class PreviewView final : public QGraphicsView
{
    Q_OBJECT

public:
    using QGraphicsView::QGraphicsView;

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent *event) override;
};
