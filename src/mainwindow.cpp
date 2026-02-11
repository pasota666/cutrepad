/*

    // ======================================================
    // TO-DO
    // ======================================================
    - GESTIÓN DE ARCHIVOS
      - Gestionar apertura de los archivos que estaban abiertos al iniciar.
      - ¿Cambiar los updateStatusBar por connect?

    - COMPILACIÓN
        - Definir rutas del compilador en el ini del lenguaje.

    - SIMBOLOS
        - Definir la expresión regular de los símbolos en el lenguaje

    - CONFIGURACIÓN
        - Crear ventanas para configurar los ini y otros aspectos del programa.

    - ESTÉTICA
        - Coloreado de sintaxis.
        - Fuente.
        - Crear en cutrepad.ini una variable "fontSize"

    - Gestión de archivos ini:
        - Mejorar el ini recién creado si no existe.
        - Debería chequear los ini para ver si están correctos y si no, no lo carga.

    // ======================================================
    // HECHO
    // ======================================================
    - [OK] Guardar todo.
    - [OK] Gestionar cerrado de pestañas.
    - [OK] Gestionar salida del programa cuando se pulsa cerrar.
    - [OK] Tener en cuenta el charset.
    - [OK] Guardar, guardar como
    - [OK] Testear en Gemix que guarda y abre en el charset adecuado.
    - [OK] Crear en cutrepad.ini una variable "language" [auto|en|es_ES]
    - [OK] Debería guardar en el dialogo de abrir archivo la última extensión usada.
    - [OK] Crear en cutrepad.ini una variable "charset" para que Gemix guarde en su charset

    // ======================================================
    // BUGS
    // ======================================================
    - [FIXED] "Guardado" no se actualiza en cuanto se escribe en la statusBar.
    - [FIXED] No se ve en la status bar la codificación.

    // ======================================================
    // IDEAS
    // ======================================================
    - Autocompletado de funciones integradas del lenguaje, con ayuda integrada visible (caja funciones y overloads).
    - Autocompletado de funciones declaradas y existentes (incluso en archivos de biblioteca).

    // ======================================================
    // HOW TO
    // ======================================================

    COMO USAR LA TRADUCCIÓN:
    - Encerrar los literales que se quieran traducir con la función tr().
    - CMakeLists está configurado para que cuando se compile el proyecto, esos literales se añadan al archivo cutrepad_es_ES.ts.
    - En ese fichero, buscar los campos con type="unfinished" y cambiar "unfinished" por la traducción correspondiente.

    COMO AGREGAR EVENTOS:
    - Añadir la imagen a la carpeta "resources"
    - Abrir "Resources.qrc" en el editor.
    - Agregar la imagen del icono.
    - En el UI:
        - Crear la entrada en el menú para insertarlo en la lista de acciones.
        - Agregar el icono en el action editor.
        - Agregar "Goto Slot Triggered"
        - Asignar shortcuts.

    COMO SUBIR AL REPOSITORIO:
    - Commit: ALT+G, ALT+C o Tools -> Git -> Local Repository -> Commit
    - Select all y rellenar mensaje.
    - Tools -> Git -> Remote Repository -> Push
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
#include <QStringConverter>
#include <QTextCodec>
#include <QMessageBox>

/************************************************************************************************
 *
 *  VENTANA PRINCIPAL
 *
 ************************************************************************************************/

