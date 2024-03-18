#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include "qjsonarray.h"
#include <QString>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>


class WeatherTool {
private:
    static QMap<QString,QString> mCityMap;
    static void initCityMap(QString filePath) {
        //读取文件
        //QString filePath="D:\Download\citycode.json";
        QFile file(filePath);
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray json = file.readAll();
        file.close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(json,&err);
        //qDebug() << "文件读取成功";
        if(filePath == "D:/Download/citycode.json"){
            if(err.error != QJsonParseError::NoError) {
                return;
            }
            QJsonArray jArray = doc.array();
            for(int i = 0;i<3241;i++) {
                QJsonObject obj = jArray[i].toObject();
                QString city_name = obj.value("cityname").toString();
                QString city_code = obj.value("adcode").toString();
                if(city_code.size()>0) {
                    mCityMap.insert(city_name,city_code);
                }
            }
            qDebug() << "构建完成";
        } else if(filePath == "D:/Download/citycode-2019-08-23.json")
        {
            //解析并写入map
            if(err.error != QJsonParseError::NoError) {
                return;
            }
            QJsonArray jArray = doc.array();
            for(int i = 0;i<3408;i++) {
                QJsonObject obj = jArray[i].toObject();
                QString city_name = obj.value("city_name").toString();
                QString city_code = obj.value("city_code").toString();
                if(city_code.size()>0) {
                    mCityMap.insert(city_name,city_code);
                }
            }
            qDebug() << "构建完成";
        }
    }
public:
    static QString getCityCode(QString cityName,QString filename) {
        if(mCityMap.isEmpty()) {
            initCityMap(filename);
        }

        QMap<QString,QString>::iterator it = mCityMap.find(cityName);
        if(it==mCityMap.end()) {
            it = mCityMap.find(cityName+"市");
        }
        if(it!=mCityMap.end()){
            return it.value();
        }

        return "";
    }
};

QMap<QString,QString> WeatherTool::mCityMap ={};

#endif // WEATHERTOOL_H
