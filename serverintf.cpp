#include "serverintf.h"
#include "inetfile.h"

#include <QMap>

/*!   Класс закрытой информации, для снятия излишней информации с интерфейсного
 * класса.
 */
class ServerIntf::Data
{
public:
    //! Указатель на внешний объект. Используется подключения слотов в downloader().
  ServerIntf *owner;
    //! Указатель на объект получения данных.
  InetFile *dwnldr;
    //! Метод создает объект.
  InetFile *downloader();
    //! Список идентификаторов и номеров привязанных к ним страниц.
  QMap<InetFileTaskId,int> tasks;
    //! Конструктор, однако.
  Data(ServerIntf *own):owner(own),dwnldr(0){}
};

ServerIntf *ServerIntf::instance()
{
  static ServerIntf * res = 0;
  try {
    return (res)?res:(res = new ServerIntf());
  } catch (...) {
  }
  return 0;
}

ServerIntf::~ServerIntf()
{
  if (d->dwnldr) {
    d->downloader()->deleteLater();
    d->dwnldr = 0;
  }
  delete d;
}

void ServerIntf::requestList(int page)
{
  if (!d->downloader()) {
    emit pageError(page,QString::fromUtf8("Приложению нехватило памяти"));
    return;
  }
  if (page!=1) {
    emit pageError(page,QString::fromUtf8("Приложение в разработке: страниц только одна"));
    return;
  }
  InetFileTaskId id;
  if (d->tasks.values().contains(page)) {
    id = d->tasks.key(page);
    d->downloader()->rmTask(id);
    d->tasks.remove(id);
  }
  //! \todo сделать загрузку урл из конфига
  id = d->downloader()->newTask(
        QString::fromUtf8("https://gist.githubusercontent.com/numbata/5ed307d7953c3f7e716f/raw/637c9df9a252a1127a6569adb2b8486a8e559682/movies.json")
        );
        //QString::fromUtf8("https://gist.githubusercontent.com/numbata/5ed307d7953c3f7e716f/raw/b7887adc444188d8aa8e61d39b82950f28c03966/movies.json"));
  d->tasks[id]=page;
}

ServerIntf::ServerIntf():
  d(new Data(this))
{

}

void ServerIntf::downloadFinish(const InetFileTaskId &id)
{
  if (!d->tasks.contains(id)) return;
  int page = d->tasks[id];
  try {
    emit pageReady(page,d->downloader()->getTaskData(id));
  } catch (...) {
    d->downloader()->rmTask(id);
    throw;
  }
}

void ServerIntf::downloadError(const InetFileTaskId &id, const QString &errorString)
{
  if (!d->tasks.contains(id)) return;
  int page = d->tasks[id];
  try {
    emit pageError(page,errorString);
  } catch (...) {
    d->downloader()->rmTask(id);
    throw;
  }
}

InetFile *ServerIntf::Data::downloader()
{
  try {
    return dwnldr?dwnldr:(dwnldr = new InetFile())->join(owner,SLOT(downloadFinish(InetFileTaskId)),SLOT(downloadError(InetFileTaskId,QString)));
  } catch (...) {
  }
  return 0;
}

/*
// --------------------------- ServerIntfTester -------------------------------

#include <QCoreApplication>
#include <QDebug>
bool ServerIntfTester::test()
{
  if (!qApp) {
    int ac = 0; char * av [] = {0,0};
    new QCoreApplication(ac,av);
  }
  pass = 1; error = QString();
  connect(serverIntf(),SIGNAL(pageReady(int,QByteArray)),this,SLOT(pageReady(int,QByteArray)));
  connect(serverIntf(),SIGNAL(pageError(int,QString)),this,SLOT(pageError(int,QString)));
  serverIntf()->requestList(1);
  while (pass == 1) qApp->processEvents();
  if (pass == 2) qDebug()<<"OK";
  else qDebug()<<"FALED"<<error;

  serverIntf()->requestList(2);
  while (pass == 1) qApp->processEvents();
  if (pass == 3) qDebug()<<"OK"<<error;
  else qDebug()<<"FALED";

  serverIntf()->requestList(1);
  serverIntf()->requestList(1);
  while ( pass== 1) qApp->processEvents();
  return true;
}

void ServerIntfTester::pageReady(int , const QByteArray &content)
{
  pass = 2;
  qDebug()<<content;
}

void ServerIntfTester::pageError(int , const QString &errorStr)
{
  pass = 3;
  error = errorStr;
}
*/
