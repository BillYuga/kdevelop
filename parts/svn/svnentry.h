//
// C++ Interface: svnentry
//
// Description: This is a simple class for parsing SVN/Entries files found in
// any directory of SVN-managed projects.
//
// Author: Mario Scalas <mario.scalas@libero.it>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//

#include <qstring.h>

class QTextStream;

struct SvnEntry
{
	enum  EntryState { invalidEntry, fileEntry, directoryEntry };

	SvnEntry();
	SvnEntry( const QString &aLine );

	bool read( QTextStream & );
	void write( QTextStream & );

	void clean();

	void parse( const QString &aLine );
	QString pack() const;

	EntryState state() const;

	QString type; // "D" or ""
	QString fileName;
	QString revision;
	QString timeStamp;
	QString options;
	QString tagDate;

	static const QString invalidMarker;
	static const QString directoryMarker;
	static const QString fileMarker;
	static const QString entrySeparator;
};
