/******************************************************************************
 * Модуль предоставляет класс интерфейса к вымышленному сервису
 * базы-кинофильмов
 ******************************************************************************/
#ifndef SERVERINTF_H
#define SERVERINTF_H

class QString;
class InetFileTaskId;

#include <QObject>

//! Класс обеспечивает интерфейс серверным API
/*! Серверный API предоставляет список фильмов разбитый на страницы.
    Для получения количества страниц необходимо обратиться к первой странице. */

class ServerIntf : public QObject
{
  Q_OBJECT
public:
      //! Экземпляр объекта.
      /*! Характер класса не требует создания более одного экзепляра. */
  static ServerIntf * instance();
  ~ServerIntf();
signals:
      //! Вызывается при успешной загрузке содержимого страницы.
      /*! @param page - номер запрашиваемой страницы;
          @param content - json содержимое страницы.*/
  void pageReady(int page, const QByteArray & content);
      //! Вызывается при возникновении ошибки при получении страницы.
      /*! @param page - номер запрашиваемой страницы;
          @param errorString - описание ошибки. */
  void pageError(int page, const QString & errorString);
public slots:
      //! Производит установку запроса на получения страницы под номером @b page.
  void requestList(int page);
private:
      //! Обращение к экземпляру класса производится при помощи instance().
  ServerIntf();
      //! Структура закрытых данных.
  class Data;
      //! Данные класса.
  Data *d;
  Q_DISABLE_COPY(ServerIntf)
private slots:
      //! Вызывается при окончании загрузки.
  void downloadFinish(const InetFileTaskId &id);
      //! Вызывается при ошибках полученя данных.
  void downloadError(const InetFileTaskId &id, const QString &errorString);
};

//! @return экземпляр класса интерфейса с API базы-кинофильмов.
inline ServerIntf *serverIntf(){return ServerIntf::instance();}

//! @file serverintf.h содержит класс интерфейса с серверным API.
/*
class ServerIntfTester : public QObject
{
  Q_OBJECT
public:
  int pass;
  QString error;
  bool test();
public slots:
  void pageReady(int page, const QByteArray & content);
  void pageError(int page, const QString & errorStr);
};
*/
#endif // SERVERINTF_H
