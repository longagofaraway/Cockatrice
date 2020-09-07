#ifndef PLAYER_H
#define PLAYER_H

#include "abstractgraphicsitem.h"
#include "carddatabase.h"
#include "carditem.h"
#include "pb/card_attributes.pb.h"
#include "pb/game_event.pb.h"
#include "tearoffmenu.h"

#include <QInputDialog>
#include <QMap>
#include <QPoint>

namespace google
{
namespace protobuf
{
class Message;
}
} // namespace google
class CardDatabase;
class DeckLoader;
class QMenu;
class QAction;
class ZoneViewZone;
class TabGame;
class AbstractCounter;
class AbstractCardItem;
class CardItem;
class ArrowTarget;
class ArrowItem;
class CardZone;
class StackZone;
class StockZone;
class ClockZone;
class LevelZone;
class ClimaxZone;
class TableZone;
class HandZone;
class PlayerTarget;
class ServerInfo_User;
class ServerInfo_Player;
class ServerInfo_Arrow;
class ServerInfo_Counter;
class CommandContainer;
class GameCommand;
class GameEvent;
class GameEventContext;
// class Event_ConnectionStateChanged;
class Event_GameSay;
class Event_Shuffle;
class Event_RollDie;
class Event_CreateArrow;
class Event_DeleteArrow;
class Event_CreateToken;
class Event_SetCardAttr;
class Event_SetCardCounter;
class Event_CreateCounter;
class Event_SetCounter;
class Event_DelCounter;
class Event_DumpZone;
class Event_StopDumpZone;
class Event_MoveCard;
class Event_FlipCard;
class Event_DestroyCard;
class Event_AttachCard;
class Event_DrawCards;
class Event_RevealCards;
class Event_ChangeZoneProperties;
class Event_Refresh;
class PendingCommand;

const int MAX_TOKENS_PER_DIALOG = 99;

class PlayerArea : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
private:
    QRectF bRect;
private slots:
    void updateBg();

public:
    enum
    {
        Type = typeOther
    };
    int type() const override
    {
        return Type;
    }

    explicit PlayerArea(QGraphicsItem *parent = nullptr);
    QRectF boundingRect() const override
    {
        return bRect;
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setSize(qreal width, qreal height);
};

struct TakeDamageOp
{
    int damage;
    int damageTaken;
};

