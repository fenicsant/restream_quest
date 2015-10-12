#include "filmlistview.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>

#include "inetfile.h"
#include "serverintf.h"
#include "filmlistitem.h"
#include "filmpreviewer.h"

//! Класс закрытой информации класса FilmListView.
class FilmListView::Data
{
public:
  class Label2Btn;
  FilmListView * owner;       //!< Внешний объект.
  // Элементы отображения на экране
  QScrollArea * scrollArea;   //!< Объект скролирования списка фильмов.
  QWidget     * container;    //!< Объект содержащий список фильмов.
  QLabel      * lLoadingError;//!< Отображение ошибки загрузки документа.
  QVBoxLayout * layout;       //!< Лайоут списка фильмов.
  QList<FilmListItem *> films;//!< Список отображаемых фильмов.

  FilmPreviewer *previewer;   //!< Обект вывода детальной информации по фильму.

                              //!  Полупрозрачный объект, закрытия превью.
  Label2Btn     *leftButtonPreview;
  // Элементы загрузки данных
  InetFile loader;            //!< Загрузчик данных.
  int errorCount;             //!< Счетчик ошибок загрузки.

                              //!  Список загружаемых обложек фильмов.
  QMap<InetFileTaskId, FilmListItem*> loadPosters;
                              //!  Список загружаемых фильмов.
  QList<FilmListItem *> shadowLoad;
                              //!  Конструктор.
  explicit Data(FilmListView *own) :
    owner(own), scrollArea(0), container(0), lLoadingError(0), layout(0),
    previewer(0), leftButtonPreview(0), errorCount(0){}
                              //! Считает ошибки. При переполнении счетчика выводит сообщение.
  bool addeErrorCount(int loss_scale);
                              //!  Производит очистку списка фоновой загрузки.
  void clearShadowLoad();
};

//! Класс на основе QLabel с обработкой нажатия кнопки мыши.
class FilmListView::Data::Label2Btn : public QLabel
{
public:
  FilmListView *parent_;            //!< Родительский объект.
  explicit Label2Btn(FilmListView *parent);  //!< Конструктор.
protected:
                                    //!  Обработка нажатия кнопки мыши.
  void mousePressEvent(QMouseEvent *ev)
  {
    if (parent_) parent_->closePreview();
    QLabel::mousePressEvent(ev);
  }
};

FilmListView::Data::Label2Btn::Label2Btn(FilmListView *parent) :
  QLabel(parent), parent_(parent)
{
  static const unsigned char shadow_png[] = {
    0x89,
    0x50,0x4e,0x47,0xd,0xa,0x1a,0xa,0x0,0x0,0x0,0xd,0x49,0x48,0x44,0x52,0x0,
    0x0,0x0,0x1e,0x0,0x0,0x0,0x2,0x8,0x6,0x0,0x0,0x0,0x4f,0x24,0x6c,0x42,
    0x0,0x0,0x0,0x9,0x70,0x48,0x59,0x73,0x0,0x0,0xb,0x13,0x0,0x0,0xb,0x13,
    0x1,0x0,0x9a,0x9c,0x18,0x0,0x0,0x0,0x20,0x63,0x48,0x52,0x4d,0x0,0x0,0x7a,
    0x25,0x0,0x0,0x80,0x83,0x0,0x0,0xf9,0xff,0x0,0x0,0x80,0xe9,0x0,0x0,0x75,
    0x30,0x0,0x0,0xea,0x60,0x0,0x0,0x3a,0x98,0x0,0x0,0x17,0x6f,0x92,0x5f,0xc5,
    0x46,0x0,0x0,0x0,0x40,0x49,0x44,0x41,0x54,0x78,0xda,0xc4,0xcb,0x4b,0xa,0xc0,
    0x20,0x10,0x4,0xd1,0x9a,0xa0,0x71,0xfc,0xe5,0xfe,0xb7,0x1d,0x37,0xa,0x8d,0x17,
    0x48,0xc3,0xa3,0x56,0x6d,0x11,0x81,0x99,0x25,0x20,0x1,0x19,0x78,0x45,0x11,0x7e,
    0xa9,0xd2,0x63,0x2,0xdf,0xee,0x10,0x1d,0x68,0xf2,0xf1,0x87,0x9f,0xb6,0x0,0x0,
    0x0,0xff,0xff,0x3,0x0,0x79,0x73,0x3,0xfe,0x0,0x1a,0xb9,0xa8,0x0,0x0,0x0,
    0x0,0x49,0x45,0x4e,0x44,0xae,0x42,0x60,0x82,
  };
  QPixmap pm;
  if (pm.loadFromData(shadow_png, 186)) {
    setScaledContents(true);
    setPixmap(pm);
  } else {
    setText("<");
  }
}

