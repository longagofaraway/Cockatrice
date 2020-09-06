#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include "carditem.h"
#include "chatview/chatview.h"
#include "translation.h"
#include "user_level.h"

class Player;
class CardZone;
class GameEventContext;

class MessageLogWidget : public ChatView
{
    Q_OBJECT
private:
    enum MessageContext
    {
        MessageContext_None,
        MessageContext_MoveCard,
        MessageContext_Mulligan
    };

    int mulliganNumber;
    Player *mulliganPlayer;
    MessageContext currentContext;
    QString messagePrefix, messageSuffix;

    const QString &tableConstant() const;
    const QString &graveyardConstant() const;
    const QString &exileConstant() const;
    const QString &handConstant() const;
    const QString &deckConstant() const;
    const QString &sideboardConstant() const;
    const QString &stackConstant() const;
    const QString &stockConstant() const;
    const QString &clockConstant() const;
    const QString &levelConstant() const;
    const QString &climaxConstant() const;

    QString sanitizeHtml(QString dirty) const;
    QString cardLink(QString cardName, QString cardCode) const;
    QPair<QString, QString>
    getFromStr(CardZone *zone, QString cardName, int position, bool ownerChange, CardZone *targetZone = nullptr) const;

public slots:
    void containerProcessingDone();
    void containerProcessingStarted(const GameEventContext &context);
    void logAlwaysRevealTopCard(Player *player, CardZone *zone, bool reveal);
    void logAttachCard(Player *player,
                       CardItem *card,
                       CardZone *startZone,
                       int oldX,
                       CardItem *targetCard,
                       CardZone *targetZone);
    void logConcede(Player *player);
    void logUnconcede(Player *player);
    void logConnectionStateChanged(Player *player, bool connectionState);
    void logCreateArrow(Player *player,
                        Player *startPlayer,
                        CardItem *startCard,
                        Player *targetPlayer,
                        CardItem *targetCard,
                        bool playerTarget);
    void logCreateToken(Player *player, QString cardName, QString pt);
    void logDeckSelect(Player *player, QString deckHash, int sideboardSize);
    void logDestroyCard(Player *player, CardItem *card);
    void logDrawCards(Player *player, int number);
    void logDumpZone(Player *player, CardZone *zone, int numberCards);
    void logFlipCard(Player *player, QString cardName, bool faceDown);
    void logGameClosed();
    void logGameStart();
    void logJoin(Player *player);
    void logJoinSpectator(QString name);
    void logKicked();
    void logLeave(Player *player, QString reason);
    void logLeaveSpectator(QString name, QString reason);
    void logNotReadyStart(Player *player);
    void logMoveCard(Player *player, CardItem *card, CardZone *startZone, int oldX, CardZone *targetZone, int newX);
    void logMulligan(Player *player, int number);
    void logReplayStarted(int gameId);
    void logReadyStart(Player *player);
    void logRevealCards(Player *player,
                        CardZone *zone,
                        int cardId,
                        CardItem *card,
                        Player *otherPlayer,
                        bool faceDown,
                        int amount);
    void logReverseTurn(Player *player, bool reversed);
    void logRollDie(Player *player, int sides, int roll);
    void logSay(Player *player, QString message);
    void logSetActivePhase(int phase);
    void logSetActivePlayer(Player *player);
    void logSetAnnotation(Player *player, CardItem *card, QString newAnnotation);
    void logSetCardCounter(Player *player, CardItem *card, int counterId, int value, int oldValue);
    void logSetCounter(Player *player, QString counterName, int value, int oldValue);
    void logSetDoesntUntap(Player *player, CardItem *card, bool doesntUntap);
    void logSetPT(Player *player, CardItem *card, QString newPT);
    void logSetSideboardLock(Player *player, bool locked);
    void logSetTapped(Player *player, CardItem *card, int tapped);
    void logShuffle(Player *player, CardZone *zone, int start, int end);
    void
    logSpectatorSay(QString spectatorName, UserLevelFlags spectatorUserLevel, QString userPrivLevel, QString message);
    void logStopDumpZone(Player *player, CardZone *zone);
    void logUnattachCard(Player *player, CardItem *card);
    void logUndoDraw(Player *player, QString cardName);
    void logRefresh(Player *player, int climaxCount);
    void logSetAttackState(Player *player, CardItem *card, CardItem::AttackState state);
    void logTrigger(Player *player, CardItem *card);
    void logTreasure(Player *player);
    void logBounce(Player *player);
    void setContextJudgeName(QString player);
    void appendHtmlServerMessage(const QString &html,
                                 bool optionalIsBold = false,
                                 QString optionalFontColor = QString()) override;

public:
    void connectToPlayer(Player *player);
    MessageLogWidget(const TabSupervisor *_tabSupervisor,
                     const UserlistProxy *_userlistProxy,
                     TabGame *_game,
                     QWidget *parent = nullptr);
};

#endif
