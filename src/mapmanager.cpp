#include "mapmanager.h"

#include "appsettings.h"
#include "config.h"
#include "infohub.h"

#include <QMutexLocker>

#include <QDirIterator>
#include <QDir>
#include <QFileInfo>

#include <QFile>
#include <QBitArray>
#include <QPair>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <algorithm>

#include <QDebug>

MapManager::MapManager(QObject *parent) : QObject(parent)
{
  loadSettings();
}


MapManager::~MapManager()
{
}

void MapManager::onSettingsChanged()
{
  QMutexLocker lk(&m_mutex);
  loadSettings();
}

void MapManager::loadSettings()
{
  AppSettings settings;

  m_root_dir.setPath(settings.valueString(MAPMANAGER_SETTINGS "root"));
  m_map_selected = settings.valueString(MAPMANAGER_SETTINGS "map_selected");
  m_provided_url = settings.valueString(MAPMANAGER_SETTINGS "provided_url");

  m_feature_osmscout = settings.valueBool(MAPMANAGER_SETTINGS "osmscout");
  m_feature_geocoder_nlp = settings.valueBool(MAPMANAGER_SETTINGS "geocoder_nlp");
  m_feature_postal_country = settings.valueBool(MAPMANAGER_SETTINGS "postal_country");

  scanDirectories();
  missingData();
  checkUpdates();
}

void MapManager::nothingAvailable()
{
  m_maps_available = QJsonObject();
  m_map_selected.clear();

  updateOsmScout();
  updateGeocoderNLP();
  updatePostal();
}

QJsonObject MapManager::loadJson(QString fname) const
{
  QFile freq(fname);
  if (!freq.open(QIODevice::ReadOnly | QIODevice::Text)) return QJsonObject();
  return QJsonDocument::fromJson(freq.readAll()).object();
}

QString MapManager::getPath(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("path").toString();
}

size_t MapManager::getSize(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("size").toString().toULong();
}

size_t MapManager::getSizeCompressed(const QJsonObject &obj, const QString &feature) const
{
  return obj.value(feature).toObject().value("size-compressed").toString().toULong();
}

QString MapManager::getId(const QJsonObject &obj) const
{
  return obj.value("id").toString();
}

QString MapManager::getPretty(const QJsonObject &obj) const
{
  if (obj.value("id").toString() == const_feature_id_postal_global)
    return tr("Address parsing language support");

  return obj.value("continent").toString() + " / " + obj.value("name").toString();
}

QDateTime MapManager::getDateTime(const QJsonObject &obj, const QString &feature) const
{
  QString t = obj.value(feature).toObject().value("timestamp").toString();
  if (t.isEmpty()) return QDateTime();
  return QDateTime::fromString(t, "yyyy-MM-dd_hh:mm");
}

void MapManager::scanDirectories()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps directory does not exist: ") + m_root_dir.absolutePath());
      nothingAvailable();
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));

  // with postal countries requested, check if we have postal global part
  QJsonObject available;
  if (m_feature_postal_country)
    {
      const QJsonObject request = req_countries.value(const_feature_id_postal_global).toObject();
      m_postal_global_path = getPath( request,
                                      const_feature_name_postal_global );
      if ( m_postal_global_path.isEmpty() )
        {
          InfoHub::logWarning(tr("No maps loaded: libpostal language support is not requested"));
          nothingAvailable();
          return;
        }

      if (!hasAvailablePostalGlobal(request) )
        {
          InfoHub::logWarning(tr("No maps loaded: libpostal language support unavailable"));
          nothingAvailable();
          return;
        }

      // add postal global to requested
      available.insert(const_feature_id_postal_global, req_countries.value(const_feature_id_postal_global).toObject());
    }

  // check whether we have all needed datasets for required countries
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      // check if we have all keys defined
      if (request.contains("id") &&
          request.contains("name") &&
          request.contains("continent") &&
          (!m_feature_geocoder_nlp || request.contains(const_feature_name_geocoder_nlp)) &&
          (!m_feature_osmscout || request.contains(const_feature_name_osmscout)) &&
          (!m_feature_postal_country || request.contains(const_feature_name_postal_country))
          )
        {
          QString id = getId(request);

          if (m_feature_geocoder_nlp && !hasAvailableGeocoderNLP(request))
            InfoHub::logWarning(tr("Missing dataset for geocoder-nlp: ") + getPretty(request));
          else if (m_feature_osmscout && !hasAvailableOsmScout(request))
            InfoHub::logWarning(tr("Missing dataset for libosmscout: ") + getPretty(request));
          else if (m_feature_postal_country && !hasAvailablePostalCountry(request))
            InfoHub::logWarning(tr("Missing country-specific dataset for libpostal: ") + getPretty(request));
          else
            available.insert(id, request);
        }
    }

  if ( available != m_maps_available )
    {
      m_maps_available = available;

      if (!m_maps_available.contains(m_map_selected))
        m_map_selected.clear();

      bool has_postal_global = m_maps_available.contains(const_feature_id_postal_global);
      if ( (m_maps_available.count() == 1 && !has_postal_global) ||
           (m_maps_available.count() == 2 && has_postal_global) ) // there is only one map, let's select it as well
        {
          for (QJsonObject::const_iterator i = m_maps_available.constBegin();
               i != m_maps_available.constEnd(); ++i)
            if (i.key() != const_feature_id_postal_global)
              {
                m_map_selected = i.key();
                break;
              }
        }

      QStringList countries, ids;
      QList<qint64> szs;
      makeCountriesList(true, countries, ids, szs);

      // print all loaded countries
      for (const auto &c: countries)
        InfoHub::logInfo(tr("Available country or territory: ") + c);

      updateOsmScout();
      updateGeocoderNLP();
      updatePostal();
    }
}

