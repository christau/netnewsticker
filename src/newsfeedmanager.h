/*
 * newsfeedmanager.h
 *
 * Copyright (c) 2007 Frerich Raabe <raabe@kde.org>
 * Modified 2009 by Chris Taubenheim <chris@taubenheim.de>
 * www.netnewsticker.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#ifndef NEWSFEEDMANAGER_H
#define NEWSFEEDMANAGER_H

#include <QMap>
#include <QObject>
#include <QUrl>
#include <QStringList>

#include <syndication/loader.h>

class NewsFeedManager : public QObject
{
    Q_OBJECT
public:
    static NewsFeedManager *self();

    void setSubscriptions( const QStringList &urls );

    const QMap<QUrl, Syndication::FeedPtr> &availableFeeds() const;

public Q_SLOTS:
    void update();
    void updateFeed( const QUrl &url );
    void removeFeed( const QUrl &url );

Q_SIGNALS:
    void feedLoaded( const QUrl &url );
    void updateFinished();

private Q_SLOTS:
    void loaderFinished( Syndication::Loader *loader, Syndication::FeedPtr feed,
                         Syndication::ErrorCode status );

private:
    NewsFeedManager();
    NewsFeedManager( const NewsFeedManager &other ); // disabled
    void operator=( const NewsFeedManager &rhs ); // disabled

    static NewsFeedManager *s_instance;

    QStringList m_subscriptions;
    QMap<Syndication::Loader *, QUrl> m_activeLoaders;
    QMap<QUrl, Syndication::FeedPtr> m_availableFeeds;
};

#endif // NEWSFEEDMANAGER_H

