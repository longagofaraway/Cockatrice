#ifndef HANDZONE_H
#define HANDZONE_H

#include "selectzone.h"

class HandZone : public SelectZone
{
    Q_OBJECT
private:
    qreal width, zoneHeight;
    bool active;

    void paintZoneOutline(QPainter *painter);
private slots:
    void updateBg();
public slots:
    void updateOrientation();

public:
    HandZone(Player *_p, bool _contentsKnown, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();
    void setWidth(qreal _width);

    void setActive(bool _active)
    {
        active = _active;
        update();
    }

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
