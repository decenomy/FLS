package=curl
$(package)_version=8.4.0
#$(package)_download_path=https://github.com/curl/curl/releases/download/curl-8_4_0/
# https://ipfs.decenomy.net/QmVBCSbyviKNaEVUWEk3pABvfFMmj8KEm6TwnQ2Y2vMeki?filename=curl-8.4.0.tar.gz
$(package)_download_path=https://ipfs.decenomy.net
$(package)_download_file=QmVBCSbyviKNaEVUWEk3pABvfFMmj8KEm6TwnQ2Y2vMeki
$(package)_file_name=$(package)-$($(package)_version).tar.gz
$(package)_sha256_hash=816e41809c043ff285e8c0f06a75a1fa250211bbfb2dc0a037eeef39f1a9e427
$(package)_dependencies=openssl

define $(package)_set_vars
  $(package)_config_opts=--with-openssl=$(host_prefix) --disable-shared --enable-static
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
