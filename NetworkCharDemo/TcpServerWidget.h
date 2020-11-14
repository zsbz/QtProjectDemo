#ifndef TCPSERVERWIDGET_H
#define TCPSERVERWIDGET_H

#include <QDialog>
#include <QTime>

namespace Ui
{
    class TcpServerWidget;
}

class QFile;
class QTcpServer;
class QTcpSocket;

class TcpServerWidget : public QDialog
{
    Q_OBJECT

public:
    explicit TcpServerWidget(QWidget* parent = nullptr);
    ~TcpServerWidget();

    void initServer();
    void refused();

protected:
    void closeEvent(QCloseEvent*);

signals:
    void signalSendFileName(QString fileName);

private slots:
    void slotSendMessage();
    void slotUpdateClientProgress(qint64 numBytes);

    void slotServerOpenBtnClicked();

    void slotServerSendBtnClicked();

    void slotServerCloseBtnClicked();

private:
    Ui::TcpServerWidget* ui;

    qint16 tcpPort;
    QTcpServer* tcpServer;
    QString fileName;
    QString theFileName;
    QFile* localFile;

    qint64 TotalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 payloadSize;
    QByteArray outBlock;

    QTcpSocket* clientConnection;

    QTime time;
};

#endif  // TCPSERVERWIDGET_H
