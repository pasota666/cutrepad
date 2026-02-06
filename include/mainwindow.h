#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "configmanager.h"
#include "codeeditor.h"
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:    
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    //void on_actionClose_Tab_triggered();
    void on_actionClose_File_triggered();
    void updateStatusBar();
    void updateCursorPosition();

    void on_actionSave_triggered();

    void on_actionSave_As_triggered();

    void on_actionSave_All_triggered();

private:
    // Variables
    Ui::MainWindow *ui;
    ConfigManager config;
    QPushButton *tabAddButton = nullptr;
    QLabel *m_fileTypeLabel;
    QLabel *m_encodingLabel;
    QLabel *m_cursorPositionLabel;
    QLabel *m_modificationLabel;

    // Funciones
    void print(const QString &msg, const QString &type = "info");
    void openFile(const QString &filePath);
    void openNewFile();
    void closeFile(int index);
    void markFileAsModified(bool modified);
    bool saveFileToDisk(CodeEditor *editor, const QString &path);
    CodeEditor* currentEditor() const;
};
#endif // MAINWINDOW_H
