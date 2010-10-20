CXX = g++
CFLAGS =  -Wall -O2 -lmysqlclient -ldl
LDFLAGS =  
DEFINES = -DHAVE_CONFIG_H
INSTALL = ../install-sh

prefix = /usr/local
exec_prefix = $(prefix)
sbindir = ${exec_prefix}/sbin
sysconfdir = /etc
localstatedir = /var

OBJECTS = helper.o logfile.o connectiontobrowser.o \
          httphandler.o params.o sockethandler.o \
          connectiontohttp.o thunder.o proxyhandler.o utils.o \
	  database_mysql.o connectiontohttp2.o 

all: thunder

thunder: $(OBJECTS)
	$(CXX) $(CFLAGS) -o thunder $(OBJECTS) $(LDFLAGS)

%.o: %.cpp %.h default.h
	$(CXX) $(CFLAGS) $(DEFINES) -c -o $@ $<

install: all
	$(INSTALL) -d $(DESTDIR)$(sbindir)
	$(INSTALL) -m 755 thunder $(DESTDIR)$(sbindir)/thunder
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/log/thunder
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/tmp/thunder
	$(INSTALL) -d -m 755 $(DESTDIR)$(localstatedir)/run/thunder
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/thunder
	$(INSTALL) -d -m 755 $(DESTDIR)$(sysconfdir)/thunder/plugins
	$(INSTALL) -d -m 777 $(DESTDIR)/thunder
	$(INSTALL) -d $(DESTDIR)/etc/init.d
	$(INSTALL) -m 644 ../etc/thunder/thunder.conf $(DESTDIR)$(sysconfdir)/thunder/thunder.conf.default
	@if [ ! -f $(DESTDIR)$(sysconfdir)/thunder/thunder.conf ]; then \
		echo "$(INSTALL) -m 644 ../etc/thunder/thunder.conf $(DESTDIR)$(sysconfdir)/thunder/thunder.conf"; \
		$(INSTALL) -m 644 ../etc/thunder/thunder.conf $(DESTDIR)$(sysconfdir)/thunder/thunder.conf; \
	else \
                echo "Updating config $(DESTDIR)$(sysconfdir)/thunder/thunder.conf"; \
                ../update-conf $(DESTDIR)$(sysconfdir)/thunder/thunder.conf; \
	fi
	@if [ ! -f $(DESTDIR)$(sysconfdir)/init.d/thunder ]; then \
                $(INSTALL) -m 755 ../etc/init.d/thunder $(DESTDIR)$(sysconfdir)/init.d/thunder; \
        else \
                echo "Not overwriting old $(DESTDIR)$(sysconfdir)/init.d/thunder.conf"; \
        fi
	@if [ ! -f $(DESTDIR)$(sysconfdir)/thunder/thunder.php ]; then \
                $(INSTALL) -m 755 ../etc/thunder/thunder.php $(DESTDIR)$(sysconfdir)/thunder/thunder.php; \
        else \
                echo "Not overwriting old $(DESTDIR)$(sysconfdir)/thunder/thunder.php"; \
        fi
	@if [ ! -f $(DESTDIR)$(sysconfdir)/thunder/thunder.lst ]; then \
		 $(INSTALL) -m 755 ../etc/thunder/thunder.lst $(DESTDIR)$(sysconfdir)/thunder/thunder.lst; \
	else \
		 echo "Not overwriting old $(DESTDIR)$(sysconfdir)/thunder/thunder.lst"; \
	fi							       
	$(INSTALL) -m 755 plugins/*.so $(DESTDIR)$(sysconfdir)/thunder/plugins/
	chmod -R a+rX $(DESTDIR)$(sysconfdir)/thunder/plugins

clean:
	cd plugins && $(MAKE) clean
	rm -f thunder *.o
