#include "ifl.h"

int main()
{
    IFL *ifl;

    ifl = IFL_init(NULL, NULL);
    if (!ifl) {
        printf("IFL init failed\n");
        goto err;
    }

    printf("IFL created successfully\n");

    IFL_fini(ifl);
    return 0;
err:
    return -1;
}
