#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "appconfig.h"
#include <QDateTime>
#include <QScrollBar>

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
    imprimirConsola(AppConfig::APP_NAME + " " + AppConfig::VERSION + " by " + AppConfig::ORGANIZATION);
    imprimirConsola(tr("Application started"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_triggered()
{

}


void MainWindow::on_actionOpen_triggered()
{

}

void MainWindow::imprimirConsola(const QString &mensaje, const QString &tipo) {
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

