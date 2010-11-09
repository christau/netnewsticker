/*
 * feedsettingswidget.cpp
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

#include "feedsettingswidget.h"
#include "newsfeedmanager.h"
#include "settings.h"

#include <KInputDialog>
#include <KProgressDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>
#include <QMessageBox>


FeedSettingsWidget::FeedSettingsWidget(QWidget *parent) :
	QWidget(parent), m_downloadMessageBox(0)
{
	ui.setupUi(this);
	ui.feedListWidget->addItems(Settings::feedUrls());
	connect(ui.feedListWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( feedItemChanged() ));
	connect(ui.addButton, SIGNAL( clicked() ), this, SLOT( addButtonClicked() ));
	connect(ui.removeButton, SIGNAL( clicked() ), this, SLOT( removeButtonClicked() ));
	connect(ui.getFeedsButton, SIGNAL( clicked() ), this, SLOT( getFeedsButtonClicked() ));

	if (ui.feedListWidget->count() > 0)
	{
		ui.feedListWidget->setCurrentRow(0);
		feedItemChanged();
	}

	ui.addButton->setIcon(KIcon("list-add"));
	ui.removeButton->setIcon(KIcon("list-remove"));
}

QStringList FeedSettingsWidget::feedUrls() const
{
	QStringList urls;
	for (int i = 0; i < ui.feedListWidget->count(); ++i)
	{
		urls.append(ui.feedListWidget->item(i)->text());
	}
	return urls;
}

void FeedSettingsWidget::feedItemChanged()
{
	QListWidgetItem *item = ui.feedListWidget->currentItem();
	ui.removeButton->setEnabled(item != 0);
	if (item == 0)
	{
		return;
	}

	QMap<QUrl, Syndication::FeedPtr> availableFeeds = NewsFeedManager::self()->availableFeeds();
	QMap<QUrl, Syndication::FeedPtr>::ConstIterator it = availableFeeds.find(item->text());
	if (it == availableFeeds.end())
	{
		kDebug( 500 )
			<< "Don't have this item " << item->text();
		return;
	}

	Syndication::FeedPtr feed = *it;
	ui.feedTitleLabel->setText(feed->title());
	ui.feedUrlLabel->setText(feed->link());
	ui.feedDescriptionLabel->setText(feed->description());

}

void FeedSettingsWidget::addButtonClicked()
{
	bool ok;
	QString
			url =
					KInputDialog::getText(i18n("New Newsfeed"), i18n("Enter the Address (URL) of the Newsfeed to be added:"), QString(), &ok, this);

	if (ok && !url.isEmpty())
	{
		m_addedFeedUrl = url;
		connect(NewsFeedManager::self(), SIGNAL( feedLoaded( const QUrl & ) ), this, SLOT( feedLoaded( const QUrl & ) ));

		NewsFeedManager::self()->updateFeed(url);
		m_downloadMessageBox = new KProgressDialog(this, i18n("Please wait..."), i18n("Please wait while the newsfeed is downloaded..."));
		m_downloadMessageBox->progressBar()->setRange(0, 0);
		m_downloadMessageBox->exec();
	}
}

void FeedSettingsWidget::removeButtonClicked()
{
	int row = ui.feedListWidget->currentRow();
	QListWidgetItem* item = ui.feedListWidget->takeItem(row);
	bool b = Settings::feedUrls().removeOne(item->text());
	//    Settings::feedUrls().clear();
	//    printf("bool:%d", Settings::feedUrls().size());
	NewsFeedManager::self()->removeFeed(item->text());
	delete item;
	const int remainingItems = ui.feedListWidget->count();
	if (remainingItems > 0)
	{
		if (row == remainingItems)
		{
			row = remainingItems - 1;
		}
		ui.feedListWidget->setCurrentRow(row);
	}
}

void FeedSettingsWidget::getFeedsButtonClicked()
{
	fd = new Ui::FeedDirectory();
	QDialog *dlg = new QDialog((QFrame*) this->parent());
	fd->setupUi(dlg);
	/*
	 * Init tree
	 */
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
	connect(dlg, SIGNAL(accepted()), this, SLOT(feedDirectoryAccepted()));
	manager->get(QNetworkRequest(QUrl("http://www.netnewsticker.org/components/com_suggestfeeds/feeds.php?func=getfeeds")));

	dlg->show();
	//http://www.netnewsticker.org/components/com_suggestfeeds/feeds.php?func=getfeeds
}
void FeedSettingsWidget::feedDirectoryAccepted()
{
	QStringList subscribedFeeds = Settings::feedUrls();
	int i;
	for (i = 0; i < fd->feedsTree->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* item = fd->feedsTree->topLevelItem(i);
		QString link = item->text(1);
		bool checked = item->checkState(0);
		if(subscribedFeeds.contains(link, Qt::CaseInsensitive))
		{
			if(!checked)
			{
				subscribedFeeds.removeOne(link);
				ui.feedListWidget->clear();
				ui.feedListWidget->addItems(Settings::feedUrls());
			}
		}
		else if(checked)
		{
			m_addedFeedUrl = link;
			connect(NewsFeedManager::self(), SIGNAL( feedLoaded( const QUrl & ) ), this, SLOT( feedLoaded( const QUrl & ) ));

			NewsFeedManager::self()->updateFeed(link);
			m_downloadMessageBox = new KProgressDialog(this, i18n("Please wait..."), i18n("Please wait while the newsfeed is downloaded..."));
			m_downloadMessageBox->progressBar()->setRange(0, 0);
			m_downloadMessageBox->exec();
		}
	}
}