// ======================================================
// Constructor de la ventana principal
// ======================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Carga la UI
    ui->setupUi(this);

    //======================================================
    // Organiza los elementos de la UI
    //======================================================

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

    //======================================================
    // Configura la statusBar
    //======================================================

    // Crea los objetos etiquetas que van en la barra
    m_fileTypeLabel = new QLabel(this);
    m_encodingLabel = new QLabel(this);
    m_cursorPositionLabel = new QLabel(this);
    m_modificationLabel = new QLabel(this);

    // Añadir las etiquetas a la barra de estado
    ui->statusbar->addPermanentWidget(m_fileTypeLabel); // Extensión del archivo
    ui->statusbar->addPermanentWidget(m_encodingLabel); // El encoding con el que se guardará el fichero
    ui->statusbar->addPermanentWidget(m_cursorPositionLabel); // Posición de cursor
    ui->statusbar->addPermanentWidget(m_modificationLabel); // Si se ha modificado el archivo

    // Establecer tamaños mínimos
    m_fileTypeLabel->setMinimumWidth(80);
    m_encodingLabel->setMinimumWidth(100);
    m_cursorPositionLabel->setMinimumWidth(100);
    m_modificationLabel->setMinimumWidth(80);

    // Cada vez que el usuario cambie de pestaña actualiza la barra
    connect(ui->tabCode, &QTabWidget::currentChanged, this, &MainWindow::updateStatusBar);        

    //======================================================
    // Configuración de pestañas
    //======================================================

    // Crea el filtro personalizado de eventos para capturar el click de nueva pestaña
    myEventFilter *filter = new myEventFilter(this);
    ui->tabCode->installEventFilter(filter);
    connect(filter, &myEventFilter::emptyAreaClicked, this, &MainWindow::openNewFile); // Click área vacía de tabs para crear nuevo archivo

    // Cerrado de pestañas en tabCode, lo conecta a closeFile()
    ui->tabCode->setTabsClosable(true); // Las tabs de código pueden cerrarse
    connect(ui->tabCode, &QTabWidget::tabCloseRequested, this, &MainWindow::closeFile);

    // Si no hay pestañas abiertas, crea una nueva
    if (ui->tabCode->count() == 0) {
        openNewFile();
    }

    // Restaura las pestañas abiertas en la última sesión
    const QStringList filesToOpen = config.getOpenFiles();
    if (!filesToOpen.isEmpty()) {
        // Usamos std::as_const para evitar el "detach" del contenedor
        for (const QString &path : std::as_const(filesToOpen)) {
            if (QFile::exists(path)) { // Verificamos que el archivo siga ahí
                openFile(path);
            }
        }
        // Restauramos la pestaña activa
        ui->tabCode->setCurrentIndex(config.getActiveTab());
    }

    //======================================================
    // Configura información
    //======================================================

    // Pone el nombre de la aplicación
    setWindowTitle(AppConfig::APP_NAME + " " + AppConfig::VERSION);

    // Imprime arranque en consola
    print(AppConfig::APP_NAME + " " + AppConfig::VERSION + " by " + AppConfig::ORGANIZATION);
    print(tr("Application started."));

}

// ======================================================
// Destructor de la ventana principal
// ======================================================
MainWindow::~MainWindow()
{
    delete ui;
}

/************************************************************************************************
 *
 *  EVENTOS DEL PROGRAMA (on_action, update, etc)
 *
 ************************************************************************************************/

// ======================================================
// Acción nuevo archivo
// ======================================================
void MainWindow::on_actionNew_triggered()
{
    openNewFile();
}

// ======================================================
// Acción abrir archivo ...
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
// Acción cerrar archivo
// ======================================================
void MainWindow::on_actionClose_File_triggered()
{
    closeFile(ui->tabCode->currentIndex());
}

// ======================================================
// Acción actualizar barra de estado
// ======================================================
void MainWindow::updateStatusBar() {

    // Obtiene el editor actual
    CodeEditor* editor = currentEditor();

    // Si no hay editor (cerramos todas las pestañas), resetea la información
    if (!editor) {
        m_fileTypeLabel->setText(tr("No file"));
        m_encodingLabel->setText(tr("No encoding"));
        m_modificationLabel->setText("");
        updateCursorPosition();
        return;
    }

    // Actualizar tipo de archivo
    QString filePath = editor->property("filePath").toString();
    if (!filePath.isEmpty()) {
        // CASO: Archivo con ruta
        QFileInfo fileInfo(filePath);
        // Muestra extension
        QString ext = fileInfo.suffix().toLower();
        m_fileTypeLabel->setText(ext.toUpper());
        // Muestra charset
        QString charset = "UTF-8"; // Por defecto
        for (const Language &lang : config.getLanguages()) {
            if (lang.extensions.contains(ext)) {
                charset = lang.charset;
                break;
            }
        }
        m_encodingLabel->setText(charset);
        // Muestra modificación
        m_modificationLabel->setText(editor->document()->isModified() ? tr("Modified") : tr("Saved"));
    } else {
        // CASO: Archivo sin ruta (sin guardar)
        m_fileTypeLabel->setText(tr("None"));
        m_encodingLabel->setText(tr("No encoding")); // O "No encoding", como prefieras
        m_modificationLabel->setText(tr("New file"));
    }

    // Actualizar posición del cursor
    updateCursorPosition();

    // Conectar señal de cambio de cursor para actualización en tiempo real
    if (editor) {
        connect(editor, &CodeEditor::cursorPositionChanged,
                this, &MainWindow::updateCursorPosition, Qt::UniqueConnection);
    }

}

