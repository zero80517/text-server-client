#include "client.h"
#include "ui_client.h"

// Done & works!
Client::Client(const QString& sHost,
               int nPort, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Client)
{
    ui->setupUi(this);

    m_pSocket = new QTcpSocket(this);
    m_pSocket->connectToHost(sHost, nPort);

    connect(m_pSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,      SLOT(slotError(QAbstractSocket::SocketError))
           );
    connect(m_pSocket, SIGNAL(disconnected()), m_pSocket, SLOT(deleteLater()));

    m_sLoadDir = QDir::rootPath();
    m_pTableWidget = ui->tableWidget;
}

// Done & works!
void Client::slotConnected()
{
    qDebug().noquote() << "Client::slotConnected> Received the connected() signal";
}

// Done & works!
void Client::slotError(QAbstractSocket::SocketError err)
{
    QString sError =
            "Error Client::slotError> " + (err == QAbstractSocket::HostNotFoundError ?
                                           "The host was not found." :
                                           err == QAbstractSocket::RemoteHostClosedError ?
                                           "The remote host is closed." :
                                           err == QAbstractSocket::ConnectionRefusedError ?
                                           "The connection was refused." :
                                           QString(m_pSocket->errorString())
                                          );
    qDebug().noquote() << sError;
}

// Done & works!
void Client::slotSendToServer(QString &sData)
{
    QString sBuffer;
    QTextStream out(&sBuffer, QTextStream::WriteOnly);
    // datasize + '\n', data
    out << qint64(sData.size() - sizeof(QChar)) << '\n'
        << sData;

    m_pSocket->write(sBuffer.toUtf8());
}

// Done & works! Very dangerous part!
void Client::slotReadyRead()
{
    QTcpSocket* pSocket = (QTcpSocket*)sender();
    QTextStream in(pSocket);
    QString sData = in.readAll();

    OperateServerTextData(sData);
}

// Done & works!
void Client::OperateServerTextData(QString sData)
{
    qint64 nFlag = (qint64) Flag::Empty;

    QTextStream in(&sData, QTextStream::ReadOnly);
    if (in.status() == QTextStream::Ok)
    {
        while (!in.atEnd())
        {
            // The first paramater is msg size plus QChar on flag and plus QChar on '\n'
            // So minus 2 to get msg size
            qint64 nMsgSize = in.readLine().toLongLong() - 2;

            // The second parameter is flag
            nFlag = in.readLine().toLongLong();

            qDebug().noquote() << "Client::OperateServerTextData> Selected flag is" << nFlag;
            qDebug().noquote() << "Client::OperateServerTextData> sData length" << sData.length();

            // operate flag
            if (nFlag == qint64(Flag::Empty))
            {
                qDebug().noquote() << "Error Client::OperateServerTextData> Got empty flag, break";
                break;
            }
            else if (nFlag == qint64(Flag::Init))
            {
                qDebug().noquote() << "Client::OperateServerTextData> Init table";
                // msg = "row1\nrow2\n..."
                QString sTable = in.readAll();
                InitTable(sTable);
            }
            else if (nFlag == qint64(Flag::Load))
            {
                qDebug().noquote() << "Client::OperateServerTextData> Load file from server";
                // msg = "filename\nfiledata"
                QString sFilename = in.readLine();
                // minus len filename and minus 1 to not count '\n'
                QString sFiledata = in.read(nMsgSize - (sFilename.size() + 1));
                LoadTextFile(sFilename, sFiledata);
            }
            else
            {
                qDebug().noquote() << "Error Client::OperateServerTextData> Got incorrect flag, break";
                break;
            }
        }
    }
    else
    {
        qDebug().noquote() << "Error Client::OperateServerTextData> can't creat textstream";
    }
}

// Done & works!
void Client::InitTable(QString sTable)
{
    QVector<Table> vTable = getvTable(sTable);

    if (vTable.isEmpty())
        return;

    qint64 nRowCount = vTable.size();
    m_pTableWidget->setRowCount(nRowCount);

    qint64 row = 0;
    qint64 col = 0;

    QTableWidgetItem  *pItem;

    for (auto i{vTable.begin()}; i < vTable.end(); ++i)
    {
        pItem = new QTableWidgetItem(i->sCreationDateTime);
        m_pTableWidget->setItem(row, col, pItem);

        ++col;
        pItem = new QTableWidgetItem(i->sFilename);
        m_pTableWidget->setItem(row, col, pItem);

        ++col;
        pItem = new QTableWidgetItem(i->sLink);
        m_pTableWidget->setItem(row, col, pItem);

        ++row;
        col = 0;
    }
}

// Done & works!
QVector<Table> Client::getvTable(QString sTable)
{
    QVector<Table> vTable;
    Table tableTmp;

    QTextStream in(&sTable, QTextStream::ReadOnly);
    if (in.status() == QTextStream::Ok)
    {
        while (!in.atEnd())
        {
            QString line = in.readLine(); // read one line at a time
            QStringList lstLine = line.split(",");

            tableTmp.sCreationDateTime = lstLine[0];
            tableTmp.sFilename = lstLine[1];
            tableTmp.sLink = lstLine[2];
            vTable.push_back(tableTmp);
        }
    }
    else
    {
        qDebug().noquote() << "Error Client::getvTable> Can't create textstream";
    }

    return vTable;
}

