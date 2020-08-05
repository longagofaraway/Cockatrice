#include "clockzone.h"

#include "abstractcarditem.h"
#include "carddragitem.h"
#include "carditem.h"
#include "pb/command_move_card.pb.h"
#include "player.h"

#include <QPainter>

ClockZone::ClockZone(Player *_p, int _zoneWidth, QGraphicsItem *parent)
    : SelectZone(_p, "clock", true, false, true, parent), zoneWidth(_zoneWidth)
{
}

void ClockZone::addCardImpl(CardItem *card, int x, int /*y*/)
{
    if (x == -1)
        x = cards.size();
    cards.insert(x, card);

    card->setParentItem(this);
    card->resetState();
    card->setVisible(true);
    card->update();
}

QRectF ClockZone::boundingRect() const
{
    return QRectF(0, 0, xspace * 2 + cardOffset * 8 + CARD_WIDTH, CARD_HEIGHT + 20);
}

void ClockZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));

    painter->save();
    painter->setOpacity(0.15);
    QPixmap border = QPixmap("theme:clockZone").scaled(boundingRect().width(), boundingRect().height());
    painter->drawPixmap(0, 0, border);
    painter->restore();
}

void ClockZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZone *startZone,
                                const QPoint & /*dropPoint*/)
{
    if (startZone == this)
        return;
    if (startZone->getPlayer() != this->getPlayer())
        return;

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(-1);
    cmd.set_y(0);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    player->sendGameCommand(cmd);
}

void ClockZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        const int cardCount = cards.size();
        qreal totalWidth = boundingRect().width();
        qreal totalHeight = boundingRect().height();
        qreal cardWidth = cards.at(0)->boundingRect().width();
        qreal cardHeight = cards.at(0)->boundingRect().height();
        qreal y = (totalHeight - cardHeight) / 2;
        qreal localCardOffset = cardOffset;

        while (xspace * 2 + localCardOffset * (cardCount - 1) + cardWidth > totalWidth)
            --localCardOffset;

        for (int i = 0; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            qreal x = xspace + localCardOffset * i;
            c->setPos(x, y);
            c->setRealZValue(i);
        }
    }
    update();
}