// ======================================================
// Acción actualizar posición del cursor
// ======================================================
void MainWindow::updateCursorPosition()
{
    if (CodeEditor* editor = currentEditor()) {
        QTextCursor cursor = editor->textCursor();
        int line = cursor.blockNumber() + 1;
        int column = cursor.positionInBlock() + 1;
        m_cursorPositionLabel->setText(QString("Ln %1, Col %2").arg(line).arg(column));
    } else {
        m_cursorPositionLabel->setText("Ln --, Col --");
    }
}

// ======================================================
// Acción guardar
// ======================================================
bool MainWindow::on_actionSave_triggered()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return false;

    QString filePath = editor->property("filePath").toString();

    if (filePath.isEmpty()) {
        // Si no tiene ruta, llamamos a "Guardar como" y devolvemos su resultado
        return on_actionSave_As_triggered();
    } else {
        // Si tiene ruta, guardamos y devolvemos si tuvo éxito
        return saveFileToDisk(editor, filePath);
    }
}
// ======================================================
// Acción guardar como
// ======================================================
bool MainWindow::on_actionSave_As_triggered()
{
    CodeEditor *editor = currentEditor();
    if (!editor) return false;

    // 1. Determinar la extensión que queremos preseleccionar
    QString currentPath = editor->property("filePath").toString();
    QString targetExt;

    if (currentPath.isEmpty()) {
        targetExt = config.getLastOpenFileExtension();
        if (targetExt.isEmpty()) targetExt = "txt";
    } else {
        targetExt = QFileInfo(currentPath).suffix().toLower();
    }

    // 2. Configurar el diálogo
    QString filters = config.getOpenFileFilter();
    QFileDialog dialog(this, tr("Save as"), config.getLastPath(), filters);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDefaultSuffix(targetExt);

    // 3. Buscar y seleccionar el filtro correcto en el desplegable
    const QStringList filterList = filters.split(" ;; ");
    // Usamos std::as_const para evitar el warning de deprecación y el detach
    for (const QString &f : std::as_const(filterList)) {
        if (f.contains("*." + targetExt)) {
            dialog.selectNameFilter(f);
            break;
        }
    }

    // 4. Ejecutar el diálogo
    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            QString path = files.first();
            QFileInfo fileInfo(path);

            // Guardar configuración
            config.setLastPath(fileInfo.path());
            config.setLastOpenFileExtension(fileInfo.suffix().toLower());

            // IMPORTANTE: Devolvemos el resultado de la escritura física
            return saveFileToDisk(editor, path);
        }
    }

    // Si el usuario cancela o cierra el diálogo, devolvemos false
    return false;
}
// ======================================================
// Acción guardar todo
// ======================================================
void MainWindow::on_actionSave_All_triggered()
{
    // Guardamos el índice de la pestaña donde estaba el usuario para volver luego
    int originalIndex = ui->tabCode->currentIndex();
    int totalTabs = ui->tabCode->count();
    int savedCount = 0;

    for (int i = 0; i < totalTabs; ++i) {
        // 1. Obtenemos el editor de la pestaña 'i'
        CodeEditor *editor = qobject_cast<CodeEditor*>(ui->tabCode->widget(i));

        if (editor && editor->document()->isModified()) {
            // 2. Cambiamos el foco a esta pestaña para que el guardado sepa qué editor usar
            ui->tabCode->setCurrentIndex(i);

            // 3. Llamamos a nuestra función de guardado inteligente
            // Si el usuario cancela un "Guardar como", el resultado será false
            if (on_actionSave_triggered()) {
                savedCount++;
            } else {
                // Si el usuario cancela, normalmente detenemos el "Guardar todo"
                // pero esto depende de tu gusto. Aquí lo dejamos continuar.
                print(tr("Save cancelled for tab %1").arg(i + 1), "warn");
            }
        }
    }

    // 4. Restauramos la pestaña donde estaba el usuario al principio
    if (ui->tabCode->count() > originalIndex) {
        ui->tabCode->setCurrentIndex(originalIndex);
    }

    if (savedCount > 0) {
        print(tr("Saved %1 modified files.").arg(savedCount));
    }
}

// ======================================================
// Click Exit
// ======================================================
void MainWindow::on_actionExit_triggered()
{
    this->close(); // Esto disparará automáticamente el closeEvent de arriba
}

