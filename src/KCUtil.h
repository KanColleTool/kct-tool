#ifndef KCUTIL_H
#define KCUTIL_H

<<<<<<< HEAD
=======
#include <random>
#include <ctime>
>>>>>>> 4b72354873efa9dd71da826e9016e05f17817500
#include <QString>
#include <QRegularExpression>
#include <QTime>
#include "crc32_tab.h"

/*
 * Standard CRC32 function, based on the one in the Mac OS X Kernel.
 * http://www.opensource.apple.com/source/xnu/xnu-1456.1.26/bsd/libkern/crc32.c
 *
 * I could probably use zlib's crc32 instead, but I don't know if that's
 * guaranteed to be linked to all Qt installations, so this is a safer bet.
 */
inline
quint32 crc32(quint32 crc, const void *buf, size_t size)
{
	const quint8 *p = (quint8*)buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}

inline
QString unescape(QString str)
{
	// Based on http://stackoverflow.com/a/7047000
	QRegularExpression re("(\\\\u[0-9a-fA-F]{4})");
	QRegularExpressionMatchIterator it = re.globalMatch(str);
	while (it.hasNext())
	{
		QRegularExpressionMatch match = it.next();
		str.replace(match.capturedStart(), match.capturedLength(),
		            QChar(match.captured(1).right(4).toUShort(0, 16)));
	}

	return str;
}

inline
QTime delta(const QDateTime &d1, const QDateTime &d2 = QDateTime::currentDateTimeUtc())
{
	// If it's already passed, normalize it to 0:00:00
	if(d1 < d2)
		return QTime(0,0,0,0);

	// Make sure both dates are in UTC, before taking the epoch-offset delta.
	qint64 msecs = d1.toUTC().toMSecsSinceEpoch() - d2.toUTC().toMSecsSinceEpoch();
	// Make a time at exactly 0:0:0:0, and add the millisecond delta to it.
	return QTime(0,0,0,0).addMSecs(msecs);
}

#define TABLE_SET_ITEM(_table, _row, _col, _value) \
	{\
		QTableWidgetItem *item = new QTableWidgetItem(); \
		item->setData(Qt::EditRole, _value); \
		_table->setItem(_row, _col, item); \
	}

#endif
