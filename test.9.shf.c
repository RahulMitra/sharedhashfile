/*
 * ============================================================================
 * Copyright (c) 2014 Hardy-Francis Enterprises Inc.
 * This file is part of SharedHashFile.
 *
 * SharedHashFile is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * SharedHashFile is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see www.gnu.org/licenses/.
 * ----------------------------------------------------------------------------
 * To use SharedHashFile in a closed-source product, commercial licenses are
 * available; email office [@] sharedhashfile [.] com for more information.
 * ============================================================================
 */

#define _GNU_SOURCE   /* See feature_test_macros(7) */
#include <sys/mman.h> /* for mremap() */
#include <string.h>   /* for memcmp() */

#include "shf.private.h"
#include "shf.h"
#include "tap.h"

int main(void)
{
    plan_tests(32);

    char  test_shf_name[256];
    char  test_shf_folder[] = "/dev/shm";
    pid_t pid               = getpid();
    SHF_SNPRINTF(1, test_shf_name, "test-%05u", pid);

                    shf_init            ();
    SHF    * shf =  shf_attach_existing (test_shf_folder, test_shf_name); ok(NULL == shf, "c: shf_attach_existing() fails for non-existing file as expected");
             shf =  shf_attach          (test_shf_folder, test_shf_name); ok(NULL != shf, "c: shf_attach()          works for non-existing file as expected");
                    SHF_MAKE_HASH       (         "key"    );
    ok(0         == shf_get_key_val_copy(shf               ), "c: shf_get_key_val_copy() could not find unput key as expected");
    ok(0         == shf_del_key_val     (shf               ), "c: shf_del_key_val()      could not find unput key as expected");
    uint32_t uid =  shf_put_key_val     (shf    , "val" , 3)                                                                   ;
    ok(      uid != SHF_UID_NONE                            , "c: shf_put_key_val()                       put key as expected");
    ok(1         == shf_get_key_val_copy(shf               ), "c: shf_get_key_val_copy() could     find   put key as expected");
    ok(3         == shf_val_len                             , "c: shf_val_len                                     as expected");
    ok(0         == memcmp              (shf_val, "val" , 3), "c: shf_val                                         as expected");
    ok(1         == shf_del_uid_val     (shf,uid           ), "c: shf_del_uid_val()      could     find   put key as expected");
    ok(0         == shf_get_key_val_copy(shf               ), "c: shf_get_key_val_copy() could not find   del key as expected");
    ok(0         == shf_del_uid_val     (shf,uid           ), "c: shf_del_uid_val()      could not find   del key as expected");
             uid =  shf_put_key_val     (shf    , "val2", 4)                                                                   ;
    ok(      uid != SHF_UID_NONE                            , "c: shf_put_key_val()                     reput key as expected");
    ok(1         == shf_get_uid_val_copy(shf,uid           ), "c: shf_get_uid_val_copy() could     find reput key as expected");
    ok(4         == shf_val_len                             , "c: shf_val_len                                     as expected");
    ok(0         == memcmp              (shf_val, "val2", 4), "c: shf_val                                         as expected");
    ok(1         == shf_del_key_val     (shf               ), "c: shf_del_key_val()      could     find reput key as expected");

    uint32_t test_pull_items  = 0;
    uint32_t test_qs          = 3;
    uint32_t test_q_items     = 10;
    uint32_t test_q_item_size = 4096;
    ok(      NULL            != shf_q_new     (shf, test_qs, test_q_items, test_q_item_size), "c: shf_q_new() returned as expected");                    /* e.g. q items created  by process a */
    uint32_t test_qid_free    = shf_q_new_name(shf, SHF_CONST_STR_AND_SIZE("qid-free")     );                                                            /* e.g. q names set qids by process a */
    uint32_t test_qid_a2b     = shf_q_new_name(shf, SHF_CONST_STR_AND_SIZE("qid-a2b" )     );
    uint32_t test_qid_b2a     = shf_q_new_name(shf, SHF_CONST_STR_AND_SIZE("qid-b2a" )     );
    ok(      test_qid_free   == shf_q_get_name(shf, SHF_CONST_STR_AND_SIZE("qid-free")     ), "c: shf_q_get_name('qid-free') returned qid as expected"); /* e.g. q names get qids by process b */
    ok(      test_qid_a2b    == shf_q_get_name(shf, SHF_CONST_STR_AND_SIZE("qid-a2b" )     ), "c: shf_q_get_name('qid-a2b' ) returned qid as expected");
    ok(      test_qid_b2a    == shf_q_get_name(shf, SHF_CONST_STR_AND_SIZE("qid-b2a" )     ), "c: shf_q_get_name('qid-b2a' ) returned qid as expected");

    test_pull_items = 0;
    while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_free          )) {                                                                                        /* e.g. q items from unused to a2b q by process a */
                           shf_q_push_head(shf, test_qid_a2b , shf_qiid);
                           SHF_CAST(uint32_t *, shf_qiid_addr)[0] = test_pull_items; /* store q item # in item */
                           test_pull_items ++;
    }
    ok(test_q_items == test_pull_items, "c: pulled & pushed items from free to a2b  as expected");

    test_pull_items = 0;
    while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_a2b          )) {                                                                                         /* e.g. q items from a2b to b2a queue by process b */
                           shf_q_push_head(shf, test_qid_b2a, shf_qiid);
                           SHF_ASSERT(test_pull_items == SHF_CAST(uint32_t *, shf_qiid_addr)[0], "INTERNAL: test expected q item %u but got %u", test_pull_items, SHF_CAST(uint32_t *, shf_qiid_addr)[0]);
                           test_pull_items ++;
    }
    ok(test_q_items == test_pull_items, "c: pulled & pushed items from a2b  to b2a  as expected");

    test_pull_items = 0;
    while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_b2a           )) {                                                                                        /* e.g. q items from a2b to b2a queue by process b */
                           shf_q_push_head(shf, test_qid_free, shf_qiid);
                           SHF_ASSERT(test_pull_items == SHF_CAST(uint32_t *, shf_qiid_addr)[0], "INTERNAL: test expected q item %u but got %u", test_pull_items, SHF_CAST(uint32_t *, shf_qiid_addr)[0]);
                           test_pull_items ++;
    }
    ok(test_q_items == test_pull_items, "c: pulled & pushed items from a2b  to free as expected");

    uint32_t test_keys = 100000;
    shf_set_data_need_factor(250);

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        for (uint32_t i = 0; i < test_keys; i++) {
            shf_make_hash(SHF_CAST(const char *, &i), sizeof(i));
            shf_put_key_val(shf, SHF_CAST(const char *, &i), sizeof(i));
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(1, "c: put expected number of              keys // estimate %.0f keys per second", test_keys / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        uint32_t keys_found = 0;
        for (uint32_t i = (test_keys * 2); i < (test_keys * 3); i++) {
            shf_make_hash(SHF_CAST(const char *, &i), sizeof(i));
            keys_found += shf_get_key_val_copy(shf);
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(0 == keys_found, "c: got expected number of non-existing keys // estimate %.0f keys per second", test_keys / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        uint32_t keys_found = 0;
        for (uint32_t i = 0; i < test_keys; i++) {
            shf_make_hash(SHF_CAST(const char *, &i), sizeof(i));
            keys_found += shf_get_key_val_copy(shf);
            SHF_ASSERT(sizeof(i) == shf_val_len, "INTERNAL: expected shf_val_len to be %lu but got %u\n", sizeof(i), shf_val_len);
            SHF_ASSERT(0 == memcmp(&i, shf_val, sizeof(i)), "INTERNAL: unexpected shf_val\n");
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(test_keys == keys_found, "c: got expected number of     existing keys // estimate %.0f keys per second", test_keys / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    ok(0 == shf_debug_get_garbage(shf), "c: graceful growth cleans up after itself as expected");

    test_q_items = 100000;

    {
        double test_start_time = shf_get_time_in_seconds();
                   shf_debug_verbosity_less();
                   shf_q_del               (shf);
        ok(NULL != shf_q_new               (shf, test_qs, test_q_items, test_q_item_size), "c: shf_q_new() returned as expected");
                   shf_debug_verbosity_more();
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(1, "c: created expected number of new queue items // estimate %.0f keys per second", test_q_items / test_elapsed_time);
    }

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        test_pull_items = 0;
        while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_free          )) {
                               shf_q_push_head(shf, test_qid_a2b , shf_qiid);
                               test_pull_items ++;
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(test_q_items == test_pull_items, "c: moved   expected number of new queue items // estimate %.0f keys per second", test_q_items / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        test_pull_items = 0;
        while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_a2b         )) {
                               shf_q_push_head(shf, test_qid_b2a, shf_qiid);
                               test_pull_items ++;
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(test_q_items == test_pull_items, "c: moved   expected number of new queue items // estimate %.0f keys per second", test_q_items / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    {
        shf_debug_verbosity_less();
        double test_start_time = shf_get_time_in_seconds();
        test_pull_items = 0;
        while(SHF_QIID_NONE != shf_q_pull_tail(shf, test_qid_b2a          )) {
                               shf_q_push_head(shf, test_qid_free, shf_qiid);
                               test_pull_items ++;
        }
        double test_elapsed_time = shf_get_time_in_seconds() - test_start_time;
        ok(test_q_items == test_pull_items, "c: moved   expected number of new queue items // estimate %.0f keys per second", test_q_items / test_elapsed_time);
        shf_debug_verbosity_more();
    }

    char test_du_folder[256]; SHF_SNPRINTF(1, test_du_folder, "du -h -d 0 %s/%s.shf ; rm -rf %s/%s.shf/", test_shf_folder, test_shf_name, test_shf_folder, test_shf_name);
    fprintf(stderr, "test: shf size before deletion: %s\n", shf_backticks(test_du_folder)); // todo: change this to auto delete mechanism

    return exit_status();
} /* main() */
