#include "climaxzone.h"

#include "abstractcarditem.h"
#include "carditem.h"

#include <QPainter>

ClimaxZone::ClimaxZone(Player *_p, QGraphicsItem *parent) : PileZone(_p, "climax", false, true, parent)
{
}

void ClimaxZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (!cards.isEmpty())
        cards.at(0)->paintPicture(painter, cards.at(0)->getTranslatedSize(painter), -90);
    else {
        painter->save();
        painter->setOpacity(0.15);
        QPixmap border = QPixmap("theme:climaxZone").scaled(boundingRect().width(), boundingRect().height());
        painter->drawPixmap(0, 0, border);
        painter->restore();
    }

    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));
}

void ClimaxZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint)
{
    if (startZone == this)
        return;
    if (startZone->getPlayer() != this->getPlayer())
        return;
    if (dragItems.size() != 1)
        return;
    if (!cards.isEmpty())
        return;

    PileZone::handleDropEvent(dragItems, startZone, dropPoint);
}
