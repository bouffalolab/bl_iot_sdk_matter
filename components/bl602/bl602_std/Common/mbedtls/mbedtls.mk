MBEDTLS_DIR:= $(MODULE_DIR)/mbedtls
MBEDTLS_OUT_DIR:= $(MODULE_OUT_DIR)/mbedtls
MBEDTLS_SRC_DIR:= $(MBEDTLS_DIR)/library

MBEDTLS_CFLAGS := 

mbedtls_sources := 
#mbedtls_sources +=  aes.c
mbedtls_sources +=  aesni.c
mbedtls_sources +=  arc4.c
mbedtls_sources +=  aria.c
mbedtls_sources +=  asn1parse.c
mbedtls_sources +=  asn1write.c
mbedtls_sources +=  base64.c
#mbedtls_sources +=  bignum.c
mbedtls_sources +=  blowfish.c
mbedtls_sources +=  camellia.c
mbedtls_sources +=  ccm.c
mbedtls_sources +=  certs.c
mbedtls_sources +=  chacha20.c
mbedtls_sources +=  chachapoly.c
mbedtls_sources +=  cipher.c
mbedtls_sources +=  cipher_wrap.c
mbedtls_sources +=  cmac.c
mbedtls_sources +=  ctr_drbg.c
mbedtls_sources +=  debug.c
mbedtls_sources +=  des.c
mbedtls_sources +=  dhm.c
mbedtls_sources +=  ecdh.c
mbedtls_sources +=  ecdsa.c
mbedtls_sources +=  ecjpake.c
mbedtls_sources +=  ecp.c
mbedtls_sources +=  ecp_curves.c
#mbedtls_sources +=  entropy.c
#mbedtls_sources +=  entropy_poll.c
mbedtls_sources +=  error.c
mbedtls_sources +=  gcm.c
mbedtls_sources +=  havege.c
mbedtls_sources +=  hkdf.c
mbedtls_sources +=  hmac_drbg.c
mbedtls_sources +=  md.c
mbedtls_sources +=  md2.c
mbedtls_sources +=  md4.c
mbedtls_sources +=  md5.c
mbedtls_sources +=  md_wrap.c
mbedtls_sources +=  memory_buffer_alloc.c
mbedtls_sources +=  net_sockets.c
mbedtls_sources +=  nist_kw.c
mbedtls_sources +=  oid.c
mbedtls_sources +=  padlock.c
mbedtls_sources +=  pem.c
mbedtls_sources +=  pk.c
mbedtls_sources +=  pk_wrap.c
mbedtls_sources +=  pkcs5.c
mbedtls_sources +=  pkcs11.c
mbedtls_sources +=  pkcs12.c
mbedtls_sources +=  pkparse.c
mbedtls_sources +=  pkwrite.c
mbedtls_sources +=  platform.c
mbedtls_sources +=  platform_util.c
mbedtls_sources +=  poly1305.c
mbedtls_sources +=  ripemd160.c
mbedtls_sources +=  rsa.c
mbedtls_sources +=  rsa_internal.c
#mbedtls_sources +=  sha1.c
#mbedtls_sources +=  sha256.c
mbedtls_sources +=  sha512.c
mbedtls_sources +=  ssl_cache.c
mbedtls_sources +=  ssl_ciphersuites.c
mbedtls_sources +=  ssl_cli.c
mbedtls_sources +=  ssl_cookie.c
mbedtls_sources +=  ssl_srv.c
mbedtls_sources +=  ssl_ticket.c
mbedtls_sources +=  ssl_tls.c
mbedtls_sources +=  threading.c
mbedtls_sources +=  timing.c
mbedtls_sources +=  version.c
#mbedtls_sources +=  version_feature.c
mbedtls_sources +=  x509.c
mbedtls_sources +=  x509_create.c
mbedtls_sources +=  x509_crl.c
mbedtls_sources +=  x509_crt.c
mbedtls_sources +=  x509_csr.c
mbedtls_sources +=  x509write_crt.c
mbedtls_sources +=  x509write_csr.c
mbedtls_sources +=  xtea.c

include $(MBEDTLS_DIR)/bflb_port/bflb_port.mk

mbedtls_objs := $(addprefix $(MBEDTLS_OUT_DIR)/, $(subst .c,.o,$(mbedtls_sources)))

mbedtls_objs += $(bflb_port_objs)

$(MBEDTLS_OUT_DIR)/%.o:$(MBEDTLS_SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "cc $<"
	$(AT)$(CC) -c $(GLOBAL_CFLAGS) $(COMMON_CFLAGS) $(MBEDTLS_CFLAGS) $(GLOBAL_INCLUDE) $(COMMON_INCLUDE) $< -o $@