/*

    TO-DO:
    - Crear en cutrepad.ini una variable "language" [auto|en|es]
    - Crear en cutrepad.ini una variable "fontSize"
    - Crear en cutrepad.ini una variable "charset"
    - Debería chequear los ini para ver si están correctos y si no, no lo carga.

    HECHO:
    - [OK] Debería guardar en el dialogo de abrir archivo la última extensión usada.

    CARACTERÍSTICAS AVANZADAS:
    - Autocompletado de funciones integradas del lenguaje, con ayuda integrada visible (caja funciones y overloads)
    - Autocompletado de funciones declaradas y existentes (incluso en archivos de biblioteca).
    - Acceso a herramientas externas.

*/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appconfig.h"
#include "codeeditor.h"
#include "myeventfilter.h"
#include <QDateTime>
#include <QScrollBar>
#include <QFile>
#include <QFileDialog>
#include <QTabBar>
#include <QRegularExpression>

/************************************************************************************************
 *
 *  VENTANA PRINCIPAL
 *
 ************************************************************************************************/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Carga la UI
    ui->setupUi(this);            

    // Organiza la parte superior (tabAux y tabCode)
    ui->tabAux->setMinimumWidth(50); // Establece los tamaños mínimos
    ui->tabCode->setMinimumWidth(50);    
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
    ui->splitHorizontal->setSizes({376, 200}); // Tamaños por defecto (suman 768)

    // Crea el filtro personalizado de eventos
    myEventFilter *filter = new myEventFilter(this);
    ui->tabCode->installEventFilter(filter);
    connect(filter, &myEventFilter::emptyAreaClicked, this, &MainWindow::openNewFile); // Click área vacía de tabs para crear nuevo archivo

    // Cerrado de pestañas en tabCode
    ui->tabCode->setTabsClosable(true); // Las tabs de código pueden cerrarse
    connect(ui->tabCode, &QTabWidget::tabCloseRequested, this, &MainWindow::closeFile);

    // Para finalizar, imprime información de la aplicación
    setWindowTitle(AppConfig::APP_NAME + " " + AppConfig::VERSION);
    print(AppConfig::APP_NAME + " " + AppConfig::VERSION + " by " + AppConfig::ORGANIZATION);
    print(tr("Application started."));

    // Si no hay pestañas abiertas, crea una nueva
    if (ui->tabCode->count() == 0) {
        openNewFile();
    }

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

// ======================================================
// Nuevo Archivo
// ======================================================
void MainWindow::on_actionNew_triggered()
{
    openNewFile();
}

// ======================================================
// Abrir Archivo ...
// ======================================================
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>

void MainWindow::on_actionOpen_triggered()
{
    // Obtenemos los filtros
    QString filters = config.getOpenFileFilter();

    // Obtenemos la última extensión seleccionada
    QString lastExt = config.getLastOpenFileExtension();

    // Abre el diálogo
    QFileDialog dialog(this, tr("Open File"), config.getLastPath());
    dialog.setNameFilter(filters);
    dialog.setFileMode(QFileDialog::ExistingFile);

    // Seleccionamos el último filtro usado
    QStringList filterList = filters.split(" ;; ");
    for (QStringList::const_iterator it = filterList.cbegin(); it != filterList.cend(); ++it) {
        const QString &f = *it;
        if (f.contains("*." + lastExt)) {
            dialog.selectNameFilter(f);
            break;
        }
    }

    // Si el usuario cancela, el proceso termina
    if (dialog.exec() != QDialog::Accepted)
        return;

    // Obtiene los archivos seleccionados
    QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.isEmpty())
        return;

    // Obtiene la ruta del archivo
    QString filePath = selectedFiles.first();

    // Guardar la última ruta
    QFileInfo fileInfo(filePath);
    config.setLastPath(fileInfo.absolutePath());

    // Guardar la extensión del filtro seleccionado
    QString selectedFilter = dialog.selectedNameFilter();
    QRegularExpression re("\\*\\.(\\w+)");
    QRegularExpressionMatch match = re.match(selectedFilter);
    if (match.hasMatch()) {
        QString ext = match.captured(1);
        config.setLastOpenFileExtension(ext);
    }

    // Abrir el archivo
    openFile(filePath);
}

