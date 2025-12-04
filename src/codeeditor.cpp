#include "codeeditor.h"

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {

    // Espacio del tabulador
    setTabStopDistance(40.0);

}
