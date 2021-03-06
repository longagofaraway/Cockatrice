#include <QGraphicsScene>
#include <QPainter>
#include <QSet>
#include <cmath>
#ifdef _WIN32
#include "round.h"
#endif /* _WIN32 */
#include "arrowitem.h"
#include "carddatabase.h"
#include "carddragitem.h"
#include "carditem.h"
#include "pb/command_attach_card.pb.h"
#include "pb/command_flip_card.pb.h"
#include "pb/command_move_card.pb.h"
#include "pb/command_set_card_attr.pb.h"
#include "player.h"
#include "settingscache.h"
#include "tablezone.h"
#include "thememanager.h"

const QColor TableZone::BACKGROUND_COLOR = QColor(100, 100, 100);
const QColor TableZone::FADE_MASK = QColor(0, 0, 0, 80);
const QColor TableZone::GRADIENT_COLOR = QColor(255, 255, 255, 150);
const QColor TableZone::GRADIENT_COLORLESS = QColor(255, 255, 255, 0);

TableZone::TableZone(Player *_p, QGraphicsItem *parent)
    : SelectZone(_p, "table", true, false, true, parent), active(false)
{
    connect(themeManager, SIGNAL(themeChanged()), this, SLOT(updateBg()));
    connect(&SettingsCache::instance(), SIGNAL(invertVerticalCoordinateChanged()), this, SLOT(reorganizeCards()));

    updateBg();

    height = MARGIN_TOP + MARGIN_BOTTOM + TABLEROWS * CARD_HEIGHT + (TABLEROWS - 1) * PADDING_Y;
    width = MIN_WIDTH;
    currentMinimumWidth = width;

    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
}

void TableZone::updateBg()
{
    update();
}

QRectF TableZone::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

bool TableZone::isInverted() const
{
    return ((player->getMirrored() && !SettingsCache::instance().getInvertVerticalCoordinate()) ||
            (!player->getMirrored() && SettingsCache::instance().getInvertVerticalCoordinate()));
}

void TableZone::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QBrush brush = themeManager->getTableBgBrush();

    // If the player is other than Player 1
    if (player->getId() > 0) {
        // The player's id starts with 0 so in order to get the correct image we need to add 1
        brush = themeManager->getExtraTableBgBrush(QString::number(player->getId() + 1));
    }
    painter->fillRect(boundingRect(), brush);

    if (active) {
        paintZoneOutline(painter);
    } else {
        // inactive player gets a darker table zone with a semi transparent black mask
        // this means if the user provides a custom background it will fade
        painter->fillRect(boundingRect(), FADE_MASK);
    }
}

/**
   Render a soft outline around the edge of the TableZone.

   @param painter QPainter object
 */
void TableZone::paintZoneOutline(QPainter *painter)
{
    QLinearGradient grad1(0, 0, 0, 1);
    grad1.setCoordinateMode(QGradient::ObjectBoundingMode);
    grad1.setColorAt(0, GRADIENT_COLOR);
    grad1.setColorAt(1, GRADIENT_COLORLESS);
    painter->fillRect(QRectF(0, 0, width, BOX_LINE_WIDTH), QBrush(grad1));

    grad1.setFinalStop(1, 0);
    painter->fillRect(QRectF(0, 0, BOX_LINE_WIDTH, height), QBrush(grad1));

    grad1.setStart(0, 1);
    grad1.setFinalStop(0, 0);
    painter->fillRect(QRectF(0, height - BOX_LINE_WIDTH, width, BOX_LINE_WIDTH), QBrush(grad1));

    grad1.setStart(1, 0);
    painter->fillRect(QRectF(width - BOX_LINE_WIDTH, 0, BOX_LINE_WIDTH, height), QBrush(grad1));
}

void TableZone::addCardImpl(CardItem *card, int _x, int _y)
{
    cards.append(card);
    card->setGridPoint(QPoint(_x, _y));

    card->setParentItem(this);
    card->setVisible(true);
    card->update();
}

void TableZone::handleDropEvent(const QList<CardDragItem *> &dragItems, CardZone *startZone, const QPoint &dropPoint)
{
    handleDropEventByGrid(dragItems, startZone, mapToGrid(dropPoint));
}

