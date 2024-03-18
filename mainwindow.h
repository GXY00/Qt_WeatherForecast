#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "weatherdata.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QByteArray>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void contextMenuEvent(QContextMenuEvent * event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void getWeatherInfo(QString cityName);
    void getTodayWeatherInfo(QString cityName);
    void getIp();
    void parseJson(QByteArray &bytearray);
    void initUi();
    void initTm();
    void initVector();
    Today mToday;
    Day mDay[6];

private slots:
    void on_refresh_clicked();

    void on_search_clicked();

    void on_download_clicked();

private:
    void onReplied(QNetworkReply* reply);

private:
    Ui::MainWindow *ui;
    QMenu *mExitMenu;//退出菜单
    QAction *mExitAct;//退出的行为
    QPoint mOffset;//窗口移动时，鼠标与窗口左上角的偏移

    QNetworkAccessManager* mNetAccessManager;

    QString mCityName;
    QMap<QString,QString> mTypeMap;
    QVector<QLabel*>labellist,typelist,qualitylist,windlist;
};
#endif // MAINWINDOW_H
