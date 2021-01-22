#include <stdio.h>
#include <mbedtls/bignum.h>

/* simple program for large integer addition */

int main()
{

    mbedtls_mpi a;
    mbedtls_mpi b;
    mbedtls_mpi c;

#ifdef DEBUG
    mbedtls_mpi ans;
    mbedtls_mpi_init(&ans);
    mbedtls_mpi_grow(&ans, 32);
    mbedtls_mpi_read_string(&ans, 10, "114415764801983701634287292923478115233706135480122126270381839250222226513740244470241386649496601542699894450496223523194330564325726972380045456701239745270558009119614135771174220461972200428487828398765709662806006507303326486925756547065490814226365523356887571023586387331596575809517676484591641274155");
#endif

    mbedtls_mpi_init(&a);
    mbedtls_mpi_init(&b);
    mbedtls_mpi_init(&c);


    mbedtls_mpi_grow(&a, 32); /* Make a hold a 1024-bit integer */
    mbedtls_mpi_grow(&b, 32); /* Make b hold a 1024-bit integer */
    mbedtls_mpi_grow(&c, 32); /* Make c hold a 1024-bit integer (a, b are chosen so that a+b fits in 1024 bits)*/

    mbedtls_mpi_read_string(&a, 10, "105063345230616104657348013634297701786006542128941909083101185888832813981927541578449103885990606406091215423988641232290414368860661235234905516208831410002395649542617648887800946357680235133745987328106517234870888931439344639465469355722657253870347038180634028206997149618094761699250765049161158009075");
    mbedtls_mpi_read_string(&b, 10, "9352419571367596976939279289180413447699593351180217187280653361389412531812702891792282763505995136608679026507582290903916195465065737145139940492408335268162359576996486883373274104291965294741841070659192427935117575863981847460287191342833560356018485176253542816589237713501814110266911435430483265080");
    mbedtls_mpi_read_string(&c, 10, "0"); 

    int ret = mbedtls_mpi_add_mpi(&c, &a, &b);
    if(ret != 0) {
        return -1;
    }


#ifdef DEBUG

    ret = mbedtls_mpi_cmp_mpi(&c, &ans);
    if(ret != 0) {
        printf("Did not get expected answer\n");
        return -1;
    }
    else {
        printf("Answer is correct\n");
        return -1;
    }


    mbedtls_mpi_free(&ans);
#endif

    mbedtls_mpi_free(&a);
    mbedtls_mpi_free(&b);
    mbedtls_mpi_free(&c);

    return 0;
}
