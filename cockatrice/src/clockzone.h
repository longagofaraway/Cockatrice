#ifndef CLOCKZONE_H
#define CLOCKZONE_H

#include "selectzone.h"

class ClockZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneWidth;

public:
    ClockZone(Player *_p, int _zoneWidth, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
