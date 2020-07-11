#include "levelzone.h"

#include "pb/command_move_card.pb.h"
#include "carddragitem.h"
#include "abstractcarditem.h"
#include "carditem.h"
#include "player.h"

#include <QPainter>

LevelZone::LevelZone(Player *_p, QGraphicsItem *parent)
    : SelectZone(_p, "level", true, false, true, parent)
{
}

void LevelZone::addCardImpl(CardItem *card, int /*x*/, int /*y*/)
{
    cards.append(card);

    card->setParentItem(this);
    card->resetState();
	card->setTapped(AbstractCardItem::Tapped);
    card->setVisible(true);
    card->update();
}

QRectF LevelZone::boundingRect() const
{
    return QRectF(0, 0, CARD_HEIGHT + 10, CARD_WIDTH * 2 + 5);
}

void LevelZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));
    painter->setPen(QColor(255, 255, 255, 40));
    painter->drawRect(boundingRect());
}

void LevelZone::handleDropEvent(const QList<CardDragItem *> &dragItems,
                                CardZone *startZone,
                                const QPoint & /*dropPoint*/)
{
    if (startZone == this)
        return;
    if (startZone->getPlayer() != this->getPlayer())
        return;
    if (dragItems.size() != 1)
        return;
    if (cards.size() >= 3)
        return;

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(0);
    cmd.set_y(0);

    cmd.mutable_cards_to_move()->add_card()->set_card_id(dragItems[0]->getId());

    player->sendGameCommand(cmd);
}

void LevelZone::reorganizeCards()
{
    if (!cards.isEmpty()) {
        const qreal x = (boundingRect().width() - cards.at(0)->boundingRect().width()) / 2;
        const qreal cardHeight = cards.at(0)->boundingRect().height();
        const qreal totalHeight = boundingRect().height();
        const qreal cardOffset = 25;
        for (int i = 0; i < cards.size(); i++) {
            CardItem *c = cards.at(i);
            qreal y = i * cardOffset;
            if (!player->getMirrored())
                y = totalHeight - y - cardHeight;
            c->setPos(x, y);
            c->setRealZValue(i);
        }
    }
    update();
}
