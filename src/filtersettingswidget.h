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
#ifndef FILTERSETTINGSWIDGET_H
#define FILTERSETTINGSWIDGET_H

#include "ui_filtersettings.h"

#include <QWidget>
#include <QTreeWidget>

class ArticleFilter
{
public:
	typedef QList<int> List;

	ArticleFilter(const QString & = I18N_NOOP(QString::fromLatin1("Show")), const QString & =
			I18N_NOOP(QString::fromLatin1("contain")), const QString & = QString::null, bool = true);
	QString action() const
	{
		return m_action;
	}
	void setAction(const QString &action)
	{
		m_action = action;
	}

	QString condition() const
	{
		return m_condition;
	}

	void setCondition(const QString &condition)
	{
		m_condition = condition;
	}

	QString expression() const
	{
		return m_expression;
	}

	void setExpression(const QString &expression)
	{
		m_expression = expression;
	}

	void setFeedUrl(const QString &url)
	{
		m_url = url;
	}

	QString feedUrl() const
	{
		return m_url;
	}

	bool enabled() const
	{
		return m_enabled;
	}
	void setEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	unsigned int id() const
	{
		return m_id;
	}
	void setId(const unsigned int id)
	{
		m_id = id;
	}

	bool matches(const QString&) const;

private:
	QString m_url;
	QString m_action;
	QString m_condition;
	QString m_expression;
	bool m_enabled;
	unsigned int m_id;
};

class FilterSettingsWidget: public QWidget
{
Q_OBJECT
public:
	FilterSettingsWidget(QWidget *parent);
	QStringList filterEntries() const;
	void initNewsSources();

private Q_SLOTS:
private:
	Ui::FilterSettings ui;
	void addFilter(const ArticleFilter &fd);

protected slots:
	void slotAddFilter();
	void slotRemoveFilter();
	void slotFilterSelectionChanged(QTreeWidgetItem *, int column);
	void slotFilterActionChanged(const QString &);
	void slotFilterConditionChanged(const QString &);
	void slotFilterExpressionChanged(const QString &);
	void slotFilterNewsSourceChanged(const QString &);
};

#endif // !defined(FILTERSETTINGSWIDGET_H)
