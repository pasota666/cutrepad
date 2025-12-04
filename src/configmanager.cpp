#include "configmanager.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QDebug>

// ======================================================
// Constructor
// ======================================================
ConfigManager::ConfigManager() {
    // Ruta del archivo de configuración en el directorio del ejecutable
    QString configFile = QDir(QCoreApplication::applicationDirPath()).filePath("cutrepad.ini");

    // Crear settings y archivo por defecto si no existe
    if (!QFileInfo::exists(configFile)) {
        settings = new QSettings(configFile, QSettings::IniFormat);
        createDefaultConfigFile();
    } else {
        settings = new QSettings(configFile, QSettings::IniFormat);
    }

    // Cargar lenguajes desde /languages en la carpeta del ejecutable
    loadLanguages();
}

// ======================================================
// Destructor
// ======================================================
ConfigManager::~ConfigManager() {
    delete settings; // Elimina el puntero a settings
}

// ======================================================
// Crea un archivo de configuración por defecto
// ======================================================
void ConfigManager::createDefaultConfigFile() {

    // Categoría App
    settings->beginGroup("App");
    settings->setValue("lastPath", QDir::homePath());
    settings->endGroup();

    // Categoría SyntaxHighlight
    settings->beginGroup("SyntaxHighlight");
    settings->setValue("keywordColor", "#4b99d6");
    settings->endGroup();

    settings->sync();
}

// ======================================================
// Obtiene la última ruta que se usó para abrir un archivo
// ======================================================
QString ConfigManager::getLastPath() const {
    return settings->value("App/lastPath", QDir::homePath()).toString();
}

// ======================================================
// Establece la última ruta
// ======================================================
void ConfigManager::setLastPath(const QString &path) {
    settings->setValue("App/lastPath", path);
}

// ======================================================
// Carga los lenguajes de los archivos .ini de /languages
// ======================================================
void ConfigManager::loadLanguages() {
    languages.clear();
    QString langPath = QDir(QCoreApplication::applicationDirPath()).filePath("languages");
    QDir langDir(langPath);
    if (!langDir.exists()) {
        qWarning() << "[ConfigManager] La carpeta de lenguajes no existe:" << langPath;
        return;
    }
    QStringList files = langDir.entryList(QStringList() << "*.ini", QDir::Files);

    for (auto it = files.cbegin(); it != files.cend(); ++it) {
        const QString &fileName = *it;
        QString fullPath = langDir.filePath(fileName);
        QSettings s(fullPath, QSettings::IniFormat);

        Language lang;
        lang.order = s.value("Language/order", 99).toInt();
        lang.name = s.value("Language/languageName").toString();

        QString exts = s.value("Language/extensions").toString();
        lang.extensions = exts.split(",", Qt::SkipEmptyParts);

        for (QString &e : lang.extensions)
            e = e.trimmed();

        if (lang.name.isEmpty())
            lang.name = fileName;

        languages.append(lang);
    }

    // Ordenar por order ascendente
    std::sort(languages.begin(), languages.end(),
              [](const Language &a, const Language &b) {
                  return a.order < b.order;
              }
              );
}

// ======================================================
// Devuelve copia de la lista de lenguajes cargados
// ======================================================
QList<Language> ConfigManager::getLanguages() const {
    return languages;
}

// ======================================================
// Genera filtro para QFileDialog
// ======================================================
QString ConfigManager::getOpenFileFilter() {
    QStringList filters;

    for (auto it = languages.cbegin(); it != languages.cend(); ++it) {
        const Language &lang = *it;

        if (lang.extensions.isEmpty())
            continue;

        QStringList extsWithDot;
        for (const QString &ext : lang.extensions)
            extsWithDot.append("*." + ext);

        QString filter = QString("%1 File (%2)").arg(lang.name, extsWithDot.join(" "));
        filters.append(filter);
    }

    filters.append("Text File (*.txt)");
    filters.append("All Files (*.*)");

    return filters.join(" ;; ");
}

// ======================================================
// Almacena última extensión usada en el diálogo OpenFile
// ======================================================
void ConfigManager::setLastOpenFileExtension(const QString &ext) {
    settings->setValue("Dialogs/LastOpenExtension", ext);
}

// ======================================================
// Recupera última extensión usada en el diálogo Openfile
// ======================================================
QString ConfigManager::getLastOpenFileExtension() const {
    return settings->value("Dialogs/LastOpenExtension", "").toString();
}
