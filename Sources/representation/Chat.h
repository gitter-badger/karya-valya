#pragma once

#include <vector>
#include <string>

#include "core/objects/Mob.h"

#include <QTextBrowser>
#include <QObject>

class Chat: public QObject
{
    Q_OBJECT
public:
    static bool IsOOCMessage(const std::string& text);
    Chat();
    void PostTextFor(const std::string& str, id_ptr_on<IOnMapObject> owner);

    void PostText(const std::string& str);

    void PostOOCText(const std::string& who, const std::string& str);

    void PostSimpleText(const std::string& str, size_t tile_id);
    void PostDamage(const std::string& by, const std::string& who, const std::string& object, size_t tile_id);
    void PostWords(const std::string& who, const std::string& text, size_t tile_id);
signals:
    void insertHtmlIntoChat(QString html);
};

void SetLogToFile();

Chat& GetChat();
void SetChat(Chat* chat);

/*void SetTextBrowser(QTextBrowser* tb);
QTextBrowser& GetTextBrowser();*/
