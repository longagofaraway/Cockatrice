#include "loaddbpage.h"

#include "oraclewizard.h"
#include "settingscache.h"

#include <QBuffer>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkReply>
#include <QSaveFile>

#ifdef HAS_LZMA
#include "lzma/decompress.h"
#endif

#ifdef HAS_ZLIB
#include "zip/unzip.h"
#endif

#define ZIP_SIGNATURE "PK"
// Xz stream header: 0xFD + "7zXZ"
#define XZ_SIGNATURE "\xFD\x37\x7A\x58\x5A"

namespace
{
constexpr const char *gDefaultDbUrl = "https://boardhunt.ru/Weiss_Schwarz_DBv4.zip";
}

LoadDbPage::LoadDbPage(QWidget *parent) : OracleWizardPage(parent)
{
    jobDone = false;
    urlLineEdit = new QLineEdit(this);
    fileLineEdit = new QLineEdit(this);

    urlRadioButton = new QRadioButton(this);
    fileRadioButton = new QRadioButton(this);

    urlButton = new QPushButton(this);
    connect(urlButton, SIGNAL(clicked()), this, SLOT(actRestoreDefaultUrl()));

    fileButton = new QPushButton(this);
    connect(fileButton, SIGNAL(clicked()), this, SLOT(actLoadSetsFile()));

    progressLabel = new QLabel(this);
    progressBar = new QProgressBar(this);

    urlRadioButton->setChecked(true);

    auto *layout = new QGridLayout(this);
    layout->addWidget(urlRadioButton, 0, 0);
    layout->addWidget(urlLineEdit, 0, 1);
    layout->addWidget(urlButton, 1, 1, Qt::AlignRight);
    layout->addWidget(fileRadioButton, 2, 0);
    layout->addWidget(fileLineEdit, 2, 1);
    layout->addWidget(fileButton, 3, 1, Qt::AlignRight);
    layout->addWidget(progressLabel, 4, 0);
    layout->addWidget(progressBar, 4, 1);

    setLayout(layout);
}

void LoadDbPage::initializePage()
{
    urlLineEdit->setText(gDefaultDbUrl);

    progressLabel->hide();
    progressBar->hide();
}

void LoadDbPage::retranslateUi()
{
    setTitle(tr("Source selection"));
    setSubTitle(tr("Please specify a compatible source for the list of sets and cards. "
                   "You can specify a URL address that will be downloaded or "
                   "use an existing file from your computer."));

    urlRadioButton->setText(tr("Download URL:"));
    fileRadioButton->setText(tr("Local file:"));
    urlButton->setText(tr("Restore default URL"));
    fileButton->setText(tr("Choose file..."));
}

void LoadDbPage::actRestoreDefaultUrl()
{
    urlLineEdit->setText(gDefaultDbUrl);
}

void LoadDbPage::actLoadSetsFile()
{
    QFileDialog dialog(this, tr("Load sets file"));
    dialog.setFileMode(QFileDialog::ExistingFile);

    QString extensions = "*.xml";
    dialog.setNameFilter(tr("Sets XML file (%1)").arg(extensions));

    if (!fileLineEdit->text().isEmpty() && QFile::exists(fileLineEdit->text())) {
        dialog.selectFile(fileLineEdit->text());
    }

    if (!dialog.exec()) {
        return;
    }

    fileLineEdit->setText(dialog.selectedFiles().at(0));

    fileRadioButton->setChecked(true);
}

void LoadDbPage::downloadFile(QUrl url)
{
    if (url.host() == "yadi.sk") {
        QString proxyUrl =
            "https://cloud-api.yandex.net/v1/disk/public/resources/download?public_key=" + url.toString();
        QUrl yurl(proxyUrl);
        QNetworkReply *yreply = wizard()->nam->get(QNetworkRequest(proxyUrl));
        connect(yreply, &QNetworkReply::finished, [this, yreply]() {
            if (yreply->error() == QNetworkReply::NoError) {
                QByteArray jsonData = yreply->readAll();
                QJsonParseError jsonError;
                QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonData, &jsonError);

                if (jsonError.error == QJsonParseError::NoError) {
                    QVariantMap jsonMap = jsonResponse.toVariant().toMap();
                    QString newUrlStr = jsonMap["href"].toString();
                    if (newUrlStr.isEmpty()) {
                        QMessageBox::critical(this, tr("Error"), tr("Some network error."));
                    }

                    QUrl newUrl(newUrlStr);
                    QNetworkReply *reply = wizard()->nam->get(QNetworkRequest(newUrl));
                    connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinished()));
                    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this,
                            SLOT(actDownloadProgress(qint64, qint64)));
                }
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(yreply->errorString()));

                wizard()->enableButtons();
                setEnabled(true);
            }

            yreply->deleteLater();
        });
    } else {
        QNetworkReply *reply = wizard()->nam->get(QNetworkRequest(url));

        connect(reply, SIGNAL(finished()), this, SLOT(actDownloadFinished()));
        connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(actDownloadProgress(qint64, qint64)));
    }
}

