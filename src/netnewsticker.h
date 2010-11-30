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
#ifndef NETNEWSTICKER_HEADER
#define NETNEWSTICKER_HEADER

#include <QList>
#include <Plasma/Applet>
#include <deque>
#include <QMap>

#include "item.h"
#include "settingsdialog.h"

class QSizeF;

class NetNewsTicker: public Plasma::Applet {
Q_OBJECT
public:
	NetNewsTicker(QObject *parent, const QVariantList &args);
	~NetNewsTicker();

	void paintInterface(QPainter *painter,
			const QStyleOptionGraphicsItem *option, const QRect& contentsRect);
	void showConfigurationInterface();
	void init();
	virtual QList<QAction*> contextualActions();

public slots:
	void feedsUpdated();
	void feedLoaded(const QUrl &url);


protected:
	virtual void dropEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
	void initScrollTimer();
private:
	bool m_horizontalScrolling;
	int m_mouseXOffs;
	int m_mouseDelta;
	float m_decay;
	float m_stepSize;
	int m_position;
	int m_padding;
	int m_height;
	int m_iconWidth;
	int m_hotItem;
	bool m_mousePressed;
	int m_mouseXPos;
	bool m_doInitWidth;
	QTimer* m_pTimer;
	QTimer* m_pMoveElapsedTimer;
	QTimer* m_pFeedUpdateTimer;
	QAction* m_pUpdateFeedsAction;
	QColor m_colFont;
	QColor m_colHoverFont;
	std::deque<Item> m_items;
	QFont m_font;
	QMap<int, QPixmap> m_iconMap;
	SettingsDialog* m_settingsDialog;
	bool m_feedsLoaded;
	/*
	 * Distance between the news items in vertical scrolling mode
	 */
	int m_itemVSpacing;
private slots:
	void moveTimeoutElapsed();
	void animate();
	void updateFeeds();
	void settingsChanged(const QString &);
	void settingsAccepted();
	void settingsRejected();
//	void sizeChanged();
};

K_EXPORT_PLASMA_APPLET(netnewsticker, NetNewsTicker)

#endif
