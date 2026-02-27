CROSS ?= 0
CONFIGURE_ARGS ?=

ifeq ($(CROSS),1)
MESON_BUILD_DIR := .meson/build/mingw-win32-linux
CROSS_FILE_ARG := --cross-file meson/cross/mingw32.ini
else
MESON_BUILD_DIR := .meson/build/mingw-win32-local
CROSS_FILE_ARG :=
endif

MESON := uv run --group build meson

.PHONY: all configure compile bundle clean test distclean

all: bundle

configure:
	@if [ -f "$(MESON_BUILD_DIR)/build.ninja" ]; then \
		echo "Reconfiguring Meson build directory: $(MESON_BUILD_DIR)"; \
		$(MESON) setup --reconfigure "$(MESON_BUILD_DIR)" $(CROSS_FILE_ARG) $(CONFIGURE_ARGS); \
	else \
		echo "Configuring Meson build directory: $(MESON_BUILD_DIR)"; \
		$(MESON) setup "$(MESON_BUILD_DIR)" --buildtype release $(CROSS_FILE_ARG) $(CONFIGURE_ARGS); \
	fi

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