// Done & works!
void Client::LoadTextFile(QString sFilename, QString sFiledata)
{
    QString sFullname = QDir(m_sLoadDir).filePath(sFilename);
    QFile file(sFullname);

    if (file.open(QFile::WriteOnly))
    {
        file.write(sFiledata.toUtf8());
        qDebug().noquote().nospace() << "Client::LoadTextFile>"
                                     << "saved file = " << sFullname
                                     << "; saved data = " << sFiledata;
    }
    else
    {
        qDebug().noquote().nospace() << "Error Client::LoadTextFile> Unable to create file"
                                     << sFullname << "to save data";
    }
    file.close();
}

// Done & works!
void Client::on_SaveButton_clicked()
{
    QString sFullname = QFileDialog::getOpenFileName(this,
                                                     "Open a text file",
                                                     m_sLoadDir,
                                                     "Text files (*.txt)"
                                                    );
    if (sFullname.isEmpty())
        return;

    QFileInfo fi(sFullname);
    QString sFilename = fi.fileName();

    qDebug().noquote() << "Client::on_SaveButton_clicked> Save file"
                       << sFullname << "to server";

    SendFileToServer(sFullname);
}

// Done and works!
void Client::SendFileToServer(QString sFullname)
{
    QFileInfo fi(sFullname);
    QString sFilename = fi.fileName();

    QString sData;
    QTextStream out(&sData, QTextStream::WriteOnly);

    // flag + '\n', filename + '\n', data
    out << qint64(Flag::Save) << '\n'
        << sFilename << '\n'
        << GetTextDataFromFile(sFullname);

    slotSendToServer(sData);
}

// Done and works!
QString Client::GetTextDataFromFile(QString sFullname)
{
    QString sFiledata;
    QFile file(sFullname);
    if (file.open(QIODevice::ReadOnly))
    {
        sFiledata = file.readAll();
    }
    else
    {
        qDebug().noquote() << "Error Client::GetTextDataFromFile> Can't open file to read";
    }
    file.close();

    return sFiledata;
}

// Done & works!
void Client::on_LoadButton_clicked()
{
    QString sNewLoadDir = QFileDialog::getExistingDirectory(this,
                                                            "Open Directory",
                                                            m_sLoadDir,
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks
                                                           );

    if (sNewLoadDir.isEmpty())
        return;

    m_sLoadDir = sNewLoadDir;

    QString sItems = GetSelectedFilenames();
    if (sItems.isEmpty())
        return;

    QString sData;
    QTextStream out(&sData, QTextStream::WriteOnly);
    if (out.status() == QTextStream::Ok)
    {
        // flag + '\n', selected filenames
        out << qint64(Flag::Load) << '\n'
            << sItems;
        qDebug().noquote() << QStringLiteral("Client::on_LoadButton_clicked> Selected flag is %1").arg(qint64(Flag::Load));
    }
    else
    {
        qDebug().noquote() << "Error Client::on_LoadButton_clicked> Can't create textstream";
    }

    slotSendToServer(sData);
}

// Done & works!
QString Client::GetSelectedFilenames()
{
    QString sItems;
    QList<QTableWidgetItem *> vTableSelectedItem = m_pTableWidget->selectedItems();

    if (vTableSelectedItem.empty())
        return sItems;

    QTableWidgetItem *pItem;
    QTextStream out(&sItems, QTextStream::WriteOnly);

    if (out.status() == QTextStream::Ok)
    {
        QString sFilename;
        int col = 1; // filename column
        foreach (pItem, vTableSelectedItem)
        {
            if (pItem->column() == col)
            {
                sFilename = pItem->text();
                out << sFilename << '\n';
                qDebug().noquote() << "Client::GetSelectedFilenames> selected item =" << sFilename;
            }
            else
            {
                qDebug().noquote() << "Error Client::GetSelectedFilenames> selected wrong item, select item in filename column";
            }
        }
    }
    else
    {
        qDebug().noquote() << "Error Client::GetSelectedFilenames> Can't open textstream to fill sItems";
    }

    return sItems;
}

// Done & works!
void Client::on_tableWidget_cellDoubleClicked(int row, int column)
{
    // third column with index 2(i.e. columns are numbered from 0) is "Link"
    if (column == 2)
    {
        QTableWidgetItem *pItem = m_pTableWidget->item(row, column);
        QString sLink = pItem->text();
        QDesktopServices::openUrl(QUrl(sLink, QUrl::TolerantMode));
    }
}

void Client::deleteLater()
{
   qDebug().noquote() << "Client::deleteLater> Client disconnected with descriptor"
                      << m_pSocket->socketDescriptor();
}

// Done & works!
Client::~Client()
{
    delete ui;
}
