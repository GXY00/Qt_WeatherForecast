#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "weathertool.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /*QMainWindow *mainWindow = new QMainWindow();
    QString imagePath = "path_to_your_image.jpg"; // 图片的路径

    // 将图片路径转换为URL格式
    QUrl imageUrl = QUrl::fromLocalFile(imagePath);
    QString imageUrlString = imageUrl.toString();

    // 设置MainWindow的样式表，将背景设为图片
    QString styleSheet = QString("background-image: url(%1);").arg(imageUrlString);
    mainWindow->setStyleSheet(styleSheet);

    mainWindow->show();*/

    setWindowFlag(Qt::FramelessWindowHint);//设置窗口无边框

    //构建右键菜单
    mExitMenu = new QMenu(this);
    mExitAct = new QAction();

    mExitAct->setText("退出");
    mExitAct->setIcon(QIcon(":/res/close.png"));

    mExitMenu->addAction(mExitAct);

    connect(mExitAct,&QAction::triggered,this,[=]{
        qApp->exit(0);
    });

    setWindowIcon(QIcon(":/res/icon.png"));
    mNetAccessManager = new QNetworkAccessManager(this);
    connect(mNetAccessManager,&QNetworkAccessManager::finished,this,&MainWindow::onReplied);
    //getWeatherInfo("101010100");
    initTm();
    initVector();
    getIp();

    getWeatherInfo(mCityName);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//重写父类虚函数
//默认忽略右键菜单
void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    //弹出右键菜单
    mExitMenu->exec(QCursor::pos());
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    mOffset = event->globalPos() - this->pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    this->move(event->globalPos()-mOffset);
}

void MainWindow::onReplied(QNetworkReply *reply)
{
//    qDebug() << "天气信息请求成功";

    int status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(status_code==200){
        QByteArray byteArray = reply->readAll();
        parseJson(byteArray);
        initUi();
        //qDebug() << byteArray.data();
//        qDebug()<< "operation:" << reply->operation();
//        qDebug()<< "status code:" << status_code;
//        qDebug()<< "url:" << reply->url();
//        qDebug()<< "raw header:" << reply->rawHeaderList();
    }
    else{
        QMessageBox::critical(this,"错误","连接失败",QMessageBox::YesAll);
    }

    reply->deleteLater();
}


void MainWindow::getWeatherInfo(QString cityName)
{

    QString cityCode = WeatherTool::getCityCode(cityName,"D:/Download/citycode-2019-08-23.json");
    QUrl url("http://t.weather.itboy.net/api/weather/city/"+cityCode);
    mNetAccessManager->get(QNetworkRequest(url));
}

