
APP_DEFINE_FILE=../../include/AppDefines.h
FLAG=0

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