// ======================================================
// Evento cerrar el programa
// ======================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    // Almacena la pestaña activa
    config.setActiveTab(ui->tabCode->currentIndex());

    // Dispara diálogos de guardado y asignará los paths a los archivos nuevos
    bool canClose = true;
    for (int i = 0; i < ui->tabCode->count(); ++i) {
        CodeEditor* editor = qobject_cast<CodeEditor*>(ui->tabCode->widget(i));
        if (editor) {
            // Opcional: enfocar la pestaña que se está preguntando
            ui->tabCode->setCurrentIndex(i);
            QString nombreArchivo = ui->tabCode->tabText(i).replace("*", "");
            if (!checkUnsavedFiles(editor, nombreArchivo)) {
                canClose = false;
                break;
            }
        }
    }

    // Si todo se ha guardado o descartado correctamente...
    if (canClose) {
        // Recolectamos los paths definitivos
        QStringList openFiles;
        for (int i = 0; i < ui->tabCode->count(); ++i) {
            if (auto editor = qobject_cast<CodeEditor*>(ui->tabCode->widget(i))) {
                // Si el usuario guardó el "Untitled", aquí ya tendrá su path real
                QString path = editor->property("filePath").toString();
                if (!path.isEmpty()) {
                    openFiles << path;
                }
            }
        }

        // Guardamos los paths abiertos en la configuración final
        config.setOpenFiles(openFiles);

        event->accept();
    } else {
        event->ignore();
    }
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

    // 1. En caso de que el archivo ya está abierto, va a esa pestaña
    for (int i = 0; i < ui->tabCode->count(); ++i) {
        if (auto editor = qobject_cast<CodeEditor*>(ui->tabCode->widget(i))) {
            QString path = editor->property("filePath").toString();
            if (path == filePath) {
                ui->tabCode->setCurrentIndex(i);
                return;
            }
        }
    }

    // 2. Si solo hay una pestaña y es un archivo nuevo sin modificar, la cierra
    if (ui->tabCode->count() == 1) {
        CodeEditor *editor = qobject_cast<CodeEditor *>(ui->tabCode->widget(0));
        if (editor) {
            QString path = editor->property("filePath").toString();
            if (path.isEmpty() && !editor->document()->isModified()) {
                closeFile(0);
            }
        }
    }

    // 3. Determinar el charset según la extensión antes de leer
    QFileInfo fileInfo(filePath);
    QString ext = fileInfo.suffix().toLower();
    QString charsetName = "UTF-8"; // Valor por defecto

    for (const Language &lang : config.getLanguages()) {
        if (lang.extensions.contains(ext)) {
            charsetName = lang.charset.trimmed().remove(QChar('"'));
            break;
        }
    }

    // 4. Intentamos abrir el archivo en modo binario para que el Codec gestione el texto
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        print(tr("Can't open: ") + filePath, "error");
        return;
    }

    // 5. Leer contenido y aplicar el Codec si es Windows-1252
    QByteArray data = file.readAll();
    file.close();

    QString text;
    if (charsetName == "Windows-1252") {
        QTextCodec *codec = QTextCodec::codecForName("Windows-1252");
        if (codec) {
            text = codec->toUnicode(data);
        } else {
            text = QString::fromUtf8(data); // Fallback
        }
    } else {
        text = QString::fromUtf8(data);
    }

    // 6. Crea el editor y carga el texto procesado
    CodeEditor *editor = new CodeEditor();
    editor->setPlainText(text);
    editor->setProperty("filePath", filePath);

    // 7. Conector para cuando se modifica el archivo
    connect(editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::markFileAsModified);
    connect(editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::updateStatusBar);

    // 8. Carga el editor en una pestaña con su nombre
    QString baseName = fileInfo.fileName();
    ui->tabCode->addTab(editor, baseName);
    ui->tabCode->setCurrentWidget(editor);

    // Feedback y actualización de UI
    print(tr("File open: %1 [%2]").arg(baseName, charsetName));
    //updateStatusBar();
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
    connect(editor->document(), &QTextDocument::modificationChanged, this, &MainWindow::updateStatusBar);

    // Asocia el editor a una pestaña
    ui->tabCode->addTab(editor, tr("Untitled"));
    ui->tabCode->setCurrentWidget(editor);
    print(tr("New file created."));

    //updateStatusBar();

}

