include tools/tool-versions.env

CROSS ?= 0

ifeq ($(CROSS),1)
CONFIGURE_MODE := --cross
MESON_BUILD_DIR := .meson/build/mingw-win32-linux
else
CONFIGURE_MODE :=
MESON_BUILD_DIR := .meson/build/mingw-win32-local
endif

MESON := uv tool run --python "$(PYTHON_VERSION)" --from "meson==$(MESON_VERSION)" --with ninja meson

.PHONY: all configure compile bundle clean test distclean

all: bundle

configure:
	bash ./configure $(CONFIGURE_MODE) $(CONFIGURE_ARGS)

compile: configure
	$(MESON) compile -C "$(MESON_BUILD_DIR)"

bundle: configure
	$(MESON) compile -C "$(MESON_BUILD_DIR)" bundle

clean:
	@if [ -f "$(MESON_BUILD_DIR)/build.ninja" ]; then \
		$(MESON) compile -C "$(MESON_BUILD_DIR)" --clean; \
	else \
		echo "Nothing to clean in $(MESON_BUILD_DIR)"; \
	fi

test: configure
	$(MESON) test -C "$(MESON_BUILD_DIR)"

distclean:
	rm -rf .meson/build/mingw-win32-local .meson/build/mingw-win32-linux build dist
