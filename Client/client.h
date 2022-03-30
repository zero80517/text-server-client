#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtWidgets>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>

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
    QVector<Table> getvTable(QString sTable);
    void LoadTextFile(QString sFilename, QString sFiledata);
    void SendFileToServer(QString sFullname);
    QString GetTextDataFromFile(QString sFullname);
    QString GetSelectedFilenames();

public slots:
    void slotConnected   (                            );
    void slotError       (QAbstractSocket::SocketError);
    void slotReadyRead   (                            );
    void slotSendToServer(QString &sData              );
    void deleteLater     (                            );

private slots:
    void on_SaveButton_clicked();
    void on_LoadButton_clicked();
    void on_tableWidget_cellDoubleClicked(int row, int column);
};
#endif // CLIENT_H
