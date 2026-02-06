#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ConfigManager config;
    QString userLang = config.getAppLanguage();
    QTranslator translator;
    bool loaded = false;

    if (userLang == "auto") {
        // Lógica original de detección por sistema
        const QStringList uiLanguages = QLocale::system().uiLanguages();
        for (const QString &locale : uiLanguages) {
            const QString baseName = "cutrepad_" + QLocale(locale).name();
            if (translator.load(":/i18n/" + baseName)) {
                loaded = true;
                break;
            }
        }
    } else {
        // Forzamos el idioma definido por el usuario (ej: "es" o "en")
        // Esto busca archivos como :/i18n/cutrepad_es.qm
        if (translator.load(":/i18n/cutrepad_" + userLang)) {
            loaded = true;
        }
    }

    if (loaded) {
        a.installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
