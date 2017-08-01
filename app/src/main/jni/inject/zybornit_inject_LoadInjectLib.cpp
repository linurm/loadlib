/*
 ============================================================================
 Name        : libinject.c
 Author      :  
 Version     :
 Copyright   : 
 Description : Android shared library inject helper
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/exec_elf.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include "zybornit_inject_LoadInjectLib.h"

#define ENABLE_DEBUG 0

#define PTRACE_PEEKTEXT 1
#define PTRACE_POKETEXT 4
//#define PTRACE_ATTACH    16
#define PTRACE_CONT    7
//#define PTRACE_DETACH   17
#define PTRACE_SYSCALL    24
#define CPSR_T_MASK        ( 1u << 5 )

#define  MAX_PATH 0x100

#define REMOTE_ADDR(addr, local_base, remote_base) ( (uint32_t)(addr) + (uint32_t)(remote_base) - (uint32_t)(local_base) )

const char *libc_path = "/system/lib/libc.so";
const char *linker_path = "/system/bin/linker";


#if ENABLE_DEBUG
#define DEBUG_PRINT(format,args...) \
        LOGD(format, ##args)
#else
#define DEBUG_PRINT(format, args...)
#endif

AddInfo *mallocAddinfo(u_int32_t addrs, u_int32_t addre) {
    AddInfo *addr;
    addr = (AddInfo *) malloc(sizeof(AddInfo));
    addr->start_addr = addrs;
    addr->sizen = addre - addrs;
    addr->next = NULL;
    if (addr->start_addr == 0x8000)
        addr->start_addr = 0;
    return addr;
}

AddInfo *get_module_base(pid_t pid, const char *module_name) {
    FILE *fp;
    AddInfo *addr = NULL;
    AddInfo *addrinfo = NULL;
    AddInfo *addrf = NULL;
    AddInfo *addrp = NULL;
//    AddInfo *addrn = NULL;
    char *pch;
    char *pend;
    int found = 0;
    u_int32_t addrs = 0;
    u_int32_t addre = 0;
    char filename[32];
    char line[1024];

    if (pid < 0) {
        /* self process */
        snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }

    fp = fopen(filename, "r");

    if (fp != NULL) {
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                DL_DEBUG("_____ %s", line);
                pch = strtok(line, "-");
                addrs = strtoul(pch, NULL, 16);
                pend = strtok(NULL, " ");
                addre = strtoul(pend, NULL, 16);
                if (addrinfo != NULL) {
                    addrf = addrinfo;
                    do {
                        addrp = addrf;
                        if (addrf->start_addr != addrs) {
                            addrf = addrf->next;
                        } else {
                            found = 1;
                            break;
                        }
                    } while (addrf != NULL);
                    if (found == 0) {
                        addr = mallocAddinfo(addrs, addre);
                        if (addrp != NULL)
                            addrp->next = addr;
                    }
                    found = 0;
                } else {
                    if (found == 0) {
                        addr = mallocAddinfo(addrs, addre);
                        if (addrp != NULL)
                            addrp->next = addr;
                    }
                    addrinfo = addr;
                }
            }
        }
        fclose(fp);
    }

    return addrinfo;
}

void releaseAddInfo(AddInfo *pAddInfo) {
    AddInfo *p1;
    AddInfo *p2;
    p1 = pAddInfo;
    while (p1 != NULL) {
        if (p1->next != NULL) {
            p2 = p1->next;
            free(p1);
            p1 = p2;
        } else {
            free(p1);
            p1 = NULL;
        }
    }
}

void printAddInfo(AddInfo *pAddInfo) {
    AddInfo *p;
    if (pAddInfo != NULL) {

        p = pAddInfo;
        do {
            DL_DEBUG("0x%x --- 0x%x", p->start_addr, p->sizen);
            p = p->next;
        } while (p != NULL);
    } else {
        DL_DEBUG("The AddInfo list is null");
    }
}

