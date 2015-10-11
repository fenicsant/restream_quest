#ifndef FILMLISTVIEW_H
#define FILMLISTVIEW_H


class QByteArray;
class QString;

class InetFile;
struct InetFileTaskId;
class FilmListItem;
class FilmPreviewer;

#include <QWidget>

//! Класс предназначен для загрузки, обновления и отображения списка фильмов.
class FilmListView : public QWidget
{
  Q_OBJECT
public:
  void previewFilm(FilmListItem *film);
  //! Конструктор
  explicit FilmListView(QWidget *parent = 0);
signals:
  void reloadFinished();
public slots:
  void beginReload();
  void closePreview();
protected:
  void resizeEvent(QResizeEvent *);
private:
  class Data;
  Data *d;
  enum {
    PreviewIdentSize = 50
  };
private slots:
  void pageReady(int page, const QByteArray & content);
  void pageError(int page, const QString & errorStr);
  void fileFinished(const InetFileTaskId & id);
  void fileError(const InetFileTaskId & id, const QString & errorStr);
};

#endif // FILMLISTVIEW_H
