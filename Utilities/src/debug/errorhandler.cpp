#include "include/debug/errorhandler.hpp"

void errorHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
  switch (type)
  {
    case QtInfoMsg:
      fprintf(stderr, "\033[1;37mInfo: %s\033[0m\n", msg.toLocal8Bit().data());
      break;
    case QtDebugMsg:
      fprintf(stderr, "\033[1;32mDebug: %s\033[0m\n", msg.toLocal8Bit().data());
      break;
    case QtWarningMsg:
      fprintf(stderr, "\033[1;33mWarning: %s\033[0m\n",
              msg.toLocal8Bit().data());
      break;
    case QtCriticalMsg:
      fprintf(stderr, "\033[31mCritical: %s\033[0m\n",
              msg.toLocal8Bit().data());
      break;
    case QtFatalMsg:
      fprintf(stderr, "\033[1;31mFatal: %s\033[0m\n", msg.toLocal8Bit().data());
      abort();
  }
}