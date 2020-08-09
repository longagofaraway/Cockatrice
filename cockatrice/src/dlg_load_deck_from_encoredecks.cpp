#include "dlg_load_deck_from_encoredecks.h"

#include "deck_loader.h"
#include "settingscache.h"

#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>

DlgLoadDeckFromEncoreDecks::DlgLoadDeckFromEncoreDecks(QWidget *parent) : QDialog(parent), deckList(nullptr)
{
    urlEdit = new QLineEdit(this);
    urlLabel = new QLabel(this);
    urlLabel->setText("Enter URL: ");

    networkManager = new QNetworkAccessManager(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(actOK()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(urlLabel);
    mainLayout->addWidget(urlEdit);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Load deck from Encore Decks"));
    resize(350, 95);
}

void DlgLoadDeckFromEncoreDecks::actOK()
{
    QString deckUrl = urlEdit->text();
    int pos = deckUrl.lastIndexOf("/");
    if (pos == -1) {
        QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
        return;
    }

    QString deckId = deckUrl.mid(pos + 1);
    QString encoreDecksDeckApiUrl = "https://www.encoredecks.com/api/deck/" + deckId;
    QUrl url = QUrl::fromUserInput(encoreDecksDeckApiUrl);
    if (!url.isValid()) {
        QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
        return;
    }

    QNetworkReply *reply = networkManager->get(QNetworkRequest(url));

    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinished()));
}

namespace
{
QString parseSerial(QJsonObject jsonCard)
{
    if (!(jsonCard.contains("side") && jsonCard.contains("set") && jsonCard.contains("sid") &&
          jsonCard.contains("release")))
        return "";

    QString sid = jsonCard["sid"].toString();
    if (sid.startsWith("E", Qt::CaseInsensitive))
        sid = sid.mid(1);
    if (sid.size() >= 2 && (sid[1] == "e" || sid[1] == "E"))
        sid = sid.remove(1, 1);
    return jsonCard["set"].toString() + "/" + jsonCard["side"].toString() + jsonCard["release"].toString() + "-" + sid;
}

QString mapToPlain(QHash<QString, int> &map)
{
    QString plain;
    for (auto it = map.begin(); it != map.end(); ++it)
        plain += QString::number(it.value()) + " " + it.key() + "\n";

    return plain;
}
} // namespace

void DlgLoadDeckFromEncoreDecks::actDownloadFinished()
{
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();

    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Json parse error: %1.").arg(jsonError.errorString()));
        reply->deleteLater();
        return;
    }

    QJsonObject json = jsonResponse.object();
    if (!json.contains("name") || !json["name"].isString()) {
        QMessageBox::critical(this, tr("Error"), tr("Json parse error: no deck name field"));
        reply->deleteLater();
        return;
    }
    QString deckName = json["name"].toString();

    if (!json.contains("description") || !json["description"].isString()) {
        QMessageBox::critical(this, tr("Error"), tr("Json parse error: no description field"));
        reply->deleteLater();
        return;
    }
    QString description = json["description"].toString();
    description = description.replace("\n", "\n// ");

    if (!json.contains("cards") || !json["cards"].isArray()) {
        QMessageBox::critical(this, tr("Error"), tr("Json parse error: no cards field"));
        reply->deleteLater();
        return;
    }

    QHash<QString, int> map;
    QJsonArray jsonCards = json["cards"].toArray();
    for (int i = 0; i < jsonCards.size(); ++i) {
        map[parseSerial(jsonCards[i].toObject())]++;
    }

    reply->deleteLater();

    QString deckPlain = mapToPlain(map);
    QString deckAnnotated = "// " + deckName + "\n\n";
    deckAnnotated += "// " + description + "\n\n" + deckPlain;
    QTextStream stream(&deckAnnotated);

    auto *deckLoader = new DeckLoader;
    if (deckLoader->loadFromStream_Plain(stream)) {
        deckList = deckLoader;
        accept();
        return;
    }

    QMessageBox::critical(this, tr("Error"), tr("Deck parsing error."));
}
