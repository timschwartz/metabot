AM_CXXFLAGS = -std=c++11 -static-libstdc++ -static-libgcc -I$(top_srcdir)/include $(SSL_CFLAGS) $(LUA_CFLAGS)
LDADD = $(SSL_LIBS) $(LUA_LIBS) -lpthread
bin_PROGRAMS = metabot
metabot_SOURCES = metabot.cpp md5.cpp bot.cpp net.cpp
