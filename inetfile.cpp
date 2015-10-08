#include "inetfile.h"

#include <QList>
#include <QBuffer>
#include <QNetworkAccessManager>

/*!   Класс закрытой информации, для снятия излишней информации с интерфейсного
 * класса.
 */
class InetFile::Data
{
public:
      //! Структура элемента очереди на получения данных.
  struct QueueItem {
      //! Состояние элемента. Если положительный, то номер в пуле закачки.
    int poolIndex;
      //! Адрес в инете, при ошибке содержит ее описание.
    QString url;
      //! Указатель на устройство вывода получаемых данных.
    QIODevice * file;
      //! Флаг того, что устройство является QBuffer созданным при создании объекта.
    bool fileMostDestroy;

    QueueItem(const QString &URL, QIODevice *afile):
      poolIndex(TSWaitForStarting),url(URL),file((afile)?afile:new QBuffer()),
      fileMostDestroy(afile==0){if (fileMostDestroy) ((QBuffer*)file)->open(QBuffer::ReadWrite);}

    ~QueueItem(){if (fileMostDestroy) delete file;}
  };
      //! Структура элемента пула получения информации.
  struct PoolItem {
      //! Указатель на загружаемый элемент очереди, признак незанятости (если 0).
    QueueItem *item;
      //! Загруженность файла в десятых долях процентах. Если размер не задан - возвращает 50%.
    int progres;
      //! Объект сетевого интерфейса
    QNetworkReply *reply;
  };
      //  Использовал ConstIterator, чтобы меньше писать
  typedef QList<QueueItem *> Queue;

      //! Указатель на внешний объект. Используется при вызове сигналов.
  InetFile *owner;
      //! Очередь всех запросов на получение данных.
  Queue queue;
      //! Пул активных загрузок.
  QVector<PoolItem>  pool;
      //! Указатель на объект получения данных по сети.
  QNetworkAccessManager *fmanager;

      //! Возвращает индекс свободного элемента пула.
  int getFreePoolIndex()const;
      //! Начинает новую загрузку из очереди.
  void startNext();
      //! Освобождает элемент пула от активной закачки.
  void releasePool(PoolItem &pit);
      //! Возвращает индекс в пуле по адресу объекта сетевого интерфейса.
  int poolIndexByReply(QObject *reply);
      //! Возвращает fmanager, создает объект при необходимости.
  QNetworkAccessManager * manager();
  Data(InetFile *own):owner(own),pool(PoolSize),fmanager(0){}
  ~Data();
};


InetFile *InetFile::join(QObject *receiver, const char *finishMember,
                    const char *errorMember, const char *progressMember)
{
  if (finishMember) connect(this,SIGNAL(finished(InetFileTaskId)),receiver,finishMember);
  if (errorMember) connect(this,SIGNAL(error(InetFileTaskId,QString)),receiver,errorMember);
  if (progressMember) connect(this,SIGNAL(progress(InetFileTaskId,int)),receiver,progressMember);
  return this;
}

InetFileTaskId InetFile::newTask(const QString &url, QIODevice *file)
{
  Data::QueueItem * item = new Data::QueueItem(url,file);
  d->queue.append(item);
  InetFileTaskId id;
  id.uid = (void *)item;
  d->startNext();
  return id;
}

int InetFile::getTaskState(InetFileTaskId id) const
{
  Data::QueueItem * item = reinterpret_cast<InetFile::Data::QueueItem*>(id.uid);
  if (!d->queue.contains(item)) return TSRemoved;
  if (item->poolIndex>=0)
    return d->pool[item->poolIndex].progres;
  return item->poolIndex;
}

void InetFile::rmTask(InetFileTaskId id)
{
  Data::QueueItem * item = reinterpret_cast<InetFile::Data::QueueItem*>(id.uid);
  if (!d->queue.contains(item)) return;
  d->queue.removeAll(item);
  delete item;
}

QByteArray InetFile::getTaskData(InetFileTaskId id) const
{
  Data::QueueItem * item = reinterpret_cast<InetFile::Data::QueueItem*>(id.uid);
  if (!d->queue.contains(item)) return 0;
  if (!item->fileMostDestroy) return QByteArray();
  return static_cast<QBuffer*>(item->file)->data();
}

QString InetFile::getTaskError(InetFileTaskId id) const
{
  Data::QueueItem * item = reinterpret_cast<InetFile::Data::QueueItem*>(id.uid);
  if (!d->queue.contains(item)) return 0;
  if (!item->poolIndex!=TSFailed) return QString();
  return item->url;
}

InetFile::InetFile(QObject *parent) :
  QObject(parent),d(new Data(this))
{

}

InetFile::~InetFile()
{
  delete d;
}

