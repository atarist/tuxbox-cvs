#include <lib/base/eerror.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <lib/gui/emessage.h>

#ifdef MEMLEAK_CHECK
AllocList *allocList;
pthread_mutex_t memLock=PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;
#else
	#include <lib/system/elock.h>
#endif
pthread_mutex_t signalLock=PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

int infatal=0;

Signal2<void, int, const eString&> logOutput;
int logOutputConsole=1;

void eFatal(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	{
		singleLock s(signalLock);
		logOutput(lvlFatal, buf);
	}
	fprintf(stderr, "%s\n",buf );
	if (!infatal)
	{
		infatal=1;
		eMessageBox msg(buf, "FATAL ERROR", eMessageBox::iconError|eMessageBox::btOK);
		msg.show();
		msg.exec();
	}
	_exit(0);
}

#ifdef DEBUG
void eDebug(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	if (logOutputConsole)
		fprintf(stderr, "%s\n", buf);
	else
	{
		singleLock s(signalLock);
		logOutput(lvlDebug, eString(buf) + "\n");
	}
}

void eDebugNoNewLine(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	if (logOutputConsole)
		fprintf(stderr, buf);
	else
	{
		singleLock s(signalLock);
		logOutput(lvlDebug, buf);
	}
}

void eWarning(const char* fmt, ...)
{
	char buf[1024];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);
	if (logOutputConsole)
		fprintf(stderr, "%s\n", buf);
	else
	{
		singleLock s(signalLock);
		logOutput(lvlWarning, eString(buf) + "\n");
	}
}
#endif // DEBUG
