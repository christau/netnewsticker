/*
 * This File is part of NetNewsTicker
 * (c)2009 Chris Taubenheim <chris@taubenheim.de>
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
#include "netnewsticker.h"
#include <stdio.h>
#include <QPainter>
#include <QFontMetrics>
#include <QSizeF>
#include <QTimer>
#include <QtNetwork/QHttp>
#include <QFile>
#include <QList>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneResizeEvent>
#include <QFontMetrics>
#include <QDesktopServices>
#include <QAction>
#include <QMessageBox>

#include <syndication/item.h>
#include <syndication/loader.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <KTemporaryFile>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <KIO/NetAccess>

#include "newsfeedmanager.h"
#include "settings.h"
#include "filtersettingswidget.h"

/*
 * y offset where the vertical scrolled items begin to draw
 */
const int Y_OFFSET = 5;

static QString unescape(const QString &s)
{
	QString t = s;
	t.replace(QLatin1String("&amp;"), QLatin1String("&"));
	t.replace(QLatin1String("&quot;"), QLatin1String("'"));
	t.replace(QLatin1String("&apos;"), QLatin1String("\""));
	t.replace(QLatin1String("&lt;"), QLatin1String("<"));
	t.replace(QLatin1String("&gt;"), QLatin1String(">"));
	return t;
}

NetNewsTicker::NetNewsTicker(QObject *parent, const QVariantList &args) :
	Plasma::Applet(parent, args)
{
	this->setAcceptDrops(true);
	m_pTimer = 0;
	setAspectRatioMode(Plasma::IgnoreAspectRatio);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	m_feedsLoaded = false;
	m_mouseXOffs = 0;
	m_stepSize = -(float) Settings::scrollingDistance();
	m_decay = 0;
	m_doInitWidth = true;
	m_mousePressed = false;
	m_settingsDialog = 0;
	m_height = 0;
	m_iconWidth = 0;
	m_hotItem = -1;
	m_font = Settings::font();
	m_position = 0;
	setBackgroundHints(DefaultBackground);
	setFont(m_font);
	m_colFont = Settings::color();
	m_colHoverFont = QColor::fromRgb(255, 0, 0);
	m_pUpdateFeedsAction = new QAction(i18n("Update Feeds"), this);
	connect(m_pUpdateFeedsAction, SIGNAL(triggered()), this, SLOT(updateFeeds()));
	//	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	m_itemVSpacing = 0;
}

NetNewsTicker::~NetNewsTicker()
{
	if (hasFailedToLaunch())
	{
		// Do some cleanup here
	}
	else
	{
		// Save settings
	}
}

void NetNewsTicker::init()
{
	setAcceptHoverEvents(true);
	connect(NewsFeedManager::self(), SIGNAL( updateFinished() ), this, SLOT( feedsUpdated() ));
	NewsFeedManager::self()->setSubscriptions(Settings::feedUrls());
	NewsFeedManager::self()->update();
	m_horizontalScrolling = Settings::scrollHorizontal();

	initScrollTimer();
	m_pMoveElapsedTimer = new QTimer(this);
	connect(m_pMoveElapsedTimer, SIGNAL(timeout()), SLOT(moveTimeoutElapsed()));
	m_pMoveElapsedTimer->setInterval(500);
	m_pMoveElapsedTimer->setSingleShot(true);

	m_pFeedUpdateTimer = new QTimer(this);
	connect(m_pFeedUpdateTimer, SIGNAL(timeout()), SLOT(updateFeeds()));
	m_pFeedUpdateTimer->setInterval(Settings::updateInterval() * 60 * 1000);
}

QList<QAction*> NetNewsTicker::contextualActions()
{
	QList<QAction*> list;
	list.append(m_pUpdateFeedsAction);
	return list;
}
void NetNewsTicker::dropEvent(QGraphicsSceneDragDropEvent * event)
{
	QList<QUrl> list = event->mimeData()->urls();
	if (list.count() > 0)
	{
		connect(NewsFeedManager::self(), SIGNAL( feedLoaded( const QUrl & ) ), this, SLOT( feedLoaded( const QUrl & ) ));
		NewsFeedManager::self()->updateFeed(list.at(0));
	}
}

