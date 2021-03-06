#include "stockzone.h"

#include "carddragitem.h"
#include "carditem.h"
#include "pb/command_move_card.pb.h"
#include "player.h"
#include "thememanager.h"

#include <QPainter>

StockZone::StockZone(Player *_p, int _zoneHeight, QGraphicsItem *parent)
    : SelectZone(_p, "stock", false, true, false, parent), zoneHeight(_zoneHeight)
{
}

void StockZone::addCardImpl(CardItem *card, int /*x*/, int /*y*/)
{
    card->setId(cards.size());
    cards.append(card);

    if (!cards.getContentsKnown()) {
        card->setName();
    }
    card->setParentItem(this);
    card->resetState();
    card->setVisible(true);
    card->setTapped(AbstractCardItem::Tapped);
    card->update();
}

QRectF StockZone::boundingRect() const
{
    return QRectF(0, 0, CARD_HEIGHT + 15, zoneHeight);
}

void StockZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));

    painter->save();
    painter->setOpacity(0.15);
    QPixmap border = QPixmap("theme:stockZone").scaled(boundingRect().width(), boundingRect().height());
    painter->drawPixmap(0, 0, border);
    painter->restore();
}

void StockZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
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
    cmd.set_x(0);
    cmd.set_y(0);

    for (int i = 0; i < dragItems.size(); ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[i]->getId());

    player->sendGameCommand(cmd);
}

void StockZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        const int cardCount = cards.size();
        const qreal topOffset = 0;
        const qreal leftOffset = (boundingRect().width() - cards.at(0)->boundingRect().width()) / 2;
        const qreal totalHeight = boundingRect().height();
        const qreal cardHeight = cards.at(0)->boundingRect().height();
        qreal cardOffset = 25;

        while (2 * topOffset + (cardCount - 1) * cardOffset + cardHeight > totalHeight)
            --cardOffset;

        for (int i = 0; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            qreal y = topOffset + cardOffset * i;
            if (player->getMirrored())
                y = totalHeight - y - cardHeight;
            c->setPos(leftOffset, y);

            c->setRealZValue(i);
        }
    }
    update();
}

CardItem *StockZone::takeCard(int position, int cardId, bool /* canResize */)
{
    CardItem *result = CardZone::takeCard(position, cardId);
    for (int i = 0; i < cards.size(); ++i)
        cards[i]->setId(i);
    return result;
}

void StockZone::removeCard(CardItem *card)
{
    CardZone::removeCard(card);
    for (int i = 0; i < cards.size(); ++i)
        cards[i]->setId(i);
}
