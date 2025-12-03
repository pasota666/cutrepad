#include "myeventfilter.h"

bool myEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    // filtramos eventos de doble click de ratón
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        // El objeto clickeado debe ser un QTabWidget o sale
        QTabWidget *tabWidget = qobject_cast<QTabWidget *>(obj);
        if (!tabWidget)
            return false;

        // Detecta si se ha pulsado en la zona vacía a la derecha del último tab
        if (mouseEvent->position().toPoint().x() > tabWidget->tabBar()->width() &&
            mouseEvent->position().toPoint().y() < tabWidget->tabBar()->height()) {
            emit emptyAreaClicked();
        }
    }

    return QObject::eventFilter(obj, event);
}
