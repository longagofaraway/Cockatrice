#include "clockzone.h"

#include "abstractcarditem.h"
#include "carddragitem.h"
#include "carditem.h"
#include "pb/command_move_card.pb.h"
#include "player.h"

#include <QPainter>

ClockZone::ClockZone(Player *_p, QGraphicsItem *parent)
    : SelectZone(_p, "clock", true, false, true, parent), active(false)
{
    dmgEllipse.setZone(this);
    dmgEllipse.setParentItem(this);
    dmgEllipse.setBrush(Qt::white);
    dmgEllipse.setAcceptHoverEvents(true);
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
    qreal width = xspace * 2 + cardOffset * 8 + CARD_WIDTH * 2;
    return QRectF(0, 0, width, CARD_HEIGHT + 20);
}

void ClockZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    painter->fillRect(boundingRect(), QBrush(QColor("transparent")));

    painter->save();
    if (active)
        painter->setOpacity(0.35);
    else
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

        int offsetNum = (cardCount > 6) ? (cardCount - 2) : (cardCount - 1);
        while (xspace * 2 + localCardOffset * offsetNum + cardWidth * 2 + dmgEllipse.boundingRect().width() >
               totalWidth)
            --localCardOffset;

        int i = 0;
        for (; i < cardCount; i++) {
            if (i == 6)
                break;
            CardItem *c = cards.at(i);
            qreal x = xspace + localCardOffset * i;
            c->setPos(x, y);
            c->setRealZValue(i);
        }
        dmgEllipse.setNumber(xspace + localCardOffset * (i - 1) + cardWidth + 5, i);

        for (; i < cardCount; i++) {
            CardItem *c = cards.at(i);
            qreal x = xspace + localCardOffset * (i - 1) + cardWidth + 5 * 2 + dmgEllipse.boundingRect().width();
            c->setPos(x, y);
            c->setRealZValue(i);
        }
    } else {
        dmgEllipse.hide();
    }
    update();
}

void ClockZone::levelUp()
{
    Command_MoveCard cmd;
    cmd.set_start_zone(getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone("grave");
    cmd.set_x(0);

    int cardCount = cards.size() > 6 ? 6 : cards.size();
    for (int i = 0; i < cardCount; ++i)
        cmd.mutable_cards_to_move()->add_card()->set_card_id(cards[i]->getId());

    player->sendGameCommand(cmd);
}

ClockEllipse::ClockEllipse(QGraphicsItem *parent) : QObject(), QGraphicsEllipseItem(parent)
{
    mFont.setFamily("Serif");
    mFont.setPixelSize(28);
    mFont.setWeight(QFont::Bold);
}

void ClockEllipse::setNumber(qreal xOffset, int count)
{
    setOpacity(1.0);

    mCount = count;
    QFontMetrics fm(mFont);
    double w = 1.3 * fm.horizontalAdvance(QString::number(count));
    double h = fm.height() * 1.3;
    if (w < h)
        w = h;

    qreal y = (parentItem()->boundingRect().height() - h) / 2.0;
    setRect(0, 0, w, h);
    setPos(xOffset, y);
}

void ClockEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsEllipseItem::paint(painter, option, widget);

    if (!mIsHovered) {
        painter->save();
        painter->setPen(Qt::black);
        painter->setFont(mFont);
        painter->drawText(rect(), Qt::AlignCenter, QString::number(mCount));
        painter->restore();
    } else {
        QPixmap hov =
            QPixmap("theme:icons/arrow_left_green").scaled(boundingRect().width() - 8, boundingRect().height() - 8);
        painter->drawPixmap(4 + mArrowXOffset, 4 + mArrowYOffset, hov);
    }
}

void ClockEllipse::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    mIsHovered = true;
    update();
}

void ClockEllipse::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mIsHovered = false;
    update();
}

void ClockEllipse::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mArrowXOffset += 1;
    mArrowYOffset += 1;
    update();
}
void ClockEllipse::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mArrowXOffset -= 1;
    mArrowYOffset -= 1;
    mZone->levelUp();
    update();
}