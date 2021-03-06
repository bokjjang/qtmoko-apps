#include <qtopialog.h>
#include <QApplication>

#include "googlesession.h"
#include "googlesync.h"

GoogleSync::GoogleSync(QObject *parent)
: QObject(parent), session(new GoogleSession(this)), inProgress(false)
{
  connect(session, SIGNAL(error(GoogleSession::Error, QString)), SLOT(googleError(GoogleSession::Error, QString)));
  connect(session, SIGNAL(authenticated()), SLOT(googleAuth()));
  connect(session, SIGNAL(groupsFetched(QHash<QString, QString>)), SLOT(googleGroups(QHash<QString, QString>)));
  connect(session, SIGNAL(contactsFetched(QList<QContact>)), SLOT(googleContacts(QList<QContact>)));
  connect(session, SIGNAL(stateChanged(GoogleSession::State )), SIGNAL(stateChanged(GoogleSession::State)) );
}

GoogleSync::~GoogleSync()
{
}

bool GoogleSync::start(const QString &login, const QString &passwd, bool setskip, bool setRemoveAll)
{
  if (inProgress)
    return false;
  inProgress = true;
  skip = setskip;
  removeAll = setRemoveAll;
  session->login(login, passwd);
}

void GoogleSync::googleError(GoogleSession::Error err, const QString &reason)
{
    inProgress = false;
    qCritical() << "Google error" << reason;
}

void GoogleSync::googleAuth()
{
  qLog(Synchronization) << "Google authenticated";
  session->fetchGroups();
}

void GoogleSync::googleGroups(QHash<QString, QString> groups)
{
  qLog(Synchronization) << "Groups!";
  groupMap = groups;
  for (QHash<QString, QString>::iterator it=groupMap.begin(); it!=groupMap.end(); it++)
  {
    qLog(Synchronization) << it.key() << "->" << it.value();
  }
  session->setGroups(groups);
  session->updateGroups();
  
  session->fetchContacts();
}

void GoogleSync::googleContacts(QList<QContact> contacts)
{
  qLog(Synchronization) << "Got contacts. Updating" << skip << removeAll;
  session->updateContacts(contacts, skip, removeAll);
  qLog(Synchronization) << "Terminating";
  QApplication::instance()->exit(0);
}
