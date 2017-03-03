//gcc -O2 -Wall -g -o bin/search search.c -lm
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

//#define NDEBUG
#include <assert.h>

#include "slo_timers.h"
#include "geometry_combinatorics.h"

#define OT_DB_IMPLEMENTATION
#include "ot_db.h"

#define N 9

void get_thrackle_for_each_ot (int k)
{
    uint64_t id = 0;
    order_type_t *ot = order_type_new (N, NULL);
    triangle_set_t *curr_set = malloc (sizeof(triangle_set_t)+(k-1)*sizeof(triangle_t));
    curr_set->k = k;

    db_seek (ot, id);

    subset_it_t *triangle_it = subset_it_new (N, 3, NULL);
    subset_it_t *triangle_set_it = subset_it_new (triangle_it->size, k, NULL);

    subset_it_precompute (triangle_it);
    subset_it_precompute (triangle_set_it);

    subset_it_seek (triangle_set_it, 20620000000);

    while (!db_is_eof ()) {
        db_next (ot);
        while (1) {
            int i;
            //subset_it_print (triangle_set_it);
            for (i=0; i<k; i++) {
                subset_it_seek (triangle_it, triangle_set_it->idx[i]);
                curr_set->e[i].v[0] = ot->pts[triangle_it->idx[0]];
                curr_set->e[i].v[1] = ot->pts[triangle_it->idx[1]];
                curr_set->e[i].v[2] = ot->pts[triangle_it->idx[2]];
            }

            if (is_thrackle(curr_set)) {
            //if (is_edge_disjoint_set(curr_set)) {
                break;
            } else {
                if (!subset_it_next (triangle_set_it)) {
                    subset_it_seek (triangle_set_it, 0);
                }
            }

            if (triangle_set_it->id % 1000000000 == 0) {
                printf ("Currently on Triangle set: %"PRIu64"\n ot:%"PRIu64"\n", triangle_set_it->id, id);
            }
        }
        printf ("%ld: %ld\n", id, triangle_set_it->id);
        //if (id==2) {
        //    return;
        //}
        id++;
    }
    free (triangle_it);
    free (triangle_set_it);
}

void count_thrackles (int k)
{
    uint64_t id = 0;
    order_type_t *ot = order_type_new (N, NULL);
    triangle_set_t *curr_set = malloc (sizeof(triangle_set_t)+(k-1)*sizeof(triangle_t));
    curr_set->k = k;

    db_seek (ot, id);

    subset_it_t *triangle_it = subset_it_new (N, 3, NULL);
    subset_it_t *triangle_set_it = subset_it_new (triangle_it->size, k, NULL);

    subset_it_precompute (triangle_it);
    subset_it_precompute (triangle_set_it);

    while (!db_is_eof()) {
        db_next (ot);
        uint64_t count = 0;
        subset_it_seek (triangle_set_it, 0);
        while (1) {
            //BEGIN_WALL_CLOCK;
            int i;
            for (i=0; i<k; i++) {
                subset_it_seek (triangle_it, triangle_set_it->idx[i]);
                //printf ("\t");
                //subset_it_print (triangle_it);
                curr_set->e[i].v[0] = ot->pts[triangle_it->idx[0]];
                curr_set->e[i].v[1] = ot->pts[triangle_it->idx[1]];
                curr_set->e[i].v[2] = ot->pts[triangle_it->idx[2]];
            }

            if (is_thrackle(curr_set)) {
            //if (is_edge_disjoint_set(curr_set)) {
                //printf ("\t\t");
                subset_it_print (triangle_set_it);
                count++;
            }
            if (!subset_it_next (triangle_set_it)){
                break;
            }
            //PROBE_WALL_CLOCK("Triangle set loop");
        }
        printf ("%ld: %ld\n", id, count);
        if (id==0) {
            return;
        }
        id++;
    }
    free (triangle_it);
    free (triangle_set_it);
}

void print_differing_triples (int n, uint64_t ot_id_1, uint64_t ot_id_2)
{
    order_type_t *ot_1 = order_type_new (N, NULL);
    db_seek (ot_1, ot_id_1);
    ot_triples_t *ot_triples_1 = ot_triples_new (ot_1, NULL);

    order_type_t *ot_2 = order_type_new (N, NULL);
    db_seek (ot_2, ot_id_2);
    ot_triples_t *ot_triples_2 = ot_triples_new (ot_2, NULL);

    int32_t trip_id = 0;
    while (trip_id < ot_triples_1->size) {
        if (ot_triples_1->triples[trip_id] != ot_triples_2->triples[trip_id]) {
            print_triple (N, trip_id);
        }
        trip_id++;
    }
}

