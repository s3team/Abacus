
#include <gcrypt.h>
#include <stdio.h>

static void
show_sexp(gcry_sexp_t a)
{
    char *buf;
    size_t size;

    size = gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
    buf = (char *)gcry_xmalloc(size);

    gcry_sexp_sprint(a, GCRYSEXP_FMT_ADVANCED, buf, size);
    fprintf(stdout, "%.*s", (int)size, buf);
    gcry_free(buf);
}

int main()
{
    gcry_sexp_t keyparm, key, pub_key, pri_key;
    gcry_sexp_t plain, plain1, cipher, l;
    gcry_mpi_t x0, x1;
    int have_flags;

    int rc = gcry_sexp_new(&keyparm,
                           "(genkey\n"
                           " (rsa\n"
                           "  (nbits 4:1024)\n"
                           "  (rsa-use-e 5:65539)\n"
                           " ))",
                           0, 1);
#ifdef DEBUG
    if (rc)
        printf("error creating S-expression: %s\n", gpg_strerror(rc));
#endif
    rc = gcry_pk_genkey(&key, keyparm);
    gcry_sexp_release(keyparm);
    pub_key = gcry_sexp_find_token(key, "public-key", 0);
    pri_key = gcry_sexp_find_token(key, "private-key", 0);
#ifdef DEBUG
    if (rc)
        printf("error generating RSA key: %s\n", gpg_strerror(rc));
    if (!pub_key)
        printf("public part missing in key\n");
    if (!pri_key)
        printf("private part missing in key\n");
#endif

    rc = gcry_sexp_build(&plain, NULL, "(data (flags raw) (value %s))", "hello world");
#ifdef DEBUG
    if (rc)
        printf("converting data for encryption failed: %s\n", gcry_strerror(rc));
#endif

    /* Extract data from plaintext.  */
    l = gcry_sexp_find_token(plain, "value", 0);
    x0 = gcry_sexp_nth_mpi(l, 1, GCRYMPI_FMT_USG);
    gcry_sexp_release(l);

    /* Encrypt data.  */
    rc = gcry_pk_encrypt(&cipher, plain, pub_key);

    if (rc)
        printf("encryption failed.\n");
    else
        printf("encryption success.\n");

    show_sexp(cipher);

    l = gcry_sexp_find_token(cipher, "flags", 0);
    have_flags = !!l;
    gcry_sexp_release(l);

    /* Decrypt data.  */
    rc = gcry_pk_decrypt(&plain1, cipher, pri_key);
    if (rc)
        printf("decryption failed.\n");
    else
        printf("decryption success.\n");

    /* Extract decrypted data.  Note that for compatibility reasons, the
	output of gcry_pk_decrypt depends on whether a flags lists (even
	if empty) occurs in its input data.  Because we passed the output
	of encrypt directly to decrypt, such a flag value won't be there
	as of today.  We check it anyway. */
    l = gcry_sexp_find_token(plain1, "value", 0);
    if (l)
    {
        if (!have_flags)
            printf("compatibility mode of pk_decrypt broken.\n");
        x1 = gcry_sexp_nth_mpi(l, 1, GCRYMPI_FMT_USG);
        gcry_sexp_release(l);
    }
    else
    {
        if (have_flags)
            printf("compatibility mode of pk_decrypt broken.\n");
        x1 = gcry_sexp_nth_mpi(plain1, 0, GCRYMPI_FMT_USG);
    }

    /* Compare.  */
    if (gcry_mpi_cmp(x0, x1))
        printf("data corrupted.\n");
    else
        printf("data corret.\n");

    gcry_sexp_release(plain);
    gcry_sexp_release(plain1);

    return 0;
}
