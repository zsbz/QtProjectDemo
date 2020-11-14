#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

namespace Ui
{
    class MainWidget;
}

// 枚举变量标志信息的类型
enum MessageType
{
    Message = 0,      // 消息
    NewParticipant,   // 新用户加入
    ParticipantLeft,  // 用户退出
    FileName,         // 文件名
    Refuse            // 拒绝接受文件
};

class TcpServerWidget;
class QUdpSocket;
class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget();

protected:
    // 处理新用户加入
    void newParticipant(QString userName, QString localHostName, QString ipAddress);
    // 处理用户离开
    void participantLeft(QString userName, QString localHostName, QString time);
    // 使用UDP广播发送信息
    void sendMessage(MessageType type, QString serverAddress = "");

    QString getIP();
    QString getUserName();
    // 获得要发送的消息
    QString getMessage();
    // 是否接收文件
    void hasPendingFile(QString userName, QString serverAddress, QString clientAddress, QString fileName);
    // 保存聊天记录
    bool saveFile(const QString& fileName);

    void closeEvent(QCloseEvent*);

private slots:
    // 接收UDP信息
    void slotProcessPendingDatagrams();
    void slotOnSendBtnClicked();

    void slotGetFileName(QString fileName);
    void slotSendFileBtnClicked();

private:
    Ui::MainWidget* ui;

    QUdpSocket* udpSocket;
    qint16 port;

    QString fileName;
    TcpServerWidget* server;

    QColor color;
};

#endif  // MAINWIDGET_H