void FilmListView::previewFilm(FilmListItem *film)
{
  if (!d->previewer) {
    d->previewer = new FilmPreviewer(this);
    d->previewer->setGeometry(PreviewIdentSize, 0, width()-PreviewIdentSize, height());
  }
  if (!d->leftButtonPreview) {
    d->leftButtonPreview = new Data::Label2Btn(this);
    d->leftButtonPreview->setGeometry(0, 0, PreviewIdentSize, height());
  }
  d->previewer->setFilm(film);
  d->previewer->show();
  d->previewer->raise();
  d->leftButtonPreview->show();
  d->leftButtonPreview->raise();
}

FilmListView::FilmListView(QWidget *parent) :
  QWidget(parent), d(new Data(this))
{
  connect(serverIntf(), SIGNAL(pageReady(int, QByteArray)), this, SLOT(pageReady(int, QByteArray)));
  connect(serverIntf(), SIGNAL(pageError(int, QString)), this, SLOT(pageError(int, QString)));
  connect(&d->loader, SIGNAL(finished(InetFileTaskId)), this, SLOT(fileFinished(InetFileTaskId)));
  connect(&d->loader, SIGNAL(error(InetFileTaskId, QString)), this, SLOT(fileError(InetFileTaskId, QString)));
  try {
    QVBoxLayout * lay = new QVBoxLayout();
    d->scrollArea = new QScrollArea(this);
    d->container = new QWidget(this);
    d->layout = new QVBoxLayout();
    d->container->setLayout(d->layout);
    d->scrollArea->setWidget(d->container);
    d->scrollArea->setWidgetResizable(true);
    lay->addWidget(d->scrollArea);
    lay->setMargin(0);
    setLayout(lay);
  } catch(...) {
    delete d->layout;
    delete d->container;
    delete d->scrollArea;
    delete d;
    throw;
  }
}

void FilmListView::beginReload()
{
  serverIntf()->requestList(1);
}

void FilmListView::closePreview()
{
  if (d->previewer) d->previewer->hide();
  if (d->leftButtonPreview) d->leftButtonPreview->hide();
}

void FilmListView::resizeEvent(QResizeEvent *ev)
{
  if (d->previewer) d->previewer->resize(ev->size().width()-PreviewIdentSize, ev->size().height());
  if (d->leftButtonPreview) d->leftButtonPreview->resize(PreviewIdentSize, ev->size().height());
}

