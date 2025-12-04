#ifndef MYEVENTFILTER_H
#define MYEVENTFILTER_H

#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QTabWidget>
#include <QTabBar>

class myEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit myEventFilter(QObject *parent = nullptr)
        : QObject(parent) {}

signals:
    void emptyAreaClicked(); // se emite cuando se hace clic en la zona vacía

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif // MYEVENTFILTER_H
