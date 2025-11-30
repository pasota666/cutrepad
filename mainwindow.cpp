#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "appconfig.h"
#include <QDateTime>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Carga la UI
    ui->setupUi(this);

    // Organiza la parte superior (tabAux y tabCode)
    ui->tabAux->setMinimumWidth(50); // Establece los tamaños mínimos
    ui->tabCode->setMinimumWidth(50);
    ui->tabCode->setTabsClosable(true); // Las tabs de código pueden cerrarse
    ui->splitVertical->setChildrenCollapsible(false); // No desaparecen los widgets si se estira mucho
    ui->splitVertical->setStretchFactor(0, 0); // El slot 0 (izquierda) no cambia su tamaño (0)
    ui->splitVertical->setStretchFactor(1, 1); // El slot 1 (derecha) absorbe todo el tamaño extra (1)
    ui->splitVertical->setSizes({250, 774}); // Tamaños por defecto (suman 1024)

    // Organiza la parte inferior (tabConsole)
    ui->tabConsole->setMinimumHeight(50); // Establece los tamaños mínimos
    ui->splitHorizontal->setMinimumHeight(50);
    ui->splitHorizontal->setChildrenCollapsible(false); // No desaparecen los widgets si se estira mucho
    ui->splitHorizontal->setStretchFactor(0, 1); // El slot 0 (arriba) absorbe todo el tamaño extra (1)
    ui->splitHorizontal->setStretchFactor(1, 0); // El slot 1 (abajo) no cambia de tamaño (0)
    //ui->splitHorizontal->setSizes({376, 200}); // Tamaños por defecto (suman 576)
    ui->splitHorizontal->setSizes({488, 280}); // Tamaños por defecto (suman 768)

    setWindowTitle(AppConfig::APP_NAME + " " + AppConfig::VERSION);
    print(AppConfig::APP_NAME + " " + AppConfig::VERSION + " by " + AppConfig::ORGANIZATION);
    print(tr("Application started"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************************************************************
 *
 *  ACCIONES DE BOTONES
 *
 ************************************************************************************************/

void MainWindow::on_actionNew_triggered()
{
    openNewFile();
}


void MainWindow::on_actionOpen_triggered()
{        
    // Muestra el dialogo para abrir el archivo
    QString filter = "Prg File (*.prg) ;; Text File (*.txt) ;; All Files (*.*)";
    QString file = QFileDialog::getOpenFileName(this, tr("Open file"), config.getLastPath(), config.getOpenFileFilter());
    if (file.isEmpty()) {
        return;
    }
    // Actualizar lastPath antes de abrir el archivo
    QFileInfo fileInfo(file);
    config.setLastPath(fileInfo.absolutePath());
    // Abre el archivo
    openFile(file);
}

/************************************************************************************************
 *
 *  CONSOLA
 *
 ************************************************************************************************/

void MainWindow::print(const QString &mensaje, const QString &tipo) {
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    QString texto = timestamp + mensaje;

    QTextCharFormat format;
    if (tipo == "error") {
        format.setForeground(Qt::red);
    } else if (tipo == "success") {
        format.setForeground(Qt::green);
    } else {
        format.setForeground(Qt::white);
    }

    QTextCursor cursor(ui->txtStatus->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(texto + "\n", format);

    ui->txtStatus->verticalScrollBar()->setValue(ui->txtStatus->verticalScrollBar()->maximum());
}

/************************************************************************************************
 *
 *  OPERACIONES SOBRE ARCHIVOS
 *
 ************************************************************************************************/

void MainWindow::openFile(const QString &filePath) {

    // TO-DO: Verificar que el archivo no está abierto ya

    // Intentamos abrir el archivo
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        print("Error al abrir: " + filePath, "error");
        return;
    }

}

void MainWindow::openNewFile(){


}

