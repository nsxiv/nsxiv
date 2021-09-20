PREFIX = /usr/local
ICONS  = 16x16.png 32x32.png 48x48.png 64x64.png 128x128.png

.PHONY: install uninstall

install:
	@echo "INSTALL icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		mkdir -p "$$dir"; \
		cp "$$f" "$$dir/nsxiv.png"; \
		chmod 644 "$$dir/nsxiv.png"; \
	done

uninstall:
	@echo "REMOVE icon"
	for f in $(ICONS); do \
		dir="$(DESTDIR)$(PREFIX)/share/icons/hicolor/$${f%.png}/apps"; \
		rm -f "$$dir/nsxiv.png"; \
	done