void InetFile::dwnldProgres(qint64 bytesReceived, qint64 bytesTotal)
{
  int poolindex = d->poolIndexByReply(sender());
  if (poolindex<0) return;
  if (bytesTotal<1) bytesTotal = bytesReceived*2;
  int tenthOfPercent = (bytesTotal<1000)?(bytesReceived*1000/bytesTotal)
            :((bytesReceived<1000)?1:(bytesReceived/(bytesTotal/1000)));
  Data::PoolItem & pit = d->pool[poolindex];
  if (!pit.item) return;
  pit.item->file->write(pit.reply->readAll());
  pit.progres = tenthOfPercent;
  InetFileTaskId id;
  id.uid = (void*)pit.item;
  emit progress(id,tenthOfPercent);
}

void InetFile::dwnldError(QNetworkReply::NetworkError /*code*/)
{
  int poolindex = d->poolIndexByReply(sender());
  if (poolindex<0) return;
  //if (d->pool[poolindex].reply->error()==QNetworkReply::NoError)
  d->releasePool(d->pool[poolindex]);
}

void InetFile::dwnldFinished()
{
  int poolindex = d->poolIndexByReply(sender());
  if (poolindex<0) return;
  d->releasePool(d->pool[poolindex]);
}


int InetFile::Data::getFreePoolIndex() const
{
  int res = pool.size()-1;
  while (res>=0 && pool[res].item) --res;
  return res;
}

void InetFile::Data::startNext()
{
  Queue::ConstIterator qi = queue.begin();
  Queue::ConstIterator qend = queue.end();
  while( qi!=qend && (*qi)->poolIndex != TSWaitForStarting) ++qi;
  if (qi==qend) return;
  int poolIndex = getFreePoolIndex();
  if (poolIndex<0) return;
  PoolItem & pit = pool[poolIndex];
  pit.item = *qi;
  pit.progres = 0;
  if (!manager()) {
    pit.item = 0;
    return;
  }
  { // для уменьшения размера стека
    QNetworkRequest request = QNetworkRequest(QUrl(pit.item->url));
    /* нужно было в Qt4
     QSslConfiguration conf = request.sslConfiguration();
    conf.setProtocol(QSsl::AnyProtocol);
    conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(conf);*/
    pit.reply = manager()->get(request);
  }
  connect(pit.reply,SIGNAL(error(QNetworkReply::NetworkError)),owner,SLOT(dwnldError(QNetworkReply::NetworkError)));
  connect(pit.reply,SIGNAL(finished()),owner,SLOT(dwnldFinished()));
  connect(pit.reply,SIGNAL(downloadProgress(qint64,qint64)),owner,SLOT(dwnldProgres(qint64,qint64)));
  if (!pit.reply->isRunning()) {
    if (pit.reply->error()==QNetworkReply::NoError) {
      if (!pit.reply->isFinished()) return;
    }
    releasePool(pit);
    return;
  }
  pit.item->poolIndex = poolIndex;
  InetFileTaskId id;
  id.uid = (void*)pit.item;
  emit owner->progress(id,0);
}

void InetFile::Data::releasePool(PoolItem &pit)
{
  if (pit.reply==0) {
    if (pit.item!=0) {
      pit.item->poolIndex = TSFinished;
      pit.item = 0;
    }
    return;
  }
  if (pit.item == 0) return;
  if (pit.reply->isReadable())
    pit.item->file->write(pit.reply->readAll());
  pit.item->poolIndex = (pit.reply->error()==QNetworkReply::NoError)?TSFinished:TSFailed;
  if (pit.item->poolIndex == TSFailed) pit.item->url = pit.reply->errorString();
  pit.reply->deleteLater();
  pit.reply = 0;
  pit.progres = 0;
  QueueItem * item = pit.item;
  pit.item = 0;
  InetFileTaskId id;
  id.uid = (void*)item;
  if (item->poolIndex == TSFailed) emit owner->error(id,item->url);
  else emit owner->finished(id);
  startNext();
}

int InetFile::Data::poolIndexByReply(QObject *reply)
{
  //QNetworkReply *r = qobject_cast<QNetworkReply *>(sender);
  //if (!r) return -1;
  // по правилам наследования QObject предок должен быть первым.
  int res = pool.size()-1;
  while (res >= 0 && pool[res].reply!=reply) --res;
  return res;
}

QNetworkAccessManager *InetFile::Data::manager()
{
  try {
    return (fmanager)?fmanager:(fmanager = new QNetworkAccessManager());
  } catch (...) {
  }
  return 0;
}

InetFile::Data::~Data()
{
  for(int pi = pool.size()-1; pi>=0; --pi) {
    PoolItem & pit = pool[pi];
    if (pit.reply) {
      pit.reply->abort();
      pit.reply->deleteLater(); // чтобы аборт нормально сделать
      pit.item = 0;
    }
  }
  while (!queue.isEmpty()) {
    delete queue.last();
    queue.removeLast();
  }
  delete fmanager;
}