QString MapManager::getInstalledCountries()
{
  QMutexLocker lk(&m_mutex);
  return makeCountriesListAsJSON(true);
}

QString MapManager::getProvidedCountries()
{
  QMutexLocker lk(&m_mutex);
  return makeCountriesListAsJSON(false);
}

void MapManager::makeCountriesList(bool list_available, QStringList &countries, QStringList &ids, QList<qint64> &sz)
{
  QList< QPair<QString, QString> > available;

  QJsonObject objlist;
  QHash<QString, size_t> sizes;

  if (list_available) objlist = m_maps_available;
  else objlist = loadJson(fullPath(const_fname_countries_provided));

  for (QJsonObject::const_iterator i = objlist.constBegin();
       i != objlist.constEnd(); ++i )
    if (i.key() != const_feature_id_postal_global && i.key() != const_feature_id_url)
      {
        QJsonObject c = i.value().toObject();
        QString id = getId(c);

        available.append(qMakePair(getPretty(c), id));

        size_t s = 0;
        if (m_feature_osmscout && c.contains(const_feature_name_osmscout))
          s += getSize(c,const_feature_name_osmscout);
        if (m_feature_geocoder_nlp && c.contains(const_feature_name_geocoder_nlp))
          s += getSize(c,const_feature_name_geocoder_nlp);
        if (m_feature_postal_country && c.contains(const_feature_name_postal_country))
          s += getSize(c,const_feature_name_postal_country);

        sizes[id] = s;
      }

  std::sort(available.begin(), available.end());

  countries.clear();
  ids.clear();
  sz.clear();
  for (const auto &i: available)
    {
      countries.append(i.first);
      ids.append(i.second);
      sz.append(sizes[i.second]);
    }
}

QString MapManager::makeCountriesListAsJSON(bool list_available)
{
  QStringList countries;
  QStringList ids;
  QList<qint64> sz;

  makeCountriesList(list_available, countries, ids, sz);

  QJsonArray arr;
  for (int i = 0; i < ids.size(); ++i)
    {
      QJsonObject obj;
      obj.insert("name", countries[i]);
      obj.insert("id", ids[i]);
      obj.insert("size", sz[i]);
      arr.append(obj);
    }

  QJsonDocument doc(arr);
  return doc.toJson();
}

