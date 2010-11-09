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
#include "item.h"

Item::Item(QString title, QString link) : m_title(title), m_link(link), m_width(0)
{

}

const QString Item::getTitle() const
{
	return m_title;
}
const QString Item::getLink() const
{
	return m_link;
}


