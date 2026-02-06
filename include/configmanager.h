#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QSettings>
#include <QStringList>
#include <QList>

struct Language {
    int order;
    QString name;
    QStringList extensions;
    QString charset;
};

class ConfigManager {
public:

    ConfigManager();
    ~ConfigManager();

    QString getLastPath() const;
    void setLastPath(const QString &path);

    QString getOpenFileFilter();
    QList<Language> getLanguages() const;

    void setLastOpenFileExtension(const QString &ext);
    QString getLastOpenFileExtension() const;

    QString getAppLanguage() const;
    void setAppLanguage(const QString &lang);

private:
    QSettings *settings;
    void createDefaultConfigFile();
    void loadLanguages();
    QList<Language> languages;
};

#endif // CONFIGMANAGER_H
