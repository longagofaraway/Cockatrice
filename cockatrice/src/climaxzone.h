#ifndef CLIMAXZONE_H
#define CLIMAXZONE_H

#include "pilezone.h"

class ClimaxZone : public PileZone
{
    Q_OBJECT
public:
    ClimaxZone(Player *_p, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    //QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
};

#endif
