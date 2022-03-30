#include "server.h"
#include "flag.h"

// Done & works!
Server::Server(int nPort, QObject *parent) : QObject(parent)
{
    qDebug().noquote() << "Server::Server> Start Server";
    m_ptcpServer = new QTcpServer(this);
    if (!m_ptcpServer->listen(QHostAddress::Any, nPort))
    {
        QMessageBox::critical(0,
                              "Error Server::Server",
                              "Unable to start the server: "
                              + m_ptcpServer->errorString()
                             );
        m_ptcpServer->close();
        return;
    }

    connect(m_ptcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    InitSavedFilesDir();
    InitTableFile();
}

// Done & works!
void Server::InitSavedFilesDir()
{
    QString sLocalSavedFilesDir = QString(LOCAL_SAVED_FILES_DIR);
    QString sApplicationDir = QDir::currentPath(); // path to .../build-server-...
    m_sSavedFilesDir = QDir(QDir(sApplicationDir).filePath(sLocalSavedFilesDir)).absolutePath();

    QDir dirSavedFiles(m_sSavedFilesDir);
    if (dirSavedFiles.exists(m_sSavedFilesDir))
    {
        qDebug().noquote() << "Server::InitSavedFilesDir> Saved files directory ="
                           << m_sSavedFilesDir;
    }
    else
    {
        if (dirSavedFiles.mkdir(m_sSavedFilesDir))
        {
            qDebug().noquote() << "Server::InitSavedFilesDir> Saved files directory ="
                               << m_sSavedFilesDir << "; was created";
        }
        else
        {
            qDebug().noquote() << "Server::InitSavedFilesDir> Can't create Saved files directory ="
                               << m_sSavedFilesDir;
        }
    }
}

// Done & works!
void Server::InitTableFile()
{
    QString sLocalTablePath = QString(LOCAL_TABLE_PATH);
    QString sApplicationDir = QDir::currentPath(); // path to .../build-client-...
    m_sTableFilePath = QDir(QDir(sApplicationDir).filePath(sLocalTablePath)).absolutePath();

    QFile fileTable(LOCAL_TABLE_PATH);

    if (fileTable.exists())
    {
        qDebug().noquote() << "Server::InitTableFile> Table file ="
                           << m_sTableFilePath;
    }
    else
    {
        if (fileTable.open(QIODevice::WriteOnly))
        {
            qDebug().noquote() << "Server::InitTableFile> Table file ="
                               << m_sTableFilePath << "; was created";
        }
        else
        {
            qDebug().noquote() << "Server::InitTableFile> Can't create Table file ="
                               << m_sTableFilePath;
        }
    }

    fileTable.close();
}

// Done & works! But add vsSocket to update table on all sockets
void Server::slotNewConnection()
{
    QTcpSocket* pSocket = m_ptcpServer->nextPendingConnection();
    connect(pSocket, SIGNAL(disconnected()), pSocket, SLOT(deleteLater()));
    connect(pSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    qDebug().noquote() << "Server::slotNewConnection> Client connected with descriptor"
                       << pSocket->socketDescriptor();

    InitTableOnClient(pSocket);
}

// Done & works!
void Server::InitTableOnClient(QTcpSocket *pSocket)
{
    QString sData;
    QTextStream out(&sData, QTextStream::WriteOnly);

    QFile fileTable(m_sTableFilePath);
    if (fileTable.open(QFile::ReadOnly))
    {
        QTextStream in(&fileTable);
        if(in.status() == QTextStream::Ok)
        {
            // flag, tabledata
            out << qint64(Flag::Init) << '\n'
                << in.readAll();
        }
        else
        {
            qDebug() << "Error Server::InitTableOnClient> Unable to create textstream";
        }
    }
    else
    {
        qDebug().noquote() << "Error Server::InitTableOnClient> Unable to read table file";
    }
    fileTable.close();

    sendToClient(pSocket, sData);
}

// Done & works!
void Server::InsertFileIntoTable(QString sFilename)
{
    QFile file(m_sTableFilePath);

    if (file.open(QFile::Append))
    {
        QTextStream out(&file);
        out << QDateTime::currentDateTime().toString("dd.MM.yyyy/hh:mm:ss") << ","
            << sFilename << ","
            << "file:///" + QDir(m_sSavedFilesDir).filePath(sFilename) << '\n';
    }
    else
    {
        qDebug().noquote() << "Error Server::InsertFileIntoTable> Can't append data to file";
    }
    file.close();
}

// Done & works!
void Server::sendToClient(QTcpSocket *pSocket, const QString &sData)
{
    QString sMsg;
    QTextStream out(&sMsg, QTextStream::WriteOnly);
    if (out.status() == QTextStream::Ok)
    {
//        // datasize - sizeof(QChar), data
//        out << qint64(sData.length() - sizeof(QChar))  << (QChar)'\n'
//            << sData;
        // datasize - sizeof(QChar), data
        out << qint64(sData.length())  << (QChar)'\n'
            << sData;

        pSocket->write(sMsg.toUtf8());
    }
    else
    {
        qDebug().noquote() << "Error Server::sendToClient> Unable to create textstream";
    }
}

// Done & works! Very dangerous part!
void Server::slotReadClient()
{
/*
//    qDebug().noquote() << "\nServer::slotReadClient> Read msg from client";
//    QTcpSocket* pSocket = (QTcpSocket*)sender();
//    QString sData;
//    qint64 nMsgSize = -1;

//    QTextStream in(pSocket);
//    if (in.status() == QTextStream::Ok)
//    {
//        // get datasize first
//        if (pSocket->bytesAvailable() && nMsgSize == -1)
//        {
//            nMsgSize = in.readLine().toLongLong();
//            qDebug().noquote() << "Server::slotReadClient> Msg size ="
//                               << nMsgSize;
//        }

//        while (pSocket->bytesAvailable() < nMsgSize - qint64(sizeof(qint64))) {
//            if (!pSocket->waitForReadyRead(100))
//            {
//                qDebug().noquote() << "Server::slotReadClient> Too long to get client's response, break";
//                pSocket->disconnectFromHost();
//                break;
//            }
//        }

//        sData = in.readAll();

//        OperateClientTextData(pSocket, sData);
//    }
//    else
//    {
//        qDebug().noquote() << "Error Server::slotReadClient> Can't create textstream";
//    }
*/

    qDebug().noquote() << "\nClient::slotReadyRead> Read msg from server";

    QTcpSocket* pSocket = (QTcpSocket*)sender();
    QString sData;
    qint64 nMsgSize = 0;

    QTextStream in(pSocket);
    if (in.status() == QTextStream::Ok)
    {
        if (nMsgSize == 0)
        {
            nMsgSize = in.readLine().toLongLong();
            qDebug().noquote() << "Server::slotReadClient> Msg size =" << nMsgSize;
        }
        sData.append(in.readAll());
        while (pSocket->bytesAvailable() > 0)
        {
            sData.append(in.readAll());
        }

        OperateClientTextData(pSocket, sData);
    }
    else
    {
        qDebug().noquote() << "Error Server::slotReadClient> Can't create textstream";
    }
}

// Done & works! But add functions to multiple sockets
void Server::OperateClientTextData(QTcpSocket *pSocket, QString &sData)
{
    qint64 nFlag = (qint64) Flag::Empty;

    QTextStream in(&sData, QTextStream::ReadOnly);
    if (in.status() == QTextStream::Ok)
    {
        // The second parameter is flag
        nFlag = in.readLine().toLongLong();

        qDebug().noquote() << "Server::OperateClientTextData> Selected flag is" << nFlag;
        qDebug().noquote() << "Server::OperateClientTextData> sData length" << sData.length();

        // operate flag
        if (nFlag == qint64(Flag::Empty))
        {
            qDebug().noquote() << "Error Server::OperateClientTextData> Got empty flag, break";
        }
        else if (nFlag == qint64(Flag::Save))
        {
            QString sFilename = in.readLine();
            QString sFiledata = in.readAll();
            qDebug().noquote() << "Server::OperateClientTextData> Save client's file with name" << sFilename;
            SaveClientTextFile(sFilename, sFiledata);
            InsertFileIntoTable(sFilename);
            // Update table on one client?
            InitTableOnClient(pSocket);
        }
        else if (nFlag == qint64(Flag::Load))
        {
            qDebug().noquote() << "Server::OperateClientTextData> Load files from server to client";
            QString sFilenames = in.readAll();
            LoadTextFilesToClient(pSocket, sFilenames);
        }
        else
        {
            qDebug().noquote() << "Error Server::OperateClientTextData> Got incorrect flag";
        }
    }
    else
    {
        qDebug() << "Error Server::OperateClientTextData> can't creat textstream";
    }
}

// Done & works!
void Server::SaveClientTextFile(QString sFilename, QString sFiledata)
{
    QString sFullname = QDir(m_sSavedFilesDir).filePath(sFilename);
    QFile file(sFullname);

    if (file.open(QFile::WriteOnly))
    {
        file.write(sFiledata.toUtf8());
        qDebug().noquote() << "Server::SaveClientTextFile>"
                           << "saved file =" << sFullname
                           << "; saved data =" << sFiledata;
    }
    else
    {
        qDebug().noquote() << "Error Server::SaveClientTextFile> Unable to create file"
                           << sFullname << "to save data";
    }
    file.close();
}

// Done & works!
void Server::LoadTextFilesToClient(QTcpSocket *pSocket, QString sFilenames)
{
    QTextStream in(&sFilenames, QTextStream::ReadOnly);

    if(in.status() == QTextStream::Ok)
    {
        QVector<QString> vsTableFilename;
        while (!in.atEnd())
        {
            QString sFilename;
            sFilename = in.readLine();
            vsTableFilename.push_back(sFilename);
            qDebug().noquote() << QStringLiteral("Server::LoadTextFilesToClient> Selected file is %1").arg(sFilename);

            SendFileToClient(pSocket, sFilename);
        }
        SendLoadTableFileToClient(pSocket, sFilenames);
    }
    else
    {
        qDebug().noquote() << "Error Server::LoadTextFilesToClient> Can't create textstream";
    }
}

// Done & works!
void Server::SendFileToClient(QTcpSocket *pSocket, QString sFilename)
{
    QString sData;
    QTextStream out(&sData, QTextStream::WriteOnly);

    if (out.status() == QTextStream::Ok)
    {
        out << qint64(Flag::Load) << '\n'
            << sFilename << '\n'
            << LoadFileDataInString(sFilename);

        sendToClient(pSocket, sData);
    }
    else
    {
        qDebug().noquote() << "Error Server::SendFileToClient> Can't create textstream";
    }
}

// Done & works!
QString Server::LoadFileDataInString(QString sFilename)
{
    QString str;

    QString sFullname = QDir(m_sSavedFilesDir).filePath(sFilename);
    QFile file(sFullname);
    if (file.open(QFile::ReadOnly))
    {
        str = file.readAll();
    }
    else
    {
        qDebug().noquote() << "Error Server::LoadFileDataInString> Can't open file"
                           << sFullname + "to read data";
    }
    file.close();

    return str;
}

// Done & works!
void Server::SendLoadTableFileToClient(QTcpSocket *pSocket, QString sFilenames)
{
    if (sFilenames.isEmpty())
        return;

    QString sData;
    QTextStream out(&sData, QTextStream::WriteOnly);

    if (out.status() == QTextStream::Ok)
    {
        QString sTablename = "LoadTable.txt";
        out << qint64(Flag::Load) << '\n'
            << sTablename << '\n'
            << sFilenames;

        sendToClient(pSocket, sData);
    }
    else
    {
        qDebug() << "Error Server::SendLoadTableFileToClient> Can't create textstream";
    }
}