void NetNewsTicker::feedLoaded(const QUrl &url)
{
	QStringList urls = Settings::feedUrls();
	urls.append(url.toString());
	Settings::setFeedUrls(urls);
	disconnect(NewsFeedManager::self(), SIGNAL( feedLoaded( const QUrl & ) ), this, SLOT( feedLoaded( const QUrl & ) ));
	Settings::self()->writeConfig();
	updateFeeds();
}

void NetNewsTicker::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
	//	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	if (Settings::uIStyle())
	{
		p->fillRect(contentsRect, QColor::fromRgb(222, 222, 222));
	}
	p->setRenderHint(QPainter::SmoothPixmapTransform);
	p->setRenderHint(QPainter::Antialiasing);

	p->save();
	p->setFont(m_font);
	p->setClipRect(contentsRect);

	if (!m_items.empty() && m_feedsLoaded)
	{
		/*
		 * Init height of the font
		 */
		if (m_height == 0)
		{
			m_height = contentsRect.top() + p->fontMetrics().ascent() - p->fontMetrics().descent() + 2;
			m_iconWidth = m_height / 2;
			if (m_iconWidth < 16)
				m_iconWidth = 16;
		}

		if (m_doInitWidth)
		{
			m_padding = p->fontMetrics().width(" +++ ");
			//get the real width for all items
			for (std::deque<Item>::iterator itTmp = m_items.begin(); itTmp != m_items.end(); ++itTmp)
			{
				itTmp->setWidth(p->fontMetrics().width(itTmp->getTitle()));
			}
			m_doInitWidth = false;
		}
		if (!m_horizontalScrolling)
		{
			int itemIdx = 0;
			int yPos = m_position;
			for (std::deque<Item>::iterator it = m_items.begin(); it != m_items.end(); ++it)
			{
				int x = contentsRect.width() / 2 - it->getWidth() / 2;
				if (x < 20 + m_iconWidth)
					x = 20 + m_iconWidth;
				if (it->iconId() > 0 && m_iconMap.contains(it->iconId()))
				{
					QPixmap icon = m_iconMap[it->iconId()];
					p->drawPixmap(QRect(x - 5 - m_iconWidth, yPos - m_height / 2, m_iconWidth, m_iconWidth), icon, icon.rect());
				}

				if (itemIdx == m_hotItem)
				{
					p->setPen(m_colHoverFont);
				}
				else
				{
					p->setPen(m_colFont);
				}

				++itemIdx;
				p->drawText(x, yPos, it->getTitle());
				yPos += m_height + m_itemVSpacing;
				p->setPen(m_colFont);
				p->drawText(contentsRect.width() / 2, yPos, "+++");
				yPos += m_height + m_itemVSpacing;
				if (yPos > contentsRect.height() + m_height)
					break;
			}

		}
		else
		{
			int w = m_position;
			int itemIdx = 0;
			int yPos = contentsRect.top() + contentsRect.height() / 2 + m_height / 2;
			for (std::deque<Item>::iterator it = m_items.begin(); it != m_items.end(); ++it)
			{
				if (it->iconId() > 0 && m_iconMap.contains(it->iconId()))
				{
					QPixmap icon = m_iconMap[it->iconId()];
					p->drawPixmap(QRect(w + 5, contentsRect.top() + contentsRect.height() / 2 - m_iconWidth / 2, m_iconWidth, m_iconWidth), icon, icon.rect());
				}
				w += m_iconWidth + 10;
				if (itemIdx == m_hotItem)
				{
					p->setPen(m_colHoverFont);
				}
				else
				{
					p->setPen(m_colFont);
				}

				++itemIdx;
				p->drawText(w, yPos, it->getTitle());
				//			printf("drawn:%d w_:%d w:%d\n", itemIdx,w, it->getWidth());
				w += it->getWidth();
				p->setPen(m_colFont);
				p->drawText(w, yPos, " +++ ");
				w += m_padding;
				if (w > contentsRect.width())
					break;
			}

		}
	}
	else
	{
		int yPos = contentsRect.top() + contentsRect.height() / 2 + m_height / 2;
		p->setPen(m_colFont);
		p->drawText(10, yPos, i18n("Loading Feeds..."));
	}
	p->restore();
}

