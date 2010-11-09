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
#ifndef FEEDSETTINGSWIDGET_H
#define FEEDSETTINGSWIDGET_H

#include "ui_feedsettings.h"
#include "ui_feeddirectory.h"

#include <QWidget>
#include <QNetworkReply>

class KProgressDialog;
class QListWidgetItem;

class FeedSettingsWidget: public QWidget
{
Q_OBJECT
public:
	FeedSettingsWidget(QWidget *parent);

	QStringList feedUrls() const;

private Q_SLOTS:
	void feedItemChanged();
	void addButtonClicked();
	void getFeedsButtonClicked();
	void removeButtonClicked();
	void feedLoaded(const QUrl &url);
	void replyFinished(QNetworkReply* reply);
	void feedDirectoryAccepted();
private:
	Ui::FeedSettings ui;
	Ui::FeedDirectory *fd;
	KProgressDialog *m_downloadMessageBox;
	QString m_addedFeedUrl;
};

#endif // !defined(FEEDSETTINGSWIDGET_H)
