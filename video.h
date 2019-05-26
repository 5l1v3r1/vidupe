#ifndef VIDEO_H
#define VIDEO_H

#include <QDebug>               //generic includes go here as video.h is used by many files
#include <QRunnable>
#include <QProcess>
#include <QBuffer>
#include <QTemporaryDir>
#include <opencv2/imgproc/imgproc.hpp>
#include "prefs.h"
#include "db.h"

class Video : public QObject, public QRunnable
{
    Q_OBJECT

public:
    Video(const Prefs &prefsParam, const QString &filenameParam);
    void run();

    QString filename;
    int64_t size = 0;
    QDateTime modified;
    int64_t duration = 0;
    int bitrate = 0;
    double framerate = 0;
    QString codec;
    QString audio;
    short width = 0;
    short height = 0;
    QByteArray thumbnail;
    cv::Mat grayThumb;
    uint64_t hash = 0;

private slots:
    void getMetadata(const QString &filename);
    int takeScreenCaptures(const Db &cache);
    void processThumbnail(QImage &image);
    uint64_t computePhash(const cv::Mat &input);
    QImage minimizeImage(const QImage &image) const;
    QString msToHHMMSS(const int64_t &time) const;

public slots:
    QImage captureAt(const int &percent, const int &ofDuration=100) const;

signals:
    void acceptVideo(const QString &filename) const;
    void rejectVideo(Video *deleteMe) const;

private:
    static Prefs _prefs;
    static int _jpegQuality;

    enum _returnValues { _success, _failure };

    static constexpr int _okJpegQuality      = 60;
    static constexpr int _lowJpegQuality     = 25;
    static constexpr int _hugeAmountVideos   = 200000;
    static constexpr int _goBackwardsPercent = 7;       //if capture fails, retry but omit this much from end
    static constexpr int _videoStillUsable   = 86;      //86% of video duration is considered usable
    static constexpr int _thumbnailMaxWidth  = 448;     //small size to save memory and cache space
    static constexpr int _thumbnailMaxHeight = 336;
    static constexpr int _pHashSize          = 32;      //phash generated from 32x32 image
    static constexpr int _ssimSize           = 16;      //larger than 16x16 seems to have slower comparison
    static constexpr int _almostBlackBitmap  = 1500;    //monochrome thumbnail if less shades of gray than this
};

#endif // VIDEO_H