void *get_remote_addr(pid_t target_pid, const char *module_name, void *local_addr) {


    void *local_handle, *remote_handle;

    local_handle = get_module_base(-1, module_name);
    remote_handle = get_module_base(target_pid, module_name);


    DL_DEBUG("[+] get_remote_addr: local[0x%x], remote[0x%x] [0x%x]\n",
             (unsigned int) local_handle,
             (unsigned int) remote_handle, (unsigned int) local_addr);

    return (void *) ((uint32_t) local_addr + (uint32_t) remote_handle -
                     (uint32_t) local_handle);
}

AddInfo *getLibMemAddr(pid_t target_pid, const char *module_name) {
    AddInfo *local_handle;
    AddInfo *remote_handle;

//    local_handle = get_module_base(-1, module_name);
    remote_handle = get_module_base(target_pid, module_name);
    printAddInfo(remote_handle);
    DL_DEBUG("[+] get_remote_addr: local[0x%x], remote[0x%x]\n", (unsigned int) local_handle,
             (unsigned int) remote_handle);
    return remote_handle;
//    return (void *) ((uint32_t) local_addr + (uint32_t) remote_handle - (uint32_t) local_handle);
}


int find_pid_of(const char *process_name) {
    int id;
    pid_t pid = -1;
    DIR *dir;
    FILE *fp;
    char filename[32];
    char cmdline[256];

    struct dirent *entry;

    if (process_name == NULL)
        return -1;

    dir = opendir("/proc");
    if (dir == NULL)
        return -1;

    while ((entry = readdir(dir)) != NULL) {
        id = atoi(entry->d_name);
        if (id != 0) {
            sprintf(filename, "/proc/%d/cmdline", id);
            fp = fopen(filename, "r");
            if (fp) {
                fgets(cmdline, sizeof(cmdline), fp);
                fclose(fp);
                if (strcmp(process_name, cmdline) == 0) {
                    /* process found */
                    pid = id;
                    break;
                }
            }
        }
    }

    closedir(dir);

    return pid;
}


int changeLibFuncAddr(AddInfo *addr, const char *dlib, const char *symbol, void *replace_func,
                      void **old_func);

int injectLibFunc(pid_t target_pid, const char *soname, const char *symbol, void *replace_func,
                  void **old_func) {
    void *addr;
    AddInfo *base_addr;
    base_addr = getLibMemAddr(target_pid, soname);
    changeLibFuncAddr(base_addr, soname, symbol, replace_func, old_func);
    releaseAddInfo(base_addr);
    return (int) addr;
}