void TableZone::handleDropEventByGrid(const QList<CardDragItem *> &dragItems,
                                      CardZone *startZone,
                                      const QPoint &gridPoint)
{
    // if a card already present - attach to it facedown
    QPoint basePoint = gridPoint;
    basePoint.setX((basePoint.x() / 3) * 3);
    CardItem *baseCard = getCardFromGrid(basePoint);

    Command_MoveCard cmd;
    cmd.set_start_player_id(startZone->getPlayer()->getId());
    cmd.set_start_zone(startZone->getName().toStdString());
    cmd.set_target_player_id(player->getId());
    cmd.set_target_zone(getName().toStdString());
    cmd.set_x(gridPoint.x());
    cmd.set_y(gridPoint.y());

    for (const auto &item : dragItems) {
        bool attach = baseCard && (baseCard->getId() != item->getId());
        CardToMove *ctm = cmd.mutable_cards_to_move()->add_card();
        ctm->set_card_id(item->getId());
        ctm->set_face_down(item->getFaceDown() || attach);
        if (startZone->getName() != name && !(item->getFaceDown() || attach)) {
            const auto &info = item->getItem()->getInfo();
            if (info) {
                ctm->set_pt(info->getPowTough().toStdString());
            }
        }
    }

    startZone->getPlayer()->sendGameCommand(cmd);
}

void TableZone::reorganizeCards()
{
    QSet<ArrowItem *> arrowsToUpdate;

    // Calculate card stack widths so mapping functions work properly
    computeCardStackWidths();

    for (int i = 0; i < cards.size(); ++i) {
        QPoint gridPoint = cards[i]->getGridPos();
        if (gridPoint.x() == -1)
            continue;

        int gridPointY = gridPoint.y();

        QPointF mapPoint = mapFromGrid(gridPoint);
        qreal x = mapPoint.x();
        qreal y = mapPoint.y();

        int numberAttachedCards = cards[i]->getAttachedCards().size();
        // second row is too close to clock zone
        if (numberAttachedCards)
            y += gridPointY * 10 * (isInverted() ? 1 : -1);
        qreal actualX = x + numberAttachedCards * STACKED_CARD_OFFSET_X;
        qreal actualY = y;

        qreal baseCardYOffset = isInverted() ? 0 : 10;
        qreal attachedCardsOffset = isInverted() ? -10 : 0;
        if (numberAttachedCards)
            actualY += baseCardYOffset;

        cards[i]->setPos(actualX, actualY);
        cards[i]->setRealZValue((actualY + CARD_HEIGHT) * 100000 + (actualX + 1) * 100);

        QListIterator<CardItem *> attachedCardIterator(cards[i]->getAttachedCards());
        int j = 0;
        while (attachedCardIterator.hasNext()) {
            ++j;
            CardItem *attachedCard = attachedCardIterator.next();
            qreal childX = actualX - j * STACKED_CARD_OFFSET_X;
            qreal childY = y + attachedCardsOffset;
            attachedCard->setPos(childX, childY);
            attachedCard->setRealZValue((childY + CARD_HEIGHT) * 100000 + (childX + 1) * 100);
            for (ArrowItem *item : attachedCard->getArrowsFrom()) {
                arrowsToUpdate.insert(item);
            }
            for (ArrowItem *item : attachedCard->getArrowsTo()) {
                arrowsToUpdate.insert(item);
            }
        }

        for (ArrowItem *item : cards[i]->getArrowsFrom()) {
            arrowsToUpdate.insert(item);
        }
        for (ArrowItem *item : cards[i]->getArrowsTo()) {
            arrowsToUpdate.insert(item);
        }
    }
    for (ArrowItem *item : arrowsToUpdate) {
        item->updatePath();
    }

    resizeToContents();
    update();
}

void TableZone::toggleTapped()
{
    QList<QGraphicsItem *> selectedItems = scene()->selectedItems();
    CardItem::TapState srcState = CardItem::Reversed;
    for (int i = 0; i < selectedItems.size(); i++) {
        CardItem::TapState st = qgraphicsitem_cast<CardItem *>(selectedItems[i])->getTapped();
        if (st == CardItem::Standing) {
            srcState = CardItem::Standing;
            break;
        }
        if (st == CardItem::Tapped)
            srcState = CardItem::Tapped;
    }
    QList<const ::google::protobuf::Message *> cmdList;
    for (int i = 0; i < selectedItems.size(); i++) {
        CardItem *temp = qgraphicsitem_cast<CardItem *>(selectedItems[i]);
        if (temp->getTapped() == srcState) {
            Command_SetCardAttr *cmd = new Command_SetCardAttr;
            cmd->set_zone(name.toStdString());
            cmd->set_card_id(temp->getId());
            cmd->set_attribute(AttrTapped);
            cmd->set_attr_value(std::to_string(temp->nextTapState(srcState)));

            bool noLog = false;
            if (i == 0)
                noLog = player->attackOnTap(cmdList, temp);

            if (noLog)
                cmd->set_no_log(true);
            cmdList.append(cmd);
        }
    }
    player->sendGameCommand(player->prepareGameCommand(cmdList));
}