class Player : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
signals:
    void openDeckEditor(const DeckLoader *deck);
    void newCardAdded(AbstractCardItem *card);
    // Log events
    void logSay(Player *player, QString message);
    void logShuffle(Player *player, CardZone *zone, int start, int end);
    void logRollDie(Player *player, int sides, int roll);
    void logCreateArrow(Player *player,
                        Player *startPlayer,
                        CardItem *startCard,
                        Player *targetPlayer,
                        CardItem *targetCard,
                        bool _playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt);
    void logDrawCards(Player *player, int number);
    void logUndoDraw(Player *player, QString cardName);
    void logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logDestroyCard(Player *player, CardItem *card);
    void logAttachCard(Player *player,
                       CardItem *card,
                       CardZone *startZone,
                       int oldX,
                       CardItem *targetCard,
                       CardZone *targetZone);
    void logUnattachCard(Player *player, CardItem *card);
    void logSetCardCounter(Player *player, CardItem *card, int counterId, int value, int oldValue);
    void logSetTapped(Player *player, CardItem *card, int tapped);
    void logSetCounter(Player *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logDumpZone(Player *player, CardZone *zone, int numberCards);
    void logStopDumpZone(Player *player, CardZone *zone);
    void logRevealCards(Player *player,
                        CardZone *zone,
                        int cardId,
                        CardItem *card,
                        Player *otherPlayer,
                        bool faceDown,
                        int amount);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
    void logRefresh(Player *player, int climaxCount);
    void logSetAttackState(Player *player, CardItem *card, CardItem::AttackState state);
    void logTrigger(Player *player, CardItem *card);
    void logTreasure(Player *player);
    void logBounce(Player *player);
    void logTakeDamage(Player *player, int damage);
    void logCancelDamage(Player *player);

    void sizeChanged();
    void playerCountChanged();
public slots:
    void actUntapAll();
    void actRollDie();
    void actCreateToken();
    void actCreateAnotherToken();
    void actShuffle();
    void actRefresh();
    void actDrawCard();
    void actDrawCards();
    void actUndoDraw();
    void actMulligan();
    void actMoveTopCardToPlay();
    void actMoveTopCardToPlayFaceDown();
    void actMoveTopCardToGrave();
    void actMoveTopCardToExile();
    void actMoveTopCardsToGrave();
    void actMoveTopCardsToExile();
    void actMoveTopCardToBottom();
    void actMoveBottomCardToGrave();
    void actMoveTopStockToGrave();

    void actViewLibrary();
    void actViewHand();
    void actViewClock();
    void actViewStock();
    void actViewTopCards();
    void actAlwaysRevealTopCard();
    void actViewGraveyard();
    void actRevealRandomGraveyardCard();
    void actViewRfg();
    void actViewSideboard();
    void actViewNextTopCard();

    void actSayMessage();
private slots:
    void addPlayer(Player *player);
    void removePlayer(Player *player);
    void playerListActionTriggered();

    void updateBoundingRect();
    void rearrangeZones();

    void actOpenDeckInDeckEditor();
    void actCreatePredefinedToken();
    void actCreateRelatedCard();
    void actCreateAllRelatedCards();
    void cardMenuAction();
    void actMoveCardXCardsFromTop();
    void actCardCounterTrigger();
    void actAttach();
    void actUnattach();
    void actDrawArrow();
    void actIncPT(int deltaP, int deltaT);
    void actResetPT();
    void actSetPT();
    void actIncP();
    void actDecP();
    void actIncT();
    void actDecT();
    void actIncPT();
    void actDecPT();
    void actFlowP();
    void actFlowT();
    void actSetAnnotation();
    void actPlay();
    void actHide();
    void actPlayFacedown();
    void refreshShortcuts();

private:
    TabGame *game;
    QMenu *sbMenu, *countersMenu, *sayMenu, *createPredefinedTokenMenu, *mRevealLibrary, *mRevealTopCard, *mRevealHand,
        *mRevealRandomHandCard, *mRevealRandomGraveyardCard;
    TearOffMenu *moveGraveMenu, *moveRfgMenu, *graveMenu, *moveHandMenu, *handMenu, *libraryMenu, *rfgMenu, *playerMenu,
        *clockMenu, *stockMenu, *stackMenu;
    QList<QMenu *> playerLists;
    QList<QAction *> allPlayersActions;
    QAction *aMoveHandToTopLibrary, *aMoveHandToBottomLibrary, *aMoveHandToGrave, *aMoveHandToRfg,
        *aMoveGraveToTopLibrary, *aMoveGraveToBottomLibrary, *aMoveGraveToHand, *aMoveGraveToRfg, *aMoveRfgToTopLibrary,
        *aMoveRfgToBottomLibrary, *aMoveRfgToHand, *aMoveRfgToGrave, *aViewHand, *aViewLibrary, *aViewTopCards,
        *aAlwaysRevealTopCard, *aOpenDeckInDeckEditor, *aMoveTopCardToGraveyard, *aMoveTopCardToExile,
        *aMoveTopCardsToGraveyard, *aMoveTopCardsToExile, *aMoveTopCardToBottom, *aViewGraveyard, *aViewRfg,
        *aViewSideboard, *aDrawCard, *aDrawCards, *aUndoDraw, *aMulligan, *aShuffle, *aMoveTopToPlay,
        *aMoveTopToPlayFaceDown, *aUntapAll, *aRollDie, *aCreateToken, *aCreateAnotherToken, *aCardMenu,
        *aMoveBottomCardToGrave;
    QAction *aMoveClockToGrave, *aViewClock, *aViewStock, *aMoveToStock, *aMoveToClock, *aMoveToBottomClock,
        *aShuffleStock, *aMoveStackToStock, *aMoveStackToClock, *aMoveStackToGrave, *aTapHovered, *aRefresh,
        *aMoveTopStockToGraveyard, *aViewNextTopCard, *aFlipHovered, *aSideAttack;

    QList<QAction *> aAddCounter, aSetCounter, aRemoveCounter;
    QAction *aPlay, *aPlayFacedown, *aHide, *aTap, *aDoesntUntap, *aAttach, *aUnattach, *aDrawArrow, *aSetPT, *aResetPT,
        *aIncP, *aDecP, *aIncT, *aDecT, *aIncPT, *aDecPT, *aFlowP, *aFlowT, *aSetAnnotation, *aFlip, *aPeek, *aClone,
        *aMoveToTopLibrary, *aMoveToBottomLibrary, *aMoveToHand, *aMoveToGraveyard, *aMoveToExile,
        *aMoveToXfromTopOfLibrary;

    bool shortcutsActive;
    int defaultNumberTopCards;
    int defaultNumberTopCardsToPlaceBelow;
    QString lastTokenName, lastTokenColor, lastTokenPT, lastTokenAnnotation;
    bool lastTokenDestroy;
    int lastTokenTableRow;
    ServerInfo_User *userInfo;
    int id;
    bool active;
    bool local;
    bool judge;
    bool mirrored;
    bool handVisible;
    bool conceded;

    bool dialogSemaphore;
    bool clearCardsToDelete();
    QList<CardItem *> cardsToDelete;

    CardItem *attackingCard;
    TakeDamageOp damageOp;

    DeckLoader *deck;
    QStringList predefinedTokens;

    PlayerArea *playerArea;
    QMap<QString, CardZone *> zones;
    StackZone *stack;
    StockZone *stock;
    ClockZone *clock;
    LevelZone *level;
    ClimaxZone *climax;
    TableZone *table;
    HandZone *hand;
    PlayerTarget *playerTarget;

    void setCardAttrHelper(const GameEventContext &context,
                           CardItem *card,
                           CardAttribute attribute,
                           const QString &avalue,
                           bool allCards);
    void addRelatedCardActions(const CardItem *card, QMenu *cardMenu);
    void addRelatedCardView(const CardItem *card, QMenu *cardMenu);
    void createCard(const CardItem *sourceCard, const QString &dbCardName, bool attach = false);
    void createAttachedCard(const CardItem *sourceCard, const QString &dbCardName);
    bool createRelatedFromRelation(const CardItem *sourceCard, const CardRelation *cardRelation);

    QRectF bRect;

    QMap<int, AbstractCounter *> counters;
    QMap<int, ArrowItem *> arrows;
    void rearrangeCounters();

    void initSayMenu();

    // void eventConnectionStateChanged(const Event_ConnectionStateChanged &event);
    void eventGameSay(const Event_GameSay &event);
    void eventShuffle(const Event_Shuffle &event);
    void eventRollDie(const Event_RollDie &event);
    void eventCreateArrow(const Event_CreateArrow &event);
    void eventDeleteArrow(const Event_DeleteArrow &event);
    void eventCreateToken(const Event_CreateToken &event);
    void eventSetCardAttr(const Event_SetCardAttr &event, const GameEventContext &context);
    void eventSetCardCounter(const Event_SetCardCounter &event);
    void eventCreateCounter(const Event_CreateCounter &event);
    void eventSetCounter(const Event_SetCounter &event);
    void eventDelCounter(const Event_DelCounter &event);
    void eventDumpZone(const Event_DumpZone &event);
    void eventStopDumpZone(const Event_StopDumpZone &event);
    void eventMoveCard(const Event_MoveCard &event, const GameEventContext &context);
    void eventFlipCard(const Event_FlipCard &event);
    void eventDestroyCard(const Event_DestroyCard &event);
    void eventAttachCard(const Event_AttachCard &event);
    void eventDrawCards(const Event_DrawCards &event);
    void eventRefresh(const Event_Refresh &event);
    void eventRevealCards(const Event_RevealCards &event);
    void eventChangeZoneProperties(const Event_ChangeZoneProperties &event);

public:
    static const int counterAreaWidth = 55;
    enum CardMenuActionType
    {
        cmTap,
        cmUntap,
        cmDoesntUntap,
        cmFlip,
        cmPeek,
        cmSideAttack,
        cmClone,
        cmMoveToTopLibrary,
        cmMoveToBottomLibrary,
        cmMoveToHand,
        cmMoveToGraveyard,
        cmMoveToExile,
        cmMoveToStock,
        cmMoveToClock,
        cmMoveToBottomClock
    };
    enum CardsToReveal
    {
        RANDOM_CARD_FROM_ZONE = -2
    };

    enum
    {
        Type = typeOther
    };
    int type() const override
    {
        return Type;
    }
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void playCard(CardItem *c, bool faceDown, bool tapped);
    void addCard(CardItem *c);
    void deleteCard(CardItem *c);
    void addZone(CardZone *z);

    AbstractCounter *addCounter(const ServerInfo_Counter &counter);
    AbstractCounter *addCounter(int counterId, const QString &name, QColor color, int radius, int value);
    void delCounter(int counterId);
    void clearCounters();

    void incPTHovered(int deltaP, int deltaT);
    void incPTFrontRow(int power, int soul);
    void incCardPT(CardItem *card, int deltaP, int deltaS);

    ArrowItem *addArrow(const ServerInfo_Arrow &arrow);
    ArrowItem *addArrow(int arrowId, CardItem *startCard, ArrowTarget *targetItem, const QColor &color);
    void delArrow(int arrowId);
    void removeArrow(ArrowItem *arrow);
    void clearArrows();
    PlayerTarget *getPlayerTarget() const
    {
        return playerTarget;
    }

    Player(const ServerInfo_User &info, int _id, bool _local, bool _judge, TabGame *_parent);
    ~Player() override;

    void retranslateUi();
    void clear();
    TabGame *getGame() const
    {
        return game;
    }
    void setDeck(const DeckLoader &_deck);
    QMenu *getPlayerMenu() const
    {
        return playerMenu;
    }
    int getId() const
    {
        return id;
    }
    QString getName() const;
    ServerInfo_User *getUserInfo() const
    {
        return userInfo;
    }
    bool getLocal() const
    {
        return local;
    }
    bool getLocalOrJudge() const
    {
        return local || judge;
    }
    bool getJudge() const
    {
        return judge;
    }
    bool getMirrored() const
    {
        return mirrored;
    }
    const QMap<QString, CardZone *> &getZones() const
    {
        return zones;
    }
    const QMap<int, ArrowItem *> &getArrows() const
    {
        return arrows;
    }
    void setCardMenu(QMenu *menu);
    QMenu *getCardMenu() const;
    void updateCardMenu(const CardItem *card);
    bool getActive() const
    {
        return active;
    }
    void setActive(bool _active);
    void setShortcutsActive();
    void setShortcutsInactive();
    void updateZones();

    void setConceded(bool _conceded);
    bool getConceded() const
    {
        return conceded;
    }

    void setGameStarted();

    qreal getMinimumWidth() const;
    void setMirrored(bool _mirrored);
    void processSceneSizeChange(int newPlayerWidth);

    void processPlayerInfo(const ServerInfo_Player &info);
    void processCardAttachment(const ServerInfo_Player &info);

    void processGameEvent(GameEvent::GameEventType type, const GameEvent &event, const GameEventContext &context);

    PendingCommand *prepareGameCommand(const ::google::protobuf::Message &cmd);
    PendingCommand *prepareGameCommand(const QList<const ::google::protobuf::Message *> &cmdList);
    void sendGameCommand(PendingCommand *pend);
    void sendGameCommand(const google::protobuf::Message &command);

    void setLastToken(CardInfoPtr cardInfo);
    void processClimax(CardItem *card);
    void processTrigger(CardItem *card);
    void processOpponentTrigger(CardItem *card);
    void setCardAttackState(QList<const ::google::protobuf::Message *> &commandList,
                            CardItem *card,
                            CardItem::AttackState state);
    void attackOnTap(QList<const ::google::protobuf::Message *> &commandList, CardItem *card);
    void setAttackingCard(CardItem *card);
    CardItem *getAttackingCard()
    {
        return attackingCard;
    }
    void cardLeftTable(CardItem *card);
    void resetAttackingCard();
    void doTrigger();
    void takeDamage();
    void takeDamageInit(int _damage);
    void takeDamageUpdate(CardItem *card);
    void takeDamageCommand();
    int getIncomingDamage();
    void moveCard(CardItem *card, QString targetZone);
    void performBattle();

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) override;
};

#endif
