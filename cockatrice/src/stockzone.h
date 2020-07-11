#ifndef STOCKZONE_H
#define STOCKZONE_H

#include "selectzone.h"

class StockZone : public SelectZone
{
    Q_OBJECT
private:
    qreal zoneHeight;

public:
    StockZone(Player *_p, int _zoneHeight, QGraphicsItem *parent = nullptr);
    void handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint);
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void reorganizeCards();
    CardItem *takeCard(int position, int cardId, bool canResize = true);
    void removeCard(CardItem *card);

protected:
    void addCardImpl(CardItem *card, int x, int y);
};

#endif
