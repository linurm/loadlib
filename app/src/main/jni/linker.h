/*
 * Copyright (C) 2008 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _LINKER_H_
#define _LINKER_H_

#include <unistd.h>
#include <sys/types.h>
#include <elf.h>
#include <sys/exec_elf.h>

#include <link.h>

#define R_ARM_ABS32              2
#define R_ARM_REL32              3
#define R_ARM_COPY               20
#define R_ARM_GLOB_DAT           21
#define  R_ARM_JUMP_SLOT         22
#define R_ARM_RELATIVE           23

#define CMP(name, STR) (memcmp(name,STR,sizeof(STR))==0)


void do_android_update_LD_LIBRARY_PATH(const char *ld_library_path);
// Magic shared structures that GDB knows about.

struct link_map_t {
    uintptr_t l_addr;
    char *l_name;
    uintptr_t l_ld;
    link_map_t *l_next;
    link_map_t *l_prev;
};

// Values for r_debug->state
enum {
    RT_CONSISTENT,
    RT_ADD,
    RT_DELETE
};

struct r_debug {
    int32_t r_version;
    link_map_t *r_map;

    void (*r_brk)(void);

    int32_t r_state;
    uintptr_t r_ldbase;
};

#define FLAG_LINKED     0x00000001
#define FLAG_EXE        0x00000004 // The main executable
#define FLAG_LINKER     0x00000010 // The linker itself

#define SOINFO_NAME_LEN 128

typedef void (*linker_function_t)();

struct soinfo {
public:
    char name[SOINFO_NAME_LEN];
    const Elf32_Phdr *phdr;
    size_t phnum;
    Elf32_Addr entry;
    Elf32_Addr base;
    unsigned size;

    uint32_t unused1;  // DO NOT USE, maintained for compatibility.

    Elf32_Dyn *dynamic;

    uint32_t unused2; // DO NOT USE, maintained for compatibility
    uint32_t unused3; // DO NOT USE, maintained for compatibility

    soinfo *next;
    unsigned flags;

    const char *strtab;
    Elf32_Sym *symtab;

    size_t nbucket;
    size_t nchain;
    unsigned *bucket;
    unsigned *chain;

    unsigned *plt_got;

    Elf32_Rel *plt_rel;
    size_t plt_rel_count;

    Elf32_Rel *rel;
    size_t rel_count;

    linker_function_t *preinit_array;
    size_t preinit_array_count;

    linker_function_t *init_array;
    size_t init_array_count;
    linker_function_t *fini_array;
    size_t fini_array_count;

    linker_function_t init_func;
    linker_function_t fini_func;


    // ARM EABI section used for stack unwinding.
    unsigned *ARM_exidx;
    size_t ARM_exidx_count;


    size_t ref_count;
    link_map_t link_map;

    bool constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    Elf32_Addr load_bias;

    bool has_text_relocations;
    bool has_DT_SYMBOLIC;

    void CallConstructors();

    void CallDestructors();

    void CallPreInitConstructors();

public:
    void CallArray2(const char *array_name, linker_function_t *functions, size_t count,
                    bool reverse);

private:
    void CallFunction2(const char *function_name, linker_function_t function);

private:
    void CallArray(const char *array_name, linker_function_t *functions, size_t count,
                   bool reverse);

    void CallFunction(const char *function_name, linker_function_t function);
};



// These aren't defined in <sys/exec_elf.h>.
#ifndef DT_PREINIT_ARRAY
#define DT_PREINIT_ARRAY   32
#endif
#ifndef DT_PREINIT_ARRAYSZ
#define DT_PREINIT_ARRAYSZ 33
#endif

soinfo *do_zlopen(const char *name, int flags);

int do_zlclose(soinfo *si);

Elf32_Sym *dlsym_linear_lookup(const char *name, soinfo **found, soinfo *start);

soinfo *find_containing_library(const void *addr);

Elf32_Sym *dladdr_find_symbol(soinfo *si, const void *addr);

Elf32_Sym *dlsym_handle_lookup(soinfo *si, const char *name);

soinfo *find_library(const char *name);

extern soinfo libdl_info;
#endif
