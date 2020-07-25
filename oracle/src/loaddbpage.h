#pragma once

#include "pagetemplates.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>

class LoadDbPage : public OracleWizardPage
{
    Q_OBJECT
public:
    explicit LoadDbPage(QWidget *parent = nullptr);
    void retranslateUi() override;

protected:
    bool validatePage() override;

private:
    void initializePage() override;
    bool saveToFile(QByteArray data);
    void downloadFile(QUrl url);

    QRadioButton *urlRadioButton;
    QRadioButton *fileRadioButton;
    QLineEdit *urlLineEdit;
    QLineEdit *fileLineEdit;
    QPushButton *urlButton;
    QPushButton *fileButton;
    QLabel *progressLabel;
    QProgressBar *progressBar;
    QSettings *settings;

    bool jobDone;

private slots:
    void actLoadSetsFile();
    void actRestoreDefaultUrl();
    void actDownloadProgress(qint64 received, qint64 total);
    void actDownloadFinished();
};