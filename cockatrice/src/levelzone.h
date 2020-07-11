#ifndef LEVELZONE_H
#define LEVELZONE_H

#include "selectzone.h"

class LevelZone : public SelectZone
{
    Q_OBJECT
public:
    LevelZone(Player *_p, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
