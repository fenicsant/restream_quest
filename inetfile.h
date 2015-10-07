#ifndef INETFILE_H
#define INETFILE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QNetworkReply>
class QIODevice;

//! Класс предоставляет простой интерфейс для получения файлов из интернета
/*! Основное назначение класса упростить превращение строки URL в данные файла.
 *
 * Для получения содержимого файла вызывается метод newTask.
 *
 * После окончания скачивания файла, как успешного, так и прерванного ошибкой
 * задачу необходимо удалить методом rmTask.
 *
 * При внимательном рассмотрении, класс не расчитан на большое количество
 * объектов получателей. Это обусловлено тем, что для всех объектов будут
 * вызываться события получения файлов, как "своих", так и "чужих".
 *
 * На данный момент одним из слабых мест класса является ограничение на 6
 * паралельных загрузок для одного хоста в используемом классе
 * QNetworkAccessManager.
 */

//! Идентификатор задачи
struct InetFileTaskId
{void*uid;InetFileTaskId():uid(0){}};

class InetFile : public QObject
{
  Q_OBJECT
public:
    //! Подключает сигналы текущего объекта к слотам receiver'а
  void join(QObject *receiver,const char *finishMember, const char *errorMember, const char * progressMember=0);
    //! Создает новую задачу на получение файла
  InetFileTaskId newTask(const QString & url, QIODevice * file = 0);
    //! Раширенные состояния задачи
  enum TaskExtStates {
    //! Ожидает начала выполнения
    TSWaitForStarting = -1,
    //! Окончена, ожидает утилизации
    TSFinished        = -2,
    //! Прервана по причине ошибки
    TSFailed          = -3,
    //! Удален
    TSRemoved         = -4
  };
    //! Возвращает состояние задачи
  int getTaskState(InetFileTaskId id)const;
    //! Удаляет задачу. Если задача еще выполняется, то она прерывается
  void rmTask(InetFileTaskId id);
    //! Возвращает устройство вывода данных
  QByteArray getTaskData(InetFileTaskId id)const;
    //! Возвращает устройство вывода данных
  QString getTaskError(InetFileTaskId id)const;
    //! Конструктор
  InetFile(QObject *parent = 0);
    //! Деструктор
  ~InetFile();
  enum {
    //! Количество параллельных закачек файлов
    PoolSize = 6
  };
signals:
    //! Вызывается после окончания получения файла.
    /*! @param id - присвоеный идентификатор в очереди получения. */
  void finished(const InetFileTaskId & id);
    //! Вызывается при получении файла для контроля процесса полученния данных
    /*! @param id - присвоеный идентификаторв очереди получения;
     *  @param tenthOfPercent - состояние загруженности в десятых процета. */
  void progress(const InetFileTaskId & id,int tenthOfPercent);
    //! Вызывается при прерывании получения файла в следствии ошибки
    /*! @param id - присвоеный идентификатор в очереди получения;
     *  @param errorString - описание ошибки. */
  void error(const InetFileTaskId & id, const QString & errorString);
private:
  class Data;
  Data *d;
private slots:
    //! вызывается при получении части данных
  void dwnldProgres(qint64 bytesReceived, qint64 bytesTotal);
    //! вызывается при ошибках полученя данных
  void dwnldError(QNetworkReply::NetworkError code);
    //! вызывается при окончании загрузки
  void dwnldFinished();
};

#endif // INETFILE_H
