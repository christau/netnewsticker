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
#ifndef ITEM_H_
#define ITEM_H_



#include <QString>
#include <QSize>

class Item
{
public:
	Item(QString title, QString link);

	const QString getTitle() const;
	const QString getLink() const;
	int	getWidth() const
	{
		return m_width;
	}
	void setWidth(int width)
	{
		m_width = width;
	}
	int iconId()
	{
		return m_iconId;
	}
	void setIconId(int iconId)
	{
		m_iconId = iconId;
	}
private:
	QString m_title;
	QString m_description;
	QString m_link;
	int	m_width;
	int m_iconId;
};

#endif /* ITEM_H_ */
