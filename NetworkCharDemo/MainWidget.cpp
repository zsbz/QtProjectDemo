#include "MainWidget.h"
#include "ui_MainWidget.h"

#include <QColorDialog>
#include <QDateTime>
#include <QFileDialog>
#include <QHostInfo>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QProcess>
#include <QScrollBar>
#include <QUdpSocket>

#include "TcpClientWidget.h"
#include "TcpServerWidget.h"

MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    udpSocket = new QUdpSocket(this);
    port = 45454;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWidget::slotProcessPendingDatagrams);

    sendMessage(NewParticipant);

    server = new TcpServerWidget(this);
    connect(server, &TcpServerWidget::signalSendFileName, this, &MainWidget::slotGetFileName);

    connect(ui->sendButton, &QPushButton::clicked, this, &MainWidget::slotOnSendBtnClicked);
    connect(ui->sendToolBtn, &QToolButton::clicked, this, &MainWidget::slotSendFileBtnClicked);
}

MainWidget::~MainWidget() { delete ui; }

void MainWidget::newParticipant(QString userName, QString localHostName, QString ipAddress)
{
    bool isEmpty = ui->userTableWidget->findItems(localHostName, Qt::MatchExactly).isEmpty();
    if (isEmpty)
    {
        QTableWidgetItem* user = new QTableWidgetItem(userName);
        QTableWidgetItem* host = new QTableWidgetItem(localHostName);
        QTableWidgetItem* ip = new QTableWidgetItem(ipAddress);

        ui->userTableWidget->insertRow(0);
        ui->userTableWidget->setItem(0, 0, user);
        ui->userTableWidget->setItem(0, 1, host);
        ui->userTableWidget->setItem(0, 2, ip);
        ui->messageBrowser->setTextColor(Qt::gray);
        ui->messageBrowser->setCurrentFont(QFont("Times New Roman", 10));
        ui->messageBrowser->append(tr("%1 在线！").arg(userName));
        ui->userNumLabel->setText(tr("在线人数：%1").arg(ui->userTableWidget->rowCount()));

        sendMessage(NewParticipant);
    }
}

void MainWidget::participantLeft(QString userName, QString localHostName, QString time)
{
    int rowNum = ui->userTableWidget->findItems(localHostName, Qt::MatchExactly).first()->row();
    ui->userTableWidget->removeRow(rowNum);
    ui->messageBrowser->setTextColor(Qt::gray);
    ui->messageBrowser->setCurrentFont(QFont("Times New Roman", 10));
    ui->messageBrowser->append(tr("%1 于 %2 离开！").arg(userName).arg(time));
    ui->userNumLabel->setText(tr("在线人数：%1").arg(ui->userTableWidget->rowCount()));
}

void MainWidget::sendMessage(MessageType type, QString serverAddress)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;

    switch (type)
    {
    case Message:
        if (ui->messageTextEdit->toPlainText() == "")
        {
            QMessageBox::warning(0, tr("警告"), tr("发送内容不能为空"), QMessageBox::Ok);
            return;
        }
        out << address << getMessage();
        ui->messageBrowser->verticalScrollBar()->setValue(ui->messageBrowser->verticalScrollBar()->maximum());
        break;

    case NewParticipant: out << address; break;

    case ParticipantLeft: break;

    case FileName: break;

    case Refuse: break;
    }
    udpSocket->writeDatagram(data, data.length(), QHostAddress::Broadcast, port);
}

QString MainWidget::getIP()
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol)
            return address.toString();
    }
    return 0;
}

QString MainWidget::getUserName()
{
    QStringList envVariables;
    envVariables << "USERNAME.*"
                 << "USER.*"
                 << "USERDOMAIN.*"
                 << "HOSTNAME.*"
                 << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1)
        {
            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2)
            {
                return stringList.at(1);
                break;
            }
        }
    }
    return "unknown";
}

QString MainWidget::getMessage()
{
    QString msg = ui->messageTextEdit->toHtml();

    ui->messageTextEdit->clear();
    ui->messageTextEdit->setFocus();
    return msg;
}

void MainWidget::hasPendingFile(QString userName, QString serverAddress, QString clientAddress, QString fileName)
{
    QString ipAddress = getIP();
    if (ipAddress == clientAddress)
    {
        int btn = QMessageBox::information(this, tr("接受文件"), tr("来自%1(%2)的文件：%3,是否接收？").arg(userName).arg(serverAddress).arg(fileName),
                                           QMessageBox::Yes, QMessageBox::No);
        if (btn == QMessageBox::Yes)
        {
            QString name = QFileDialog::getSaveFileName(0, tr("保存文件"), fileName);
            if (!name.isEmpty())
            {
                TcpClientWidget* client = new TcpClientWidget(this);
                client->setFileName(name);
                client->setHostAddress(QHostAddress(serverAddress));
                client->show();
            }
        }
        else
        {
            sendMessage(Refuse, serverAddress);
        }
    }
}

bool MainWidget::saveFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("保存文件"), tr("无法保存文件 %1:\n %2").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->messageBrowser->toPlainText();

    return true;
}

void MainWidget::closeEvent(QCloseEvent* event)
{
    sendMessage(ParticipantLeft);
    QWidget::closeEvent(event);
}

void MainWidget::slotProcessPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QDataStream in(&datagram, QIODevice::ReadOnly);
        int messageType;
        in >> messageType;
        QString userName, localHostName, ipAddress, message;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        switch (messageType)
        {
        case Message:
            in >> userName >> localHostName >> ipAddress >> message;
            ui->messageBrowser->setTextColor(Qt::blue);
            ui->messageBrowser->setCurrentFont(QFont("Times New Roman", 12));
            ui->messageBrowser->append("[ " + userName + " ] " + time);
            ui->messageBrowser->append(message);
            break;

        case NewParticipant:
            in >> userName >> localHostName >> ipAddress;
            newParticipant(userName, localHostName, ipAddress);
            break;

        case ParticipantLeft:
            in >> userName >> localHostName;
            participantLeft(userName, localHostName, time);
            break;

        case FileName: break;

        case Refuse: break;
        }
    }
}

void MainWidget::slotOnSendBtnClicked() { sendMessage(Message); }

void MainWidget::slotGetFileName(QString name)
{
    fileName = name;
    sendMessage(FileName);
}

void MainWidget::slotSendFileBtnClicked()
{
    if (ui->userTableWidget->selectedItems().isEmpty())
    {
        QMessageBox::warning(0, tr("选择用户"), tr("请先从用户列表选择要传送的用户！"), QMessageBox::Ok);
        return;
    }
    server->show();
    server->initServer();
}