/*void *elf_hook(char const *module_filename,
               void const *module_address, char const *name, void const *substitution)
{
    static size_t pagesize;

    int descriptor;  //file descriptor of shared module

    Elf_Shdr
            *dynsym = NULL,  // ".dynsym" section header
            *rel_plt = NULL,  // ".rel.plt" section header
            *rel_dyn = NULL;  // ".rel.dyn" section header

    Elf_Sym
            *symbol = NULL;  //symbol table entry for symbol named "name"

    Elf_Rel
            *rel_plt_table = NULL,  //array with ".rel.plt" entries
            *rel_dyn_table = NULL;  //array with ".rel.dyn" entries

    size_t
            i,
            name_index = 0,  //index of symbol named "name" in ".dyn.sym"
            rel_plt_amount = 0,  // amount of ".rel.plt" entries
            rel_dyn_amount = 0,  // amount of ".rel.dyn" entries
            *name_address = NULL;  //address of relocation for symbol named "name"

    void *original = NULL;  //address of the symbol being substituted

    if (NULL == module_address || NULL == name || NULL == substitution)
        return original;

    if (!pagesize)
        pagesize = sysconf(_SC_PAGESIZE);

    descriptor = open(module_filename, O_RDONLY);

    if (descriptor < 0)
        return original;

    if (
            section_by_type(descriptor, SHT_DYNSYM, &dynsym) ||  //get ".dynsym" section
            //actually, we need only the index of symbol named "name" in the ".dynsym" table
            symbol_by_name(descriptor, dynsym, name, &symbol, &name_index) ||
            //get ".rel.plt" (for 32-bit) or ".rela.plt" (for 64-bit) section
            section_by_name(descriptor, REL_PLT, &rel_plt) ||
            section_by_name(descriptor, REL_DYN, &rel_dyn)
        //get ".rel.dyn" (for 32-bit) or ".rela.dyn" (for 64-bit) section
            )
    {  //if something went wrong
        free(dynsym);
        free(rel_plt);
        free(rel_dyn);
        free(symbol);
        close(descriptor);

        return original;
    }
//release the data used
    free(dynsym);
    free(symbol);

    rel_plt_table = (Elf_Rel *)(((size_t)module_address) + rel_plt->sh_addr);  //init the ".rel.plt" array
    rel_plt_amount = rel_plt->sh_size / sizeof(Elf_Rel);  //and get its size

    rel_dyn_table = (Elf_Rel *)(((size_t)module_address) + rel_dyn->sh_addr);  //init the ".rel.dyn" array
    rel_dyn_amount = rel_dyn->sh_size / sizeof(Elf_Rel);  //and get its size
//release the data used
    free(rel_plt);
    free(rel_dyn);
//and descriptor
    close(descriptor);
//now we've got ".rel.plt" (needed for PIC) table
//and ".rel.dyn" (for non-PIC) table and the symbol's index
    for (i = 0; i < rel_plt_amount; ++i)  //lookup the ".rel.plt" table
        if (ELF_R_SYM(rel_plt_table[i].r_info) == name_index)
            //if we found the symbol to substitute in ".rel.plt"
        {
            original = (void *)*(size_t *)(((size_t)module_address) +
                                           rel_plt_table[i].r_offset);  //save the original function address
            *(size_t *)(((size_t)module_address) +
                        rel_plt_table[i].r_offset) = (size_t)substitution;
            //and replace it with the substitutional

            break;  //the target symbol appears in ".rel.plt" only once
        }

    if (original)
        return original;
//we will get here only with 32-bit non-PIC module
    for (i = 0; i < rel_dyn_amount; ++i)  //lookup the ".rel.dyn" table
        if (ELF_R_SYM(rel_dyn_table[i].r_info) == name_index)
            //if we found the symbol to substitute in ".rel.dyn"
        {
            name_address = (size_t *)(((size_t)module_address) + rel_dyn_table[i].r_offset);
            //get the relocation address (address of a relative CALL (0xE8) instruction's argument)

            if (!original)
                original = (void *)(*name_address + (size_t)name_address + sizeof(size_t));
            //calculate an address of the original function by a relative CALL (0xE8) instruction's argument

            mprotect((void *)(((size_t)name_address) & (((size_t)-1) ^ (pagesize - 1))),
                     pagesize, PROT_READ | PROT_WRITE);  //mark a memory page that contains the relocation as writable

            if (errno)
                return NULL;

            *name_address = (size_t)substitution - (size_t)name_address - sizeof(size_t);
            //calculate a new relative CALL (0xE8) instruction's argument for the substitutional function and write it down

            mprotect((void *)(((size_t)name_address) & (((size_t)-1) ^ (pagesize - 1))),
                     pagesize, PROT_READ | PROT_EXEC);  //mark a memory page that contains the relocation back as executable

            if (errno)  //if something went wrong
            {
                *name_address = (size_t)original -
                                (size_t)name_address - sizeof(size_t);  //then restore the original function address

                return NULL;
            }
        }

    return original;
}
*/
#define PAGE_START(addr) (~(getpagesize() - 1) & (addr))

static int modifyMemAccess(void *addr, int prots) {
    void *page_start_addr = (void *) PAGE_START((uint32_t) addr);
    return mprotect(page_start_addr, getpagesize(), prots);
}

static int clearCache(void *addr, size_t len) {
    void *end = (uint8_t *) addr + len;
//    syscall(0xf0002, addr, end);
}

static int replaceFunc(void *addr, void *replace_func, void **old_func) {
    int res = 0;

    if (*(void **) addr == replace_func) {
        DL_DEBUG("addr %p had been replace.", addr);
        goto fails;
    }

    if (!*old_func) {
        DL_DEBUG("old_func %p save", old_func);
        *old_func = *(void **) addr;
    }

    if (modifyMemAccess((void *) addr, PROT_READ | PROT_WRITE)) {
        DL_DEBUG("[-] modifymemAccess fails, error %s.", strerror(errno));
        res = 1;
        goto fails;
    }

    *(void **) addr = replace_func;

//    clearCache(addr, getpagesize());
    goto fails;
    DL_DEBUG("[+] old_func is %p, replace_func is %p, new_func %p.", *old_func, replace_func,
             (void *) addr);

    fails:
    return res;
}

