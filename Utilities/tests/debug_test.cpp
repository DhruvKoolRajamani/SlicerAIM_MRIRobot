#include "include/debug/debug.hpp"
#include "include/debug/errorhandler.hpp"
#include <QApplication>
#include <QtGlobal>

int main(int argc, char** argv)
{
  qInstallMessageHandler(errorHandler);
  QApplication a(argc, argv);
  qDebug() << "This is a debug.";
  qWarning() << "This is a warning.";
  qCritical() << "This is critical!";
  qFatal("This is FATAL!");
  return 0;
}