// ======================================================
// Cerrar Archivo
// ======================================================
void MainWindow::on_actionClose_File_triggered()
{
    closeFile(ui->tabCode->currentIndex());
}

/************************************************************************************************
 *
 *  CONSOLA
 *
 ************************************************************************************************/

// ======================================================
// Imprime un mensaje en la consola
// ======================================================
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

// ======================================================
// Abre un archivo
// ======================================================
void MainWindow::openFile(const QString &filePath) {

    // En caso de que el archivo ya está abierto, va a esa pestaña
    for (int i = 0; i < ui->tabCode->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(ui->tabCode->widget(i))) {
            QString path = editor->property("filePath").toString();
            if (path == filePath) {
                ui->tabCode->setCurrentIndex(i);
                return;
            }
        }
    }

    // Si solo hay una pestaña y es un archivo nuevo sin modificar, la cierra
    if (ui->tabCode->count() == 1) {
        CodeEditor *editor = qobject_cast<CodeEditor *>(ui->tabCode->widget(0));
        if (editor) {
            QString path = editor->property("filePath").toString();
            if (path.isEmpty() && !editor->document()->isModified()) {
                closeFile(0);
            }
        }
    }

    // Intentamos abrir el archivo
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        print(tr("Can't open: ") + filePath, "error");
        return;
    } else {
        print(tr("File open: ") + filePath);
    }

    // Obtiene el path del archivo
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.fileName();

    // Lee el archivo de texto
    QString text = file.readAll();
    file.close();

    // Crea un editor
    CodeEditor *editor = new CodeEditor();
    editor->setPlainText(text);
    editor->setProperty("filePath", filePath);

    // Carga el editor en una pestaña
    ui->tabCode->addTab(editor, baseName);
    ui->tabCode->setCurrentWidget(editor);

}

// ======================================================
// Crea un archivo nuevo
// ======================================================
void MainWindow::openNewFile(){

    // Comprueba que no existe un archivo creado sin modificar antes de crear uno nuevo
    for (int i = 0; i < ui->tabCode->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor *>(ui->tabCode->widget(i));
        if (!editor)
            continue;
        // Archivo sin ruta → archivo nuevo
        QString path = editor->property("filePath").toString();
        // Solo consideramos archivos nuevos
        if (path.isEmpty()) {
            // Descartamos si está modificado (tiene *)
            QString title = ui->tabCode->tabText(i);
            if (!title.endsWith("*")) {
                // Seleccionarlo en vez de crear otro
                ui->tabCode->setCurrentIndex(i);
                print(tr("A blank file is already open."));
                return;
            }
        }
    }

    // Crea un editor de código sin ruta asignada
    CodeEditor *editor = new CodeEditor();
    editor->setProperty("filePath", "");

    // Conector para cuando se modifica el archivo
    connect(editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::markFileAsModified);

    // Asocia el editor a una pestaña
    ui->tabCode->addTab(editor, tr("Untitled"));
    ui->tabCode->setCurrentWidget(editor);
    print(tr("New file created."));

}

// ======================================================
// Cierra un archivo
// ======================================================
void MainWindow::closeFile(int index) {
    // Selecciona la pestaña
    QWidget* widget = ui->tabCode->widget(index);
    if(!widget) return;
    // Eliminar la pestaña
    ui->tabCode->removeTab(index);
    delete widget;
}

// ======================================================
// Modifica un archivo
// ======================================================
void MainWindow::markFileAsModified(bool modified) {

    // Obtenemos el objeto documento que manda el emisor
    QTextDocument *doc = qobject_cast<QTextDocument *>(sender());
    if (!doc)
        return;

    // Recorremos las pestañas hasta encontrar la pestaña del documento recibido
    for (int i = 0; i < ui->tabCode->count(); ++i) {
        CodeEditor *editor = qobject_cast<CodeEditor *>(ui->tabCode->widget(i));
        if (editor && editor->document() == doc) {
            QString title = ui->tabCode->tabText(i);
            // Cuando guardamos (modified=false) quitamos el *
            if (title.endsWith("*"))
                title.chop(1);
            // Cuando modificamos (modified=true) añadimos el *
            if (modified)
                title += "*";
            ui->tabCode->setTabText(i, title);
            break;
        }
    }
}