void MapManager::addCountry(QString id)
{
  QMutexLocker lk(&m_mutex);

  if (!m_maps_available.contains(id) && m_root_dir.exists() && m_root_dir.exists(const_fname_countries_provided))
    {
      QJsonObject possible = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (possible.contains(id) && possible.value(id).toObject().value("id") == id)
        {
          requested.insert(id, possible.value(id).toObject());

          QJsonDocument doc(requested);
          QFile file(fullPath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
      missingData();
    }
}

void MapManager::rmCountry(QString id)
{
  QMutexLocker lk(&m_mutex);

  if (m_root_dir.exists() && m_root_dir.exists(const_fname_countries_requested))
    {
      QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

      if (requested.contains(id))
        {
          InfoHub::logInfo(tr("Removing country from requested list: ") + getPretty(requested.value(id).toObject()));

          requested.remove(id);

          QJsonDocument doc(requested);
          QFile file(fullPath(const_fname_countries_requested));
          file.open(QIODevice::WriteOnly | QIODevice::Text);
          file.write( doc.toJson() );
        }

      scanDirectories();
      missingData();
    }
}

void MapManager::missingData()
{
  if (!m_root_dir.exists())
    {
      InfoHub::logWarning(tr("Maps directory does not exist: ") + m_root_dir.absolutePath());
      return;
    }

  // load list of requested countries and features
  if (!m_root_dir.exists(const_fname_countries_requested))
    {
      InfoHub::logWarning(tr("No maps were requested"));
      nothingAvailable();
      return;
    }

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));

  // get URLs
  QJsonObject provided = loadJson(fullPath(const_fname_countries_provided));
  QHash<QString, QString> url;
  if (provided.contains(const_feature_id_url))
    {
      const QJsonObject o = provided.value(const_feature_id_url).toObject();
      QString base = o.value("base").toString();
      for (QJsonObject::const_iterator i=o.constBegin(); i!=o.end(); ++i)
        if (i.key()!="base")
          url[i.key()] = base + "/" + i.value().toString();
    }

  // fill missing data
  m_missing_data.clear();

  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      FilesToDownload missing;

      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      if (m_feature_osmscout && request.contains(const_feature_name_osmscout))
        checkMissingOsmScout(request, url.value(const_feature_name_osmscout), missing);
      if (m_feature_geocoder_nlp && request.contains(const_feature_name_geocoder_nlp))
        checkMissingGeocoderNLP(request, url.value(const_feature_name_geocoder_nlp), missing);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_country))
        checkMissingPostalCountry(request, url.value(const_feature_name_postal_country), missing);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_global))
        checkMissingPostalGlobal(request, url.value(const_feature_name_postal_global), missing);

      if (missing.files.length() > 0)
        {
          missing.id = getId(request);
          missing.pretty = getPretty(request);
          m_missing_data.append(missing);
        }
    }

  if (m_missing_data.length() > 0)
    {
      for (const auto &m: m_missing_data)
        {
          InfoHub::logInfo(tr("Missing data: ") +
                           m.pretty + QString(" (%L1)").arg(m.tostore));
        }
    }
}

QString MapManager::fullPath(QString path) const
{
  if (path.isEmpty()) return QString();
  QDir dir(m_root_dir.canonicalPath());
  return dir.filePath(path);
}


////////////////////////////////////////////////////////////
/// support for downloads

bool MapManager::getCountries()
{
  QMutexLocker lk(&m_mutex);

  if (m_missing_data.length() < 1) return true; // all has been downloaded already
  if (m_missing_data[0].files.length() < 1)
    {
      InfoHub::logError("Internal error: missing data has no files");
      return false;
    }

  bool started = startDownload( m_missing_data[0].files[0].url + ".bz2",
      m_missing_data[0].files[0].path, "BZ2" );

  if (started) m_download_type = Countries;
  return started;
}

bool MapManager::downloading()
{
  QMutexLocker lk(&m_mutex);
  return m_file_downloader;
}

bool MapManager::startDownload(const QString &url, const QString &path, const QString &mode)
{
  // check if someone is downloading already
  if ( m_file_downloader ) return false;

  m_last_reported_downloaded = 0;
  m_last_reported_written = 0;

  m_file_downloader = new FileDownloader(&m_network_manager, url, path, mode, this);
  if (!m_file_downloader)
    {
      InfoHub::logError("Failed to allocate FileDownloader"); // technical message, no need to translate
      return false;
    }

  if ( !bool(*m_file_downloader) )
    {
      InfoHub::logWarning(tr("Error starting the download of") + " " + path);
      return false;
    }

  connect(m_file_downloader.data(), &FileDownloader::finished, this, &MapManager::onDownloadFinished);
  connect(m_file_downloader.data(), &FileDownloader::error, this, &MapManager::onDownloadError);
  connect(m_file_downloader.data(), &FileDownloader::downloadedBytes, this, &MapManager::onDownloadedBytes);
  connect(m_file_downloader.data(), &FileDownloader::writtenBytes, this, &MapManager::onWrittenBytes);

  return true;
}

