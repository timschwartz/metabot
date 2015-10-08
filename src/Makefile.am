AM_CXXFLAGS = -std=c++11 -I$(top_srcdir)/include $(SSL_CFLAGS) $(LUA_CFLAGS)
AM_LDFLAGS = -pthread -static-libstdc++ -static-libgcc
LDADD = $(SSL_LIBS) $(LUA_LIBS) 
bin_PROGRAMS = metabot
metabot_SOURCES = metabot.cpp md5.cpp bot.cpp net.cpp