void FeedSettingsWidget::replyFinished(QNetworkReply* reply)
{
	QDomDocument doc("netnewsticker");
	if (!doc.setContent(reply))
	{
		QMessageBox::warning(this, "Loading", "Failed to load file.");
	}

	QDomElement root = doc.documentElement();
	if (root.tagName() != "netnewsticker")
	{
		QMessageBox::warning(this, "Loading", "Invalid file.");
		return;
	}

	QStringList subscribedFeeds = Settings::feedUrls();
	QDomNode n = root.firstChild();
	while (!n.isNull())
	{
		QDomElement e = n.toElement();
		if (!e.isNull())
		{
			if (e.tagName() == "feed")
			{
				QString cat = e.attribute("category", "");
				QString name = e.attribute("name", "");
				QString link = e.attribute("link", "");

				if (!subscribedFeeds.contains(link, Qt::CaseInsensitive))
				{
					QTreeWidgetItem *item = new QTreeWidgetItem(fd->feedsTree);
					item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
					item->setCheckState(0, Qt::Unchecked);
					item->setText(0, name);
					item->setText(1, link);
				}
			}
		}

		n = n.nextSibling();
	}
	//    QTreeWidgetItem *cities = new QTreeWidgetItem(fd->feedsTree);
	//    cities->setText(0, tr("Cities"));

	//    QTreeWidgetItem *osloItem = new QTreeWidgetItem(cities);
	//    osloItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	//    osloItem->setCheckState(0,Qt::Checked);
	//    osloItem->setText(0, tr("Oslo"));
	//    osloItem->setText(1, tr("Yes"));
	//    line.sprintf("count: %d", reply->readBufferSize());
	//    osloItem->setText(0, line);
	reply->deleteLater();
}

void FeedSettingsWidget::feedLoaded(const QUrl &url)
{
	if (url.toString() != m_addedFeedUrl)
	{
		return;
	}
	Settings::feedUrls().append(url.toString());
	disconnect(NewsFeedManager::self(), SIGNAL( feedLoaded( const QUrl & ) ), this, SLOT( feedLoaded( const QUrl & ) ));

	delete m_downloadMessageBox;
	m_downloadMessageBox = 0;

	QListWidgetItem *item = new QListWidgetItem(url.toString());
	ui.feedListWidget->addItem(item);
	ui.feedListWidget->setCurrentItem(item);
}

#include "../build/feedsettingswidget.moc"

