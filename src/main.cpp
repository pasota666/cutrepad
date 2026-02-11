#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // Crea el objeto aplicación
    QApplication a(argc, argv);

    // Crea el objeto config y translator
    ConfigManager config;
    QString userLang = config.getAppLanguage();
    QTranslator translator;
    bool loaded = false;

    // Si el lenguaje está definido como "auto" en el ini
    if (userLang == "auto") {
        // Obtiene el lenguaje del sistema
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = "cutrepad_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                loaded = true;
                break;
            }
        }
    } else {
        // Forzamos el idioma definido por el usuario (ej: "es_ES" o "en")
        // Esto busca archivos en la carpeta translations. (Ej. /i18n/cutrepad_es_ES.qm)
        if (translator.load(":/i18n/cutrepad_" + userLang)) {
            loaded = true;
        }
    }

    // Si se ha podido cargar una traducción, la aplica al programa
    if (loaded) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
