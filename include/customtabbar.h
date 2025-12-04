#ifndef CUSTOMTABBAR_H
#define CUSTOMTABBAR_H

#include <QTabBar>
#include <QMouseEvent>

class CustomTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit CustomTabBar(QWidget *parent = nullptr) : QTabBar(parent) {}

signals:
    void emptyAreaDoubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        int t = tabAt(event->pos());
        if (t == -1) {
            emit emptyAreaDoubleClicked();
            // no call to base: we've handled it
        } else {
            QTabBar::mouseDoubleClickEvent(event); // default behaviour for tabs
        }
    }
};

#endif // CUSTOMTABBAR_H