void NetNewsTicker::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	animate();
	m_position += event->delta() / 3;
}
void NetNewsTicker::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_mousePressed = true;
		if (m_horizontalScrolling)
		{
			m_mouseXPos = event->pos().x();
		}
		else
		{
			m_mouseXPos = event->pos().y();
		}

		m_mouseXOffs = m_mouseXPos;
		m_mouseDelta = 0;
	}
}

void NetNewsTicker::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	m_hotItem = -1;
	setCursor(Qt::ArrowCursor);
}

void NetNewsTicker::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (m_mousePressed)
	{
		if (!m_pMoveElapsedTimer->isActive())
		{
			m_pMoveElapsedTimer->start();
		}
		if (m_mouseXOffs == 0)
			m_mouseXOffs = m_horizontalScrolling ? event->pos().x() : event->pos().y();
		if (m_horizontalScrolling)
		{
			m_mouseDelta -= m_mouseXOffs - event->pos().x();
			int x = event->pos().x();
			m_position += x - m_mouseXPos;
			m_mouseXPos = x;
		}
		else
		{
			m_mouseDelta = event->pos().y() - m_mouseXOffs;
			int y = event->pos().y();
			m_position += y - m_mouseXPos;
			m_mouseXPos = y;
		}
	}
}

void NetNewsTicker::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	m_mousePressed = false;
	//if the user moved the mouse too much
	//do not check for opening the url
	if (abs(m_mouseDelta) < 20)
	{
		if (m_hotItem == -1)
			return;
		int item = 0;
		for (std::deque<Item>::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			if (m_hotItem == item)
				QDesktopServices::openUrl(it->getLink());
			++item;
		}
	}
	else
	{
		if (m_mouseDelta < -20)
		{
			m_decay = 1.05;
			m_stepSize = m_mouseDelta / 10;
			if (m_stepSize > -(float) Settings::scrollingDistance())
				m_stepSize = -(float) Settings::scrollingDistance();
		}
		else if (m_mouseDelta > 20)
		{
			m_decay = 1.03;
			m_stepSize = m_mouseDelta / 10;
			if (m_stepSize < (float) Settings::scrollingDistance())
				m_stepSize = (float) Settings::scrollingDistance();
		}
	}

	m_mouseXOffs = 0;
	if (m_pMoveElapsedTimer->isActive())
	{
		m_pMoveElapsedTimer->stop();
	}

}

void NetNewsTicker::moveTimeoutElapsed()
{
	m_mouseXOffs = 0;
}

void NetNewsTicker::showConfigurationInterface()
{
	if (!m_settingsDialog)
	{
		m_settingsDialog = new SettingsDialog;
		connect(m_settingsDialog, SIGNAL( settingsChanged( const QString & ) ), this, SLOT( settingsChanged( const QString & ) ));
		connect(m_settingsDialog, SIGNAL( accepted() ), this, SLOT( settingsAccepted() ));
	}
	m_settingsDialog->show();
}
void NetNewsTicker::settingsAccepted()
{
	m_settingsDialog->applySettings();
	Settings::self()->writeConfig();
}

void NetNewsTicker::settingsChanged(const QString& str)
{
	Settings::self()->writeConfig();
	m_colFont = Settings::color();
	m_font = Settings::font();
	m_doInitWidth = true;
	m_pTimer->setInterval(Settings::scrollingSpeed());
	NewsFeedManager::self()->update();
	m_horizontalScrolling = Settings::scrollHorizontal();
	m_stepSize = -(float) Settings::scrollingDistance();

	initScrollTimer();
}

