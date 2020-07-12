#include "climaxzone.h"

#include "abstractcarditem.h"
#include "carditem.h"

#include <QPainter>

ClimaxZone::ClimaxZone(Player *_p, QGraphicsItem *parent)
    : PileZone(_p, "climax", false, true, parent)
{}

void ClimaxZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    if (!cards.isEmpty())
        cards.at(0)->paintPicture(painter, cards.at(0)->getTranslatedSize(painter), -90);
    else {
        painter->save();
        painter->setOpacity(0.3);

        painter->translate((float)CARD_WIDTH / 2, (float)CARD_HEIGHT / 2);
        painter->rotate(-90);
        painter->translate((float)-CARD_WIDTH / 2, (float)-CARD_HEIGHT / 2);

        QPixmap pixmap = QPixmap("theme:icons/climaxes_icons");
		qreal xOff = (CARD_WIDTH - pixmap.width()) / 2;
        qreal yOff = (CARD_HEIGHT - pixmap.height()) / 2;
        painter->drawPixmap(QPointF(xOff, yOff), pixmap);
        painter->restore();
    }

    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));
    painter->setPen(QColor(255, 255, 255, 40));
    painter->drawLine(boundingRect().topLeft(), boundingRect().topRight());
    painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
    painter->drawLine(boundingRect().topRight(), boundingRect().bottomRight());
}

void ClimaxZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZone *startZone,
                                const QPoint & dropPoint)
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
