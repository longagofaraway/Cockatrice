#pragma once

#include <QDialog>

class DeckLoader;
class QLineEdit;
class QLabel;
class QNetworkAccessManager;

class DlgLoadDeckFromEncoreDecks : public QDialog
{
    Q_OBJECT
private slots:
    void actOK();
    void actDownloadFinished();

private:
    DeckLoader *deckList;
    QLineEdit *urlEdit;
    QLabel *urlLabel;
    QNetworkAccessManager *networkManager;

public:
    explicit DlgLoadDeckFromEncoreDecks(QWidget *parent = nullptr);
    DeckLoader *getDeckList() const
    {
        return deckList;
    }
};