void NetNewsTicker::initScrollTimer()
{
	if (m_pTimer == 0)
	{
		m_pTimer = new QTimer(this);
		connect(m_pTimer, SIGNAL(timeout()), SLOT(animate()));
		m_pTimer->start(0);
	}
	int msec = (1. / Settings::scrollingSpeed()) * 1000;
	m_pTimer->setInterval(msec);
}

void NetNewsTicker::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	if (m_mousePressed)
	{

	}
	else
	{
		if (m_horizontalScrolling)
		{
			int w = m_position + m_iconWidth + 10;
			int x = event->pos().x();
			int y = event->pos().y();
			m_hotItem = -1;
			int item = 0;
			int yMin = boundingRect().top() + boundingRect().height() / 2 - m_height / 2;
			int yMax = boundingRect().top() + boundingRect().height() / 2 + m_height / 2;
			for (std::deque<Item>::iterator it = m_items.begin(); it != m_items.end(); ++it)
			{
				int tmp = it->getWidth();

				if (x > w && x < (w + tmp) && (y > yMin) && (y < yMax))
				{
					m_hotItem = item;
					setCursor(Qt::PointingHandCursor);
					break;
				}
				w += tmp + m_padding + m_iconWidth;
				if (w > this->size().width())
					break;
				++item;
			}
			if (m_hotItem == -1)
				setCursor(Qt::ArrowCursor);
		}
		else
		{
			int y = event->pos().y();
			int item = 0;
			for (std::deque<Item>::iterator it = m_items.begin(); it != m_items.end(); ++it)
			{
				int h = item * 2 * m_height + item * m_itemVSpacing /*+ Y_OFFSET*/+ m_position;
				if ((y > h - m_height) && (y < h))
				{
					m_hotItem = item;
					setCursor(Qt::PointingHandCursor);
					break;
				}
				if (h > this->size().height())
				{
					setCursor(Qt::ArrowCursor);
					m_hotItem = -1;
					break;
				}
				++item;
			}

			update();
		}
	}

}

void NetNewsTicker::updateFeeds()
{
	m_feedsLoaded = false;
	NewsFeedManager::self()->setSubscriptions(Settings::feedUrls());
	NewsFeedManager::self()->update();
}

void NetNewsTicker::animate()
{
	if (m_items.empty() || !m_feedsLoaded)
	{
		return;
	}
	if (!m_horizontalScrolling)
	{
		if (!m_mousePressed)
		{
			if (m_decay != 0)
			{
				m_position += m_stepSize;
				m_stepSize /= m_decay;

				if (m_stepSize < 1.0 && m_stepSize > -1.0)
				{
					if (m_stepSize < 0)
					{
						m_stepSize = -(float) Settings::scrollingDistance();
					}
					else
					{
						m_stepSize = (float) Settings::scrollingDistance();
					}
					m_decay = 0;
				}
			}
			else
			{
				m_position += m_stepSize;
			}

		}
		//		printf("m_dist:%f\n", m_stepSize);

		if (m_height == -1)
		{
			return;
		}
		if (abs(m_position) > (2 * m_height + m_itemVSpacing))
		{
			m_items.push_back(*m_items.begin());
			m_items.pop_front();
			m_position = 0;
		}
		else if (m_position > 0)
		{
			m_items.push_front(*(m_items.end() - 1));
			m_items.pop_back();
			m_position -= 2 * m_height + m_itemVSpacing;
		}
	}
	else
	{
		if (!m_mousePressed)
		{
			if (m_decay != 0)
			{
				m_position += m_stepSize;
				m_stepSize /= m_decay;
				if (m_stepSize < 1.0 && m_stepSize > -1.0)
				{
					if (m_stepSize < 0)
					{
						m_stepSize = -(float) Settings::scrollingDistance();
					}
					else
					{
						m_stepSize = (float) Settings::scrollingDistance();
					}
					m_decay = 0;
				}
			}
			else
			{
				m_position += m_stepSize;
			}

		}
		//		printf("m_dist:%f\n", m_stepSize);

		int w = m_items.begin()->getWidth();
		if (w == -1)
		{
			return;
		}
		if (abs(m_position) > (w + m_padding + m_iconWidth + 10))
		{
			m_items.push_back(*m_items.begin());
			m_items.pop_front();
			m_position = 0;
		}
		else if (m_position > 0)
		{
			m_items.push_front(*(m_items.end() - 1));
			m_items.pop_back();
			m_position -= m_items.begin()->getWidth() + (m_padding + m_iconWidth + 10);
		}
	}

	update();
}

