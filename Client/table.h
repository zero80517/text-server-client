#ifndef TABLE_H
#define TABLE_H

#include <QString>

// QTableWidget numbers itself, so table's header columns will be:
// №, Creation's Date/Time, Filename, Link
struct Table
{
    QString sCreationDateTime;
    QString sFilename;
    QString sLink;
};

#endif // TABLE_H
