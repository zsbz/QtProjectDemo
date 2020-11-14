#ifndef TCPCLIENTWIDGET_H
#define TCPCLIENTWIDGET_H

#include <QDialog>
#include <QFile>
#include <QHostAddress>
#include <QTime>

namespace Ui
{
    class TcpClientWidget;
}

class QTcpSocket;
class TcpClientWidget : public QDialog
{
    Q_OBJECT

public:
    explicit TcpClientWidget(QWidget* parent = nullptr);
    ~TcpClientWidget();

    void setHostAddress(QHostAddress address);
    void setFileName(QString fileName);

protected:
    void closeEvent(QCloseEvent*);

private slots:
    void slotTcpClientCancleBtnClicked();
    void slotTcpClientCloseBtnClicked();

    void slotNewConnect();
    void slotReadMessage();
    void slotDisplayError(QAbstractSocket::SocketError);

private:
    Ui::TcpClientWidget* ui;

    QTcpSocket* tcpClient;
    quint16 blockSize;
    QHostAddress hostAddress;
    qint16 tcpPort;

    qint64 TotalBytes;
    qint64 bytesReceived;
    qint64 bytesToReceive;
    qint64 fileNameSize;
    QString fileName;
    QFile* localFile;
    QByteArray inBlock;

    QTime time;
};

#endif  // TCPCLIENTWIDGET_H
