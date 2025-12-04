#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

class CodeEditor : public QPlainTextEdit {

    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);



};

#endif // CODEEDITOR_H