void print_edge_disjoint_sets (int n, int k)
{
    int count = 0;
    order_type_t *ot = order_type_new (n, NULL);
    triangle_set_t *curr_set = malloc (sizeof(triangle_set_t)+(k-1)*sizeof(triangle_t));
    curr_set->k = k;

    subset_it_t *triangle_it = subset_it_new (n, 3, NULL);
    subset_it_t *triangle_set_it = subset_it_new (triangle_it->size, k, NULL);

    subset_it_precompute (triangle_it);
    subset_it_precompute (triangle_set_it);

    db_seek (ot, 0);
    while (1) {
        int i;
        for (i=0; i<k; i++) {
            subset_it_seek (triangle_it, triangle_set_it->idx[i]);
            curr_set->e[i].v[0] = ot->pts[triangle_it->idx[0]];
            curr_set->e[i].v[1] = ot->pts[triangle_it->idx[1]];
            curr_set->e[i].v[2] = ot->pts[triangle_it->idx[2]];
        }

        if (is_edge_disjoint_set(curr_set)) {
            array_print (triangle_set_it->idx, k);
            //subset_it_print (triangle_set_it);
            count++;
        }

        if (!subset_it_next (triangle_set_it)) {
            break;
        }
    }
    printf ("total count: %d\n", count);
    free (triangle_it);
    free (triangle_set_it);
}

void fast_edge_disjoint_sets (int n, int k)
{
    int *all_edj_sets = malloc(840*12*12*11*10*sizeof(int));

    triangle_set_t *curr_set = malloc (sizeof(triangle_set_t)+(k-1)*sizeof(triangle_t));
    curr_set->k = k;

    order_type_t *ot = order_type_new (n, NULL);
    db_seek (ot, 0);

    subset_it_t *triangle_it = subset_it_new (n, 3, NULL);
    subset_it_precompute (triangle_it);

    int num_found;
    generate_edge_disjoint_triangle_sets (n, k, all_edj_sets, &num_found);
    printf ("Sets found: %d\n", num_found);

    int i;
    while (!db_is_eof()) {
        int found_th = 0;
        for (i=0; i<num_found*k; i+=k) {
            int j;
            for (j=0; j<k; j++) {
                subset_it_seek (triangle_it, all_edj_sets[i+j]);
                curr_set->e[j].v[0] = ot->pts[triangle_it->idx[0]];
                curr_set->e[j].v[1] = ot->pts[triangle_it->idx[1]];
                curr_set->e[j].v[2] = ot->pts[triangle_it->idx[2]];
            }
            if (is_thrackle(curr_set)) {
                found_th++;
                //printf ("%"PRIu64": ", subset_it_id_for_idx (binomial(n,3), &all_edj_sets[i], k));
                //array_print (&all_edj_sets[i], k);
            }
        }
        printf ("%d %d\n", ot->id, found_th);
        db_next (ot);
    }

    free (triangle_it);
}

void file_read (int file, void *pos,  size_t size)
{
    int bytes_read = read (file, pos, size);
    if ( bytes_read < size) {
        printf ("Did not read full file\n"
                "asked for: %ld\n"
                "received: %d\n", size, bytes_read);
    }
}

int* get_all_thrackles (int n, int k, uint32_t ot_id, int *num_found)
{
    char filename[200];
    snprintf (filename, ARRAY_SIZE(filename), ".cache/n_%d-ot_%d-th_all.bin", n, ot_id);

    int file = open (filename, O_RDONLY);
    if (file == -1) {
        open_database (n);
        order_type_t *ot = order_type_new (n, NULL);
        db_seek (ot, ot_id);

        iterate_threackles_backtracking (n, k, ot, filename);
        file = open (filename, O_RDONLY);
    }

    int *res;
    search_info_t info;
    file_read (file, &info, sizeof (search_info_t));

    res = malloc (info.num_found*info.k*sizeof(int));
    file_read (file, res, info.num_found*info.k*sizeof(int));
    *num_found = info.num_found;
    return res;
}

int main ()
{
    if (!open_database (N)){
        printf ("Could not open database\n");
        return -1;
    }

    if (mkdir (".cache", 0775) == 0) {
        printf ("Creating .cache dir\n");
    }

    //get_thrackle_for_each_ot (12);
    //count_thrackles (8);
    //print_differing_triples (N, 0, 1);
    //print_edge_disjoint_sets (8, 8);
    //generate_edge_disjoint_triangle_sets (10, 13);
    //fast_edge_disjoint_sets (9, 10);

    int n = 9;
    int k = 10;
    int num_found;
    int *thrackles = get_all_thrackles (n, k, 0, &num_found);
    int i;
    for (i=0; i<num_found*k; i+=k) {
        int *thrackle = &thrackles[i];

        int total_triangles = binomial (n, 3);
        uint64_t choosen_set_id = subset_it_id_for_idx (total_triangles, thrackle, k);
        printf ("%"PRIu64": ", choosen_set_id);
        array_print (thrackle, k);
    }
}
