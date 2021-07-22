#include "filter.h"
#include <QPainter>
#include <QtConcurrent>
#include <QPair>

filter::filter(QObject *parent) : QObject(parent)
{

}

QImage filter::createTempImage()
{
    QImage tempImage = QImage(image.width() + 2 * (filterSize / 2),
                        image.height() + 2 * (filterSize / 2),
                        QImage::Format_RGB32);

    QPainter painter(&tempImage);
    painter.drawImage(QRectF(filterSize/2,filterSize/2,image.width(),image.height()),image);

    QImage leftPart = image.copy(QRect(0, 0, filterSize / 2, image.height()));
    painter.drawImage(QPoint{0, filterSize / 2}, leftPart.mirrored(true));

    QImage rightPart = image.copy(QRect(image.width()-filterSize / 2, 0 ,filterSize / 2, image.height()));
    painter.drawImage(QPoint{tempImage.width()-filterSize / 2, filterSize / 2}, rightPart.mirrored(true));

    QImage topPart = tempImage.copy(QRect(0, filterSize / 2, tempImage.width() , filterSize / 2));
    painter.drawImage(QPoint{0, 0}, topPart.mirrored(false));

    QImage bottomPart = tempImage.copy(QRect(0, tempImage.height() - filterSize , tempImage.width() , filterSize / 2 ));
    painter.drawImage(QPoint{0, tempImage.height() - filterSize / 2}, topPart.mirrored(false));
    tImage =tempImage;
    return tempImage;
}

void filter::normMatrix()
{
    double s = std::accumulate(fMatrix.begin(), fMatrix.end(), 0.0);
    for (int i = 0;i < int(fMatrix.size());i++){
        fMatrix[i] /= s;
    }
}


filter::FilterController* filter::applyMatrixToPart(FilterController *c)
{
    auto i = c->imagePart;
    auto resImg = i;
    auto fSize = c->f->filterSize;
    int w = i.width() - fSize + 1;
    int h = i.height() - fSize + 1;

    int pixelsCount= w * h;

    for(int k = 0;k < pixelsCount; k++){
        if (c->f->isStopped)
            break;
        int x = k % w ;
        int y = k / w ;
        double red = 0;
        double green = 0;
        double blue = 0;
        for (int j = 0; j < fSize * fSize;j++){
            int dx = j % fSize ;
            int dy = j / fSize ;
            auto rgb = i.pixelColor(x + dx, y+dy).rgba();
            red += qRed(rgb) * c->f->fMatrix[j];
            green += qGreen(rgb) * c->f->fMatrix[j];
            blue += qBlue(rgb) * c->f->fMatrix[j];
        }

        auto rgb = i.pixelColor(x , y).rgba();
        auto alpha = qAlpha(rgb);
        resImg.setPixel(x+fSize/2,y+fSize/2,qRgba(int(red),int(green),int(blue),alpha));
        c->f->addReadyP();
    }
    resImg = resImg.copy(QRect(fSize/2,fSize/2, w, h));
    c->imagePart = resImg;
    return c;
}

void filter::startAp(int tCount)
{
    isStopped = false;
    images.clear();
    normMatrix();
    createTempImage();
    int w = tImage.width();
    int h = image.height() / tCount + filterSize - 1;
    for (int i = 0; i < tCount-1; i++){
        QImage im = tImage.copy(QRect(0, (image.height() / tCount) * i, w, h));
        auto controller = new FilterController;
        controller->imagePart = im;
        controller->imageNumber = i;
        controller->f = this;
        images.push_back(controller);
    }
    QImage im = tImage.copy(QRect(0, image.height()/tCount*(tCount-1),
                                  w, image.height() % tCount +h));
    auto controller = new FilterController;
    controller->imagePart = im;
    controller->imageNumber = tCount - 1;
    controller->f = this;

    images.push_back(controller);

    totalPixelsCount=image.height()*image.width();
    pixelsReady=0;
}

void filter::reduceToImg(QImage& imag, FilterController *c)
{
    auto i =c->imagePart;
    auto n =c->imageNumber;
    if (imag.isNull()) {
        imag = QImage(c->f->imageSize, QImage::Format_RGB32);
    }
    QPainter painter(&imag);
    painter.drawImage(QPoint(0, (imag.height()/c->f->threadCount) * n), i);
    delete c;
}

void filter::addReadyP()
{
    pixelsReady++;
    emit progress(pixelsReady * 100 / totalPixelsCount + 1);
}

void filter::stop()
{
    isStopped = true;
}



