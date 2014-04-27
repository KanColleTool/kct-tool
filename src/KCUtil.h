#ifndef KCUTIL_H
#define KCUTIL_H

#include <random>
#include <QString>
#include <QRegularExpression>
#include <QTime>
#include "crc32_tab.h"

/*
 * Standard CRC32 function, this particular one being copypasted from the
 * opensource portions of the OSX Kernel, with just some formatting and
 * a nonportable include fixed, as well as using the Qt integer types.
 *
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

//
// Taken from http://stackoverflow.com/a/7047000
//
inline
QString unescape(QString str)
{
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
	// Make sure both dates are in UTC, before taking the epoch-offset delta.
	qint64 msecs = d1.toUTC().toMSecsSinceEpoch() - d2.toUTC().toMSecsSinceEpoch();
	// Make a time at exactly 0:0:0:0, and add the millisecond delta to it.
	return QTime(0,0,0,0).addMSecs(msecs);
}

inline
QString apiPortSignature(unsigned int u)
{
	// We could be using std::default_random_engine, but I like Mersenne Twisters
	// I also do not like wasting real entropy, so cheaping out with time() it is
	std::mt19937 engine((unsigned int)time(0));
	std::uniform_int_distribution<quint32> dist(0, 9);
	
	// Constants used by the game (deobfuscated; they were originally strings in Base 31 >_>)
	unsigned int loc[] = {1171, 1841, 2517, 3101, 4819, 5233, 6311, 7977, 8103, 9377, 1000};
	
	// This is overcomplicated as fuck, and really not secure in any way
	// Note for p2: f = don't use e-notation, 0 = no decimals
	QString p1 = QString::number(loc[10] + u % loc[10]);
	QString p2 = QString::number((9999999999 - floor(QDateTime::currentMSecsSinceEpoch() / (float)loc[10]) - u) * loc[u % 10], 'f', 0);
	QString p3 =
		QString::number(dist(engine)) +
		QString::number(dist(engine)) +
		QString::number(dist(engine)) +
		QString::number(dist(engine));
	QString signature = p1 + p2 + p3;
	
	return signature;
}

#define TABLE_SET_ITEM(_table, _row, _col, _value) \
	{\
		QTableWidgetItem *item = new QTableWidgetItem(); \
		item->setData(Qt::EditRole, _value); \
		_table->setItem(_row, _col, item); \
	}

#endif