void MapManager::onDownloadFinished(QString path)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logInfo(tr("File downloaded:") + " " + path);
  cleanupDownload();

  if (m_download_type == Countries)
    {
      if (m_missing_data.length() < 1 ||
          m_missing_data[0].files.length() < 1 ||
          m_missing_data[0].files[0].path != path)
        {
          lk.unlock();
          InfoHub::logError("Internal error: missing data has no files while one was downloaded or an unexpected file was downloaded");
          onDownloadError("Internal error: processing via error handling methods");
          return;
        }

      m_missing_data[0].files.pop_front();
      m_missing_data[0].todownload -= m_last_reported_downloaded;
      m_missing_data[0].tostore -= m_last_reported_written;
      if (m_missing_data[0].files.length() == 0)
        {
          m_missing_data.pop_front();
          scanDirectories();
        }

      m_download_type = NotKnown;
      lk.unlock();
      getCountries();
    }
  else if (m_download_type == ProvidedList)
    {
      m_download_type = NotKnown;
      checkUpdates();
    }
  else
    m_download_type = NotKnown;
}

void MapManager::onDownloadError(QString err)
{
  QMutexLocker lk(&m_mutex);

  InfoHub::logWarning(err);
  cleanupDownload();

  InfoHub::logWarning(tr("Dropping all downloads"));
  m_missing_data.clear();
  m_download_type = NotKnown;
}

void MapManager::cleanupDownload()
{
  if (m_file_downloader)
    {
      m_file_downloader->disconnect();
      m_file_downloader->deleteLater();
      m_file_downloader = QPointer<FileDownloader>();
    }
}

void MapManager::onDownloadProgress()
{
  static QString last_message;

  QString txt;

  if ( m_download_type == ProvidedList )
    txt = QString(tr("List of provided countries and features: %L1 (D) / %L2 (W) MB")).
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);

  else if (m_download_type == Countries )
    {
      if (m_missing_data.length() > 0)
        {
          txt = QString(tr("%1: %L2 (D) / %L3 (W) MB")).
              arg(m_missing_data[0].pretty).
              arg((m_missing_data[0].todownload - m_last_reported_downloaded)/1024/1024).
              arg((m_missing_data[0].tostore - m_last_reported_written)/1024/1024);
        }
    }
  else
    txt = QString("Unknown: %L1 (D) %L2 (W) MB").
        arg(m_last_reported_downloaded/1024/1024).
        arg(m_last_reported_written/1024/1024);

  if (txt != last_message )
    {
      last_message = txt;
      emit downloadProgress(txt);

      qDebug() << txt;
    }
}

void MapManager::onDownloadedBytes(size_t sz)
{
  m_last_reported_downloaded = sz;
  onDownloadProgress();
}

void MapManager::onWrittenBytes(size_t sz)
{
  m_last_reported_written = sz;
  onDownloadProgress();
}

////////////////////////////////////////////////////////////
/// support for cleanup

qint64 MapManager::getNonNeededFilesList(QStringList &files)
{
  QMutexLocker lk(&m_mutex);
  qint64 notNeededSize = 0;

  m_not_needed_files.clear();
  files.clear();

  // this is mutex protected as well
  lk.unlock();
  if (downloading()) return false;
  lk.relock();

  // fill up needed files
  QSet<QString> wanted;
  wanted.insert(fullPath(const_fname_countries_requested));
  wanted.insert(fullPath(const_fname_countries_provided));

  QJsonObject req_countries = loadJson(fullPath(const_fname_countries_requested));
  for (QJsonObject::const_iterator request_iter = req_countries.constBegin();
       request_iter != req_countries.constEnd(); ++request_iter)
    {
      const QJsonObject request = request_iter.value().toObject();
      if (request.empty()) continue;

      if (m_feature_osmscout && request.contains(const_feature_name_osmscout))
        fillWantedOsmScout(request, wanted);
      if (m_feature_geocoder_nlp && request.contains(const_feature_name_geocoder_nlp))
        fillWantedGeocoderNLP(request, wanted);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_country))
        fillWantedPostalCountry(request, wanted);
      if (m_feature_postal_country && request.contains(const_feature_name_postal_global))
        fillWantedPostalGlobal(request, wanted);
    }

  QDir dir(QDir::cleanPath(fullPath(".")));
  dir.setFilter(QDir::Files);
  QDirIterator dirIter( dir, QDirIterator::Subdirectories);
  while (dirIter.hasNext())
    {
      QString path = dirIter.next();
      QFileInfo fi(path);

      if (!fi.isFile()) continue;

      if ( !wanted.contains(path) )
        {
          notNeededSize += fi.size();
          files.append(fullPath(path));
        }
    }

  m_not_needed_files = files;

  return notNeededSize;
}

