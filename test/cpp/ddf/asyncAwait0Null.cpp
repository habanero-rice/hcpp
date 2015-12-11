/*
 *  RICE University
 *  Habanero Team
 *  
 *  This file is part of HC Test.
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "hclib_cpp.h"

void async_fct(void * arg) {
    void ** argv = (void **) arg;
    int index = *((int *) argv[0]);
    hclib_ddf_t * ddf = (hclib_ddf_t *) argv[1];
    printf("Running async %d\n", index/2);
    printf("Async %d putting in DDF %d @ %p\n", index/2, index, ddf);
    hclib_ddf_put(ddf, NO_DATUM);
    free(argv);
}

/*
 * Create async await and enable them (by a put) in the 
 * reverse order they've been created.
 */
int main(int argc, char ** argv) {
    int n = 5;
    hclib::ddf_t ** ddf_list = (hclib::ddf_t **)malloc(
            sizeof(hclib::ddf_t *) * (2*(n+1)));
    hclib::launch(&argc, argv, [=]() {
        hclib::finish([=]() {
            int index = 0;
            // Building 'n' NULL-terminated lists of a single DDF each
            for (index = 0 ; index <= n; index++) {
                ddf_list[index*2] = hclib::ddf_create();
                printf("Creating ddf  %p at ddf_list @ %p \n",
                        &ddf_list[index*2], hclib::ddf_get(ddf_list[index*2]));
                ddf_list[index*2+1] = NULL;
            }
            for(index=n-1; index>=1; index--) {
                printf("Creating async %d\n", index);
                // Build async's arguments
                printf("Creating async %d await on %p will enable %p\n", index,
                        &(ddf_list[(index-1)*2]), &(ddf_list[index*2]));
                hclib::asyncAwait(ddf_list[(index-1)*2], [=]() {
                    hclib::ddf_t *ddf = ddf_list[index * 2];
                    int index = index * 2;
                    hclib::ddf_put(ddf, NO_DATUM); });
            }
            printf("Putting in DDF 0\n");
            hclib::ddf_put(ddf_list[0], NO_DATUM);
        });
        // freeing everything up
        for (int index = 0 ; index <= n; index++) {
            free(hclib::ddf_get(ddf_list[index*2]));
            hclib::ddf_free(ddf_list[index*2]);
        }
        free(ddf_list);
    });
    return 0;
}
