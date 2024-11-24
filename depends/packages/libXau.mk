package=libXau
$(package)_version=1.0.8
# $(package)_download_path=https://xorg.freedesktop.org/releases/individual/lib/
# https://ipfs.decenomy.net/QmVZ9Gzz5ESap4GSapnx9myDdGA4jdfCaZYFk6Rk73E3My?filename=libXau-1.0.8.tar.bz2
$(package)_download_path=https://ipfs.decenomy.net
$(package)_download_file=QmVZ9Gzz5ESap4GSapnx9myDdGA4jdfCaZYFk6Rk73E3My
$(package)_file_name=$(package)-$($(package)_version).tar.bz2
$(package)_sha256_hash=fdd477320aeb5cdd67272838722d6b7d544887dfe7de46e1e7cc0c27c2bea4f2
$(package)_dependencies=xproto

define $(package)_set_vars
  $(package)_config_opts=--disable-shared
  $(package)_config_opts_linux=--with-pic
endef

define $(package)_preprocess_cmds
  cp -f $(BASEDIR)/config.guess $(BASEDIR)/config.sub .
endef

define $(package)_config_cmds
  $($(package)_autoconf)
endef

define $(package)_build_cmds
  $(MAKE)
endef

define $(package)_stage_cmds
  $(MAKE) DESTDIR=$($(package)_staging_dir) install
endef