#define R_ARM_ABS32 0x02
#define R_ARM_GLOB_DAT 0x15
#define R_ARM_JUMP_SLOT 0x16

void getLibSegmentInfo() {

}

//typedef void (*test_fun)(void);
//
//test_fun old_test = NULL;
//
//void testHook(void) {
//    DL_DEBUG("this is testHook() in libinject.so after inject");
//    if (old_test != NULL)
//        old_test();
//    return;
//}
typedef int (*test_fun)(int clock_id, timespec *tp);

test_fun old_test = NULL;

static int halfHourFlag = 0;

int clock_gettime_hook(int clock_id, timespec *tp) {
    timespec t;
    int flag;
    flag = old_test(clock_id, &t);
    tp->tv_nsec = t.tv_nsec;
    tp->tv_sec = t.tv_sec + (halfHourFlag * 30 * 60 - 5);

    return flag;

}


JNIEXPORT jstring JNICALL
Java_zybornit_inject_LoadInjectLib_addHalfHour(JNIEnv *env, jobject obj, jstring j_str) {
    halfHourFlag += 1;
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_zybornit_inject_LoadInjectLib_decHalfHour(JNIEnv *env, jobject obj, jstring j_str) {
    halfHourFlag -= 1;
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_zybornit_inject_LoadInjectLib_addHour(JNIEnv *env, jobject obj, jstring j_str) {
    halfHourFlag += 2;
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_zybornit_inject_LoadInjectLib_decHour(JNIEnv *env, jobject obj, jstring j_str) {
    halfHourFlag -= 2;
    return NULL;
}

JNIEXPORT jstring JNICALL
Java_zybornit_inject_LoadInjectLib_injectLib(JNIEnv *env, jobject obj, jstring j_str, jstring j_lib,jstring j_func) {
    const char *c_str = NULL;
    const char *c_str_libname = NULL;
    const char *c_str_fun = NULL;
    pid_t target_pid;
    jboolean isCopy;
    c_str = env->GetStringUTFChars(j_str, &isCopy);
    if (c_str == NULL) {
        return NULL;
    }

    target_pid = find_pid_of(c_str);

    c_str_libname = env->GetStringUTFChars(j_lib, &isCopy);
    if (c_str_libname == NULL) {
        return NULL;
    }
    c_str_fun = env->GetStringUTFChars(j_func, &isCopy);
    if (c_str_fun == NULL) {
        return NULL;
    }

    DL_DEBUG("find  \"%d\" pid", target_pid);
    injectLibFunc(target_pid, c_str_libname, c_str_fun, (char *) clock_gettime_hook,
                  (void **) &old_test);
    return NULL;
}

int changeLibFuncAddr(AddInfo *addr, const char *dlib, const char *symbol, void *replace_func,
                      void **old_func) {
    Elf32_Ehdr *elf = (Elf32_Ehdr *) (addr->start_addr);
    int i;
    char *string_table;
//    Elf32_Phdr *phdr;
//    Elf32_Shdr *shdr;
    DL_DEBUG("++++++++++");
    ElfHandle *handle = (ElfHandle *) malloc(sizeof(ElfHandle));;
    ElfInfo elfInfo;
    handle->base = (void *) (addr->start_addr);
    elfInfo.handle = handle;
//    elfInfo.ehdr = (Elf32_Ehdr *)addr;
//    elfInfo.phdr = addr + elf->e_phoff;
//    elfInfo.shdr = addr + elf->e_shoff;
//    elfInfo.shstr = NULL;


    getElfInfoBySegmentView(elfInfo, handle);

    int shnum = elfInfo.ehdr->e_shnum;
    int shentsize = elfInfo.ehdr->e_shentsize;
    DL_DEBUG("%d %x", shnum, shentsize);

//    for(i = 0; i < shnum; i++){
//        Elf32_Shdr *s = elfInfo.shdr + i;
//        if(s->sh_type == SHT_PROGBITS){
//
//        }
//    }

//    string_table = (char *) (elfInfo.shstr);
//    printHex((void *) string_table);


    Elf32_Phdr *dynamic = NULL;
    Elf32_Word size = 0;

//    getSegmentInfo(elfInfo, PT_DYNAMIC, &dynamic, &size, &elfInfo.dyn);

    Elf32_Sym *sym = NULL;
    int symidx = 0;

    findSymByName(elfInfo, symbol, &sym, &symidx);

    if (!sym) {
        DL_DEBUG("[-] Could not find symbol %s", symbol);
        goto fails;
    } else {
        DL_DEBUG("[+] sym %x %x, symidx %d.", sym, ((int) sym - (int) (elfInfo.elf_base)),
                 symidx);
    }
    DL_DEBUG("+++++++++++%d", elfInfo.relpltsz);

    for (int i = 0; i < elfInfo.relpltsz; i++) {
        Elf32_Rel &rel = elfInfo.relplt[i];
//        printHex((void *) rel.r_info);

        if (ELF32_R_SYM(rel.r_info) == symidx && ELF32_R_TYPE(rel.r_info) == R_ARM_JUMP_SLOT) {
//            printHex((__uint32_t *) rel.r_info);
            void *addr = (void *) (elfInfo.elf_base + rel.r_offset);
            DL_DEBUG("find symidx 0x%x", addr);
            if (replaceFunc(addr, replace_func, old_func)) {
                DL_DEBUG("replace function error");
                goto fails;
            }
            //only once
            break;
        }
    }

    //DL_DEBUG("------------%d", elfInfo.reldynsz);
    for (int i = 0; i < elfInfo.reldynsz; i++) {
        Elf32_Rel &rel = elfInfo.reldyn[i];
        //printHex((void *) rel.r_info);
        if (ELF32_R_SYM(rel.r_info) == symidx &&
            (ELF32_R_TYPE(rel.r_info) == R_ARM_ABS32
             || ELF32_R_TYPE(rel.r_info) == R_ARM_GLOB_DAT)) {
            DL_DEBUG("find glob");
            void *addr = (void *) (elfInfo.elf_base + rel.r_offset);
            if (replaceFunc(addr, replace_func, old_func)) {
                DL_DEBUG("replace function error2");
                goto fails;
            }
        }
    }
//    DL_DEBUG("find glob2");


    elfInfo.ehdr->e_shentsize;
    elfInfo.ehdr->e_shnum;
    elfInfo.ehdr->e_shstrndx * elfInfo.ehdr->e_shentsize;


    fails:
    return 0;
//    void *p;
//    int n, i;
//    elf->e_phoff;
//    shdr = addr + elf->e_shoff;
//    n = elf->e_phnum;
//    p = addr + elf->e_phoff;
//    phdr = (Elf32_Phdr *) p;
//    printHex(elf->e_shoff);
//    printHex((void *) shdr - addr);
//    for (i = 0; i < n; phdr++, i++) {
//        //p += elf->e_phentsize;
////        printWordHex(phdr->p_type);
//        if (phdr->p_type != PT_LOAD)
//            continue;
////        printWordHex(phdr->)
//        printWordHex(phdr);
////        phdr++;
//
//    }
    DL_DEBUG("=========0x%x 0x%x 0x%x", elf->e_phoff, elf->e_phentsize, elf->e_phnum);
//    printHex(addr);
    return 0;
}

extern void testf(void);


int injectLib() {
    pid_t target_pid;
    int flag;


    target_pid = find_pid_of("zybornit.loadlib");
    DL_DEBUG("find  \"%d\" pid", target_pid);
//    flag = inject_remote_process(target_pid, "/dev/yuki/payload.so", "hook_entry", "I'm parameter!",
//                                 strlen("I'm parameter!"));
//    DL_DEBUG("inject_remote_process  \"%d\" pid", flag);
    //void (*ff)(void);
    //ff = testf;
//    injectLibFunc(target_pid, "libtest1.so", "testf", (char *) testHook, (void **) &old_test);
//    injectLibFunc(target_pid, "libLoadlib.so", "testf", (char *) testHook, (void **) &old_test);
//    inject_so();

    return 0;
}

