package=xproto
$(package)_version=7.0.26
# $(package)_download_path=https://xorg.freedesktop.org/releases/individual/proto
# https://ipfs.decenomy.net/QmTb3hRqX7E5FZ8Ms1Um8huoi2ZxABGRpmuCZz4T1NWpRx?filename=xproto-7.0.26.tar.bz2
$(package)_download_path=https://ipfs.decenomy.net
$(package)_download_file=QmTb3hRqX7E5FZ8Ms1Um8huoi2ZxABGRpmuCZz4T1NWpRx
$(package)_file_name=$(package)-$($(package)_version).tar.bz2
$(package)_sha256_hash=636162c1759805a5a0114a369dffdeccb8af8c859ef6e1445f26a4e6e046514f

define $(package)_set_vars
$(package)_config_opts=--disable-shared
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
