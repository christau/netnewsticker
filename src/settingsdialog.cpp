/*
 * settingsdialog.cpp
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
#include "settingsdialog.h"
#include "settings.h"
#include "feedsettingswidget.h"
#include "filtersettingswidget.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
	KConfigDialog(parent, "settings", Settings::self())
{
	setFaceType(KPageDialog::Tabbed);

	QWidget *page = new QWidget(0);
	{
		visUi.setupUi(page);
	}
	addPage(page, i18n("Appearance"), "preferences-desktop-theme");

	m_feedSettingsWidget = new FeedSettingsWidget(0);
	addPage(m_feedSettingsWidget, i18n("Feed Access"), "application-rss+xml");

	m_filterSettingsWidget = new FilterSettingsWidget(0);
	addPage(m_filterSettingsWidget, i18n("Filter Properties"), "application-rss+xml");
	connect(this, SIGNAL(settingsChanged(const QString&)), this, SLOT(slotSettingsChanged(const QString&)));
//	connect(m_filterSettingsWidget->, SIGNAL(settingsChanged()), this, SLOT(settingsChangedSlot()));
	emit widgetModified();

}

void SettingsDialog::slotSettingsChanged(const QString& dialogName)
{
	applySettings();
}

void SettingsDialog::applySettings() const
{
	Settings::setFilterEntries(m_filterSettingsWidget->filterEntries());
	Settings::setFeedUrls(m_feedSettingsWidget->feedUrls());
}

QStringList SettingsDialog::feedUrls() const
{
	return m_feedSettingsWidget->feedUrls();
}

QStringList SettingsDialog::filterEntries() const
{
	return m_filterSettingsWidget->filterEntries();
}

#include "settingsdialog.moc"

