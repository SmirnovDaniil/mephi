#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <filter.h>
#include <QtConcurrent>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseButton_clicked();

    void on_sizeButton_clicked();

    void on_endButton_clicked();

    void on_randomButton_clicked();

    void on_threadCountButton_clicked();

    void on_settingsButton_clicked();

    void on_imageButton_clicked();

    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_saveButton_clicked();

    void OnProgressChanged(int percent);

    void OnFinished();

private:
    Ui::MainWindow *ui;
    int size;
    filter *f;
    QImage resImg;
    QFutureWatcher<QImage> w;
};
#endif // MAINWINDOW_H
