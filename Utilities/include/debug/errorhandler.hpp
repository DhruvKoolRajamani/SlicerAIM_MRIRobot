#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QDebug>
#include <QtGlobal>

void errorHandler(QtMsgType type, const QMessageLogContext&,
                  const QString& msg);
#endif  // ERRORHANDLER_H