bool MapManager::deleteNonNeededFiles(const QStringList &files)
{
  QMutexLocker lk(&m_mutex);
  if ( files != m_not_needed_files )
    {
      InfoHub::logError("Internal error: list of files given to delete does not matched with an expected one");
      m_not_needed_files.clear();
      return false;
    }

  for (auto fname: m_not_needed_files)
    {
      if ( !m_root_dir.remove(fname) )
        {
          InfoHub::logWarning(tr("Error while deleting file:") + " " + fname);
          InfoHub::logWarning(tr("Cancelling the removal of remaining files."));
          m_not_needed_files.clear();
          return false;
        }

      InfoHub::logInfo(tr("File removed during cleanup:") + " " + fname);
    }

  m_not_needed_files.clear();
  return true;
}

////////////////////////////////////////////////////////////
/// support for updates

bool MapManager::updateProvided()
{
  QMutexLocker lk(&m_mutex);
  bool started = startDownload(m_provided_url,
                               fullPath(const_fname_countries_provided),
                               QString());
  if (started) m_download_type = ProvidedList;
  return started;
}

void MapManager::checkUpdates()
{
  m_last_found_updates = QJsonObject();

  if ( m_root_dir.exists() &&
       m_root_dir.exists(const_fname_countries_requested) &&
       m_root_dir.exists(const_fname_countries_provided) )
    {
      QJsonObject possible_list = loadJson(fullPath(const_fname_countries_provided));
      QJsonObject requested_list = loadJson(fullPath(const_fname_countries_requested));

      for (QJsonObject::const_iterator request_iter = requested_list.constBegin();
           request_iter != requested_list.constEnd(); ++request_iter)
        {
          QJsonObject update;

          const QJsonObject request = request_iter.value().toObject();
          if (request.empty()) continue;

          const QJsonObject possible = possible_list.value(request_iter.key()).toObject();
          if (possible.empty()) continue;

#define COMP_DATETIME(feature) \
  if (m_feature_##feature && \
  request.contains(const_feature_name_##feature) && \
  possible.contains(const_feature_name_##feature) && \
  getDateTime(request, const_feature_name_##feature) < getDateTime(possible, const_feature_name_##feature)) \
  update.insert(const_feature_name_##feature, possible.value(const_feature_name_##feature).toObject());

          COMP_DATETIME(osmscout);
          COMP_DATETIME(geocoder_nlp);
          COMP_DATETIME(postal_country);

          if (m_feature_postal_country &&
              request.contains(const_feature_name_postal_global) &&
              possible.contains(const_feature_name_postal_global) &&
              getDateTime(request, const_feature_name_postal_global) < getDateTime(possible, const_feature_name_postal_global))
            update.insert(const_feature_name_postal_global, possible.value(const_feature_name_postal_global).toObject());


          if (!update.empty())
            {
              update.insert("id", request_iter.key());
              update.insert("pretty", getPretty(possible) );

              m_last_found_updates.insert(request_iter.key(), update);
            }
        }
    }
  else
    InfoHub::logWarning(tr("Cannot check for updates due to missing directory or files"));

  QJsonDocument doc(m_last_found_updates);
  emit updatesFound(doc.toJson());
  qDebug() << doc.toJson().constData();
}

QString MapManager::updatesFound()
{
  QMutexLocker lk(&m_mutex);
  return QJsonDocument(m_last_found_updates).toJson();
}

void MapManager::getUpdates()
{
  QMutexLocker lk(&m_mutex);

  QJsonObject requested = loadJson(fullPath(const_fname_countries_requested));

  for (QJsonObject::const_iterator iter = m_last_found_updates.constBegin();
       iter != m_last_found_updates.constEnd(); ++iter)
    {
      if (!requested.contains(iter.key())) continue;

      const QJsonObject update = iter.value().toObject();
      QJsonObject requpdated = requested.value(iter.key()).toObject();

      if ( m_feature_osmscout &&
           update.contains(const_feature_name_osmscout) )
        {
          deleteFilesOsmScout(update);
          requpdated.insert(const_feature_name_osmscout, update.value(const_feature_name_osmscout).toObject());
        }
      if ( m_feature_geocoder_nlp &&
           update.contains(const_feature_name_geocoder_nlp) )
        {
          deleteFilesGeocoderNLP(update);
          requpdated.insert(const_feature_name_geocoder_nlp, update.value(const_feature_name_geocoder_nlp).toObject());
        }
      if ( m_feature_postal_country &&
           update.contains(const_feature_name_postal_global) )
        {
          deleteFilesPostalGlobal(update);
          requpdated.insert(const_feature_name_postal_global, update.value(const_feature_name_postal_global).toObject());
        }
      if ( m_feature_postal_country &&
           update.contains(const_feature_name_postal_country) )
        {
          deleteFilesPostalCountry(update);
          requpdated.insert(const_feature_name_postal_country, update.value(const_feature_name_postal_country).toObject());
        }

      requested.insert(iter.key(), requpdated);
    }

  { // write updated requested json to a file
    QJsonDocument doc(requested);
    QFile file(fullPath(const_fname_countries_requested));
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write( doc.toJson() );
  }

  scanDirectories();
  missingData();

  lk.unlock();
  getCountries();
}

////////////////////////////////////////////////////////////
/// SUPPORT FOR BACKENDS
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// backend general

bool MapManager::hasAvailable(const QJsonObject &request,
                              const QString &feature,
                              const QStringList &files) const
{
  QString path = getPath(request, feature);
  for (const auto &f: files)
    if (!m_root_dir.exists(path + "/" + f))
      return false;
  return true;
}

void MapManager::checkMissingFiles(const QJsonObject &request,
                                   const QString &feature,
                                   const QString &url,
                                   const QStringList &files,
                                   FilesToDownload &missing) const
{
  QString path = getPath(request, feature);
  bool added = false;

  for (const auto &f: files)
    if (!m_root_dir.exists(path + "/" + f))
      {
        added = true;
        FileTask t;
        t.path = fullPath(path + "/" + f);
        t.url = url + "/" + path + "/" + f;
        missing.files.append(t);
      }

  if (added)
    {
      // this is an upper limit of the sizes. its smaller in reality if
      // the feature is downloaded partially already
      missing.todownload += getSizeCompressed(request, feature);
      missing.tostore  += getSize(request, feature);
    }
}

void MapManager::fillWantedFiles(const QJsonObject &request,
                                 const QString &feature,
                                 const QStringList &files,
                                 QSet<QString> &wanted) const
{
  QString path = getPath(request, feature);
  for (const auto &f: files)
    wanted.insert( fullPath(path + "/" + f) );
}

void MapManager::deleteFiles(const QJsonObject &request,
                             const QString &feature,
                             const QStringList &files)
{
  QString path = getPath(request, feature);
  for (const auto &f: files)
    {
      QString fp = path + "/" + f;
      if (m_root_dir.remove(fp))
        InfoHub::logInfo(tr("Removed file: %1").arg(fp));
      else
        InfoHub::logInfo(tr("Failed to remove file: %1").arg(fp));
    }
}

////////////////////////////////////////////////////////////
/// libosmscout support
const static QStringList osmscout_files{
  "areaarea.idx", "areanode.idx", "areas.dat", "areasopt.dat", "areaway.idx", "bounding.dat",
  "intersections.dat", "intersections.idx", "location.idx", "nodes.dat", "router2.dat", "router.dat",
  "router.idx", "textloc.dat", "textother.dat", "textpoi.dat", "textregion.dat",
  "water.idx", "ways.dat", "waysopt.dat", "types.dat"};

bool MapManager::hasAvailableOsmScout(const QJsonObject &request) const
{
  return hasAvailable(request, const_feature_name_osmscout, osmscout_files);
}

void MapManager::updateOsmScout()
{
  AppSettings settings;

  QString path;
  if (m_feature_osmscout)
    path = fullPath( getPath(m_maps_available.value(m_map_selected).toObject(),
                             const_feature_name_osmscout) );

  if (settings.valueString(OSM_SETTINGS "map") != path)
    {
      settings.setValue(OSM_SETTINGS "map", path);
      emit databaseOsmScoutChanged(path);
    }
}

void MapManager::checkMissingOsmScout(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_osmscout, url, osmscout_files, missing);
}

void MapManager::fillWantedOsmScout(const QJsonObject &request, QSet<QString> &wanted) const
{
  fillWantedFiles(request, const_feature_name_osmscout, osmscout_files, wanted);
}

void MapManager::deleteFilesOsmScout(const QJsonObject &request)
{
  deleteFiles(request, const_feature_name_osmscout, osmscout_files);
}

////////////////////////////////////////////////////////////
/// Geocoder NLP support
const static QStringList geocodernlp_files{
  "location.sqlite"};

bool MapManager::hasAvailableGeocoderNLP(const QJsonObject &request) const
{
  return hasAvailable(request, const_feature_name_geocoder_nlp, geocodernlp_files);
}

void MapManager::updateGeocoderNLP()
{
  AppSettings settings;

  QString path;

  // version of the geocoder where all data is in a single file
  if (m_feature_geocoder_nlp)
    path = fullPath( getPath( m_maps_available.value(m_map_selected).toObject(),
                              const_feature_name_geocoder_nlp ) + "/" +  geocodernlp_files[0] );

  if (settings.valueString(GEOMASTER_SETTINGS "geocoder_path") != path)
    {
      settings.setValue(GEOMASTER_SETTINGS "geocoder_path", path);
      emit databaseGeocoderNLPChanged(path);
    }
}

void MapManager::checkMissingGeocoderNLP(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_geocoder_nlp, url, geocodernlp_files, missing);
}

void MapManager::fillWantedGeocoderNLP(const QJsonObject &request, QSet<QString> &wanted) const
{
  fillWantedFiles(request, const_feature_name_geocoder_nlp, geocodernlp_files, wanted);
}

void MapManager::deleteFilesGeocoderNLP(const QJsonObject &request)
{
  deleteFiles(request, const_feature_name_geocoder_nlp, geocodernlp_files);
}

////////////////////////////////////////////////////////////
/// libpostal support

const static QStringList postal_global_files{
  "address_expansions/address_dictionary.dat", "language_classifier/language_classifier.dat",
  "numex/numex.dat", "transliteration/transliteration.dat" };

const static QStringList postal_country_files{
  "address_parser/address_parser.dat", "address_parser/address_parser_phrases.trie",
  "address_parser/address_parser_vocab.trie", "geodb/geodb_feature_graph.dat",
  "geodb/geodb_features.trie", "geodb/geodb_names.trie", "geodb/geodb_postal_codes.dat",
  "geodb/geodb.spi", "geodb/geodb.spl" };

bool MapManager::hasAvailablePostalGlobal(const QJsonObject &request) const
{
  return hasAvailable(request, const_feature_name_postal_global, postal_global_files);
}

bool MapManager::hasAvailablePostalCountry(const QJsonObject &request) const
{
  return hasAvailable(request, const_feature_name_postal_country, postal_country_files);
}

void MapManager::updatePostal()
{
  AppSettings settings;

  QString path_global;
  QString path_country;

  if (m_feature_postal_country)
    {
      path_global = fullPath( m_postal_global_path );
      path_country = fullPath( getPath( m_maps_available.value(m_map_selected).toObject(),
                                        const_feature_name_postal_country ) );
    }

  if (settings.valueString(GEOMASTER_SETTINGS "postal_main_dir") != path_global ||
      settings.valueString(GEOMASTER_SETTINGS "postal_country_dir") != path_country )
    {
      settings.setValue(GEOMASTER_SETTINGS "postal_main_dir", path_global);
      settings.setValue(GEOMASTER_SETTINGS "postal_country_dir", path_country);
      emit databasePostalChanged(path_global, path_country);
    }
}

void MapManager::checkMissingPostalCountry(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_postal_country, url, postal_country_files, missing);
}

void MapManager::fillWantedPostalCountry(const QJsonObject &request, QSet<QString> &wanted) const
{
  fillWantedFiles(request, const_feature_name_postal_country, postal_country_files, wanted);
}

void MapManager::checkMissingPostalGlobal(const QJsonObject &request, const QString &url, FilesToDownload &missing) const
{
  checkMissingFiles(request, const_feature_name_postal_global, url, postal_global_files, missing);
}

void MapManager::fillWantedPostalGlobal(const QJsonObject &request, QSet<QString> &wanted) const
{
  fillWantedFiles(request, const_feature_name_postal_global, postal_global_files, wanted);
}

void MapManager::deleteFilesPostalGlobal(const QJsonObject &request)
{
  deleteFiles(request, const_feature_name_postal_global, postal_global_files);
}

void MapManager::deleteFilesPostalCountry(const QJsonObject &request)
{
  deleteFiles(request, const_feature_name_postal_country, postal_country_files);
}