void TableZone::actTapHovered()
{
    for (int i = 0; i < cards.size(); i++) {
        if (cards[i]->getHovered()) {
            QList<const ::google::protobuf::Message *> cmdList;
            auto *cmd = new Command_SetCardAttr;
            cmd->set_zone(name.toStdString());
            cmd->set_card_id(cards[i]->getId());
            cmd->set_attribute(AttrTapped);
            cmd->set_attr_value(std::to_string(cards[i]->nextTapState()));

            if (player->attackOnTap(cmdList, cards[i]))
                cmd->set_no_log(true);

            cmdList.append(cmd);

            player->sendGameCommand(player->prepareGameCommand(cmdList));
            break;
        }
    }
}

void TableZone::actFlipHovered()
{
    for (int i = 0; i < cards.size(); i++) {
        if (cards[i]->getHovered()) {
            QList<const ::google::protobuf::Message *> cmdList;
            auto *cmd = new Command_FlipCard;
            cmd->set_zone(name.toStdString());
            cmd->set_card_id(cards[i]->getId());
            cmd->set_face_down(!cards[i]->getFaceDown());
            if (cards[i]->getFaceDown()) {
                CardInfoPtr ci = cards[i]->getInfo();
                if (ci) {
                    cmd->set_pt(ci->getPowTough().toStdString());
                }
            }
            cmdList.append(cmd);
            player->sendGameCommand(player->prepareGameCommand(cmdList));
            break;
        }
    }
}

CardItem *TableZone::takeCard(int position, int cardId, bool canResize)
{
    CardItem *result = CardZone::takeCard(position, cardId);
    if (canResize)
        resizeToContents();
    player->cardLeftTable(result);
    return result;
}

void TableZone::resizeToContents()
{
    int xMax = 0;

    // Find rightmost card position, which includes the left margin amount.
    for (int i = 0; i < cards.size(); ++i)
        if (cards[i]->pos().x() > xMax)
            xMax = (int)cards[i]->pos().x();

    // Minimum width is the rightmost card position plus enough room for
    // another card with padding, then margin.
    currentMinimumWidth = xMax + (2 * CARD_WIDTH) + PADDING_X + MARGIN_RIGHT;

    if (currentMinimumWidth < MIN_WIDTH)
        currentMinimumWidth = MIN_WIDTH;

    if (currentMinimumWidth != width) {
        prepareGeometryChange();
        width = currentMinimumWidth;
        emit sizeChanged();
    }
}

CardItem *TableZone::getCardFromGrid(const QPoint &gridPoint) const
{
    for (int i = 0; i < cards.size(); i++)
        if (cards.at(i)->getGridPoint() == gridPoint)
            return cards.at(i);
    return nullptr;
}

CardItem *TableZone::getCardFromCoords(const QPointF &point) const
{
    QPoint gridPoint = mapToGrid(point);
    return getCardFromGrid(gridPoint);
}

void TableZone::computeCardStackWidths()
{
    // Each card stack is three grid points worth of card locations.
    // First pass: compute the number of cards at each card stack.
    QMap<int, int> cardStackCount;
    for (int i = 0; i < cards.size(); ++i) {
        const QPoint &gridPoint = cards[i]->getGridPos();
        if (gridPoint.x() == -1)
            continue;

        const int key = getCardStackMapKey(gridPoint.x() / 3, gridPoint.y());
        cardStackCount.insert(key, cardStackCount.value(key, 0) + 1);
    }

    // Second pass: compute the width at each card stack.
    cardStackWidth.clear();
    for (int i = 0; i < cards.size(); ++i) {
        const QPoint &gridPoint = cards[i]->getGridPos();
        if (gridPoint.x() == -1)
            continue;

        const int key = getCardStackMapKey(gridPoint.x() / 3, gridPoint.y());
        const int stackCount = cardStackCount.value(key, 0);
        if (stackCount == 1)
            cardStackWidth.insert(key, CARD_WIDTH + cards[i]->getAttachedCards().size() * STACKED_CARD_OFFSET_X);
        else
            cardStackWidth.insert(key, CARD_WIDTH + (stackCount - 1) * STACKED_CARD_OFFSET_X);
    }
}

