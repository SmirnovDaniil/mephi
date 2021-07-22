#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    f = new filter();
    f->threadCount = QThreadPool::globalInstance()->maxThreadCount();
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    connect(f, &filter::progress, this, &MainWindow::OnProgressChanged);

    connect(&w, &QFutureWatcher<QImage>::finished, this, [=]()
    {
        resImg = w.result();
        QPixmap p = QPixmap::fromImage(resImg);
        ui->resImageLabel->setScaledContents(true);
        ui->resImageLabel->setPixmap(p.scaled(ui->resImageLabel->width(),ui->resImageLabel->height()));
    }
    );

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Открыть изображение",QDir::currentPath(),"Image files (*.png *.jpg *.jpeg *.bmp *.ico)");
        if (!fileName.isNull()){
            int w = ui->imageLabel->width();
            int h = ui->imageLabel->height();
            ui->browseLineEdit->setText(fileName);
            QPixmap p = QPixmap(fileName);
            QImage i = QImage(fileName);
            ui->imageLabel->setScaledContents(true);
            ui->imageLabel->setPixmap(p.scaled(w,h));
            f->image = i;
            f->imageSize=i.size();
        }
}

void MainWindow::on_sizeButton_clicked()
{
    size = ui->sizeLineEdit->text().toInt();
    ui->tableWidget->setColumnCount(size);
    ui->tableWidget->setRowCount(size);
    f->filterSize = size;
}

void MainWindow::on_endButton_clicked()
{

    f->fMatrix.clear();
    for (int i = 0; i < size; i++ ) {
        for (int j = 0; j < size; j++){
            f->fMatrix.push_back(ui->tableWidget->item(i,j)->data(0).toDouble());
        }
    }
}

void MainWindow::on_randomButton_clicked()
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    f->fMatrix.clear();

    std::normal_distribution<double> distribution (1.0,1.0);
    for (int i = 0; i < size; i++ ) {
        for (int j = 0; j < size; j++){
            double k=distribution(generator);
            f->fMatrix.push_back(k);
            ui->tableWidget->setItem(i, j, new QTableWidgetItem(QString::number(k)));
        }
    }
}

void MainWindow::on_threadCountButton_clicked()
{
    QThreadPool::globalInstance()->setMaxThreadCount(ui->threadCountLineEdit->text().toInt());
    f->threadCount = QThreadPool::globalInstance()->maxThreadCount();
}

void MainWindow::on_settingsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_imageButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_startButton_clicked()
{
   f->startAp(QThreadPool::globalInstance()->maxThreadCount());
   QFuture<QImage> fut = QtConcurrent::mappedReduced(f->images, &filter::applyMatrixToPart, &filter::reduceToImg, QtConcurrent::OrderedReduce);
   w.setFuture(fut);
}

void MainWindow::on_stopButton_clicked()
{
    f->stop();
}

void MainWindow::on_saveButton_clicked()
{
    resImg.save(QString("rImage.jpg"));
}

void MainWindow::OnProgressChanged(int percent)
{
    ui->progressBar->setValue(percent);
}

void MainWindow::OnFinished()
{
       QPixmap p = QPixmap::fromImage(f->rImage);
       resImg = f->rImage;
       ui->resImageLabel->setPixmap(p.scaled(ui->resImageLabel->width(),ui->resImageLabel->height()));
}