void LoadDbPage::actDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        progressBar->setMaximum(static_cast<int>(total));
        progressBar->setValue(static_cast<int>(received));
    }
    progressLabel->setText(tr("Downloading (%1MB)").arg((int)received / (1024 * 1024)));
}

void LoadDbPage::actDownloadFinished()
{
    auto *reply = dynamic_cast<QNetworkReply *>(sender());
    QNetworkReply::NetworkError errorCode = reply->error();
    if (errorCode != QNetworkReply::NoError) {
        QMessageBox::critical(this, tr("Error"), tr("Network error: %1.").arg(reply->errorString()));

        wizard()->enableButtons();
        setEnabled(true);

        reply->deleteLater();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode == 301 || statusCode == 302) {
        QString sss = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        qDebug() << "following redirect url:" << redirectUrl.toString();
        downloadFile(redirectUrl);
        reply->deleteLater();
        return;
    }

    progressLabel->hide();
    progressBar->hide();

    wizard()->enableButtons();
    setEnabled(true);

    bool res = saveToFile(reply->readAll());
    reply->deleteLater();
    if (!res)
        return;

    jobDone = true;
    wizard()->next();
}

bool LoadDbPage::saveToFile(QByteArray data)
{
    // unzip the file if needed
    if (data.startsWith(XZ_SIGNATURE)) {
#ifdef HAS_LZMA
        // zipped file
        auto *inBuffer = new QBuffer(&data);
        auto *outBuffer = new QBuffer(this);
        inBuffer->open(QBuffer::ReadOnly);
        outBuffer->open(QBuffer::WriteOnly);
        XzDecompressor xz;
        if (!xz.decompress(inBuffer, outBuffer)) {
            QMessageBox::critical(this, tr("Error"), tr("Xz extraction failed."));
            return false;
        }

        data = outBuffer->data();
#else
        QMessageBox::critical(this, tr("Error"),
                              tr("Sorry, this version of Oracle does not support xz compressed files."));
        return false;
#endif
    } else if (data.startsWith(ZIP_SIGNATURE)) {
#ifdef HAS_ZLIB
        // zipped file
        auto *inBuffer = new QBuffer(&data);
        auto *outBuffer = new QBuffer(this);
        QString fileName;
        UnZip::ErrorCode ec;
        UnZip uz;

        ec = uz.openArchive(inBuffer);
        if (ec != UnZip::Ok) {
            QMessageBox::critical(this, tr("Error"), tr("Failed to open Zip archive: %1.").arg(uz.formatError(ec)));
            return false;
        }

        if (uz.fileList().size() != 1) {
            QMessageBox::critical(this, tr("Error"),
                                  tr("Zip extraction failed: the Zip archive doesn't contain exactly one file."));
            return false;
        }
        fileName = uz.fileList().at(0);

        outBuffer->open(QBuffer::ReadWrite);
        ec = uz.extractFile(fileName, outBuffer);
        if (ec != UnZip::Ok) {
            QMessageBox::critical(this, tr("Error"), tr("Zip extraction failed: %1.").arg(uz.formatError(ec)));
            uz.closeArchive();
            return false;
        }

        data = outBuffer->data();
#else
        QMessageBox::critical(this, tr("Error"), tr("Sorry, this version of Oracle does not support zipped files."));
        return false;
#endif
    }

    QString defaultPath = SettingsCache::instance().getCardDatabasePath();
    QSaveFile file(defaultPath);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (file.write(data) != data.size())
        return false;

    return file.commit();
}

bool LoadDbPage::validatePage()
{
    if (jobDone)
        return true;

    if (urlRadioButton->isChecked()) {
        auto dd = urlLineEdit->text();
        QUrl url = QUrl::fromUserInput(urlLineEdit->text());
        if (!url.isValid()) {
            QMessageBox::critical(this, tr("Error"), tr("The provided URL is not valid."));
            return false;
        }

        progressLabel->setText(tr("Downloading (0MB)"));
        // show an infinite progressbar
        progressBar->setMaximum(0);
        progressBar->setMinimum(0);
        progressBar->setValue(0);
        progressLabel->show();
        progressBar->show();

        wizard()->disableButtons();
        setEnabled(false);

        downloadFile(url);
    } else if (fileRadioButton->isChecked()) {
        QFile setsFile(fileLineEdit->text());
        if (!setsFile.exists()) {
            QMessageBox::critical(this, tr("Error"), tr("Please choose a file."));
            return false;
        }

        if (!setsFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(nullptr, tr("Error"), tr("Cannot open file '%1'.").arg(fileLineEdit->text()));
            return false;
        }

        if (!saveToFile(setsFile.readAll()))
            return false;

        jobDone = true;
        wizard()->next();
        return true;
    }

    return false;
}
