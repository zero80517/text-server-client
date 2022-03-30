#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtWidgets>
#include <QDir>
#include <QFileDialog>

#include "../Server/flag.h"
#include "table.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Client; }
QT_END_NAMESPACE

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(const QString& sHost, int nPort, QWidget *parent = nullptr);
    ~Client();

private:
    Ui::Client *ui;
    QTcpSocket *m_pSocket;
    QTableWidget *m_pTableWidget;
    QString m_sTableFilePath;
    QString m_sLoadDir;

private:
    void OperateServerTextData(QString sData);
    void InitTable(QString sTable);
    void LoadTextFile(QString sFilename, QString sFiledata);
    QVector<Table> getvTable(QString sTable);
    void SendFileToServer(QString sFullname);
    QString GetTextDataFromFile(QString sFullname);
    QString GetSelectedFilenames();

public slots:
    void slotConnected   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotReadyRead   (                            );
    void slotSendToServer(QString &sData              );

private slots:
    void on_SaveButton_clicked();
    void on_LoadButton_clicked();

};
#endif // CLIENT_H
