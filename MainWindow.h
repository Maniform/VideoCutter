#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QSettings>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    int stringToSec(QString text);
    QString getEndTime();

private slots:
    void onVideoInPushButtonClicked();
    void onCutVideoPushButtonClicked();
    void onStopCutPushButtonClicked();

    void onReadyRead();
    void onCutVideoProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
private:
    Ui::MainWindow *ui;

    QSettings settings;
    QProcess ffmpeg;
    int videoDuration;
};
#endif // MAINWINDOW_H
