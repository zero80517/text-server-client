#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDir>

#define LOCAL_SAVED_FILES_DIR "SavedFiles"
#define LOCAL_TABLE_PATH "Table.txt"

class Server : public QObject
{
    Q_OBJECT

private:
    QTcpServer* m_ptcpServer;
    QVector<QTcpSocket *> m_vpSocket;
    QString     m_sSavedFilesDir;
    QString     m_sTableFilePath;

private:
    void sendToClient(QTcpSocket *pSocket, const QString &sData);

private:
    void InitSavedFilesDir();
    void InitTableFile();
    void InitTableOnClient(QTcpSocket *pSocket);
    void UpdateTableAllClients();
    void InsertFileIntoTable(QString sFilename);
    void OperateClientTextData(QTcpSocket *pSocket, QString &sData);
    void SaveClientTextFile(QString sFilename, QString sFiledata);
    void LoadTextFilesToClient(QTcpSocket *pSocket, QString sFilenames);
    void SendFileToClient(QTcpSocket *pSocket, QString sFilename);
    QString LoadFileDataInString(QString sFilename);
    void SendLoadTableFileToClient(QTcpSocket *pSocket, QString sFilenames);

public:
    explicit Server(int nPort, QObject *parent = nullptr);

public slots:
    virtual void slotNewConnection();
            void slotReadClient();

};

#endif // SERVER_H
