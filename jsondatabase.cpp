#include "jsondatabase.h"
#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

QJsonArray get_lesson_videos_json_arr(const QPointer<Lesson>& lesson) {
    QJsonArray v_arr;
    for (const auto &video: lesson->videos)
    {
        QJsonObject v_obj;
        v_obj["id"] = video->id;
        v_obj["name"] = video->name;
        v_obj["url"] = video->url;
        v_arr.append(v_obj);
    }
    return v_arr;
}


QJsonArray combine_videos_of_two_lessons(const QPointer<Lesson>& l1, const QPointer<Lesson>& l2) {
    QJsonArray v_arr;
    for (const auto &video: l1->videos) {
        auto it = std::find_if(v_arr.begin(), v_arr.end(),
                               [video] (const QJsonValue& val) { return val["url"].toString() == video->url; });
        if (it != v_arr.end()) {
            // already added to array
            continue;
        }
        QJsonObject v_obj;
        v_obj["id"] = video->id;
        v_obj["name"] = video->name;
        v_obj["url"] = video->url;
        v_arr.append(v_obj);
    }
    for (const auto &video: l2->videos) {
        auto it = std::find_if(v_arr.begin(), v_arr.end(),
                               [video] (const QJsonValue& val) { return val["url"].toString() == video->url; });
        if (it != v_arr.end()) {
            // already added to array
            continue;
        }
        QJsonObject v_obj;
        v_obj["id"] = video->id;
        v_obj["name"] = video->name;
        v_obj["url"] = video->url;
        v_arr.append(v_obj);
    }

    return v_arr;
}


void JsonDatabase::save_lessons(const QList<QPointer<Lesson>> &lessons)
{
    if (!lessons.isEmpty())
    {
        QList<QPointer<Lesson>> lessons_in_db = JsonDatabase::retrieve_lessons();
        QFile output_file(JsonDatabase::database_name());
        if (output_file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            QJsonArray arr;
            for (const auto &lesson: lessons)
            {
                QPointer<Lesson> _lesson;
                QJsonObject obj;
                auto it = std::find_if(lessons_in_db.begin(), lessons_in_db.end(),
                                       [lesson] (const QPointer<Lesson>& _lesson)
                    {
                        return _lesson->name == lesson->name && _lesson->teacher == lesson->teacher;
                    }
                );
                if (it != lessons_in_db.end()) {
                    // lesson already in db, merge video lists
                    _lesson = *it;
                    obj["id"] = lesson->id;
                    obj["teacher"] = lesson->teacher;
                    obj["name"] = lesson->name;
                    obj["videos"] = combine_videos_of_two_lessons(lesson, _lesson);
                    arr.append(obj);
                    continue;
                } else _lesson = lesson;
                obj["id"] = lesson->id;
                obj["teacher"] = lesson->teacher;
                obj["name"] = lesson->name;
                obj["videos"] = get_lesson_videos_json_arr(lesson);
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
    qDebug() << "database location: " << JsonDatabase::database_name();
    QFile inputFile(JsonDatabase::database_name());
    QList<QPointer<Lesson>> lessons;

    if (!inputFile.exists()) {
        qDebug() << "Writing from: " << JsonDatabase::default_json_name() << " to: " << JsonDatabase::database_name();
        QFile defaultFile(JsonDatabase::default_json_name());
        defaultFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QByteArray defaultArr = defaultFile.readAll();
        if (inputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Wrote to " << JsonDatabase::database_name();
            inputFile.write(defaultArr);
        } else {
            qDebug() << "Unable to write to: " << JsonDatabase::database_name();
        }
        defaultFile.close();
        inputFile.close();
    }

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
