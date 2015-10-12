#ifndef INETFILE_H
#define INETFILE_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QNetworkReply>
class QIODevice;

//  Идентификатор задачи.
struct InetFileTaskId;

//! Класс предоставляет простой интерфейс для получения файлов из интернета.
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

class InetFile : public QObject
{
  Q_OBJECT
public:
    //! Подключает сигналы текущего объекта к слотам receiver'а.
  InetFile * join(QObject *receiver, const char *finishMember,
                  const char *errorMember, const char * progressMember=0);
    //! Создает новую задачу на получение файла.
  InetFileTaskId newTask(const QString & url, QIODevice * file = 0);
    //! Раширенные состояния задачи.
  enum TaskExtStates {
    //! Ожидает начала выполнения.
    TSWaitForStarting = -1,
    //! Окончена, ожидает утилизации.
    TSFinished        = -2,
    //! Прервана по причине ошибки.
    TSFailed          = -3,
    //! Удален.
    TSRemoved         = -4
  };
    //! Возвращает состояние задачи.
    /*! При положительных значениях количество полученных данных в десятых долях
     * процента. В случае отрицательных значений, возвращается одно из значений
     * TaskExtStates. */
  int getTaskState(InetFileTaskId id)const;
    //! Удаляет задачу. Если задача еще выполняется, то она прерывается.
  void rmTask(InetFileTaskId id);
    //! Возвращает данные получаемые в задаче.
    /*! Если задается file в методе newTask, данные возвращаться не будут.
     * Метод предназначен для получения данных только для случаев внутренней
     * буферизации данных. */
  QByteArray getTaskData(InetFileTaskId id)const;
    //! Возвращает описани ошибки получения данных. @sa getTaskState(), TSFailed.
  QString getTaskError(InetFileTaskId id)const;
    //! Конструктор.
  explicit InetFile(QObject *parent = 0);
    //! Деструктор.
  ~InetFile();
  enum {
    //! Количество параллельных закачек файлов.
    PoolSize = 6
  };
signals:
    //! Вызывается после окончания получения файла.
    /*! @param id - присвоеный идентификатор в очереди получения. */
  void finished(const InetFileTaskId & id);
    //! Вызывается при получении файла для контроля процесса полученния данных
    /*! @param id - присвоеный идентификаторв очереди получения;
     *  @param tenthOfPercent - состояние загруженности в десятых процета. */
  void progress(const InetFileTaskId & id, int tenthOfPercent);
    //! Вызывается при прерывании получения файла в следствии ошибки
    /*! @param id - присвоеный идентификатор в очереди получения;
     *  @param errorString - описание ошибки. */
  void error(const InetFileTaskId & id, const QString & errorString);
private:
    //! Структура закрытых данных.
  class Data;
    //! Данные класса.
  Data *d;
  Q_DISABLE_COPY(InetFile)
private slots:
    //! Вызывается при получении части данных.
  void dwnldProgres(qint64 bytesReceived, qint64 bytesTotal);
    //! Вызывается при ошибках полученя данных.
  void dwnldError(QNetworkReply::NetworkError code);
    //! Вызывается при окончании загрузки.
  void dwnldFinished();
};

//!  Идентификатор задачи.
/*! Структура предназначена для идентификации задач получения файлов.
 * При работе в классе InetFileTaskId вначале проверяется на корректность
 * указателя. Если указатель не корректен, то задача считается удаленной. */
struct InetFileTaskId
{
  void* uid; //!< Уникальное значение, которое делает идентификатор неповторимым.
  InetFileTaskId():uid(0){}
};
//! Оператор необходимый для сортировки в классах Qt.
inline bool operator <(const InetFileTaskId & id1, const InetFileTaskId & id2)
{return id1.uid<id2.uid;}
//! Оператор необходимый для сортировки в классах Qt.
inline bool operator ==(const InetFileTaskId & id1, const InetFileTaskId & id2)
{return id1.uid==id2.uid;}

//! @file inetfile.h Содержит класс получения данных из интернета.

#endif // INETFILE_H
