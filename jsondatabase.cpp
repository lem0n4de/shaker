#include "jsondatabase.h"
#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

void JsonDatabase::save_lessons(const QList<QPointer<Lesson>> &lessons)
{
    if (!lessons.isEmpty())
    {
        QFile output_file(JsonDatabase::database_name());
        if (output_file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            QJsonArray arr;
            for (const auto &lesson: lessons)
            {
                QJsonObject obj;
                obj["id"] = lesson->id;
                obj["teacher"] = lesson->teacher;
                obj["name"] = lesson->name;
                QJsonArray v_arr;
                for (const auto &video: lesson->videos)
                {
                    QJsonObject v_obj;
                    v_obj["id"] = video->id;
                    v_obj["name"] = video->name;
                    v_obj["url"] = video->url;
                    v_arr.append(v_obj);
                }
                obj["videos"] = v_arr;
                arr.append(obj);
            }
            QJsonDocument doc;
            doc.setArray(arr);
            auto rv = output_file.write(doc.toJson());
            output_file.close();
            if (rv == -1)
            {
                qCritical() << "error while saving";
                return;
            }
            qDebug() << "saved file";
        } else
        {
            qDebug() << "couldn't open file";
        }
    } else
    {
        qDebug() << "empty lessons";
    }
}

QList<QPointer<Lesson> > JsonDatabase::retrieve_lessons()
{
    QByteArray json_byte;
    QFile inputFile(JsonDatabase::database_name());
    QList<QPointer<Lesson>> lessons;

    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        json_byte = inputFile.readAll();
        QJsonDocument doc(QJsonDocument::fromJson(json_byte));
        if (!doc.isEmpty() && doc.isArray())
        {
            QJsonArray arr = doc.array();
            for (auto item: arr)
            {
                QJsonObject element = item.toObject();
                QString id = element["id"].toString();
                QString teacher = element["teacher"].toString();
                QString name = element["name"].toString();
                // TODO Add videos to lesson->videos
                QPointer<Lesson> l = new Lesson(id, name, teacher);

                QJsonArray videos_obj = element["videos"].toArray();
                for (auto v_item: videos_obj)
                {
                    QJsonObject v_elem = v_item.toObject();
                    QString v_id = v_elem["id"].toString();
                    QString v_name = v_elem["name"].toString();
                    QString v_url = v_elem["url"].toString();
                    QPointer<Video> v = new Video(v_id, v_name, l->teacher, v_url);
                    v->lesson_name = l->name;
                    l->videos.push_back(v);
                }
                lessons.push_back(l);
            }
        }
    } else
    {
        throw std::invalid_argument("lessons.json not found");
    }
    inputFile.close();
    return lessons;
}

