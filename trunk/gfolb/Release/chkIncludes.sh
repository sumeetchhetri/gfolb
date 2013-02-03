#!/bin/sh

APP_DEFINE_FILE=../include/AppDefines.h
FLAG=0

IS_OS_CYGWIN=`uname|tr '[A-Z]' '[a-z]'|awk 'index($0,"cygwin") != 0 {print "cygwin"}'`
IS_OS_BSD=`uname|tr '[A-Z]' '[a-z]'|awk 'index($0,"bsd") != 0 {print "bsd"}'`
IS_OS_SOLARIS=`uname|tr '[A-Z]' '[a-z]'|awk 'index($0,"sunos") != 0 {print "sunos"}'`
IS_OS_LINUX=`uname|tr '[A-Z]' '[a-z]'|awk 'index($0,"linux") != 0 {print "linux"}'`

echo '#include <sys/epoll.h>' > .test.h
echo '' > $APP_DEFINE_FILE
if gcc -E .test.h > /dev/null 2>&1
 then
  echo '#include <sys/epoll.h>' >> $APP_DEFINE_FILE
  echo '#define USE_EPOLL 1' >> $APP_DEFINE_FILE
  FLAG=1
 else 
  echo '#ifdef USE_EPOLL' >> $APP_DEFINE_FILE
  echo '# undef USE_EPOLL' >> $APP_DEFINE_FILE
  echo '#endif' >> $APP_DEFINE_FILE
fi

if [ "$FLAG" = "0" ]; then
	echo '#include <sys/event.h>' > .test.h
	if gcc -E .test.h > /dev/null 2>&1
	 then
	  echo '#include <sys/event.h>' >> $APP_DEFINE_FILE
	  echo '#define USE_KQUEUE 1' >> $APP_DEFINE_FILE
	  FLAG=1
	 else 
	  echo '#ifdef USE_KQUEUE' >> $APP_DEFINE_FILE
	  echo '# undef USE_KQUEUE' >> $APP_DEFINE_FILE
	  echo '#endif' >> $APP_DEFINE_FILE
	fi
fi

if [ "$FLAG" = "0" ]; then
	echo '#include <port.h>' > .test.h
	if gcc -E .test.h > /dev/null 2>&1
	 then
	  echo '#include <port.h>' >> $APP_DEFINE_FILE
	  echo '#include <poll.h>' >> $APP_DEFINE_FILE
	  echo '#define USE_EVPORT 1' >> $APP_DEFINE_FILE
	  FLAG=1
	 else 
	  echo '#ifdef USE_EVPORT' >> $APP_DEFINE_FILE
	  echo '# undef USE_EVPORT' >> $APP_DEFINE_FILE
	  echo '#endif' >> $APP_DEFINE_FILE
	fi
fi

if [ "$FLAG" = "0" ]; then
	echo '#include <sys/devpoll.h>' > .test.h
	if gcc -E .test.h > /dev/null 2>&1
	 then
	  echo '#include <sys/devpoll.h>' >> $APP_DEFINE_FILE
	  echo '#define USE_DEVPOLL 1' >> $APP_DEFINE_FILE
	  FLAG=1
	 else 
	  echo '#ifdef USE_DEVPOLL' >> $APP_DEFINE_FILE
	  echo '# undef USE_DEVPOLL' >> $APP_DEFINE_FILE
	  echo '#endif' >> $APP_DEFINE_FILE
	fi
fi

if [ "$IS_OS_CYGWIN" = "" ]; then
	if [ "$FLAG" = "0" ]; then
		echo '#include <sys/poll.h>' > .test.h
		if gcc -E .test.h > /dev/null 2>&1
		 then
		  echo '#include <poll.h>' >> $APP_DEFINE_FILE
		  echo '#include <sys/poll.h>' >> $APP_DEFINE_FILE
		  echo '#define USE_POLL 1' >> $APP_DEFINE_FILE
		  FLAG=1
		 else 
		  echo '#ifdef USE_POLL' >> $APP_DEFINE_FILE
		  echo '# undef USE_POLL' >> $APP_DEFINE_FILE
		  echo '#endif' >> $APP_DEFINE_FILE
		fi
	fi
fi

if [ "$FLAG" = "0" ]; then
	echo '#include <sys/select.h>' > .test.h
	if gcc -E .test.h > /dev/null 2>&1
	 then
	  echo '#include <sys/select.h>' >> $APP_DEFINE_FILE
	  echo '#define USE_SELECT 1' >> $APP_DEFINE_FILE
	  FLAG=1
	 else 
	  echo '#ifdef USE_SELECT' >> $APP_DEFINE_FILE
	  echo '# undef USE_SELECT' >> $APP_DEFINE_FILE
	  echo '#endif' >> $APP_DEFINE_FILE
	fi
fi