// ======================================================
// Cierra un archivo
// ======================================================
void MainWindow::closeFile(int index) {
    // 1. Obtener el widget de la pestaña
    QWidget* widget = ui->tabCode->widget(index);
    if (!widget) return;

    CodeEditor* editor = qobject_cast<CodeEditor*>(widget);
    if (editor) {
        // 2. Si el documento tiene cambios sin guardar, preguntamos
        if (editor->document()->isModified()) {
            // Obtenemos el nombre de la pestaña (limpiando el asterisco)
            QString tabName = ui->tabCode->tabText(index).remove('*');

            QMessageBox::StandardButton resBtn = QMessageBox::question(
                this,
                tr("Unsaved Changes"),
                tr("The file '%1' has been modified.\nDo you want to save changes?").arg(tabName),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes
                );

            if (resBtn == QMessageBox::Yes) {
                // Seleccionamos la pestaña que queremos guardar para que 'on_actionSave' actúe sobre ella
                ui->tabCode->setCurrentIndex(index);

                // IMPORTANTE: Al ser bool, si on_actionSave_triggered() devuelve false
                // (porque el usuario canceló el "Guardar como"), abortamos el cierre.
                if (!on_actionSave_triggered()) {
                    return;
                }
            } else if (resBtn == QMessageBox::Cancel) {
                // Si el usuario da a "Cancelar", no hacemos nada y salimos de la función
                return;
            }

            // Si eligió "No", la ejecución sigue hacia abajo y cierra la pestaña ignorando cambios
        }
    }

    // 3. Quitar la pestaña del QTabWidget
    ui->tabCode->removeTab(index);

    // 4. Liberar la memoria del widget (el editor)
    delete widget;

    // 5. Actualizar la interfaz manualmente (hasta que pongas los connect en el constructor)
    //updateStatusBar();

    print(tr("File closed."));
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

    //updateStatusBar();
}

// ======================================================
// Guarda un archivo en disco
// ======================================================
bool MainWindow::saveFileToDisk(CodeEditor *editor, const QString &path) {
    if (!editor || path.isEmpty()) return false;

    // 1. Obtener la extensión
    QString ext = QFileInfo(path).suffix().toLower();

    // 2. Determinar el charset desde tu configuración (ini)
    QString charsetName = "UTF-8";
    for (const Language &lang : config.getLanguages()) {
        if (lang.extensions.contains(ext)) {
            charsetName = lang.charset.trimmed().remove(QChar('"'));
            break;
        }
    }

    QByteArray encodedData;

    // 3. Lógica directa: Si el ini dice Windows-1252, usamos el codec antiguo
    if (charsetName == "Windows-1252") {
        QTextCodec *codec = QTextCodec::codecForName("Windows-1252");
        if (codec) {
            encodedData = codec->fromUnicode(editor->toPlainText());
        } else {
            // Si por algún motivo el codec falla, fallback a UTF-8
            encodedData = editor->toPlainText().toUtf8();
            charsetName = "UTF-8 (Codec fail)";
        }
    } else {
        // Para todo lo demás (o si es UTF-8), guardamos en UTF-8 estándar
        encodedData = editor->toPlainText().toUtf8();
    }

    // 4. Escritura física del archivo
    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        print(tr("Error: Could not open for writing"), "error");
        return false;
    }

    file.write(encodedData);
    file.close();

    // 5. Actualizar metadatos y UI
    editor->setProperty("filePath", path);
    editor->document()->setModified(false);

    int index = ui->tabCode->indexOf(editor);
    if (index != -1) {
        ui->tabCode->setTabText(index, QFileInfo(path).fileName());
    }

    print(tr("File saved: %1 [%2]").arg(QFileInfo(path).fileName(), charsetName));

    //updateStatusBar();

    return true;
}

/************************************************************************************************
 *
 *  FUNCIONES AUXILIARES
 *
 ************************************************************************************************/

// ======================================================
// Obtiene el editor (pestaña) actual
// ======================================================

CodeEditor* MainWindow::currentEditor() const {
    return qobject_cast<CodeEditor*>(ui->tabCode->currentWidget());
}

bool MainWindow::checkUnsavedFiles(CodeEditor* editor, const QString& nombreArchivo) {
    if (!editor || !editor->document()->isModified()) {
        return true; // No hay cambios, puede continuar
    }

    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("Documento modificado"));
    msgBox.setText(QString(tr("¿Desea guardar los cambios en \"%1\" antes de cerrar?")).arg(nombreArchivo));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    switch (msgBox.exec()) {
    case QMessageBox::Save:
        on_actionSave_triggered();
        return !editor->document()->isModified(); // True si se guardó correctamente
    case QMessageBox::Discard:
        return true; // Continuar sin guardar
    default: // Cancel
        return false; // Abortar operación
    }
}

