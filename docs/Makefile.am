EXTRA_DIST = html yahttp.cfg

all-local: html

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(htmldir)
	$(INSTALL_DATA) html/*.html html/*.png html/*.map html/*.md5 $(DESTDIR)$(htmldir)
	$(INSTALL_DATA) html/search/* $(DESTDIR)$(htmldir) 

uninstall-local:
	rm -rf $(DESTDIR)$(htmldir)

html:
	$(DOXYGEN) $(srcdir)/yahttp.cfg

clean-local:
	rm -rf html
