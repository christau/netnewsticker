/*
 * filtersettingswidget.cpp
 *
 * Copyright (c) 2009 by Chris Taubenheim <chris@taubenheim.de>
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

#include <syndication/item.h>
#include <syndication/loader.h>


#include "filtersettingswidget.h"
#include "settings.h"
#include "newsfeedmanager.h"


ArticleFilter::ArticleFilter(const QString &action, const QString &condition, const QString &expression, bool enabled) :
	m_action(action), m_condition(condition), m_expression(expression), m_enabled(enabled)
{
}

bool ArticleFilter::matches(const QString& headline) const
{
	if (!enabled())
		return false;

	bool matches;

	if (condition() == i18n("contain"))
		matches = headline.contains(expression(), Qt::CaseInsensitive);
	else if (condition() == i18n("do not contain"))
		matches = !headline.contains(expression(), Qt::CaseInsensitive);
	else if (condition() == i18n("equal"))
		matches = (headline == expression());
	else if (condition() == i18n("do not equal"))
		matches = (headline != expression());
	else
	{ // condition() == i18n("match")
		QRegExp regexp = QRegExp(expression());
		matches = regexp.exactMatch(headline);
	}

	if (action() == i18n("Show"))
		matches = !matches;

	return matches;
}

FilterSettingsWidget::FilterSettingsWidget(QWidget *parent) :
	QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.filterEntries, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(slotFilterSelectionChanged(QTreeWidgetItem *, int)));
	connect(ui.comboFilterAction, SIGNAL(activated(const QString &)), SLOT(slotFilterActionChanged(const QString &)));
	connect(ui.comboFilterCondition, SIGNAL(activated(const QString &)), SLOT(slotFilterConditionChanged(const QString &)));
	connect(ui.leFilterExpression, SIGNAL(textChanged(const QString &)), SLOT(slotFilterExpressionChanged(const QString &)));
	connect(ui.bAddFilter, SIGNAL(clicked()), SLOT(slotAddFilter()));
	connect(ui.bRemoveFilter, SIGNAL(clicked()), SLOT(slotRemoveFilter()));
	connect(ui.cboNewsSources, SIGNAL(activated(const QString &)), SLOT(slotFilterNewsSourceChanged(const QString &)));

	initNewsSources();

	QStringList list = Settings::filterEntries();
	for (int i = 0; i < list.count(); ++i)
	{
		QStringList filter = list[i].split('|');
		if (filter.count() != 5)
		{
			//			printf("not reading filter entry:%s\n", list[i]);
			continue;
		}
		ArticleFilter fd;
		fd.setEnabled((filter.at(0) == "0") ? false : true);
		fd.setAction(filter.at(1));
		fd.setCondition(filter.at(2));
		fd.setExpression(filter.at(3));
		fd.setFeedUrl(filter.at(4));
		addFilter(fd);

	}
}

void FilterSettingsWidget::initNewsSources()
{
	ui.cboNewsSources->clear();
	ui.cboNewsSources->addItem(i18n("All News Sources"));
	QList<Syndication::FeedPtr> availableFeeds = NewsFeedManager::self()->availableFeeds().values();
	for (int i = 0; i < availableFeeds.count(); ++i)
	{
		ui.cboNewsSources->addItem(availableFeeds[i]->link());

	}
}

void FilterSettingsWidget::addFilter(const ArticleFilter &fd)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(ui.filterEntries);//, fd.action());
	item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->setCheckState(0, fd.enabled() ? Qt::Checked : Qt::Unchecked);
	item->setText(0, fd.action());
	item->setText(1, fd.condition());
	item->setText(2, fd.expression());
	item->setText(3, fd.feedUrl());
}

void FilterSettingsWidget::slotAddFilter()
{
	ArticleFilter fd;
	fd.setAction(ui.comboFilterAction->currentText());
	fd.setCondition(ui.comboFilterCondition->currentText());
	fd.setExpression(ui.leFilterExpression->text());
	fd.setFeedUrl(ui.cboNewsSources->currentText());
	fd.setEnabled(true);
	addFilter(fd);
}

void FilterSettingsWidget::slotRemoveFilter()
{
	QList<QTreeWidgetItem*> list = ui.filterEntries->selectedItems();
	for (int i = 0; i < list.size(); ++i)
	{
		ui.filterEntries->takeTopLevelItem(ui.filterEntries->indexOfTopLevelItem(list.at(i)));
	}
}

void FilterSettingsWidget::slotFilterActionChanged(const QString &action)
{
	QList<QTreeWidgetItem*> list = ui.filterEntries->selectedItems();
	for (int i = 0; i < list.size(); ++i)
	{
		list.at(i)->setText(0, action);
	}
}

void FilterSettingsWidget::slotFilterConditionChanged(const QString &condition)
{
	QList<QTreeWidgetItem*> list = ui.filterEntries->selectedItems();
	for (int i = 0; i < list.size(); ++i)
	{
		list.at(i)->setText(1, condition);
	}
}

void FilterSettingsWidget::slotFilterExpressionChanged(const QString &expression)
{
	QList<QTreeWidgetItem*> list = ui.filterEntries->selectedItems();
	for (int i = 0; i < list.size(); ++i)
	{
		list.at(i)->setText(2, expression);
	}
}

void FilterSettingsWidget::slotFilterSelectionChanged(QTreeWidgetItem *item, int column)
{
	for (int i = 0; i < ui.comboFilterAction->count(); i++)
		if (ui.comboFilterAction->itemText(i) == item->text(0))
		{
			ui.comboFilterAction->setCurrentIndex(i);
			break;
		}

	for (int i = 0; i < ui.comboFilterCondition->count(); i++)
		if (ui.comboFilterCondition->itemText(i) == item->text(1))
		{
			ui.comboFilterCondition->setCurrentIndex(i);
			break;
		}

	ui.leFilterExpression->setText(item->text(2));
	for (int i = 0; i < ui.cboNewsSources->count(); i++)
		if (ui.cboNewsSources->itemText(i) == item->text(3))
		{
			ui.cboNewsSources->setCurrentIndex(i);
			break;
		}

	ui.bRemoveFilter->setEnabled(item);
}

void FilterSettingsWidget::slotFilterNewsSourceChanged(const QString &newsSource)
{
	QList<QTreeWidgetItem*> list = ui.filterEntries->selectedItems();

	for (int i = 0; i < list.size(); ++i)
	{
		list.at(i)->setText(3, newsSource);
	}
}

QStringList FilterSettingsWidget::filterEntries() const
{
	QStringList filters;
	for (int i = 0; i < ui.filterEntries->topLevelItemCount(); ++i)
	{
		QString filterString = "";
		filterString += ui.filterEntries->topLevelItem(i)->checkState(0) ? "1" : "0";
		filterString += "|";
		filterString += ui.filterEntries->topLevelItem(i)->text(0);
		filterString += "|";
		filterString += ui.filterEntries->topLevelItem(i)->text(1);
		filterString += "|";
		filterString += ui.filterEntries->topLevelItem(i)->text(2);
		filterString += "|";
		filterString += ui.filterEntries->topLevelItem(i)->text(3);
		filters.append(filterString);
	}
	return filters;
}

#include "../build/filtersettingswidget.moc"

