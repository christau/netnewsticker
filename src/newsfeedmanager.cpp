/*
 * newsfeedmanager.cpp
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
#include "newsfeedmanager.h"

#include <kurl.h>

using namespace Syndication;

NewsFeedManager *NewsFeedManager::s_instance = 0;

NewsFeedManager *NewsFeedManager::self()
{
    if ( !s_instance ) {
        s_instance = new NewsFeedManager;
    }
    return s_instance;
}

NewsFeedManager::NewsFeedManager() : QObject()
{
}

void NewsFeedManager::setSubscriptions( const QStringList &urls )
{
    m_subscriptions = urls;
}

void NewsFeedManager::update()
{
    m_availableFeeds.clear();
    foreach ( const QString &url, m_subscriptions ) {
        updateFeed( url );
    }
}

void NewsFeedManager::updateFeed( const QUrl &url )
{
    Loader *loader = Loader::create( this, SLOT( loaderFinished( Syndication::Loader *, Syndication::FeedPtr, Syndication::ErrorCode ) ) );
    m_activeLoaders[ loader ] = url;
    loader->loadFrom( url );
}

void NewsFeedManager::removeFeed( const QUrl &url )
{
	m_availableFeeds.remove(url);
}

const QMap<QUrl, FeedPtr> &NewsFeedManager::availableFeeds() const
{
    return m_availableFeeds;
}

void NewsFeedManager::loaderFinished( Loader *loader, FeedPtr feed, ErrorCode status )
{
    const QUrl url = m_activeLoaders[ loader ];
    m_activeLoaders.remove( loader );

    if ( status == Syndication::Success ) {
        m_availableFeeds[ url ] = feed;

        emit feedLoaded( url );
    }

    if ( m_activeLoaders.isEmpty() ) {
        emit updateFinished();
    }
}

#include "newsfeedmanager.moc"
