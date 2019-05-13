#include "db.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QSqlQuery>

Db::Db(const QString &filename)
{
    const QFileInfo file(filename);
    _modified = file.lastModified();
    _connection = uniqueId(filename);       //connection name is unique (generated from full path+filename)
    _id = uniqueId(file.fileName());        //primary key remains same even if file is moved to other folder

    const QString dbfilename = QString("%1/cache.db").arg(QApplication::applicationDirPath());
    _db = QSqlDatabase::addDatabase("QSQLITE", _connection);
    _db.setDatabaseName(dbfilename);
    _db.open();

    createTables(dbfilename);
}

QString Db::uniqueId(const QString &filename) const
{
    if(filename == "")
        return _id;

    const QString name_modified = QString("%1_%2").arg(filename).arg(_modified.toString("yyyy-MM-dd hh:mm:ss.zzz"));
    return QCryptographicHash::hash(name_modified.toLatin1(), QCryptographicHash::Md5).toHex();
}

void Db::createTables(const QString &dbfilename) const
{
    const QFileInfo dbFile(dbfilename);     //create a cache database if one does not exist
    if(dbFile.size() == 0)
    {
        QSqlQuery query(_db);
        query.exec("CREATE TABLE metadata (id TEXT PRIMARY KEY, "
                   "size INTEGER, duration INTEGER, bitrate INTEGER, framerate REAL, "
                   "codec TEXT, audio TEXT, width INTEGER, height INTEGER)");

        query.exec("CREATE TABLE version (version TEXT)");
        query.exec(QString("INSERT INTO version VALUES('%1')").arg(APP_VERSION));
    }
}

bool Db::readMetadata(Video &video) const
{
    QSqlQuery query(_db);
    query.exec(QString("SELECT * FROM metadata WHERE id = '%1'").arg(_id));

    while(query.next())
    {
        video.modified = _modified;
        video.size = query.value(1).toLongLong();
        video.duration = query.value(2).toLongLong();
        video.bitrate = query.value(3).toUInt();
        video.framerate = query.value(4).toDouble();
        video.codec = query.value(5).toString();
        video.audio = query.value(6).toString();
        video.width = static_cast<ushort>(query.value(7).toUInt());
        video.height = static_cast<ushort>(query.value(8).toUInt());
        return true;
    }
    return false;
}

void Db::writeMetadata(const Video &video) const
{
    QSqlQuery query(_db);
    query.exec(QString("INSERT INTO metadata VALUES('%1',%2,%3,%4,%5,'%6','%7',%8,%9)")
               .arg(_id).arg(video.size).arg(video.duration).arg(video.bitrate).arg(video.framerate)
               .arg(video.codec).arg(video.audio).arg(video.width).arg(video.height));
}

bool Db::removeVideo(const QString &id) const
{
    QSqlQuery query(_db);

    bool idCached = false;
    query.exec(QString("SELECT id FROM metadata WHERE id = '%1'").arg(id));
    while(query.next())
        idCached = true;
    if(!idCached)
        return false;

    query.exec(QString("DELETE FROM metadata WHERE id = '%1'").arg(id));

    query.exec(QString("SELECT id FROM metadata WHERE id = '%1'").arg(id));
    while(query.next())
        return false;
    return true;
}