void MainWindow::getTodayWeatherInfo(QString cityName)
{
    QString cityCode = WeatherTool::getCityCode(cityName,"D:/Download/citycode.json");
    QUrl url("https://restapi.amap.com/v3/weather/weatherInfo?city="+cityCode+"&key=9849af4ae496bf9f4d6758b7d67203a9");
    QEventLoop loop;
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray byte = reply->readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(byte,&err);
    if(err.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject obj = doc.object();
    QJsonArray arr = obj.value("lives").toArray();
    QJsonObject today = arr[0].toObject();
    mToday.wendu = today.value("temperature").toString();
    mToday.shidu = today.value("humidity").toString();
    mToday.type = today.value("weather").toString();
    mToday.fx = today.value("winddirection").toString()+"风";
    mToday.fl = today.value("windpower").toString()+"级";
}

void MainWindow::getIp()
{
    QEventLoop loop;

    QNetworkAccessManager manager;
    QNetworkReply *pReply = manager.get(QNetworkRequest(QUrl("http://www.ip38.com/")));   // http://www.ip38.com/index.php?ip=您的IP
    connect(pReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();//使用事件循环的 exec 方法来启动事件循环，等待请求完成。
    QByteArray byte = pReply->readAll();
//    qDebug() << byte.data();
    QString strTemp;

    strTemp = byte.data();
    int nStartIndex = strTemp.indexOf("<font color='#FF0000'>");
    if (nStartIndex >= 0)
    {
        nStartIndex += strlen("<font color='#FF0000'>");
        int nEndIndex = strTemp.indexOf("</font>", nStartIndex);
        if (nEndIndex > nStartIndex)
        {
            mCityName = strTemp.mid(nStartIndex + 3, nEndIndex - nStartIndex - 4);
            qDebug() << mCityName;
        }

    }
    pReply->deleteLater();
}

void MainWindow::parseJson(QByteArray &bytearray)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(bytearray,&err);
    if(err.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject rootObj = doc.object();
    //qDebug() << rootObj.value("message").toString();

    //解析日期和城市
    mToday.date = rootObj.value("date").toString();
    mToday.city = rootObj.value("cityInfo").toObject().value("city").toString();

    //解析昨天
    QJsonObject objData = rootObj.value("data").toObject();
    QJsonObject objY = objData.value("yesterday").toObject();
    mDay[0].week = objY.value("week").toString();
    mDay[0].date = objY.value("ymd").toString();
    mDay[0].type = objY.value("type").toString();
    mDay[0].fx = objY.value("fx").toString();
    mDay[0].fl = objY.value("fl").toString();
    mDay[0].aqi = objY.value("aqi").toDouble();
    QString s = objY.value("high").toString().split(" ").at(1);
    mDay[0].high = s;

    s = objY.value("low").toString().split(" ").at(1);
    mDay[0].low = s;

    //解析forcast中5天的数据
    QJsonArray forecastArray = objData.value("forecast").toArray();
    for(int i=0;i<5;i++){
        QJsonObject objForecast = forecastArray[i].toObject();
        mDay[i+1].week = objForecast.value("week").toString();
        mDay[i+1].date = objForecast.value("ymd").toString();
        mDay[i+1].type = objForecast.value("type").toString();
        mDay[i+1].fx = objForecast.value("fx").toString();
        mDay[i+1].fl = objForecast.value("fl").toString();
        mDay[i+1].aqi = objForecast.value("aqi").toDouble();
        QString s = objForecast.value("high").toString().split(" ").at(1);
        mDay[i+1].high = s;

        s = objForecast.value("low").toString().split(" ").at(1);
        s=s.left(s.length()-1);
        mDay[i+1].low = s;
        mDay[i+1].notice = objForecast.value("notice").toString();
    }

    //解析今天的数据
    mToday.ganmao = objData.value("ganmao").toString();
    mToday.pm25 = objData.value("ganmao").toDouble();
    mToday.quality = objData.value("quality").toString();
    mToday.high = mDay[1].high;
    mToday.low = mDay[1].low;
    mToday.notice = mDay[1].notice;
}

void MainWindow::initUi()
{
    ui->search->setIcon(QIcon(":/res/search.png"));
    ui->refresh->setIcon(QIcon(":/res/refersh.png"));
    ui->download->setIcon(QIcon(":/res/download.png"));
    ui->city->setText(mToday.city);
    ui->wendu->setText(mToday.wendu+"°");
    ui->type->setText(mToday.type);
    ui->date->setText(QDateTime::fromString(mToday.date,"yyyyMMdd").toString("yyyy-MM-dd"));
    QString range = mToday.low+" ~ "+mToday.high;
    ui->range->setText(range);
    QString path = mTypeMap.value(mToday.type);
    QPixmap pixmap(path); // 加载图片到QPixmap对象
    ui->typeImage->setPixmap(pixmap.scaled(QSize(140,140))); // 将pixmap设置为QLabel的内容，即显示图片
    ui->ganmao->setText(mToday.ganmao);
    QPixmap wind(":/res/wind.png");
    ui->im1->setPixmap(wind.scaled(QSize(51,51)));
    ui->wind->setText(mToday.fx+mToday.fl);
    QPixmap pm(":/res/pm25.png");
    ui->im2->setPixmap(pm.scaled(QSize(51,51)));
    //qDebug()<<mToday.pm25;
    ui->pm25->setText("PM2.5："+QString::number(mToday.pm25));
    QPixmap shidu(":/res/humidity.png");
    ui->im3->setPixmap(shidu.scaled(QSize(51,51)));
    ui->shidu->setText("湿度："+mToday.shidu);
    QPixmap qual(":/res/aqi.png");
    ui->im4->setPixmap(qual.scaled(QSize(51,51)));
    ui->quality->setText("空气质量："+mToday.quality);

    for(int i=0;i<labellist.size();i++)
    {
        labellist[i]->setText(QDateTime::fromString(mDay[i].date,"yyyy-MM-dd").toString("MM/dd"));
        QString path = mTypeMap.value(mDay[i].type);
        QPixmap pixmap(path);
        typelist[i]->setPixmap(pixmap.scaled(QSize(51,51)));
        if(mDay[i].aqi<=50) {
            qualitylist[i]->setText("优");
            QString styleSheet = "background:rgb(102, 152, 24);";
            qualitylist[i]->setStyleSheet(styleSheet);
        } else if (mDay[i].aqi>50&&mDay[i].aqi<=100) {
            qualitylist[i]->setText("良");
            QString styleSheet = "background:rgb(236, 169, 27);";
            qualitylist[i]->setStyleSheet(styleSheet);
        } else if(mDay[i].aqi>100&&mDay[i].aqi<=150) {
            qualitylist[i]->setText("轻度污染");
            QString styleSheet = "background:rgb(235, 94, 85);";
            qualitylist[i]->setStyleSheet(styleSheet);
        }else if(mDay[i].aqi>150&&mDay[i].aqi<=200) {
            qualitylist[i]->setText("中度污染");
            QString styleSheet = "background:rgb(234, 34, 21);";
            qualitylist[i]->setStyleSheet(styleSheet);
        }else if(mDay[i].aqi>200&&mDay[i].aqi<=300) {
            qualitylist[i]->setText("重度污染");
            QString styleSheet = "background:rgb(173, 15, 2);";
            qualitylist[i]->setStyleSheet(styleSheet);
        }else {
            qualitylist[i]->setText("严重污染");
            QString styleSheet = "background:112, 8, 1;";
            qualitylist[i]->setStyleSheet(styleSheet);
        }
        windlist[i]->setText(mDay[i].fx+mDay[i].fl);
    }
    ui->notice->setText(mToday.notice);
}

void MainWindow::initTm()
{
    mTypeMap.insert("暴雪",":/res/type/BaoXue.png");
    mTypeMap.insert("暴雨",":/res/type/BaoYu.png");
    mTypeMap.insert("暴雨到大暴雨",":/res/type/BaoYuDaoDaBaoYu.png");
    mTypeMap.insert("大暴雨",":/res/type/DaBaoYu.png");
    mTypeMap.insert("大暴雨特大暴雨",":/res/type/DaBaoYuDaoTeDaBaoYu.png");
    mTypeMap.insert("大到暴雪",":/res/type/DaDaoBaoXue.png");
    mTypeMap.insert("大到暴雨",":/res/type/DaDaoBaoYu.png");
    mTypeMap.insert("大雪",":/res/type/DaXue.png");
    mTypeMap.insert("大雨",":/res/type/DaYu.png");
    mTypeMap.insert("冻雨",":/res/type/DongYu.png");
    mTypeMap.insert("多云",":/res/type/DuoYun.png");
    mTypeMap.insert("浮尘",":/res/type/FuChen.png");
    mTypeMap.insert("雷阵雨",":/res/type/LeiZhenYu.png");
    mTypeMap.insert("雷阵雨伴有冰雹",":/res/type/LeiZhenYuBanYouBingBao.png");
    mTypeMap.insert("霾",":/res/type/Mai.png");
    mTypeMap.insert("强沙尘暴",":/res/type/QiangShaChenBao.png");
    mTypeMap.insert("晴",":/res/type/Qing.png");
    mTypeMap.insert("沙尘暴",":/res/type/ShaChenBao.png");
    mTypeMap.insert("特大暴雨",":/res/type/TeDaBaoYu.png");
    mTypeMap.insert("雾",":/res/type/Wu.png");
    mTypeMap.insert("小到中雪",":/res/type/XiaoDaoZhongXue.png");
    mTypeMap.insert("小到中雨",":/res/type/XiaoDaoZhongYu.png");
    mTypeMap.insert("小雪",":/res/type/XiaoXue.png");
    mTypeMap.insert("小雨",":/res/type/XiaoYu.png");
    mTypeMap.insert("雪",":/res/type/Xue.png");
    mTypeMap.insert("扬沙",":/res/type/YangSha.png");
    mTypeMap.insert("阴",":/res/type/Yin.png");
    mTypeMap.insert("雨",":/res/type/Yu.png");
    mTypeMap.insert("雨夹雪",":/res/type/YuJiaXue.png");
    mTypeMap.insert("阵雪",":/res/type/ZhenXue.png");
    mTypeMap.insert("阵雨",":/res/type/ZhenYu.png");
    mTypeMap.insert("中到大雪",":/res/type/ZhongDaoDaXue.png");
    mTypeMap.insert("中到大雨",":/res/type/ZhongDaoDaYu.png");
    mTypeMap.insert("中雪",":/res/type/ZhongXue.png");
    mTypeMap.insert("中雨",":/res/type/ZhongYu.png");
}

void MainWindow::initVector()
{
    labellist.append(ui->day1);
    labellist.append(ui->day2);
    labellist.append(ui->day3);
    labellist.append(ui->day4);
    labellist.append(ui->day5);
    labellist.append(ui->day6);

    typelist.append(ui->img1);
    typelist.append(ui->img2);
    typelist.append(ui->img3);
    typelist.append(ui->img4);
    typelist.append(ui->img5);
    typelist.append(ui->img6);

    qualitylist.append(ui->q1);
    qualitylist.append(ui->q2);
    qualitylist.append(ui->q3);
    qualitylist.append(ui->q4);
    qualitylist.append(ui->q5);
    qualitylist.append(ui->q6);

    windlist.append(ui->w1);
    windlist.append(ui->w2);
    windlist.append(ui->w3);
    windlist.append(ui->w4);
    windlist.append(ui->w5);
    windlist.append(ui->w6);
}


void MainWindow::on_refresh_clicked()
{
    this->getWeatherInfo(mCityName);
    qDebug()<<"页面已刷新";
}


void MainWindow::on_search_clicked()
{
    mCityName = ui->lineEdit->text();
    getWeatherInfo(ui->lineEdit->text());
}


void MainWindow::on_download_clicked()
{
    QString filePath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "Text files (*.txt);;All files (*.*)");; // 文件保存路径

    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //使用QTextStream类构造了一个文件流stream，将其与文件对象相关联。
        //通过流操作符<<将需要写入的数据写入文件流。
        QTextStream stream(&file);
        QString data = mToday.city+" "+mToday.date+" "+mToday.type+" "+mToday.wendu+" "+mToday.shidu;
        stream << data; // 写入数据

        file.close(); // 关闭文件
        qDebug() << "File saved";
    } else {
        qDebug() << "Failed to save file";
    }
}

