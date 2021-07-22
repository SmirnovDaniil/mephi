#ifndef FILTER_H
#define FILTER_H
#include <QObject>
#include <QSize>
#include <QImage>
#include <QtConcurrent>


class filter : public QObject
{
    Q_OBJECT
public:
    struct FilterController {
        QImage imagePart;
        int imageNumber;
        filter* f;
    };
    explicit filter( QObject *parent = nullptr);
    QImage createTempImage();
    void normMatrix();
    static FilterController* applyMatrixToPart(FilterController *c);
    void startAp(int tCount);
    static void reduceToImg(QImage& image, FilterController *c);
    void addReadyP();
    void stop();


signals:
    void started();
    void stopped();
    void progress(int percent);
    void finished();


public:


    QImage image;
    int totalPixelsCount;
    int pixelsReady;
    QSize imageSize;
    int filterSize = 7;
    bool isStopped = false;
    int threadCount;
    QImage tImage;
    QImage rImage;
    std::vector<double> fMatrix;
    QVector<filter::FilterController*> images;

};

#endif // FILTER_H
