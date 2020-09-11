#ifndef CLOCKZONE_H
#define CLOCKZONE_H

#include "selectzone.h"

#include <QFont>

class ClockZone;

class ClockEllipse : public QObject, public QGraphicsEllipseItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    bool mIsHovered = false;
    int mCount = 0;
    int mArrowXOffset = 0, mArrowYOffset = 0;
    QFont mFont;
    ClockZone *mZone;

public:
    ClockEllipse(QGraphicsItem *parent = nullptr);

    void hide()
    {
        setOpacity(0.0);
    }

    void setNumber(qreal xOffset, int count);
    void setZone(ClockZone *zone)
    {
        mZone = zone;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
};

class ClockZone : public SelectZone
{
    Q_OBJECT
private:
    qreal xspace = 25;
    qreal cardOffset = 30;
    ClockEllipse dmgEllipse;
    bool active;

public:
    ClockZone(Player *_p, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();
    void levelUp();

    void setActive(bool _active)
    {
        active = _active;
        update();
    }

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