QPointF TableZone::mapFromGrid(QPoint gridPoint) const
{
    qreal x, y;

    qreal marginTop = isInverted() ? MARGIN_BOTTOM : MARGIN_TOP;

    // Start with margin plus stacked card offset
    x = MARGIN_LEFT + BACKROW_LEFT_MARGIN * gridPoint.y() + (gridPoint.x() % 3) * STACKED_CARD_OFFSET_X;

    // Add in width of card stack plus padding for each column
    for (int i = 0; i < gridPoint.x() / 3; ++i) {
        const int key = getCardStackMapKey(i, gridPoint.y());
        x += cardStackWidth.value(key, CARD_WIDTH) + PADDING_X;
    }

    if (isInverted())
        gridPoint.setY(TABLEROWS - 1 - gridPoint.y());

    // Start with margin plus stacked card offset
    y = marginTop + (gridPoint.x() % 3) * STACKED_CARD_OFFSET_Y;

    // Add in card size and padding for each row
    for (int i = 0; i < gridPoint.y(); ++i)
        y += CARD_HEIGHT + PADDING_Y;

    return QPointF(x, y);
}

QPoint TableZone::mapToGrid(const QPointF &mapPoint) const
{
    // Begin by calculating the y-coordinate of the grid space, which will be
    // used for the x-coordinate.

    // Offset point by the margin amount to reference point within grid area.
    int y = mapPoint.y() - (isInverted() ? MARGIN_BOTTOM : MARGIN_TOP);

    // Below calculation effectively rounds to the nearest grid point.
    const int gridPointHeight = CARD_HEIGHT + PADDING_Y;
    int gridPointY = (y + gridPointHeight / 2) / gridPointHeight;

    gridPointY = clampValidTableRow(gridPointY);

    if (isInverted())
        gridPointY = TABLEROWS - 1 - gridPointY;

    // Calculating the x-coordinate of the grid space requires adding up the
    // widths of each card stack along the row.

    // Offset point by the margin amount to reference point within grid area.
    int x = mapPoint.x() - MARGIN_LEFT - BACKROW_LEFT_MARGIN * gridPointY;

    // Maximum value is a card width from the right margin, referenced to the
    // grid area.
    const int xMax = width - MARGIN_LEFT - BACKROW_LEFT_MARGIN * gridPointY - MARGIN_RIGHT - CARD_WIDTH;

    int xStack = 0;
    int xNextStack = 0;
    int nextStackCol = 0;
    while ((xNextStack <= x) && (xNextStack <= xMax)) {
        xStack = xNextStack;
        const int key = getCardStackMapKey(nextStackCol, gridPointY);
        xNextStack += cardStackWidth.value(key, CARD_WIDTH) + PADDING_X;
        nextStackCol++;
    }
    int stackCol = qMax(nextStackCol - 1, 0);

    // Have the stack column, need to refine to the grid column.  Take the
    // difference between the point and the stack point and divide by stacked
    // card offsets.
    int xDiff = x - xStack;
    int gridPointX = stackCol * 3 + qMin(xDiff / STACKED_CARD_OFFSET_X, 2);

    return QPoint(gridPointX, gridPointY);
}

QPointF TableZone::closestGridPoint(const QPointF &point)
{
    QPoint gridPoint = mapToGrid(point + QPoint(1, 1));
    gridPoint.setX((gridPoint.x() / 3) * 3);
    if (getCardFromGrid(gridPoint))
        gridPoint.setX(gridPoint.x() + 1);
    if (getCardFromGrid(gridPoint))
        gridPoint.setX(gridPoint.x() + 1);
    return mapFromGrid(gridPoint);
}

int TableZone::clampValidTableRow(const int row)
{
    if (row < 0)
        return 0;
    if (row >= TABLEROWS)
        return TABLEROWS - 1;
    return row;
}
