/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "linker.h"

#include "dlfcn.h"
#include "linker_phdr.h"


/* This file hijacks the symbols stubbed out in libdl.so. */


static const char *__bionic_set_dlerror(char *new_value) {

    return "";
}


const char *zlerror() {
    const char *old_value = __bionic_set_dlerror(NULL);
    return old_value;
}

void android_update_LD_LIBRARY_PATH(const char *ld_library_path) {
    //ScopedPthreadMutexLocker locker(&gDlMutex);
    do_android_update_LD_LIBRARY_PATH(ld_library_path);
}

void *zlopen(const char *filename, int flags) {
    //ScopedPthreadMutexLocker locker(&gDlMutex);
    soinfo *result = do_zlopen(filename, flags);
    if (result == NULL) {
        DL_ERR("dlopen failed");
        return NULL;
    }
    return result;
}

void *zlsym(void *handle, const char *symbol) {
    //ScopedPthreadMutexLocker locker(&gDlMutex);

    if (handle == NULL) {
        DL_ERR("zlsym library handle is null");
        return NULL;
    }
    if (symbol == NULL) {
        DL_ERR("zlsym symbol name is null");
        return NULL;
    }

    soinfo *found = NULL;
    Elf32_Sym *sym = NULL;
    if (handle == RTLD_DEFAULT) {
        sym = dlsym_linear_lookup(symbol, &found, NULL);
    } else if (handle == RTLD_NEXT) {
        void *ret_addr = __builtin_return_address(0);
        soinfo *si = find_containing_library(ret_addr);

        sym = NULL;
        if (si && si->next) {
            sym = dlsym_linear_lookup(symbol, &found, si->next);
        }
    } else {
        found = reinterpret_cast<soinfo *>(handle);
        sym = dlsym_handle_lookup(found, symbol);
    }

    if (sym != NULL) {
        unsigned bind = ELF32_ST_BIND(sym->st_info);

        if (bind == STB_GLOBAL && sym->st_shndx != 0) {
            unsigned ret = sym->st_value + found->load_bias;

            TV_DBG("[%s][0x%x]( %s ) found", found->name,
                   ((int) ret - (int) ((soinfo *) handle)->base), symbol);
            return (void *) ret;
        }

        DL_ERR("symbol found but not global");
        return NULL;
    } else {
        DL_ERR("no %s found in %s", symbol, found->name);
        TV_DBG("no %s found in %s", symbol, found->name);
        return NULL;
    }
}

int zladdr(const void *addr, Dl_info *info) {
    //ScopedPthreadMutexLocker locker(&gDlMutex);

    // Determine if this address can be found in any library currently mapped.
    soinfo *si = find_containing_library(addr);
    if (si == NULL) {
        return 0;
    }

    memset(info, 0, sizeof(Dl_info));

    info->dli_fname = si->name;
    // Address at which the shared object is loaded.
    info->dli_fbase = (void *) si->base;

    // Determine if any symbol in the library contains the specified address.
    Elf32_Sym *sym = dladdr_find_symbol(si, addr);
    if (sym != NULL) {
        info->dli_sname = si->strtab + sym->st_name;
        info->dli_saddr = (void *) (si->load_bias + sym->st_value);
    }

    return 1;
}

int zlclose(void *handle) {
    //ScopedPthreadMutexLocker locker(&gDlMutex);
    return do_zlclose(reinterpret_cast<soinfo *>(handle));
}


//   0000000 00011111 111112 22222222 2333333 3333444444444455555555556666666 6667
//   0123456 78901234 567890 12345678 9012345 6789012345678901234567890123456 7890
#define ANDROID_LIBDL_STRTAB \
    "zlopen\0zlclose\0zlsym\0zlerror\0zladdr\0android_update_LD_LIBRARY_PATH\0dl_unwind_find_exidx\0"



// name_offset: starting index of the name in libdl_info.strtab
#define ELF32_SYM_INITIALIZER(name_offset, value, shndx) \
    { name_offset, \
      reinterpret_cast<Elf32_Addr>(reinterpret_cast<void*>(value)), \
      /* st_size */ 0, \
      (shndx == 0) ? 0 : (STB_GLOBAL << 4), \
      /* st_other */ 0, \
      shndx }

static Elf32_Sym gLibDlSymtab[] = {
        // Total length of libdl_info.strtab, including trailing 0.
        // This is actually the STH_UNDEF entry. Technically, it's
        // supposed to have st_name == 0, but instead, it points to an index
        // in the strtab with a \0 to make iterating through the symtab easier.
        ELF32_SYM_INITIALIZER(sizeof(ANDROID_LIBDL_STRTAB) - 1, NULL, 0),
        ELF32_SYM_INITIALIZER(0, &zlopen, 1),
        ELF32_SYM_INITIALIZER(7, &zlclose, 1),
        ELF32_SYM_INITIALIZER(15, &zlsym, 1),
        ELF32_SYM_INITIALIZER(21, &zlerror, 1),
        ELF32_SYM_INITIALIZER(29, &zladdr, 1),
        ELF32_SYM_INITIALIZER(36, &android_update_LD_LIBRARY_PATH, 1),
        ELF32_SYM_INITIALIZER(67, &dl_unwind_find_exidx, 1),

};

// Fake out a hash table with a single bucket.
// A search of the hash table will look through
// gLibDlSymtab starting with index [1], then
// use gLibDlChains to find the next index to
// look at.  gLibDlChains should be set up to
// walk through every element in gLibDlSymtab,
// and then end with 0 (sentinel value).
//
// That is, gLibDlChains should look like
// { 0, 2, 3, ... N, 0 } where N is the number
// of actual symbols, or nelems(gLibDlSymtab)-1
// (since the first element of gLibDlSymtab is not
// a real symbol).
//
// (see soinfo_elf_lookup())
//
// Note that adding any new symbols here requires
// stubbing them out in libdl.
static unsigned gLibDlBuckets[1] = {1};
static unsigned gLibDlChains[8] = {0, 2, 3, 4, 5, 6, 7, 0};

// This is used by the dynamic linker. Every process gets these symbols for free.
soinfo libdl_info = {
        "",
        phdr: 0, phnum: 0,
        entry: 0, base: 0, size: 0,
        unused1: 0, dynamic: 0, unused2: 0, unused3: 0,
        next: 0,
        flags: FLAG_LINKED,
        strtab: ANDROID_LIBDL_STRTAB,
        symtab: gLibDlSymtab,
        nbucket: 1,
        nchain: 8,
        bucket: gLibDlBuckets,
        chain: gLibDlChains,
        plt_got: 0, plt_rel: 0, plt_rel_count: 0, rel: 0, rel_count: 0,
        preinit_array: 0, preinit_array_count: 0, init_array: 0, init_array_count: 0,
        fini_array: 0, fini_array_count: 0, init_func: 0, fini_func: 0,
        ARM_exidx: 0, ARM_exidx_count: 0,
        ref_count: 0,
        {l_addr: 0, l_name: 0, l_ld: 0, l_next: 0, l_prev: 0,},
        constructors_called: false,
        load_bias: 0,
        has_text_relocations: false,
        has_DT_SYMBOLIC: true,
};