void FilmListView::pageReady(int page, const QByteArray &content)
{
  QJsonParseError jsonError;
  QJsonDocument jsdoc = QJsonDocument::fromJson(content, &jsonError);
  if (jsonError.error != QJsonParseError::NoError) {
    pageError(page, QString::fromUtf8(
                "JSON eroor in docum. Page%1. Position: %2. Reason: %3")
              .arg(page).arg(jsonError.offset).arg(jsonError.errorString()));
    return;
  }
  if (!jsdoc.isObject()) {
    pageError(page, QString::fromUtf8(
                "JSON eroor in docum. Page%1. Root element not is A Document")
              .arg(page));
    return;
  }
  QJsonObject json = jsdoc.object();
  static const QString sResults = QString::fromUtf8("results");
  if (!json.contains(sResults)) {
    pageError(page, QString::fromUtf8(
                "JSON eroor in docum. Page%1. Don't found a element Results")
              .arg(page));
    return;
  }
  if (!json[sResults].isArray()) {
    pageError(page, QString::fromUtf8(
                "JSON eroor in docum. Page%1. Result is not Array")
              .arg(page));
    return;
  }
  static const QString sTotalPages = QString::fromUtf8("total_pages");
  int total_pages = json.contains(sTotalPages)?json[sTotalPages].toInt():page;

  if (page==1) {
    d->clearShadowLoad();
  }

  QJsonArray jresults = json["results"].toArray();
  for(QJsonArray::ConstIterator jfilm = jresults.begin(); jfilm!=jresults.end(); ++jfilm) {
    FilmListItem * film = new FilmListItem(this);
    if (film->load(jfilm->toObject())) {
      d->shadowLoad.append(film);
      if (!film->poster_path.isEmpty()) {
        d->loadPosters[d->loader.newTask(film->poster_path)] = film;
      }
    } else delete film;
  }
  if (page==total_pages) {
    d->errorCount = 0;
    QList<FilmListItem*> toRm = d->films;
    QMap<int, FilmListItem*> byId;
    for(int i=d->films.size()-1; i>=0; --i) byId[d->films[i]->id] = d->films[i];
    const int count = d->shadowLoad.size();
    for(int i=0; i<count; ++i) {
      FilmListItem * shf = d->shadowLoad[i];
      if (byId.contains(shf->id)) {
        FilmListItem * flm = byId.value(shf->id, 0);
        if (flm) {
          int layIndex = d->layout->indexOf(flm->getAsWidget());
          d->layout->insertWidget(layIndex, shf->getAsWidget());
        } else d->layout->addWidget(shf->getAsWidget());
      } else d->layout->addWidget(shf->getAsWidget());
      d->films.append(shf);
    }
    QList<FilmListItem *> loadPosterIts = d->loadPosters.values();
    for(int i=toRm.size()-1; i>=0; --i) {
      FilmListItem * flm = toRm[i];
      d->films.removeAll(flm);
      if (loadPosterIts.contains(flm)) {
        InetFileTaskId id = d->loadPosters.key(flm);
        d->loader.rmTask(id);
        d->loadPosters.remove(id);
      }
      delete flm;
    }
    emit reloadFinished();
  }
  return;
}

void FilmListView::pageError(int page, const QString &errorStr)
{
  qWarning()<<"FilmListView: Error in Page:"<<page<<errorStr;
  if (!d->addeErrorCount((page==1)?3:2)) {
    beginReload();
  }
}

void FilmListView::fileFinished(const InetFileTaskId &id)
{
  if (d->loadPosters.contains(id)) {
    try {
      FilmListItem *item = d->loadPosters[id];
      if (item)
        item->setPoster(d->loader.getTaskData(id));
    } catch(...) {
    }
    d->loader.rmTask(id);
  }
}

void FilmListView::fileError(const InetFileTaskId &id, const QString &/*errorStr*/)
{
  if (d->loadPosters.contains(id)) {
    try {
      FilmListItem *item = d->loadPosters[id];
      if (item)
        item->setPoster(QPixmap());
    } catch(...) {
    }
    d->loader.rmTask(id);
  }
}



bool FilmListView::Data::addeErrorCount(int loss_scale)
{
  if ((errorCount+= loss_scale)<9) {
    return false;
  }

  QList<InetFileTaskId> downloads=loadPosters.keys();
  loadPosters.clear();

  clearShadowLoad();
  for(int i=downloads.size()-1; i>=0; --i) {
    loader.rmTask(downloads[i]);
  }
  for(int i=films.size()-1; i>=0; --i) delete films[i];
  films.clear();
  {
    layout->addWidget(lLoadingError = new QLabel(QString::fromUtf8(
         "<h1 align=center>Failed to get a list of movies.<br>Repeat again in a few minutes.<br>"
         "If the problem persists, contact the program vendor.</h1>"),owner));
  }
  return true;
}

void FilmListView::Data::clearShadowLoad()
{
  for(int i=shadowLoad.size()-1; i>=0; --i) {
    FilmListItem *f = shadowLoad[i];
    InetFileTaskId id = loadPosters.key(f);
    if (loadPosters.contains(id)) {
      loadPosters.remove(id);
      loader.rmTask(id);
    }
    delete f;
  }
  shadowLoad.clear();
}