void NetNewsTicker::feedsUpdated()
{
	QList<ArticleFilter> artFilters;
	QStringList filters = Settings::filterEntries();
	for (int i = 0; i < filters.count(); ++i)
	{
		QStringList filter = filters[i].split('|');
		if (filter.count() != 5)
		{
			kDebug()
				<< QString("not reading filter entry:") << filters.at(i);
			continue;
		}
		ArticleFilter fd;
		fd.setEnabled((filter.at(0) == "0") ? false : true);
		fd.setAction(filter.at(1));
		fd.setCondition(filter.at(2));
		fd.setExpression(filter.at(3));
		fd.setFeedUrl(filter.at(4));
		artFilters.append(fd);
	}
	m_items.clear();
	m_iconMap.clear();
	m_doInitWidth = true;
	m_position = 0;
	int iconIdx = 1;
	QList<Syndication::FeedPtr> availableFeeds = NewsFeedManager::self()->availableFeeds().values();
	foreach ( Syndication::FeedPtr feed, availableFeeds )
		{
			int iIdx = 0;
			const QString favIcon = KMimeType::favIconForUrl(feed->link());
			if (!favIcon.isEmpty())
			{
				QPixmap pm = SmallIcon(favIcon);
				m_iconMap[iconIdx] = pm;
				iIdx = iconIdx;
				++iconIdx;
			}
			else
			{
				//download the icon
				KTemporaryFile* tmpFile = new KTemporaryFile();
				if (tmpFile->open())
				{
					QUrl l(feed->link());
					QString url = "http://" + l.host() + "/favicon.ico";

					KIO::Job* getJob = KIO::file_copy(url, KUrl(tmpFile->fileName()), -1, KIO::Overwrite
							| KIO::HideProgressInfo);
					getJob->ui()->setWindow(0);
					if (KIO::NetAccess::synchronousRun(getJob, 0))
					{
						QPixmap p(tmpFile->fileName());
						if (p.width() > 0)
						{
							iIdx = iconIdx;
							m_iconMap[iIdx] = p;
							++iconIdx;
						}
					}
					else
					{
						iIdx = 0;
					}
				}
				tmpFile->close();
				delete tmpFile;
			}
			int itemCnt = 0;
			int maxItems = Settings::maxNewsItems();
			foreach ( Syndication::ItemPtr item, feed->items())
				{
					bool matches = false;
					bool hasFilter = false;
					if (artFilters.count() > 0)
					{
						/**
						 * Loop through the article filters
						 */
						for (int i = 0; i < artFilters.count(); ++i)
						{
							if (!artFilters[i].enabled())
								continue;
							/**
							 * Determine if there's a filter for this feed url
							 */
							hasFilter = artFilters[i].feedUrl().startsWith(feed->link()) || artFilters[i].feedUrl() == i18n("All News Sources");
							if (!hasFilter)
								continue;

							if (artFilters[i].matches(QString(item->title())))
							{
								matches = true;
								break;
							}
						}

					}
					/**
					 * Only add item if there's no filter set for this feed or if the filter matches
					 */
					if (!hasFilter || !matches)
					{
						++itemCnt;
						Item it(unescape(item->title()), item->link());
						it.setIconId(iIdx);
						m_items.push_back(it);
					}
					if (maxItems != 0 && itemCnt > maxItems)
						break;
				}

		}
	m_iconWidth = 0;
	m_height = 0;
	m_feedsLoaded = true;
	m_doInitWidth = true;
	update();
}

#include "netnewsticker.moc"
